#include <ifko.h>
#include <fko_arch.h>

static IGNODE **IG=NULL;
static int NIG=0, TNIG=0;

short Reg2Int(char *regname)
/*
 * Given a register of regname, returns integer number
 */
{
   int i;
   if (regname[0] == 'P' && regname[1] == 'C' && regname[2] == '\0')
      return(-PCREG);
   if (regname[0] == 'S' && regname[1] == 'P' && regname[2] == '\0')
      return(-REG_SP);
   for (i=IREGBEG; i < IREGEND; i++)
      if (!strcmp(archiregs[i-IREGBEG], regname)) return(-i-1);
   for (i=FREGBEG; i < FREGEND; i++)
      if (!strcmp(archfregs[i-FREGBEG], regname)) return(-i-1);
   for (i=DREGBEG; i < DREGEND; i++)
      if (!strcmp(archdregs[i-DREGBEG], regname)) return(-i-1);
   for (i=ICC0; i < NICC; i++)
      if (!strcmp(ICCREGS[i], regname)) return(-i-1);
   for (i=FCC0; i < NFCC; i++)
      if (!strcmp(FCCREGS[i], regname)) return(-i-1);
   return(0);
}

int NumberArchRegs()
{
   return(TNREG);
}
char *Int2Reg0(int i)
/*
 * Translates integral encoding to machine-specific registers
 */
{
   static char ln[128];
   char *ret=ln;

   assert (i < 0);
   i = -i;
   if (i >= IREGBEG && i < IREGEND)
      sprintf(ln, "%s", archiregs[i-IREGBEG]);
   else if (i >= FREGBEG && i < FREGEND)
      sprintf(ln, "%s", archfregs[i-FREGBEG]);
   else if (i >= DREGBEG && i < DREGEND)
      sprintf(ln, "%s", archdregs[i-DREGBEG]);
   else if (i >= ICCBEG && i < ICCEND)
      sprintf(ln, "%s", ICCREGS[i-ICCBEG]);
   else if (i >= FCCBEG && i < FCCEND)
      sprintf(ln, "%s", FCCREGS[i-FCCBEG]);
   else if (i == PCREG)
      sprintf(ln, "%s", "PC");
   else
      ret = NULL;
   return(ret);
}
char *Int2Reg(int i)
/*
 * Translates integral encoding to machine-specific registers
 */
{
   char *ret;
   ret = Int2Reg0(i);
   if (!ret)
   {
   fprintf(stderr, 
      "I=[%d,%d); F=[%d,%d); D=[%d,%d); ICC=[%d,%d); FCC=[%d,%d); PC=%d\n",
           IREGBEG, IREGEND,FREGBEG, FREGEND,DREGBEG, DREGEND,
           ICCBEG, ICCEND, FCCBEG, FCCEND, PCREG);
      fko_error(__LINE__, "Unknown register index %d, file=%s\n",
                i, __FILE__);
   }
   return(ret);
}

void SetAllTypeReg(int iv, int type)
/*
 * Modify regstate iv to set all registers of type type to 1, leaving the rest
 * of the state unchanged
 */
{
   int i, ibeg, iend;
   switch(type)
   {
#ifdef X86_64
   case T_SHORT:
#endif
   case T_INT:
      ibeg = IREGBEG;
      iend = IREGEND;
      break;
   case T_FLOAT:
      ibeg = FREGBEG;
      iend = FREGEND;
      break;
   case T_DOUBLE:
      ibeg = DREGBEG;
      iend = DREGEND;
      break;
   default:
      fko_error(__LINE__, "unknown type %d, file=%s\n", type, __FILE__);
   }
   for (i=ibeg; i < iend; i++)
      SetVecBit(iv, i-1, 1);
}

int Reg2Regstate(int k)
/*
 * Given register k, set regstate so that all registers used by k are
 * represented (i.e., on some archs, float and double regs are aliased)
 * RETURNS: bitvec with appropriate register numbers set
 */
{
   static int iv=0;
   int i;

   if (!iv) iv = NewBitVec(TNREG);
   SetVecAll(iv, 0);
   SetVecBit(iv, k-1, 1);
   if (k >= FREGBEG && k < FREGEND)
   {
      #if defined(X86) || defined(PPC)
         SetVecBit(iv, k-FREGBEG+DREGBEG-1, 1);
      #elif defined(SPARC)
         SetVecBit(iv, ((k-FREGBEG)>>1)+DREGBEG-1, 1);
      #endif
   }
   else if (k >= DREGBEG && k < DREGEND)
   {
      #if defined(X86) || defined(PPC)
         SetVecBit(iv, k-DREGBEG+FREGBEG-1, 1);
      #elif defined(SPARC)
         i = k - DREGBEG;
         if (i < 32)
         {
            SetVecBit(iv, FREGBEG+i, 1);
            SetVecBit(iv, FREGBEG+i+1, 1);
         }
      #endif
   }
   else if (k <= 0)
   {
      if (iv) KillBitVec(iv);
      iv = 0;
   }
   else if (!(k >= IREGBEG && k < IREGEND))
      fko_error(__LINE__, "Unknown register index %d, file=%s\n",
                  k, __FILE__);
   return(iv);
}

void NewIGTable(int chunk)
{
   int i, n;
   IGNODE **new;
   n = TNIG + chunk;
   new = malloc(n*sizeof(IGNODE*));
   assert(new);

   for (i=0; i != TNIG; i++)
      new[i] = IG[i];
   for (; i != n; i++)
      new[i] = NULL;
   if (IG) free(IG);
   IG = new;
   TNIG = n;
}

void KillIGNode(IGNODE *ig)
{
   if (ig->blkbeg)
      KillBlockList(ig->blkbeg);
   if (ig->blkend)
      KillBlockList(ig->blkend);
   if (ig->blkspan)
      KillBlockList(ig->blkspan);
   if (ig->ldhoist)
      KillBlockList(ig->ldhoist);
   if (ig->stpush)
      KillBlockList(ig->stpush);
   if (ig->myblkvec)
      KillBitVec(ig->myblkvec);
   if (ig->liveregs)
      KillBitVec(ig->liveregs);
   if (ig->conflicts)
      KillBitVec(ig->conflicts);
   IG[ig->ignum] = NULL;
   free(ig);
}

void KillIGTableEntries()
{
   int i;
   for (i=0; i < NIG; i++)
   {
      KillIGNode(IG[i]);
      IG[i] = NULL;
   }
   NIG = 0;
}

void KillIGTable()
{
   KillIGTableEntries();
   free(IG);
   IG = NULL;
   TNIG = 0;
}

short AddIG2Table(IGNODE *node)
{
   if (NIG == TNIG)
      NewIGTable(64);
   IG[NIG] = node;
   node->ignum = NIG;
   return(NIG++);
}

IGNODE *NewIGNode(BBLOCK *blk, short var)
{
   IGNODE *new;
   new = malloc(sizeof(IGNODE));
   assert(new);
   new->blkbeg = new->blkend = new->blkspan = new->ldhoist = new->stpush = 
                 NULL;
   if (blk)
   {
      new->myblkvec = NewBitVec(blk->bnum);
      SetVecBit(new->myblkvec, blk->bnum-1, 1);
   }
   else
      new->myblkvec = 0;
   new->liveregs = NewBitVec(TNREG);
   new->conflicts = NewBitVec(TNREG);
   new->nread = new->nwrite = 0;
   new->var = var;
   if (var)
   {
      if (IS_DEREF(STflag[var-1]))
         new->deref = var;
      else
      {
         new->deref = SToff[var-1].sa[2];
         assert(new->deref > 0);
      }
   }
   else new->deref = 0;
   new->ignum = AddIG2Table(new);
   new->reg = 0;
   return(new);
}

ushort AllRegVec()
{
   static int iv=0;
   int i;

   if (!iv)
   {
      iv = NewBitVec(TNREG);
      for (i=0; i < TNREG; i++)
         SetVecBit(iv, i, 1);
   }
   return(iv);
}

void CalcBlockIG(BBLOCK *bp)
{
   int i, j, n, k, iv;
   IGNODE *node;
   INSTQ *ip;
   int liveregs;
   const int chunk = 32;
   short *vals;
   IGNODE **myIG = NULL;   /* array of this block's IGNODES */
   int imask, igN=0, nn=0;
   extern int FKO_BVTMP;
   extern BBLOCK *bbbase;

   if (!CFUSETU2D)
      CalcInsOuts(bbbase);
   if (!INDEADU2D)
      CalcAllDeadVariables();
   if (FKO_BVTMP)
   {
      liveregs = FKO_BVTMP;
      SetVecAll(liveregs, 0);
   }
   else FKO_BVTMP = liveregs = NewBitVec(TNREG);
   if (!bp->ignodes) bp->ignodes = NewBitVec(TNREG+64);
   else SetVecAll(bp->ignodes, 0);
   if (!bp->conin) bp->conin = NewBitVec(TNREG+64);
   else SetVecAll(bp->conin, 0);
   if (!bp->conout) bp->conout = NewBitVec(TNREG+64);
   else SetVecAll(bp->conout, 0);
/*
 * Mask out unneeded regs from analysis
 */
   imask = NewBitVec(TNREG);
   SetVecBit(imask, REG_SP-1, 1);
   SetVecBit(imask, PCREG-1, 1);
   for (k=ICCBEG; k < ICCEND; k++)
      SetVecBit(imask, k-1, 1);
   for (k=FCCBEG; k < FCCEND; k++)
      SetVecBit(imask, k-1, 1);
   #ifdef X86_32
      SetVecBit(imask, -Reg2Int("@st")-1, 1);
   #endif
/*
 * Create ignode for all variables live on block entry, and add it
 * blocks conin, conout, and ignodes
 */
   vals = BitVec2StaticArray(bp->ins);
   for (n=vals[0], i=1; i <= n; i++)
   {
      k = vals[i];
      if (k >= TNREG)
      {
         node = NewIGNode(bp, k-TNREG+1);
         node->blkbeg = AddBlockToList(node->blkbeg, bp);
         node->blkend = AddBlockToList(node->blkend, bp);
         if (nn == igN)
            myIG = NewPtrTable(&igN, myIG, chunk);
         myIG[nn++] = node;
         SetVecBit(bp->conin, node->ignum, 1);
         SetVecBit(bp->conout, node->ignum, 1);
         SetVecBit(bp->ignodes, node->ignum, 1);
      }
/*
 *    If a register is live on block entry, add it to liveregs
 */
      else if (!BitVecCheck(imask, k))
      {
         iv = Reg2Regstate(k+1);
         BitVecComb(liveregs, liveregs, iv, '|');
      }
   }
/*
 * For all created IGNODEs, set it to conflict with all other conin,
 * and init its livereg to conin's livereg
 */
   vals = BitVec2StaticArray(bp->ignodes);
   for (n=vals[0], i=1; i <= n; i++)
   {
      k = vals[i];
      node = IG[k];
      node->conflicts = BitVecCopy(node->conflicts, bp->conin);
      SetVecBit(node->conflicts, node->ignum, 0);
      node->liveregs = BitVecCopy(node->liveregs, liveregs);
   }
   for (ip=bp->ainst1; ip; ip = ip->next)
   {
/*
 *   If var is referenced as a use, update nread
 */
      vals = BitVec2StaticArray(ip->use);
      for (n=vals[0], i=1; i <= n; i++)
      {
         k = vals[i];
         if (k >= TNREG)
         {
            k += 1 - TNREG;
            for (j=nn-1; j >= 0; j--)
               if (myIG[j] && myIG[j]->var == k) break;
            assert(j >= 0);
            myIG[j]->nread++;
         }
      }
/*
 *    Handle deads
 */
      vals = BitVec2StaticArray(ip->deads);
      for (n=vals[0], i=1; i <= n; i++)
      {
         k = vals[i];
         if (k >= TNREG)  /* dead item is a var */
         {
/*
 *          Find ignode associated with var
 */
            k += 1 - TNREG;
            for (j=nn-1; j >= 0; j--)
               if (myIG[j] && myIG[j]->var == k) break;
            assert(j >= 0);
/*
 *          If dying range ends with a write, indicate it
 */
            if (BitVecCheck(ip->set, k+TNREG-1))
            {
               fprintf(stderr,
                       "Inst dead on write, block=%d, inst='%s %s %s %s'!\n",
                       bp->bnum, instmnem[ip->inst[0]], op2str(ip->inst[1]),
                       op2str(ip->inst[2]), op2str(ip->inst[3]));
               fprintf(stderr, "k=%d, STentry=%d\n", k, k+TNREG-1);
               PrintInst(fopen("err.l", "w"), bbbase); exit(-1);
               myIG[j]->nwrite++;
            }
/*
 *          If var is dead, delete it from myIG and block's conout, and
 *          indicate that this is a block and instruction where range dies
 */
            SetVecBit(bp->conout, myIG[j]->ignum, 0);
            myIG[j] = NULL;
/*            node->blkend = AddBlockToList(node->blkend, bp); */
            assert(node->blkend->blk == bp);
            node->blkend->ptr = ip;
         }
/*
 *       If it is a register that's dead, delete it from liveregs
 */
         else
            SetVecBit(liveregs, k, 0);
      }
/* 
 *    Handle sets
 */
      vals = BitVec2StaticArray(ip->set);
      for (n=vals[0], i=1; i <= n; i++)
      {
         k = vals[i];
         if (k >= TNREG) /* variable is being set */
         {
            k += 1 - TNREG;
            for (j=nn-1; j >= 0; j--)
               if (myIG[j] && myIG[j]->var == k) break;
/*
 *          If variable is set and not already live, then:
 *             a. create new ignode for it
 *             b. set a conflict between it and all other active ignodes
 *             c. set ignode's liveregs to current liveregs
 *             d. add it to blocks ignodes
 */
            if (j < 0)
            {
               node = NewIGNode(bp, k);
               if (nn == igN)
                  myIG = NewPtrTable(&igN, myIG, chunk);
               myIG[nn++] = node;
               for (j=0; j != nn; j++)
                  if (myIG[j])
                     SetVecBit(myIG[j]->conflicts, node->ignum, 1);
               node->conflicts = BitVecCopy(node->conflicts, bp->conout);
               SetVecBit(bp->conout, node->ignum, 1);
               node->liveregs = BitVecCopy(node->liveregs, liveregs);
               node->blkbeg = AddBlockToList(node->blkbeg, bp);
               node->blkend = AddBlockToList(node->blkend, bp);
               node->blkbeg->ptr = ip;
               SetVecBit(bp->ignodes, node->ignum, 1);
               node->nwrite++;
            }
         }
/*
 *       If it's a register being set, add it to list of live regs as well
 *       as to the regstate of all currently live ranges
 */
         else if (!BitVecCheck(imask, k))
         {
            iv = Reg2Regstate(k+1);
            BitVecComb(liveregs, liveregs, iv, '|');
            for (j=0; j != nn; j++)
               if (myIG[j])
                  BitVecComb(myIG[j]->liveregs, myIG[j]->liveregs, iv, '|');
         }
      }
   }
   if (myIG) free(myIG);
#if 0
/*
 * Find ignodes that span this block
 */
   iv = BitVecComb(FKO_BVTMP, bp->conout, bp->conin, '&');
   vals = BitVec2StaticArray(iv);
   for (n=vals[0], i=1; i <= n; i++)
   {
      node = IG[vals[k]];
      #if IFKO_DEBUG_LEVEL >= 1
         assert(!(node->blkbeg || node->blkend));
      #endif
      node->blkspan = AddBlockToList(node->blkspan, bp);
   }
#endif
   KillBitVec(imask);
}

void CombineLiveRanges(BLIST *scope, BBLOCK *pred, int pig,
                       BBLOCK *succ, int sig)
/*
 * Merge successor IG (sig) into pred ig (pig); both blocks are known to
 * be in the register scope.
 */
{
   IGNODE *pnode, *snode, *node;
   BLIST *bl, *lp;
   INSTQ *ip;
   short *vals;
   int i, n;

   pnode = IG[pig];
   snode = IG[sig];

fprintf(stderr, "Combine pig=%d, sig=%d\n", pig, sig);
   assert(pnode->var == snode->var);
   assert(snode->blkend && snode->blkbeg);
   assert(pnode->blkend && pnode->blkbeg);
/*
 * If a block is it's own successor, we must see which IG is really the pred
 */
   if (pred == succ)
   {
      pnode->blkend = RemoveBlockFromList(pnode->blkend, pred);
      snode->blkbeg = RemoveBlockFromList(snode->blkbeg, succ);
      pnode->blkbeg = MergeBlockLists(pnode->blkbeg, snode->blkbeg);
      pnode->blkend = MergeBlockLists(snode->blkend, pnode->blkend);
      snode->blkbeg = snode->blkend = NULL;
      pnode->blkspan = MergeBlockLists(pnode->blkspan, snode->blkspan);
      snode->blkspan = NULL;
   }
   else
   {
/*
 *    To update blkspan, merge lists.
 */
      if (snode->blkspan)
      {
         pnode->blkspan = MergeBlockLists(pnode->blkspan, snode->blkspan);
         snode->blkspan = NULL;
      }
      if (!BitVecCheck(pnode->myblkvec, snode->blkbeg->blk->bnum-1))
      {
         pnode->blkspan = MergeBlockLists(pnode->blkspan, snode->blkbeg);
         snode->blkbeg = NULL;
      }
/*
 *    Merge end and beg lists, but pred is no longer an ending block, and
 *    succ is no longer a beginning block
 */
      pnode->blkbeg = MergeBlockLists(pnode->blkbeg, snode->blkbeg);
      pnode->blkend = MergeBlockLists(snode->blkend, pnode->blkend);
      pnode->blkbeg = RemoveBlockFromList(pnode->blkbeg, succ);
      pnode->blkend = RemoveBlockFromList(pnode->blkend, pred);
      snode->blkbeg = snode->blkend = NULL;
/*
 *    If succ block is not ending block, it becomes a spanned block
 */
      if ( !FindBlockInList(pnode->blkend, succ) && 
           !FindBlockInList(pnode->blkspan, succ) )
         pnode->blkspan = AddBlockToList(pnode->blkspan, succ);
   }
/*
 * The range ending in succ block may change due to new beginning in pred
 */
   bl = FindInList(pnode->blkend, succ);
   if (bl)
   {
      i = pnode->var + TNREG-1;
      for (ip=pnode->blkend->blk->ainst1; ip; ip = ip->next)
      {
         if (BitVecCheck(ip->deads, i))
         {
            bl->ptr = ip;
            break;
         }
      }
   }
   if (snode->ldhoist)
   {
      pnode->ldhoist = MergeBlockLists(pnode->ldhoist, snode->ldhoist);
      snode->ldhoist = NULL;
   }
   if (snode->stpush)
   {
      pnode->stpush = MergeBlockLists(pnode->stpush, snode->stpush);
      snode->stpush = NULL;
   }
   pnode->nread += snode->nread;
   pnode->nwrite += snode->nwrite;
   BitVecComb(pnode->myblkvec, pnode->myblkvec, snode->myblkvec, '|');
   BitVecComb(pnode->liveregs, pnode->liveregs, snode->liveregs, '|');
   BitVecComb(pnode->conflicts, pnode->conflicts, snode->conflicts, '|');
/*
 * Change snode conflicts to pnode conflicts
 */
   vals = BitVec2StaticArray(snode->conflicts);
   for (i=1, n=vals[0]; i <= n; i++)
   {
      node = IG[vals[i]];
      SetVecBit(node->conflicts, sig, 0);
      SetVecBit(node->conflicts, pig, 1);
   }
/*
 * Update block-carried IG info
 */
   for (bl=scope; bl; bl = bl->next)
   {
      if (BitVecCheck(bl->blk->ignodes, sig))
      {
         SetVecBit(bl->blk->ignodes, sig, 0);
         SetVecBit(bl->blk->ignodes, pig, 1);
         if (BitVecCheck(bl->blk->conin, sig))
         {
            SetVecBit(bl->blk->conin, sig, 0);
            SetVecBit(bl->blk->conin, pig, 1);
         }
         if (BitVecCheck(bl->blk->conout, sig))
         {
            SetVecBit(bl->blk->conout, sig, 0);
            SetVecBit(bl->blk->conout, pig, 1);
         }
      }
   }
   KillIGNode(snode);
}

void CombineBlockIG(BLIST *scope, ushort scopeblks, BBLOCK *pred, BBLOCK *succ)
/*
 * Attempt to combine preds IG with succ
 */
{
   IGNODE *node;
   int i, j, n, nn;
   short *sig, *pig;
   short k, kk;
/*
 * If both blocks are in scope, attempt to combine their live ranges
 */
   if (BitVecCheck(scopeblks, succ->bnum-1) && 
       BitVecCheck(scopeblks, pred->bnum-1) )
   {
      pig = BitVec2Array(pred->conout, 0);
      sig = BitVec2StaticArray(succ->conin);
      for (n=sig[0], i=1; i <= n; i++)
      {
         k = IG[sig[i]]->var;
         for (nn=pig[0], j=1; j <= nn; j++)
         {
            kk = pig[j];
            if (k == IG[kk]->var && sig[i] != pig[j])
               CombineLiveRanges(scope, pred, kk, succ, sig[i]);
         }
      }
      free(pig);
   }
/*
 * If pred in scope, but successor not, and LR includes a store, must push LR
 * stores to succ. Assign the next inst of the pushed stores to
 * node->stpush->ptr.
 */
   else if (BitVecCheck(scopeblks, pred->bnum-1))
   {
      pig = BitVec2StaticArray(pred->conout);
      for (n=pig[0], i=1; i <= n; i++)
      {
         node = IG[pig[i]];
         if (node->nwrite)
         {
            node->stpush = AddBlockToList(node->stpush, succ);
            if (succ->ainst1)
            {
               k = GET_INST(succ->ainst1->inst[0]);
               node->stpush->ptr = (k == LABEL) ? 
                                   succ->ainst1->next : succ->ainst1;
            }
            else node->stpush->ptr = NULL;
         }
      }
   }
/*
 * If succ in scope but pred not, pred becomes load hoist target
 * Find the prev of the hoisted loads
 */
   else /* if (BitVecCheck(scopeblks, succ->bnum-1, 1)) */
   {
      sig = BitVec2StaticArray(succ->conin);
      for (n=sig[0], i=1; i <= n; i++)
      {
         node = IG[sig[i]];
         node->ldhoist = AddBlockToList(node->ldhoist, pred);
         if (pred->ainstN)
         {
            k = GET_INST(pred->ainstN->inst[0]);
            node->ldhoist->ptr = IS_BRANCH(k) ? pred->ainstN->prev : 
                                                pred->ainstN;
         }
         else node->ldhoist->ptr = NULL;
      }
   }
}

void CalcScopeIG(BLIST *scope)
{
   static ushort blkvec=0;
   BLIST *bl, *lp;
   ushort iv;
   short *sp;

/*
 * Set blkvec to reflect all blocks in scope, and calculate each block's IG
 */
   if (blkvec) SetVecAll(blkvec, 0);
   else blkvec = NewBitVec(32);
   for (bl=scope; bl; bl = bl->next)
   {
      SetVecBit(blkvec, bl->blk->bnum-1, 1);
      CalcBlockIG(bl->blk);
   }
/*
 * Try to combine live ranges across basic blocks
 */
   for (bl=scope; bl; bl = bl->next)
   {
      CombineBlockIG(scope, blkvec, bl->blk, bl->blk->usucc);
      if (bl->blk->csucc)
         CombineBlockIG(scope, blkvec, bl->blk, bl->blk->csucc);
      for (lp=bl->blk->preds; lp; lp = lp->next)
         if (!BitVecCheck(blkvec, lp->blk->bnum-1) &&
             lp->blk != bl->blk->usucc && lp->blk != bl->blk->csucc)
            CombineBlockIG(scope, blkvec, lp->blk, bl->blk);
   }
}

void SortUnconstrainedIG(int N, IGNODE **igarr)
/*
 * Given an N-length contiguous array if IGNODEs, sorts them by nref using
 * a simple selection sort
 */
{
   int i, j, nref, maxref, maxi;
   IGNODE *maxptr;
/*
 * Perform simple selection sort
 */
   for (i=0; i < N-1; i++)
   {
      maxref = igarr[i]->nread + igarr[i]->nwrite;
      maxi = i;
      for (j=i+1; j < N; j++)
      {
         nref = igarr[j]->nread + igarr[j]->nwrite;
         if (nref > maxref)
         {
            maxi = j;
            maxref = nref;
         }
      }
      if (maxi != i)
      {
         maxptr = igarr[maxi];
         igarr[maxi] = igarr[i];
         igarr[i] = maxptr;
      }
   }
}
void SortConstrainedIG(int N, IGNODE **igs)
/*
 * Right now, this routine just calls the unconstrained sort, but eventually
 * it should be changed to optimize assignment.  For instance, when deciding
 * between conflicting variables, if one variable's liverange (LR) spans the LR
 * of multiple conflicting vars, you must compare against the sum of the refs
 * of the spanned LRs.
 */
{
   SortUnconstrainedIG(N, igs);
}

IGNODE **SortIG(int *N)
/* 
 * Sorts IGNODEs by # refs.
 * RETURNS: N-length contiguous array of sorted IGNODEs
 * This routine also ignores live ranges (LR) with too few references to
 * be worth assigning.
 */
{
   int n, ncon, i, j, nref, iv;
   IGNODE *ig;
   IGNODE **igarr;
   extern int FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(TNREG);
   iv = FKO_BVTMP;
/*
 * Find number of IG nodes used, allocate array to store, and copy to
 * contiguous storage
 */
   for (n=i=0; i < NIG; i++)
   {
      if (IG[i] && IG[i]->nwrite + IG[i]->nread > 1)
         n++;
   }
   if (n == 0)
   {
      *N = 0;
      return(NULL);
   }
   igarr = malloc(n*sizeof(IGNODE *));
   assert(igarr);
   for (j=i=0; i < NIG; i++)
      if (IG[i] && IG[i]->nwrite + IG[i]->nread > 1)
         igarr[j++] = IG[i];
/*
 * Now, sort array into two sub arrays, with the contrained nodes in the first
 * ncon elements of the array.  Constrained nodes are those nodes where the #
 * of conflicts is greater than the number of available registers.  Therefore
 * unconstrained nodes can use any register, so do reg asg on constrained nodes
 * first
 */
   for (ncon=i=0; i != n; i++)
   {
     ig = igarr[i];
     SetVecAll(iv, 0);
     SetAllTypeReg(iv, FLAG2PTYPE(STflag[ig->var-1]));
     BitVecComb(iv, iv, ig->liveregs, '-');
/*
 *   If # of regs available (j) is less than the number of conflicts, node is
 *   constrained
 */
     j = CountBitsSet(iv);
     if (j < CountBitsSet(ig->conflicts))
     {
        igarr[i] = igarr[ncon];
        igarr[ncon++] = ig;
     }
   }
fprintf(stderr, "NIG=%d, NCON=%d, NUNCON=%d\n", n, ncon, n-ncon);
   SortConstrainedIG(ncon, igarr);
   SortUnconstrainedIG(n-ncon, igarr+ncon);
   *N = n;
   return(igarr);
}

void DoIGRegAsg(int N, IGNODE **igs)
/*
 * Given an N-length array of sorted IGNODEs, perform register assignment
 * Right now, use simple algorithm for assignment, improve later.
 */
{
   int iv, i, j, n;
   IGNODE *ig;
   short *sp;
   extern int FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(TNREG);
   iv = FKO_BVTMP;

   for (i=0; i < N; i++)
   {
      ig = igs[i];
      SetVecAll(iv, 0);
      SetAllTypeReg(iv, FLAG2PTYPE(STflag[ig->var-1]));
      BitVecComb(iv, iv, ig->liveregs, '-');
/*
 *    If we have a register left, assign it
 */
      ig->reg = AnyBitsSet(iv);
      if (ig->reg)
      {
         SetVecBit(ig->liveregs, ig->reg, 1);
/*
 *       Add assigned register to liveregs of conflicting IGNODEs
 */
         sp = BitVec2StaticArray(ig->conflicts);
         if (sp)
         {
            for (j=0, n=sp[0]; j < n; j++)
               SetVecBit(ig->liveregs, ig->reg, 1);
         }
      }
      else
         fprintf(stderr, "NO FREE REGISTER FOR LR %d!!!\n", ig->ignum);
   }
}

int VarUse2RegUse(IGNODE *ig, BBLOCK *blk, INSTQ *instbeg, INSTQ *instend)
/*
 * Changes all uses of ig->var to ig->reg in block blk, between the indicated
 * instructions (inclusive).  If either is NULL, we take instbeg/end.
 */
{
   INSTQ *ip;
   int CHANGE=0;

   assert(INUSETU2D);
   assert(ig && blk);
   if (!instbeg)
      instbeg = blk->ainst1;
   if (!instend)
      instend = blk->ainstN;
   if (!instbeg || !ig->reg) return;
   assert(instend);
   do
   {
      ip = instbeg;
/*
 *    Change all uses of this ig->var to ig->reg
 */
      if (BitVecCheck(ip->use, ig->var+TNREG-1))
      {
         SetVecBit(ip->use, ig->var+TNREG-1, 0);
         SetVecBit(ip->use, ig->reg-1, 0);
         assert(ip->inst[2] == ig->deref || ip->inst[3] == ig->deref);
         if (ip->inst[2] == ig->var)
            ip->inst[2] = -ig->reg;
         if (ip->inst[3] == ig->var)
            ip->inst[3] = -ig->reg;
         switch(GET_INST(ip->inst[0]))
         {
         case LD:
            ip->inst[0] = MOV;
            break;
         case FLD:
            ip->inst[0] = FMOV;
            break;
         case FLDD:
            ip->inst[0] = FMOVD;
            break;
         case VFLD:
            ip->inst[0] = VFMOV;
            break;
         default:
            fprintf(stderr, "\n\nWARNING(%s,%d): inst %d being var2reged!!\n\n",
                    __FILE__, __LINE__, ip->inst[0]);
         }
         CalcThisUseSet(ip);
         CHANGE++;
      }
      instbeg = instbeg->next;
   }
   while(ip != instend);
   if (CHANGE) 
      INDEADU2D = CFUSETU2D = CFUSETU2D = 0;
   return(CHANGE);
}

int DoRegAsgTransforms(IGNODE *ig)
/*
 * Given the fully qualified IG, perform all of the code transformations for
 * register assignment: variable uses to register ref, variable sets become
 * register moves, and hoist ld and/or push stores for blocks not in scope
 */
{
   INSTQ *ip;
   BLIST *bl, *lp;
   enum inst mov, ld, st;
   int i, CHANGE=0;

#if IFKO_DEBUG_LEVEL >= 1
   assert(ig->blkbeg);
   assert(!ig->blkbeg->next);
   assert(ig->blkend);
   assert(!ig->blkend->next);
#endif
   i = STflag[ig->var-1];
   switch(FLAG2PTYPE(i))
   {
   case T_INT:
      mov = MOV;
      ld  = LD;
      st  = ST;
      break;
   case T_FLOAT:
      mov = FMOV;
      ld  = FLD;
      st  = FST;
      break;
   case T_DOUBLE:
      mov = FMOVD;
      ld  = FLDD;
      st  = FSTD;
      break;
   default:
      fko_error(__LINE__, "Unknown type %d (flag=%d) in %s\n", FLAG2TYPE(i),
                i, __FILE__);
   }
/*
 * Handle initial set of ig->reg required for every blkbeg
 */
   for (bl=ig->blkbeg; bl; bl = bl->next)
   {
/*
 *    If LR begins with a set, change it to a reg-reg move
 *    NOTE: we have no memory-output instructions, so assert set is ST
 */
      #if IFKO_DEBUG_LEVEL >= 1
         assert(bl->ptr);
      #endif
      ip = bl->ptr;
      if (BitVecCheck(ip->set, ig->var+TNREG-1))
      {
         CHANGE++;
         if (!IS_STORE(ip->inst[0]))
         {
            PrintThisInst(stderr, __LINE__, ip);
            exit(-1);
         }
         ip->inst[0] = mov;
         ip->inst[1] = ig->reg;
         CalcThisUseSet(ip);
      }
/*
 *    If LR does not begin with a set, we must add a load to ig->reg.
 *    If one of this begblock's pred is in ldhoist, we add the load using
 *    only the ldhoist, otherwise add it in this block.
 */
      for (lp=ig->ldhoist; lp && FindBlockInList(bl->blk->preds, lp->blk);
           lp = lp->next)
/*
 *    If no ldhoist, add the load in this block.  If blkbeg->ptr not set, add
 *    at beginning of block
 */
      if (lp)
      {
          if (bl->ptr)
             bl->ptr = InsNewInst(bl->blk, NULL, bl->ptr, ld, -ig->reg,
                                  ig->deref, 0);
          else
             bl->ptr = InsNewInst(bl->blk, NULL, bl->blk->ainst1, ld, -ig->reg,
                                  ig->deref, 0);
         CHANGE++;
         CalcThisUseSet(bl->ptr);
      }
   }
/*
 * Hoist the ld to all extra-scope blocks that may need it
 */
   for (bl=ig->ldhoist; bl; bl = bl->next)
   {
      #if IFKO_DEBUG_LEVEL >= 1
         assert(bl->ptr);
      #endif
      ip = InsNewInst(bl->blk, bl->ptr, NULL, ld, -ig->reg, ig->deref, 0);
      CalcThisUseSet(ip);
      CHANGE++;
   }
/*
 * Push any needed store to extra-scope blocks that require it
 */
   for (bl=ig->stpush; bl; bl = bl->next)
   {
      #if IFKO_DEBUG_LEVEL >= 1
         assert(bl->ptr);
      #endif
      ip = InsNewInst(bl->blk, NULL, bl->ptr, st, -ig->reg, ig->deref, 0);
      CalcThisUseSet(ip);
      CHANGE++;
   }
   if (CHANGE) 
      INDEADU2D = CFUSETU2D = CFUSETU2D = 0;
/*
 * ================================================================
 * Now we change all use of var in LR to access the ig->reg instead
 * ================================================================
 */
   for (bl=ig->blkspan; bl; bl = bl->next)
      CHANGE += VarUse2RegUse(ig, bl->blk, NULL, NULL);
   for (bl=ig->blkend; bl; bl = bl->next)
      CHANGE += VarUse2RegUse(ig, bl->blk, NULL, bl->ptr);
   for (bl=ig->blkbeg; bl; bl = bl->next)
   {
      ip = bl->ptr;
      if (ip) ip = ip->next;
      CHANGE += VarUse2RegUse(ig, bl->blk, ip, NULL);
   }
   return(CHANGE);
}

int DoScopeRegAsg(BLIST *scope)
{
   IGNODE **igs;
   int i, N;
   extern FILE *fpIG, *fpLIL, *fpST;
   void DumpIG(FILE *fpout, int N, IGNODE **igs);
   extern BBLOCK *bbbase;

   CalcScopeIG(scope);
   igs = SortIG(&N);
   DoIGRegAsg(N, igs);
fprintf(stderr, "\n\n*** NIG = %d\n", N);
   if (fpIG)
   {
      DumpIG(fpIG, N, igs);
      fclose(fpIG);
   }
   if (fpLIL)
   {
      PrintInst(fpLIL, bbbase);
      fclose(fpLIL);
      fpLIL = NULL;
   }
   if (fpST)
   {
      PrintST(fpST);
      fclose(fpST);
      fpST = NULL;
   }
#if 1
   for (i=0; i != N; i++)
      DoRegAsgTransforms(igs[i]);
#endif
   if (igs) free(igs);
   return(0);
}


static void PrintBlockWithNum(FILE *fpout, BLIST *bl)
{
   if (!bl)
   {
      fprintf(fpout, "NONE\n");
      return;
   }
   fprintf(fpout, "%4d(%5d)", bl->blk->bnum, FindInstNum(bl->blk, bl->ptr));
   for (bl=bl->next; bl; bl = bl->next)
      fprintf(fpout, ", %4d(%5d)", bl->blk->bnum, 
              FindInstNum(bl->blk, bl->ptr));
   fprintf(fpout, "\n");
}

void DumpIG(FILE *fpout, int N, IGNODE **igs)
/*
 * Dumps IG information to file
 */
{
   int i;
   IGNODE *ig;
   BLIST *bl;
   fprintf(fpout, "Printing information for %d IGNODES:\n\n", N);
   for (i=0; i < N; i++)
   {
      ig = igs[i];
      if (!ig) continue;
      fprintf(fpout, 
         "*** IG# = %5d, VAR = %5d, REG = %5d, NREAD = %6d, NWRITE = %3d\n",
              ig->ignum, ig->var, ig->reg, ig->nread, ig->nwrite);
      fprintf(fpout, "    begblks : ");
      PrintBlockWithNum(fpout, ig->blkbeg);
      fprintf(fpout, "    spanblk : ");
      if (!ig->blkspan)
         fprintf(fpout, " NONE\n");
      else
      {
         fprintf(fpout, " %5d", ig->blkspan->blk->bnum);
         for (bl=ig->blkspan->next; bl; bl = bl->next)
            fprintf(fpout, ", %5d", bl->blk->bnum);
         fprintf(fpout, "\n");
      }
      fprintf(fpout, "    endblks : ");
      PrintBlockWithNum(fpout, ig->blkend);
      fprintf(fpout, "    ldhoist : ");
      PrintBlockWithNum(fpout, ig->ldhoist);
      fprintf(fpout, "    stpush  : ");
      PrintBlockWithNum(fpout, ig->stpush);
      fprintf(fpout, "    conflict: %s\n", PrintVecList(ig->conflicts, 0));
      fprintf(fpout, "\n\n");
   }
}

/*
 * ===========================================================================
 * For every local live entering or leaving the innermost loop, we globally
 * assign it to a register for the length of the loop.  This is done before
 * any other asignment, and is part of getting the loop to our normal form.
 * The following routines are involved in doing this initial global assignment
 * ===========================================================================
 */
int AsgGlobalLoopVars(LOOPQ *loop, short *iregs, short *fregs, short *dregs)
/*
 * Finds locals live on loop entry or exit: these locals will be assigned
 * to registers for the entire loop (i.e., global instead of live range
 * assignment) so that their load/stores may be moved outside the loop.
 * For each such local, search through available registers (0 in [i,f,d]regs
 * entry means reg is available), and assign the appropriate registers.
 * NOTE: only done on innermost loop
 * RETURNS: 0 on success, non-zero on failure.
 */
{
   int iv, i, j, k, n;
   BLIST *bl;
   INSTQ *ip;
   short *sa, *s;
   extern int FKO_BVTMP;

/*
 * Find all variables set in list
 */
   if (!loop->sets) loop->sets = NewBitVec(TNREG+32);
   else SetVecAll(loop->sets, 0);
   for (bl=loop->blocks; bl; bl = bl->next)
   {
      for (ip=bl->blk->inst1; ip; ip = ip->next)
         BitVecComb(loop->sets, loop->sets, ip->set, '|');
   }
/*
 * Find regs and vars live on loop entry and exit(s)
 */
   if (!loop->outs) loop->outs = NewBitVec(TNREG+32);
   else SetVecAll(loop->outs, 0);
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(TNREG+32);
   else SetVecAll(FKO_BVTMP, 0);
   for (bl=loop->posttails; bl; bl = bl->next)
      BitVecComb(FKO_BVTMP, FKO_BVTMP, bl->blk->ins, '|');
   for (bl=loop->tails; bl; bl = bl->next)
      BitVecComb(loop->outs, loop->outs, bl->blk->outs, '|');
   BitVecComb(loop->outs, loop->outs, FKO_BVTMP, '&');
   FKO_BVTMP = iv = BitVecComb(FKO_BVTMP, loop->outs, loop->header->ins, '|');
   k = STstrlookup("_DEREF");
   SetVecBit(iv, k+TNREG-1, 0);
fprintf(stderr, "\nhost/push = %s\n\n", BV2VarNames(iv));

   sa = BitVec2Array(iv, 1);
   for (n=0, j=i=1; i <= sa[0]; i++) 
   {
      k = sa[i];
/*
 *    For locals (ignore registers), search appropriate type array, and
 *    assign variable to the first available register
 */
      if (k > TNREG)
      {
         k = STflag[k-1-TNREG];
         k = FLAG2PTYPE(k);
fprintf(stderr, "var I = %d\n", sa[i] - TNREG + 1);
         if (k == T_INT)
         {
            s = iregs;
            n = NIR;
         }
         else if (k == T_DOUBLE)
         {
            s = dregs;
            n = NDR;
         }
         else if (k == T_FLOAT)
         {
            s = fregs;
            n = NFR;
         }
         else
         {
            fko_error(__LINE__, "Unknown type %d, file=%s\n", k, __FILE__);
            return(1);
         }
         for (k=0; k != n && s[k]; k++);
         if (k != n)
            s[k] = sa[i] - TNREG;
         else
         {
            fko_error(__LINE__, "Out of regs in global asg, var=%s, file=%s\n",
                      STname[sa[i]-TNREG], __FILE__);
            return(1);
         }
      }
   }
   free(sa);
   return(0);
}

void FindInitRegUsage(BLIST *bp, short *iregs, short *fregs, short *dregs)
/*
 * Finds all registers already being used in blocks
 * for each data type, return an array of shorts NREG long.  0 means that
 * register is not used, -1 means it is being used.
 */
{
   
   int iv, i, j, n;
   extern int FKO_BVTMP;
   short *sp;

/*
 * Init all regs to unused
 */
   for (i=0; i < IREGEND-IREGBEG; i++)
      iregs[i] = 0;
   for (i=0; i < FREGEND-FREGBEG; i++)
      fregs[i] = 0;
   for (i=0; i < DREGEND-DREGBEG; i++)
      dregs[i] = 0;
/*
 * Find all vars & regs used or defed in all blocks
 */
   if (bp)
   {
      FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, bp->blk->uses);
      BitVecComb(iv, iv, bp->blk->defs, '|');
      for (bp=bp->next; bp; bp = bp->next)
      {
         BitVecComb(iv, iv, bp->blk->uses, '|');
         BitVecComb(iv, iv, bp->blk->defs, '|');
      }
   }
/*
 * Mark all registers used for unknown purposes
 */
   sp = BitVec2Array(iv, 1);
   for (n=sp[0], i=0; i <= n; i++)
   {
      j = sp[i];
      if (j < TNREG)
      {
         if (j >= IREGBEG && j < IREGEND)
            iregs[j-IREGBEG] = -1;
         else if (j >= FREGBEG && j < FREGEND)
            fregs[j-FREGBEG] = -1;
         else if (j >= DREGBEG && j < DREGEND)
            dregs[j-DREGBEG] = -1;
      }
   }
   free(sp);
fprintf(stderr, "\n%s(%d): reg usage:\n", __FILE__, __LINE__);
fprintf(stderr, "   iregs =");
for (i=0; i < IREGEND-IREGBEG; i++)
   fprintf(stderr, " %d,", iregs[i]);
fprintf(stderr, "\n   fregs =");
for (i=0; i < FREGEND-FREGBEG; i++)
   fprintf(stderr, " %d,", fregs[i]);
fprintf(stderr, "\n   dregs =");
for (i=0; i < DREGEND-DREGBEG; i++)
   fprintf(stderr, " %d,", dregs[i]);
fprintf(stderr, "\n\n");
}

int LoadStoreToMove(BLIST *blocks, int n, short *vars, short *regs)
/*
 * Replaces all loads and stores of vars in blocks with MOVs from the
 * registers indicated in regs
 */
{
   int i, iv, changes=0;
   short is;
   BLIST *bl;
   INSTQ *ip;
   extern int FKO_BVTMP;
   enum inst *movs;

/*
 * Set up n-length array showing what mov instruction to use based on data type
 */
   movs = malloc(n*sizeof(enum inst));
   assert(movs);
   for (i=0; i != n; i++)
   {
      is = -regs[i];
      if (is >= IREGBEG && is < IREGEND) movs[i] = MOV;
      else if (is >= FREGBEG && is < FREGEND) movs[i] = FMOV;
      else if (is >= DREGBEG && is < DREGEND) movs[i] = FMOVD;
/*      else if (is >= VREGBEG && is < VREGEND) movs[i] = VFMOV; */
   }
   for (bl=blocks; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         is = GET_INST(ip->inst[0]);
         switch(is)
         {
         case LD:
         case FLD:
         case FLDD:
         case VFLD:
/* 
 *       See if variable being loaded from (inst[2]) is one of our targets
 *       change LD instruction to MOV instruction
 */
            is = ip->inst[2];
            for (i=0; i != n && is != SToff[vars[i]-1].sa[2]; i++);
            if (i != n)
            {
               changes++;
               ip->inst[0] = movs[i];
               ip->inst[2] = regs[i];
/*
 *             Change var use to reg use
 */
               SetVecBit(ip->use, is-1, 0);
               SetVecBit(ip->use, -regs[i]-1, 1);
            }
            break;
         case ST:
         case FST:
         case FSTD:
         case VFST:
/* 
 *       See if variable being stored to (inst[1]) is one of our targets
 *       change ST instruction to MOV instruction
 */
            is = ip->inst[1];
            for (i=0; i != n && is != SToff[vars[i]-1].sa[2]; i++);
            if (i != n)
            {
               changes++;
               ip->inst[0] = movs[i];
               ip->inst[1] = regs[i];
/*
 *             Change var set to reg set
 */
               SetVecBit(ip->set, is-1, 0);
               SetVecBit(ip->set, -regs[i]-1, 1);
            }
            break;
         default:;
         }
      }
   }
   free(movs);
/*
 * This routine keeps the instq set/use up to date, but invalidates
 * block-level set/use (CFUSETU2D) and deads (INDEADU2D)
 */
   INDEADU2D = CFUSETU2D = 0;
   return(changes);
}

void DoLoopGlobalRegAssignment(LOOPQ *loop)
/* 
 * This routine does global register assignment for all locals referenced
 * in the loop, and live on loop entry or exit
 */
{
   short *sp;
   int i, j, n;
   short k;
   short iregs[NIR], fregs[NFR], dregs[NDR]; 
   short *vars, *regs;

   FindInitRegUsage(loop->blocks, iregs, fregs, dregs);
   assert(!AsgGlobalLoopVars(loop, iregs, fregs, dregs));
fprintf(stderr, "\n%s(%d): reg usage:\n", __FILE__, __LINE__);
fprintf(stderr, "   iregs =");
for (i=0; i < IREGEND-IREGBEG; i++)
   fprintf(stderr, " %d,", iregs[i]);
fprintf(stderr, "\n   fregs =");
for (i=0; i < FREGEND-FREGBEG; i++)
   fprintf(stderr, " %d,", fregs[i]);
fprintf(stderr, "\n   dregs =");
for (i=0; i < DREGEND-DREGBEG; i++)
   fprintf(stderr, " %d,", dregs[i]);
fprintf(stderr, "\n\n");
/*
 * Find total number of global assignments done, and allocate space to hold
 * mapping
 */
   for (n=i=0; i < NIR; i++)
      if (iregs[i] > 0) n++;
   for (i=0; i < NFR; i++)
      if (fregs[i] > 0) n++;
   for (i=0; i < NDR; i++)
      if (dregs[i] > 0) n++;
   regs = malloc(2*n*sizeof(short));
   assert(regs);
   vars = regs + n;
/*
 * Setup variable-to-register mapping, and perform register assignment
 * on body of loop
 */
   for (j=i=0; i < NIR; i++)
   {
      if (iregs[i] > 0)
      {
         vars[j] = iregs[i];
         regs[j++] = -i - IREGBEG;
      }
   }
   for (i=0; i < NFR; i++)
   {
      if (fregs[i] > 0)
      {
         vars[j] = fregs[i];
         regs[j++] = -i - FREGBEG;
      }
   }
   for (i=0; i < NDR; i++)
   {
      if (dregs[i] > 0)
      {
         vars[j] = dregs[i];
         regs[j++] = -i - DREGBEG;
      }
   }
   i = LoadStoreToMove(loop->blocks, n, vars, regs);
   free(regs);
   fprintf(stderr, "Removed %d LD/ST using global register assignment!\n", i);
/*
 * Insert appopriate LD in preheader, and ST in post-tails
 */
   assert(loop->preheader);
   assert(loop->posttails);
   PrintComment(loop->preheader, NULL, NULL, 
                "START global asg preheader load");
   PrintComment(loop->posttails->blk, NULL, loop->posttails->blk->inst1, 
                "DONE  global asg sunk stores");
   for (i=0; i < NIR; i++)
   {
      if (iregs[i] > 0)
      {
fprintf(stderr, "ihoisting/pushing %d, %s\n", iregs[i], STname[iregs[i]-1]);
         k = SToff[iregs[i]-1].sa[2];
         if (BitVecCheck(loop->header->ins, iregs[i]+TNREG-1))
            InsNewInst(loop->preheader, NULL, NULL, LD, -i-IREGBEG, k, 0);
         if (BitVecCheck(loop->outs, iregs[i]+TNREG-1) && 
             BitVecCheck(loop->sets, iregs[i]+TNREG-1))
            InsInstInBlockList(loop->posttails, 1, ST, k, -i-IREGBEG, 0);
      }
   }
   for (i=0; i < NFR; i++)
   {
      if (fregs[i] > 0)
      {
fprintf(stderr, "fhoisting/pushing %d, %s\n", fregs[i], STname[fregs[i]-1]);
         k = SToff[fregs[i]-1].sa[2];
         if (BitVecCheck(loop->header->ins, fregs[i]+TNREG-1))
            InsNewInst(loop->preheader, loop->preheader->instN, NULL, FLD,
                       -i-FREGBEG, k, 0);
         if (BitVecCheck(loop->outs, fregs[i]+TNREG-1) &&
             BitVecCheck(loop->sets, fregs[i]+TNREG-1))
            InsInstInBlockList(loop->posttails, 1, FST, k, -i-FREGBEG, 0);
      }
   }
   for (i=0; i < NDR; i++)
   {
      if (dregs[i] > 0)
      {
fprintf(stderr, "dhoisting/pushing %d, %s\n", dregs[i], STname[dregs[i]-1]);
         k = SToff[dregs[i]-1].sa[2];
         if (BitVecCheck(loop->header->ins, dregs[i]+TNREG-1))
            InsNewInst(loop->preheader, loop->preheader->instN, NULL, FLDD,
                       -i-DREGBEG, k, 0);
         if (BitVecCheck(loop->outs, dregs[i]+TNREG-1) &&
             BitVecCheck(loop->sets, dregs[i]+TNREG-1))
            InsInstInBlockList(loop->posttails, 1, FSTD, k, -i-DREGBEG, 0);
      }
   }
   PrintComment(loop->preheader, NULL, NULL, 
                "DONE  global asg preheader load");
   PrintComment(loop->posttails->blk, NULL, loop->posttails->blk->inst1, 
                "START global asg sunk stores");
   CFUSETU2D = INUSETU2D = INDEADU2D = 0;
}
