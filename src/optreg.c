#include <fko.h>
#include <fko_arch.h>

static IGNODE **IG=NULL;
static int NIG=0, TNIG=0;
void DumpIG(FILE *fpout, int N, IGNODE **igs);

int ireg2type(int ireg)
{
   int iret = -1;
   if (ireg >= IREGBEG && ireg < IREGEND)
      iret = T_INT;
   else if (ireg >= FREGBEG && ireg < FREGEND)
      iret = T_FLOAT;
   else if (ireg >= DREGBEG && ireg < DREGEND)
      iret = T_DOUBLE;
#ifdef VFREGBEG
   else if (ireg >= VFREGBEG && ireg < VFREGEND)
      iret = T_VFLOAT;
#endif
#ifdef VDREGBEG
   else if (ireg >= VDREGBEG && ireg < VDREGEND)
      iret = T_VDOUBLE;
#endif
   return(iret);
}
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
      if (!strcmp(archiregs[i-IREGBEG], regname)) return(-i);
   for (i=FREGBEG; i < FREGEND; i++)
      if (!strcmp(archfregs[i-FREGBEG], regname)) return(-i);
   for (i=DREGBEG; i < DREGEND; i++)
      if (!strcmp(archdregs[i-DREGBEG], regname)) return(-i);
#ifdef VFREGBEG
   for (i=VFREGBEG; i < VFREGEND; i++)
      if (!strcmp(archvfregs[i-VFREGBEG], regname)) return(-i);
#endif
#ifdef VDREGBEG
   for (i=VDREGBEG; i < VDREGEND; i++)
      if (!strcmp(archvdregs[i-VDREGBEG], regname)) return(-i);
#endif
   for (i=ICC0; i < NICC; i++)
      if (!strcmp(ICCREGS[i], regname)) return(-i);
   for (i=FCC0; i < NFCC; i++)
      if (!strcmp(FCCREGS[i], regname)) return(-i);
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
#ifdef VFREGBEG
   else if (i >= VFREGBEG && i < VFREGEND)
      sprintf(ln, "%s", archvfregs[i-VFREGBEG]);
#endif
#ifdef VDREGBEG
   else if (i >= VDREGBEG && i < VDREGEND)
      sprintf(ln, "%s", archvdregs[i-VDREGBEG]);
#endif
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
   case T_VFLOAT:
      ibeg = VFREGBEG;
      iend = VFREGEND;
      break;
   case T_DOUBLE:
      ibeg = DREGBEG;
      iend = DREGEND;
      break;
   case T_VDOUBLE:
      ibeg = VDREGBEG;
      iend = VDREGEND;
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

//fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   if (k <= 0)
   {
      if (iv) KillBitVec(iv);
      iv = 0;
   }
   else
   {
      if (!iv) iv = NewBitVec(TNREG);
      SetVecAll(iv, 0);
      SetVecBit(iv, k-1, 1);
      if (k >= FREGBEG && k < FREGEND)
      {
         #if defined(X86) || defined(PPC)
            SetVecBit(iv, k-FREGBEG+DREGBEG-1, 1);
            #ifdef X86
               SetVecBit(iv, k-FREGBEG+VDREGBEG-1, 1);
               SetVecBit(iv, k-FREGBEG+VFREGBEG-1, 1);
            #endif
         #elif defined(SPARC)
            SetVecBit(iv, ((k-FREGBEG)>>1)+DREGBEG-1, 1);
         #endif
      }
      else if (k >= DREGBEG && k < DREGEND)
      {
         #if defined(X86) || defined(PPC)
            SetVecBit(iv, k-DREGBEG+FREGBEG-1, 1);
            #ifdef X86
               SetVecBit(iv, k-DREGBEG+VDREGBEG-1, 1);
               SetVecBit(iv, k-DREGBEG+VFREGBEG-1, 1);
            #endif
         #elif defined(SPARC)
            i = k - DREGBEG;
            if (i < 16)
            {
               SetVecBit(iv, FREGBEG+i*2-1, 1);
               SetVecBit(iv, FREGBEG+i*2, 1);
            }
         #endif
      }
   #ifdef X86
      else if (k >= VFREGBEG && k < VFREGEND)
      {
         SetVecBit(iv, k-VFREGBEG+VDREGBEG-1, 1);
         SetVecBit(iv, k-VFREGBEG+FREGBEG-1, 1);
         SetVecBit(iv, k-VFREGBEG+DREGBEG-1, 1);
      }
      else if (k >= VDREGBEG && k < VDREGEND)
      {
         SetVecBit(iv, k-VDREGBEG+VFREGBEG-1, 1);
         SetVecBit(iv, k-VDREGBEG+FREGBEG-1, 1);
         SetVecBit(iv, k-VDREGBEG+DREGBEG-1, 1);
      }
   #endif
   }
#if 0
   else if (!(k >= IREGBEG && k < IREGEND))
      fko_error(__LINE__, "Unknown register index %d, file=%s\n",
                  k, __FILE__);
#endif
// fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   return(iv);
}

int FindLiveregs(INSTQ *here)
/*
 * Finds the livereg up to instruction here (here itself is not included
 * in the analysis)
 * RETURNS: bitvector with all live regs set
 */
{
   INSTQ *ip;
   int i, j, k, iv;
   static int liveregs=0, mask, vtmp;

   if (here)
   {
      if (!liveregs)
      {
         liveregs = NewBitVec(TNREG);
         mask     = NewBitVec(TNREG);
         vtmp     = NewBitVec(TNREG);
/*
 *       Set only those regs we're interested in
 */
         SetVecAll(mask, 0);
         SetAllTypeReg(mask, T_INT);
         SetAllTypeReg(mask, T_FLOAT);
         SetAllTypeReg(mask, T_DOUBLE);
         SetAllTypeReg(mask, T_VFLOAT);
         SetAllTypeReg(mask, T_VDOUBLE);
         SetVecBit(mask, REG_SP-1, 0);
         #ifdef X86_32
            SetVecBit(mask, Reg2Int("@st")-1, 0);
         #endif
      }
      else
         SetVecAll(liveregs, 0);

/*
 *    Add all regs live on block entry to liveregs by adding regs from ins to lr
 */
      BitVecComb(vtmp, here->myblk->ins, mask, '&');
      for (i=1; k = GetSetBitX(vtmp, i); i++)
         BitVecComb(liveregs, liveregs, Reg2Regstate(k), '|');
      for (ip=here->myblk->inst1; ip != here; ip = ip->next)
      {
/*
 *       Remove all dead regs from livereg
 */
         if (ip->deads)
         {
            BitVecComb(vtmp, ip->deads, mask, '&');
            for (i=1; k = GetSetBitX(vtmp, i); i++)
               BitVecComb(liveregs, liveregs, Reg2Regstate(k), '-');
         }
/*
 *       If a register is set, add it to liveregs
 */
         if (ip->set)
         {
            BitVecComb(vtmp, ip->set, mask, '&');
            for (i=1; k = GetSetBitX(vtmp, i); i++)
               BitVecComb(liveregs, liveregs, Reg2Regstate(k), '|');
         }
      }
   }
   else
   {
      if (liveregs)
      {
         KillBitVec(liveregs);
         KillBitVec(mask);
         KillBitVec(vtmp);
      }
      vtmp = mask = liveregs = 0;
   }
   return(liveregs);
}

int GetRegForAsg(int type, int iv, int livereg)
{
   SetVecAll(iv, 0);
   SetAllTypeReg(iv, type);
   BitVecComb(iv, iv, livereg, '-');
   SetVecBit(iv, REG_SP-1, 0);
/*
 * Don't use [f,d]retreg (%st) for x86-32
 */
   #ifdef X86_32
      SetVecBit(iv, FRETREG-1, 0);
      SetVecBit(iv, DRETREG-1, 0);
   #endif
   return(AnyBitsSet(iv));
}

int GetRegFindLR(int type, int iv, INSTQ *here)
{
   return(GetRegForAsg(type, iv, FindLiveregs(here)));
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
      if (IG[i])
      {
         KillIGNode(IG[i]);
         IG[i] = NULL;
      }
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
         assert(IS_VAR(STflag[var-1])); 
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
   short inst;
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
      inst = GET_INST(ip->inst[0]);
      if (!ACTIVE_INST(inst)) continue;
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
            node = myIG[j];
            myIG[j] = NULL;
            #if IFKO_DEBUG_LEVEL >= 1
               assert(!node->blkend);
            #endif
            node->blkend = AddBlockToList(node->blkend, bp);
            node->blkend->ptr = ip;
         }
/*
 *       If it is a register that's dead, delete it from liveregs
 * HERE HERE NOTE: shouldn't we use Reg2RegState here?
 */
         else
            BitVecComb(liveregs, liveregs, Reg2Regstate(k+1), '-');
/*            SetVecBit(liveregs, k, 0); */
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
/*
 * If this block neither begins or ends LR, it is spanned
 */
   for (i=0; i < nn; i++)
   {
      node = myIG[i];
      if (node && !node->blkbeg && !node->blkend)
         node->blkspan = AddBlockToList(node->blkspan, bp);
   }
   if (myIG) free(myIG);
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

   assert(pnode->var == snode->var);
   pnode->blkbeg  = MergeBlockLists(pnode->blkbeg, snode->blkbeg);
   pnode->blkend  = MergeBlockLists(snode->blkend, pnode->blkend);
   pnode->blkspan = MergeBlockLists(pnode->blkspan, snode->blkspan);
   snode->blkbeg = snode->blkend = snode->blkspan = NULL;
/*
 * If succ block is not ending block, it becomes a spanned block
 */
   if ( !FindBlockInList(pnode->blkend, succ) && 
        !FindBlockInList(pnode->blkspan, succ) )
      pnode->blkspan = AddBlockToList(pnode->blkspan, succ);
/*
 * The range ending in succ block may change due to new beginning in pred
 */
#if 0
   bl = FindInList(pnode->blkend, succ);
   if (bl)
   {
      i = pnode->var + TNREG-1;
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (BitVecCheck(ip->deads, i))
         {
            bl->ptr = ip;
            break;
         }
      }
   }
#endif
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
   short *sig, *pig, svar;
   short k, kk;
/*
 * If both blocks are in scope, attempt to combine their live ranges
 */
/*
fprintf(stderr, "scoping pred=%d, succ=%d\n", pred->bnum, succ->bnum);
fprintf(stderr, "pred->conout=%s\n", PrintVecList(pred->conout, 0));
fprintf(stderr, "succ->conin=%s\n", PrintVecList(pred->conin, 0));
*/
   if (BitVecCheck(scopeblks, succ->bnum-1) && 
       BitVecCheck(scopeblks, pred->bnum-1) )
   {
      pig = BitVec2Array(pred->conout, 0);
      sig = BitVec2Array(succ->conin, 0);
      for (n=sig[0], i=1; i <= n; i++)
      {
         k = sig[i];
         if (!IG[k]) continue;
         svar = IG[k]->var;
         for (nn=pig[0], j=1; j <= nn; j++)
         {
            kk = pig[j];
/*
 *          HERE, HERE: put this in to protect when sig is joined to pig
 */
            if (!IG[kk] || !IG[k]) continue;
            if (k != kk && svar == IG[kk]->var)
               CombineLiveRanges(scope, pred, kk, succ, k);
         }
      }
      free(pig);
      free(sig);
   }
/*
 * If pred in scope, but successor not, and LR includes a store, must push LR
 * stores to succ. iff the variable is live on entering succ.
 * Assign the next inst of the pushed stores to node->stpush->ptr.
 */
   else if (BitVecCheck(scopeblks, pred->bnum-1))
   {
      pig = BitVec2StaticArray(pred->conout);
      for (n=pig[0], i=1; i <= n; i++)
      {
         node = IG[pig[i]];
         if (node->nwrite && BitVecCheck(succ->ins, node->var+TNREG-1) )
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

int CheckIG(int N, IGNODE **igs)
{
   int i, j, n;
   short *sp;
   IGNODE *ig, *ig2;
   int nerr=0;

   for (i=0; i < N; i++)
   {
      if (igs[i])
      {
         ig = igs[i];
/*
 *       Make sure register is not assigned to conflicting node
 */
         if (ig->reg)
         {
            sp = BitVec2StaticArray(ig->conflicts);
            if (sp)
            {
               for (j=1, n=sp[0]; j <= n; j++)
               {
                  ig2 = IG[sp[j]];
                  if (ig2->reg == ig->reg)
                  {
                     fprintf(stderr, 
                        "Conflicting IG %d and %d both use register %d!!\n",
                             ig->ignum, ig2->ignum, ig->reg);
                     nerr++;
                  }
               }
            }
         }
      }
      else
         fprintf(stderr, "NULL IG %d\n", i);
   }
   assert(nerr == 0);
   return(nerr);
}

int Scope2BV(BLIST *scope)
{
   static int iv=0;
   BLIST *bl;

   if (scope)
   {
      if (!iv)
         iv = NewBitVec(32);
      SetVecAll(iv, 0);
      for (bl = scope; bl; bl = bl->next)
         SetVecBit(iv, bl->blk->bnum-1, 1);
   }
   else if (iv)
   {
      KillBitVec(iv);
      iv = 0;
   }
   return(iv);
}

int CalcScopeIG(BLIST *scope)
/*
 * RETURNS: total number of IG
 */
{
   static ushort blkvec=0;
   BLIST *bl, *lp;
   short *sp;
   int i, j;

   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);

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
/*   DumpIG(stderr, NIG, IG); */
/*
 * Try to combine live ranges across basic blocks
 */
   for (bl=scope; bl; bl = bl->next)
   {
      if (bl->blk->usucc)
         CombineBlockIG(scope, blkvec, bl->blk, bl->blk->usucc);
      if (bl->blk->csucc)
         CombineBlockIG(scope, blkvec, bl->blk, bl->blk->csucc);
      for (lp=bl->blk->preds; lp; lp = lp->next)
         if (!BitVecCheck(blkvec, lp->blk->bnum-1) &&
             lp->blk != bl->blk->usucc && lp->blk != bl->blk->csucc)
            CombineBlockIG(scope, blkvec, lp->blk, bl->blk);
   }
   for (j=i=0; i < NIG; i++)
      if (IG[i]) j++;
   return(j);
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

void KillUnusedIG(int N, IGNODE **igs)
/*
 * Kills all IGNODEs in IG that do not also appear in igs
 */
{
   IGNODE *node, *save;
   int n, i, j;
   BLIST *bl;
   short *sp;

   for (j=0; j < NIG; j++)
   {
      node = IG[j];
      if (node)
      {
         for (i=0; i < N && node != igs[i]; i++);
         if (i == N)
         {
/*
 *          Delete from block-carried IG
 */
            for (bl=node->blkbeg; bl; bl = bl->next)
            {
               SetVecBit(bl->blk->conin, node->ignum, 0);
               SetVecBit(bl->blk->conout, node->ignum, 0);
               SetVecBit(bl->blk->ignodes, node->ignum, 0);
            }
            for (bl=node->blkend; bl; bl = bl->next)
            {
               SetVecBit(bl->blk->conin, node->ignum, 0);
               SetVecBit(bl->blk->conout, node->ignum, 0);
               SetVecBit(bl->blk->ignodes, node->ignum, 0);
            }
            for (bl=node->blkspan; bl; bl = bl->next)
            {
               SetVecBit(bl->blk->conin, node->ignum, 0);
               SetVecBit(bl->blk->conout, node->ignum, 0);
               SetVecBit(bl->blk->ignodes, node->ignum, 0);
            }
/*
 *          Delete from conflicts
 */
            sp = BitVec2StaticArray(node->conflicts);
            for (n=sp[0], i=1; i <= n; i++)
               SetVecBit(IG[sp[i]]->conflicts, node->ignum, 0);
/*
 *          Free node space, and delete from array
 */
            KillIGNode(node);
            IG[j] = NULL;
         }
      }
   }
}

IGNODE **SortIG(int *N, int thresh)
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
      if (IG[i] && IG[i]->nwrite + IG[i]->nread >= thresh &&
          (!STname[IG[i]->var-1] || 
            strcmp(STname[IG[i]->var-1], "_NONLOCDEREF")))
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
      if (IG[i] && IG[i]->nwrite + IG[i]->nread >= thresh &&
          (!STname[IG[i]->var-1] || 
            strcmp(STname[IG[i]->var-1], "_NONLOCDEREF")))
         igarr[j++] = IG[i];
   KillUnusedIG(n, igarr);
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
   SortConstrainedIG(ncon, igarr);
   SortUnconstrainedIG(n-ncon, igarr+ncon);
   *N = n;
   return(igarr);
}

int DoIGRegAsg(int N, IGNODE **igs)
/*
 * Given an N-length array of sorted IGNODEs, perform register assignment
 * Right now, use simple algorithm for assignment, improve later.
 * RETURNS: # of IG assigned.
 */
{
   int i, j, n;
   int iret = 0;
   IGNODE *ig, *ig2;
   short *sp;
   short iv, ivused;
   extern int FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(TNREG);
   iv = FKO_BVTMP;

   for (i=0; i < N; i++)
   {
      ig = igs[i];
#if 1
      ig->reg = GetRegForAsg(FLAG2PTYPE(STflag[ig->var-1]), iv, ig->liveregs);
#else
      SetVecAll(iv, 0);
      SetAllTypeReg(iv, FLAG2PTYPE(STflag[ig->var-1]));
      BitVecComb(iv, iv, ig->liveregs, '-');
      SetVecBit(iv, REG_SP-1, 0);
/*
 *    Don't use [f,d]retreg (%sp) for x86-32
 */
      #ifdef X86_32
         SetVecBit(iv, FRETREG-1, 0);
         SetVecBit(iv, DRETREG-1, 0);
      #endif
/*
 *    If we have a register left, assign it
 */
      ig->reg = AnyBitsSet(iv);
#endif
      if (ig->reg)
      {
         ivused = Reg2Regstate(ig->reg);
         ig->reg--;
         BitVecComb(ig->liveregs, ig->liveregs, ivused, '|');
         assert(ig->reg+1 != REG_SP);
/*
 *       Add assigned register to liveregs of conflicting IGNODEs
 */
         sp = BitVec2StaticArray(ig->conflicts);
         if (sp)
         {
            for (j=1, n=sp[0]; j <= n; j++)
            {
               ig2 = IG[sp[j]];
               BitVecComb(ig2->liveregs, ig2->liveregs, ivused, '|');
            }
         }
         ig->reg++;
         iret++;
      }
      else
         fko_warn(__LINE__, "NO FREE REGISTER FOR LR %d!!!\n", ig->ignum);
   }
   return(iret);
}

int VarUse2RegUse(IGNODE *ig, BBLOCK *blk, INSTQ *instbeg, INSTQ *instend)
/*
 * Changes all uses of ig->var to ig->reg in block blk, between the indicated
 * instructions (inclusive).  If either is NULL, we take instbeg/end.
 */
{
   INSTQ *ip, *ip1;
   int CHANGE=0;
   short k;

   assert(INUSETU2D);
   assert(ig && blk);
   if (!instbeg)
      instbeg = blk->ainst1;
   if (!instend)
      instend = blk->ainstN;
   if (!instbeg || !ig->reg)
      return(0);
   assert(instend);
   do
   {
      ip = instbeg;
/*
 *    Change all uses of this ig->var to ig->reg
 */
      if (ACTIVE_INST(ip->inst[0]) && BitVecCheck(ip->use, ig->var+TNREG-1))
      {
         SetVecBit(ip->use, ig->var+TNREG-1, 0);
         SetVecBit(ip->use, ig->reg-1, 0);
         assert(ip->inst[2] == ig->deref || ip->inst[3] == ig->deref);
         if (ip->inst[2] == ig->var || ip->inst[2] == ig->deref)
            ip->inst[2] = -ig->reg;
         if (ip->inst[3] == ig->var || ip->inst[3] == ig->deref)
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
         case VDLD:
            ip->inst[0] = VDMOV;
            break;
         case VDLDS:
            #ifdef X86
               k = ig->reg;
               if (k >= DREGBEG && k < DREGEND)
                  k = k - DREGBEG + VDREGBEG;
               if (k != -ip->inst[1])
                  CalcThisUseSet(InsNewInst(NULL, NULL, ip, VDZERO, 
                                    ip->inst[1], 0, 0));
            #endif
            ip->inst[0] = VDMOVS;
            ip->inst[2] = -ig->reg;
            break;
         case VFLDS:
            #ifdef X86
               k = ig->reg;
               if (k >= FREGBEG && k < FREGEND)
                  k = k - FREGBEG + VFREGBEG;
               if (k != -ip->inst[1])
                  CalcThisUseSet(InsNewInst(NULL, NULL, ip, VFZERO, 
                                    ip->inst[1], 0, 0));
            #endif
            ip->inst[0] = VFMOVS;
            ip->inst[2] = -ig->reg;
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
   enum inst mov, ld, st, inst;
   int i, CHANGE=0;

   if (!ig->reg)  /* return if their was no available register */
      return(0);
   i = STflag[ig->var-1];
   switch(FLAG2PTYPE(i))
   {
#ifdef X86_64
   case T_SHORT:
      mov = MOVS;
      ld  = LDS;
      st  = STS;
      break;
#endif
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
   case T_VDOUBLE:
      mov = VDMOV;
      ld  = VDLD;
      st  = VDST;
      break;
   case T_VFLOAT:
      mov = VFMOV;
      ld  = VFLD;
      st  = VFST;
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
      ip = bl->ptr;
      if (ip && BitVecCheck(ip->set, ig->var+TNREG-1))
      {
         inst = GET_INST(ip->inst[0]);
         #if IFKO_DEBUG_LEVEL >= 1
            assert(bl->ptr);
         #endif
         CHANGE++;
         if (!IS_STORE(inst))
         {
            PrintThisInst(stderr, __LINE__, ip);
            exit(-1);
         }
/*
 *       If Store is of same type as variable, we just change to reg-reg move
 */
         if (inst == st)
         {
            ip->inst[0] = mov;
            ip->inst[1] = -ig->reg;
         }
/*
 *       If Store is of different type than variable (eg., fpconst init),
 *       we must insert a ld inst to the LR register
 */
         else
         {
            ip = PrintComment(bl->blk, ip, NULL, "Inserted LD from %s",
                              STname[ig->var-1] ? STname[ig->var-1] : "NULL");
            #if IFKO_DEBUG_LEVEL > 1
               fprintf(stderr, "ireg=%s getting LD ip=%d, nr=%d, nw=%d!\n\n", 
                       STname[ig->var-1], ip, ig->nread, ig->nwrite);
            #endif
            ip = InsNewInst(bl->blk, ip, NULL, ld, -ig->reg, ig->deref, 0);
            if (ip->next) bl->ptr = ip->next;
            else
               bl->ptr = InsNewInst(bl->blk, ip, NULL, COMMENT, 0, 0, 0);
         }
         CalcThisUseSet(ip);
      }
/*
 *    If LR does not begin with a set, we must add a load to ig->reg,
 *    and so we expect ldhoist to be set
 */
      else assert(0);
   }
/*
 * If this IG starts and ends in one block, with start before end, handle
 * seperately
 */
   if ( ig->blkbeg && !ig->blkbeg->next && ig->blkend && !ig->blkend->next &&
        ig->blkbeg->blk == ig->blkend->blk &&
        FindInstNum(ig->blkbeg->blk, ig->blkbeg->ptr) <
        FindInstNum(ig->blkend->blk, ig->blkend->ptr) )
   {
      assert(!ig->blkspan);
      CHANGE += VarUse2RegUse(ig, ig->blkbeg->blk, ig->blkbeg->ptr, 
                              ig->blkend->ptr);
   }
   else
   {
/*
 *    Change all use of var in LR's starting and ending blocks to access
 *    ig->reg instead
 */
      for (bl=ig->blkend; bl; bl = bl->next)
         CHANGE += VarUse2RegUse(ig, bl->blk, NULL, bl->ptr);
      for (bl=ig->blkbeg; bl; bl = bl->next)
      {
         ip = bl->ptr;
         if (ip)
            CHANGE += VarUse2RegUse(ig, bl->blk, ip, NULL);
      }
   }
/*
 * Hoist the ld to all extra-scope blocks that may need it
 */
   for (bl=ig->ldhoist; bl; bl = bl->next)
   {
      #if IFKO_DEBUG_LEVEL >= 1
         assert(bl->ptr);
         assert(ig->deref > 0);
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
      ip = InsNewInst(bl->blk, NULL, bl->ptr, st, ig->deref, -ig->reg, 0);
      CalcThisUseSet(ip);
      CHANGE++;
   }
/*
 * Now we change all use of var in LR span to access ig->reg instead
 */
   for (bl=ig->blkspan; bl; bl = bl->next)
      CHANGE += VarUse2RegUse(ig, bl->blk, NULL, NULL);
   if (CHANGE) 
      INDEADU2D = CFUSETU2D = 0;
   return(CHANGE);
}

int DoScopeRegAsg(BLIST *scope, int thresh, int *tnig)
/*
 * Performs interference graph-based register assignment on blocks listed in
 * scope.  tnig is output para giving total number of IG we found.
 * RETURNS: # of IG applied.
 */
{
   IGNODE **igs;
   int i, N, nret;
   extern FILE *fpIG, *fpLIL, *fpST;
   extern BBLOCK *bbbase;

   *tnig = CalcScopeIG(scope);
   igs = SortIG(&N, thresh);
   nret = DoIGRegAsg(N, igs);
CheckIG(N, igs);
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
   KillIGTable();
   return(nret);
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
short id;

/*
 * Find all variables set in list
 */
   if (!loop->sets) loop->sets = NewBitVec(TNREG+32);
   else SetVecAll(loop->sets, 0);
   for (bl=loop->blocks; bl; bl = bl->next)
   {
      for (ip=bl->blk->inst1; ip; ip = ip->next)
         if (ip->set)
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
   k = STstrlookup("_NONLOCDEREF");
   SetVecBit(iv, k+TNREG-1, 0);

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
#if 0
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
#endif
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
               BitVecComb(ip->use, ip->use, Reg2Regstate(-regs[i]), '|');
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
               BitVecComb(ip->set, ip->set, Reg2Regstate(-regs[i]), '|');
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

int DoLoopGlobalRegAssignment(LOOPQ *loop)
/* 
 * This routine does global register assignment for all locals referenced
 * in the loop, and live on loop entry or exit
 */
{
   short *sp;
   int i, j, n, iret;
   short k;
   short iregs[NIR], fregs[NFR], dregs[NDR]; 
   short *vars, *regs;

   FindInitRegUsage(loop->blocks, iregs, fregs, dregs);
#if 0
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
#endif
   assert(!AsgGlobalLoopVars(loop, iregs, fregs, dregs));
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
   iret = i = LoadStoreToMove(loop->blocks, n, vars, regs);
   free(regs);
/*fprintf(stderr, "Removed %d LD/ST using global register assignment!\n", i);*/
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
         k = SToff[iregs[i]-1].sa[2];
         if (BitVecCheck(loop->header->ins, iregs[i]+TNREG-1))
            CalcThisUseSet(InsNewInst(loop->preheader, NULL, NULL, LD, 
                                      -i-IREGBEG, k, 0));
         if (BitVecCheck(loop->outs, iregs[i]+TNREG-1) && 
             BitVecCheck(loop->sets, iregs[i]+TNREG-1))
            InsInstInBlockList(loop->posttails, 1, ST, k, -i-IREGBEG, 0);
      }
   }
   for (i=0; i < NFR; i++)
   {
      if (fregs[i] > 0)
      {
         k = SToff[fregs[i]-1].sa[2];
         if (BitVecCheck(loop->header->ins, fregs[i]+TNREG-1))
            CalcThisUseSet(InsNewInst(loop->preheader, loop->preheader->instN,
                                      NULL, FLD, -i-FREGBEG, k, 0));
         if (BitVecCheck(loop->outs, fregs[i]+TNREG-1) &&
             BitVecCheck(loop->sets, fregs[i]+TNREG-1))
            InsInstInBlockList(loop->posttails, 1, FST, k, -i-FREGBEG, 0);
      }
   }
   for (i=0; i < NDR; i++)
   {
      if (dregs[i] > 0)
      {
         k = SToff[dregs[i]-1].sa[2];
         if (BitVecCheck(loop->header->ins, dregs[i]+TNREG-1))
            CalcThisUseSet(InsNewInst(loop->preheader, loop->preheader->instN,
                                      NULL, FLDD, -i-DREGBEG, k, 0));
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
   return(iret);
}

static INSTQ *FindReg2RegMove(BBLOCK *bp, INSTQ *start, INSTQ *end)
/*
 * Finds first register-to-register move in bp, between [start,end]
 * if start == NULL, start at beginning of block, if end == NULL go to end
 */
{
   INSTQ *ip;
   enum inst inst;
   if (!bp) return(NULL);
   if (!start) start = bp->ainst1;
   if (!start) return(NULL);
   if (!end) end = bp->ainstN;
   do
   {
      ip = start;
      inst = GET_INST(ip->inst[0]);
      if (IS_MOVE(inst) && ip->inst[1] < 0 && ip->inst[2] < 0)
         return(ip);
      start = start->next;
   }
   while(start && ip != end);
   return(NULL);
}

int Dist2RefInBlock(BBLOCK *bp, int var, INSTQ *start, INSTQ *end)
/*
 * Finds number of instructions between start and next ref of var
 * if start == NULL, start at beginning of block, if end == NULL go to end
 * RETURNS:  # of inst from start otherwise (starting from 1), the negative
 *           # of inst in block from start if not found in block
 */
{
   INSTQ *ip;
   enum inst inst;
   int i=1;

   if (!bp) return(0);
   if (!start) start = bp->ainst1;
   if (!start) return(0);
   if (!end) end = bp->ainstN;
   do
   {
      ip = start;
      inst = GET_INST(ip->inst[0]);
      if (ACTIVE_INST(inst) && inst != LABEL && inst != NOP && inst != UNIMP)
      {
         i++;
         if (BitVecCheck(ip->use, var) || BitVecCheck(ip->set, var))
            return(i);
      }
      start = start->next;
   }
   while(start && ip != end);
   return(-i);
}

int Dist2Ref(BBLOCK *bp, int scopeblks, int var, INSTQ *start, INSTQ *end)
{
   int i, j=0, k=0;
   i = Dist2RefInBlock(bp, var, start, end);
   if (i < 0 && BitVecCheck(bp->outs, var))
   {
      if (bp->usucc && BitVecCheck(bp->usucc->ins, var) &&
          BitVecCheck(scopeblks, bp->usucc->bnum-1))
            j = Dist2Ref(bp->usucc, scopeblks, var, NULL, NULL);
      if (bp->csucc && BitVecCheck(bp->csucc->ins, var) &&
          BitVecCheck(scopeblks, bp->csucc->bnum-1))
         k = Dist2Ref(bp->csucc, scopeblks,var, NULL, NULL);
      if (k > j) j = k;
      i += j;
   }
   else if (i < 0) 
      i = -i;
   return(i);
}

int SubRegUse(short *op, short reg, short sub)
/*
 * Finds if register reg is used in instruction operand op; if it is, replaces
 * its use with a use of sub.
 * RETURNS: 1 if register was found, 0 otherwise.
 */
{
   int found=0;
   short op1 = *op;

   if (op1 == reg)
   {
      *op = sub;
      found = 1;
   }
   else if (op1 > 0 && IS_DEREF(STflag[op1-1]))
   {
      if (SToff[op1-1].sa[0] == reg)
      {
         found = 1;
         SToff[op1-1].sa[0] = sub;
      }
      if (SToff[op1-1].sa[1] == reg)
      {
         found = 1;
         SToff[op1-1].sa[1] = sub;
      }
   }
   return(found);
}

static int SuccIsCopyPropTarg(BBLOCK *ob, /* origin block */
                              BBLOCK *sb, /* successor block */
                              short dest, /* reg we are replacing */
                              short src)  /* reg we are CopyProping */
/*
 * Returns: 0 if successor sb rules out cross-block copy prop, 1 otherwise
 */
{
   int iret = 1;
   BLIST *bl;

return(0);
   if (sb)
   {
/*
 *    If succ does not have dest live on entry, CopyProp is not ruled out
 */
      if (BitVecCheck(sb->ins, dest-1))
      {
/*
 *       If succ has source live on entry, CopyProp is ruled out
 */
         iret = !BitVecCheck(sb->ins, src-1);
         if (iret)
         {
/*
 *          If any pred aside from ob has dest live on exit, rule it out
 */
            for (bl=sb->preds; bl; bl = bl->next)
               if (BitVecCheck(bl->blk->outs, dest-1) && bl->blk != ob)
                  break;
            iret = bl ? 0 : 1;
         }
      }
   }
   return(iret);
}

int CopyPropTrans0(int SRCLIVE, BLIST *scope, int scopeblks, BBLOCK *blk,
                   INSTQ *ipstart, short mov, short dest, short src)
/*
 * Performs copy prop starting from ipret, where src reg is dead
 * When src is dead, we can replace all use and set of dest with src, until
 * dest is dead or src is live again.
 */
{
   INSTQ *ip;
   int ivsrc, ivdst, FoundIt, i, j, LIVEDONE=0;
   int change=0;
   BLIST *bl;
   extern int FKO_BVTMP;
   extern BBLOCK *bbbase;

   bl = FindInList(scope, blk);
   if (bl->ptr)
      change = 1;
#if 0
fprintf(stderr, "blk=%d, SRCLIVE=%d, dest=%s", blk->bnum, SRCLIVE, Int2Reg(-dest));
fprintf(stderr, ", src=%s\n", Int2Reg(-src));
#endif
   ivdst = Reg2Regstate(src);
   ivdst = FKO_BVTMP = BitVecCopy(FKO_BVTMP, Reg2Regstate(dest));
   ivsrc = Reg2Regstate(src);
   for (ip=ipstart?ipstart->next:blk->ainst1; ip; ip = ip->next)
   {
      j = GET_INST(ip->inst[0]);
      if (!ACTIVE_INST(j))
         continue;
/*
 *    If src becomes live again, put move back, and stop copy prop, unless
 *    src becomes live due to a move from dest
 */
      if (BitVecCheckComb(ip->set, ivsrc, '&') &&
          (j != mov || ip->inst[1] != -src || ip->inst[2] != -dest))
         goto PUTMOVEBACK;
/*
 *    Stop copy prop if we see idiv on x86
 */
      #ifdef X86
         if((j == DIV || j == UDIV) && 
            (BitVecCheckComb(ip->set, ivdst, '&') ||
             BitVecCheckComb(ip->use, ivdst, '&') ||
             BitVecCheckComb(ip->use, ivsrc, '&')))
            goto PUTMOVEBACK;
      #endif
/*
 *    If we have a set of dest, change it to a use of src, and keep going with
 *    propogation if this dest updated itself
 */
      if (BitVecCheck(ip->set, dest-1))
      {
/*
 *       If the src is live, a set of dest ends the CopyProp, but not until
 *       after we have changed this inst use of dest to src
 */
         if (SRCLIVE)
         {
            #ifdef X86
               if (ip->inst[2] == -dest)
                  goto PUTMOVEBACK;
            #endif
               if (BitVecCheck(ip->use, dest-1))
                  LIVEDONE = 1;
               else
                  goto PUTMOVEBACK;
         }
         else if (ip->inst[1] == -dest)
         {
            ip->inst[1] = -src;
            BitVecComb(ip->set, ip->set, ivdst, '-');
            BitVecComb(ip->set, ip->set, ivsrc, '|');
            if (BitVecCheck(ip->deads, dest-1))
            {
               BitVecComb(ip->deads, ip->deads, ivdst, '-');
               BitVecComb(ip->deads, ip->deads, ivsrc, '|');
            }
            change++;
          }
          else  /* stop copy prop on implicit set */
             goto PUTMOVEBACK;
      }
/*
 *    If we have a use of former dest, change it to a use of src
 */
      if (BitVecCheck(ip->use, dest-1))
      {
         FoundIt = 0;
         if (ip->inst[0] != RET)
         {
            FoundIt = SubRegUse(&ip->inst[2], -dest, -src);
            FoundIt |= SubRegUse(&ip->inst[3], -dest, -src);
            if (FoundIt)
            {
               BitVecComb(ip->use, ip->use, ivdst, '-');
               BitVecComb(ip->use, ip->use, ivsrc, '|');
               change++;
               if (LIVEDONE)
               {
                  ip = ip->next;
                  INDEADU2D = 0;
                  return(change);
               }
            }
            else /* not found, is implicit use, put move back & stop */
               goto PUTMOVEBACK;
         }
         else  /* for implicit use like return, put move back & stop */
            goto PUTMOVEBACK;
      }
/*
 *    If dest is dead without updating itself, end copy prop
 */
      if (BitVecCheck(ip->deads, dest-1))
      {
         BitVecComb(ip->deads, ip->deads, ivdst, '-');
         BitVecComb(ip->deads, ip->deads, ivsrc, '|');
         INDEADU2D = 0;
         return(change);
      }
/*
 *    If src is live and we see that it dies, change SRCLIVE
 *    NOTE: a dead src by a set is handled as 1st case in loop
 */
      if (SRCLIVE && BitVecCheck(ip->deads, src-1))
         SRCLIVE = 0;
   }
/*
 * If dest was still live on exit, see if cross-block prop is possible
 */
   if ((blk->csucc || blk->usucc) && BitVecCheck(blk->outs, dest-1))
   {
      if (SuccIsCopyPropTarg(blk, blk->usucc, dest, src) &&
          SuccIsCopyPropTarg(blk, blk->csucc, dest, src))
      {
         change++;
         if (blk->usucc && BitVecCheck(blk->usucc->ins, dest-1))
         {
            bl = FindInList(scope, blk->usucc);
            if (!bl->ptr)
            {
               bl->ptr = (void*) 1;
               change += CopyPropTrans0(SRCLIVE, scope, scopeblks, blk->usucc,
                                        NULL, mov, dest, src);
               BitVecComb(blk->usucc->ins, blk->usucc->ins, ivdst, '-');
               BitVecComb(blk->usucc->ins, blk->usucc->ins, ivsrc, '|');
            }
         }
         if (blk->csucc && BitVecCheck(blk->csucc->ins, dest-1))
         {
            bl = FindInList(scope, blk->csucc);
            if (!bl->ptr)
            {
               bl->ptr = (void*) 1;
               change += CopyPropTrans0(SRCLIVE, scope, scopeblks, blk->csucc,
                                        NULL, mov, dest, src);
               BitVecComb(blk->csucc->ins, blk->csucc->ins, ivdst, '-');
               BitVecComb(blk->csucc->ins, blk->csucc->ins, ivsrc, '|');
            }
         }
         CFUSETU2D = 0;
         BitVecComb(blk->outs, blk->outs, ivdst, '-');
         BitVecComb(blk->outs, blk->outs, ivsrc, '|');
      }
      else
      {
         if (IS_BRANCH(blk->ainstN->inst[0]))
            ip = blk->ainstN;
         else 
            ip = NULL;
         goto PUTMOVEBACK;
      }
   }
   if (change) INDEADU2D = 0;
   return(change);

PUTMOVEBACK:
   if (change)
   {
      ip = InsNewInst(blk, NULL, ip, mov, -dest, -src, 0);
      CalcThisUseSet(ip);
      assert(ip->set > 0 && ip->use > 0);
      INDEADU2D = 0;
   }
   return(change);
}

INSTQ *CopyPropTrans(BLIST *scope, int scopeblks, BBLOCK *blk, INSTQ *ipret)
/*
 * Attempts to do copy prop in block blk starting at ipret
 * RETURNS: instruction following ipret after transforms or NULL if no
 *          transform is done.
 */
{
   INSTQ *ip;
   char *sp;
   int i, iv;
   int change;
   short dest, src, mov;

   dest = -ipret->inst[1];
   src = -ipret->inst[2];
/*
 * If it's a useless move, simply delete it
 */
   if (dest == src)
   {
      INDEADU2D = 0;
      return(DelInst(ipret));
   }
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);

   mov = ipret->inst[0];
/*
 * Do not do copy prop if src or dest is x87 reg
 */
   #ifdef X86_320000
      sp = Int2Reg(-src);
      if (sp[1] == 's' && sp[2] == 't')
         return(NULL);
      sp = Int2Reg(-dest);
      if (sp[1] == 's' && sp[2] == 't')
         return(NULL);
   #endif
/*
 * Do not do copy prop if src and dest are not registers of the same type
 */
   if (ireg2type(src) != ireg2type(dest))
      return(NULL);

   if (BitVecCheck(ipret->deads, src-1))
      change = CopyPropTrans0(0, scope, scopeblks, blk, ipret, mov, dest, src);
   else 
      change = CopyPropTrans0(1, scope, scopeblks, blk, ipret, mov, dest, src);
   if (change)
      ipret = DelInst(ipret);
   else
      ipret = NULL;
   return(ipret);
}

int DoCopyProp(BLIST *scope)
/*
 * Performs copy propogation on scope.
 */
{
   int scopeblks, CHANGE=0;
   INSTQ *ip=NULL, *next;
   BLIST *bl, *epil=NULL, *lp;

   scopeblks = Scope2BV(scope);
   for (bl=scope; bl; bl = bl->next)
   {
      if (bl != epil)
      {
         do
         {
            ip = FindReg2RegMove(bl->blk, ip, NULL);
            if (ip) 
            {
               next = CopyPropTrans(scope, scopeblks, bl->blk, ip);
               for (lp=bl; lp; lp = lp->next)
                  lp->ptr = NULL;
               if (next)
               {
                  ip = next;
                  CHANGE++;
               }
               else ip = ip->next;
            }
         }
         while(ip);
      }
   }
/*   fprintf(stderr, "\nCopyProp CHANGE=%d\n", CHANGE); */
   return(CHANGE);
}

int DoRevCopyPropTrans(INSTQ *ipsrc,  /* inst where src is set */
                       INSTQ *ipdst,  /* inst where dest is set */
                       int nuse)      /* # of intervening uses of src */
/*
 * Actually perform Reverse Copy Prop transform on [ipsrc,ipdst] region of blk
 * Changes all ref of src to dest
 */
{
   INSTQ *ip;
   int i, nseen=(-1);
   short src, dest, op;

   assert(ipsrc && ipdst)
   src = -ipdst->inst[2];
   dest = -ipdst->inst[1];
   for (ip = ipsrc; ip != ipdst && nseen < nuse; ip = ip->next)
   {
      if (ip->use && (BitVecCheck(ip->use, src-1)||BitVecCheck(ip->set,src-1)))
      {
         for (i=1; i < 4; i++)
         {
            op = ip->inst[i];
            if (op == -src)
               ip->inst[i] = -dest;
            else if (op > 0 && IS_DEREF(STflag[op-1]))
            {
               if (SToff[op-1].sa[0] == -src)
                  SToff[op-1].sa[0] = -dest;
               if (SToff[op-1].sa[1] == -src)
                  SToff[op-1].sa[1] = -dest;
            }
         }
         CalcThisUseSet(ip);
         nseen++;
      }
//      ipdst->inst[2] = -dest;
//      CalcThisUseSet(ipdst);
      DelInst(ipdst);
   }
   return(1);
}

int DoReverseCopyProp(BLIST *scope)
/*
 * This crippled version does only in-block rcp
 */
{
   BLIST *bl;
   INSTQ *ip, *ipp, *ips;
   int nchanges=0, nuse;
   enum inst inst;
   short src, dest;

   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);

   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainstN; ip; ip = ipp)
      {
         ipp = ip->prev;
         inst = ip->inst[0];
         if (IS_MOVE(inst) && ip->inst[1] < 0 && ip->inst[2] < 0)
         {
/*
 *          If src reg is dead on reg2reg move, possible reverseCP candidate
 *          NOTE: leave src==dest moves for CP to clean up
 */
             src = -ip->inst[2];
             dest = -ip->inst[1];
             if (BitVecCheck(ip->deads, src-1) && src != dest &&
                 ireg2type(src) == ireg2type(dest) && src != REG_SP)
             {
/*
 *              Look for inst that set src
 */
                nuse = 0;
                for (ips=ip->prev; ips; ips = ips->next)
                {
                   if (!ips->set)
                      continue;
                   if (BitVecCheck(ips->set, src-1) && 
                       !BitVecCheck(ips->use, src-1))
                      break;
                   if (BitVecCheck(ips->use, src-1))
                      nuse++;
                   if (BitVecCheck(ips->use, dest-1) ||
                       BitVecCheck(ips->set, dest-1))
                   {
                      ips = NULL;
                      break;
                   }
                }
                if (ips && ips->inst[1] == -src)
                   nchanges += DoRevCopyPropTrans(ips, ip, nuse);
             }
         }
      }
   }
   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
//   fprintf(stderr, "RCP: nchanges=%d\n", nchanges);
   return(nchanges);
}
int DoEnforceLoadStore(BLIST *scope)
/*
 * transforms all instructions that directly access memory to LD/ST followed
 * by inst operating on registers
 * NOTE: presently can't do this, because then reg asg deletes store of
 *       absval to mem, which is needed for cleanup; you wind up with
 *       illegal reg move (from VFREG to FREG)
 */
{
   BLIST *bl;
   INSTQ *ip, *ipN;
   enum inst inst;
   int j, ir;
   int nchanges=0;
   short op;
   extern int FKO_BVTMP, DTabs, DTabsd, DTnzero, DTnzerod;
   extern int DTabss, DTabsds, DTnzeros, DTnzerods;

   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(TNREG);
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);

   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         inst = ip->inst[0];
         if (ACTIVE_INST(inst) && !IS_LOAD(inst) && !IS_STORE(inst) &&
             inst != LABEL)
         {
            op = ip->inst[3];
            if (op < 1)
               op = 0;
            else if (!IS_DEREF(STflag[op-1]))
               op = 0;
            #ifdef X86
               if (ip->inst[3] >= 0)
               {
               if (inst == FABS)
                  ip->inst[3] = op = SToff[DTabss-1].sa[2];
               else if (inst == VFABS)
                  ip->inst[3] = op = SToff[DTabs-1].sa[2];
               else if (inst == FABSD)
                  ip->inst[3] = op = SToff[DTabsds-1].sa[2];
               else if (inst == VDABS)
                  ip->inst[3] = op = SToff[DTabsd-1].sa[2];
               else if (inst == FNEG)
                  ip->inst[3] = op = SToff[DTnzeros-1].sa[2];
               else if (inst == FNEGD)
                  ip->inst[3] = op = SToff[DTnzerods-1].sa[2];
               }
            #endif
            if (!op)
            {
               op = ip->inst[2];
               if (!IS_DEREF(STflag[op-1]))
                  op = 0;
            }
            if (inst == PREFR || inst == PREFW ||
                inst == PREFRS || inst == PREFWS)
               op = 0;
            if (op > 0)
            {
               assert(ip->inst[1] < 0);  /* dest op must be register */
               j = ireg2type(-ip->inst[1]);
               ir = GetRegFindLR(j, FKO_BVTMP, ip);
               if (ir)
               {
                  switch(j)
                  {
                  case T_INT:
                     inst = LD;
                     break;
                  case T_FLOAT:
                     inst = FLD;
                     break;
                  case T_DOUBLE:
                     inst = FLDD;
                     break;
                  case T_VFLOAT:
                     inst = VFLD;
                     break;
                  case T_VDOUBLE:
                     inst = VDLD;
                     break;
                  }
                  ipN = InsNewInst(NULL, NULL, ip, inst, -ir, op, 0);
                  if (op == ip->inst[3])
                     ip->inst[3] = -ir;
                  else
                  {
                     assert(ip->inst[2] == op);
                     ip->inst[2] = -ir;
                  }
                  CalcThisUseSet(ipN);
                  CalcThisUseSet(ip);
                  nchanges++;
               }
            }
         }
      }
   }
/* fprintf(stderr, "ELS, NCHANGES=%d\n\n",nchanges); */
   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
   return(nchanges);
}

INSTQ *FindNextUseInBlock(INSTQ *ip0, int var)
/*
 * RETURNS: INSTQ of next instruction that uses all vars set in iv within
 *          block starting at ip0, NULL if not found
 */
{
   INSTQ *ip;
   if (ip0)
   {
      for (ip=ip0->next; ip; ip = ip->next)
      {
         if (ip->use && BitVecCheck(ip->use, var))
            return(ip);
      }
   }
   return(NULL);
}

int DoRemoveOneUseLoads(BLIST *scope)
/*
 * Changes ld followed by one and only one use to mem-src instruction
 * (possible only on x86)
 */
{
   INSTQ *ip, *ipn, *ipuse;
   BLIST *bl;
   int nchanges=0, iv, reg;
   enum inst inst;
   short k;
   extern int FKO_BVTMP;

/*
 * Only x86 has these from-memory operations
 */
   #ifndef X86
      return(0);
   #endif
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);
   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(TNREG);
   iv = FKO_BVTMP;

   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ipn)
      {
         ipn = ip->next;
         inst = ip->inst[0];
         if (IS_LOAD(inst) && inst != VFLDS && inst != VDLDS)
         {
            k = -ip->inst[1];
            ipuse = FindNextUseInBlock(ip, k-1);
            if (ipuse && ipuse->inst[3] == ip->inst[1] &&
                BitVecCheck(ipuse->deads, k-1))
            {
               ipuse->inst[3] = ip->inst[2];
               DelInst(ip);
               CalcThisUseSet(ipuse);
               nchanges++;
            }
         }
      }
   }
   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
//   fprintf(stderr, "U1: nchanges=%d\n", nchanges);
   return(nchanges);
}

static enum inst type2move(int type)
{
   enum inst mov;
   switch(type)
   {
   case T_INT    :
      mov = MOV;
      break;
   case T_FLOAT  :
      mov = FMOV;
      break;
   case T_DOUBLE :
      mov = FMOVD;
      break;
   case T_VFLOAT :
      mov = VFMOV;
      break;
   case T_VDOUBLE:
      mov = VDMOV;
      break;
   default:
      fko_error(__LINE__, "Unknown type %d, file=%s\n", type, __FILE__);
   }
   return(mov);
}
int DoLastUseLoadRemoval(BLIST *scope)
/*
 * If there's a load that can be made implicit, and it's not handled by
 * RemoveOneUseLoads due to being set, moves it into memory by doing
 * a register-reg move (which should be cleaned up by CP & RCP)
 */
{
   BLIST *bl;
   INSTQ *ipld, *ip, *ipn, *ipN;
   int nchanges=0, i, j, k, typ;
   enum inst inst;
/*
 * Only x86 has these from-memory operations
 */
   #ifndef X86
      return(0);
   #endif
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);

   for (bl=scope; bl; bl = bl->next)
   {
      for (ipld=bl->blk->ainst1; ipld; ipld = ipn)
      {
         ipn = ipld->next;
         inst = ipld->inst[0];
         if (IS_LOAD(inst) && inst != VFLDS && inst != VDLDS)
         {
            k = -ipld->inst[1];
            for (ip=ipld->next; ip; ip = ip->next)
            {
               if (!ip->use)
                  continue;
               if (BitVecCheck(ip->use, k-1) || BitVecCheck(ip->set, k-1))
                  break;
            }
            if (ip)
            {
               j = -ip->inst[3];
               typ =  j > 0 ? ireg2type(j) : 0;
               if (j > 0 && ip->inst[1] == -k && ip->inst[2] == -k &&
                   BitVecCheck(ip->deads, j-1) && typ == ireg2type(k))
               {
                  ipN = InsNewInst(NULL, NULL, ip, type2move(typ), -k, -j, 0);
                  CalcThisUseSet(ipN);
                  ip->inst[3] = ipld->inst[2];
                  CalcThisUseSet(ip);
                  DelInst(ipld);
                  nchanges++;
               }
            }
         }
      }
   }
   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
   return(nchanges);
}
