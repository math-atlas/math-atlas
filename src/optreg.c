#include <ifko.h>
#include <fko_arch.h>

static IGNODE **IG=NULL;
static int NIG=0, TNIG=0;

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
      SetVecBit(node->myblkvec, blk->bnum-1, 1);
   }
   else
      new->myblkvec = 0;
   new->liveregs = new->conflicts = 0;
   new->nread = new->nwrite = 0;
   new->var = var;
   new->ignum = AddIG2Table(new);
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
   IGNODE **myIG;   /* array of this block's IGNODES */
   int igN, nn=0;
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
   if (!bp->conin) bp->conout = NewBitVec(TNREG+64);
   else SetVecAll(bp->conout, 0);
   vals = BitVec2StaticArray(bp->ins);
/*
 * Create ignode for all variables live on block entry, and add it
 * blocks conin, conout, and ignodes
 */
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
      else
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
            if (!BitVecCheck(ip->set, k))
               myIG[j]->nwrite++;
/*
 *          If var is dead, delete it from myIG and block's conout, and
 *          indicate that this is a block and instruction where range dies
 */
            SetVecBit(bp->conout, myIG[j]->ignum, 0);
            myIG[j] = NULL;
            node->blkend = AddBlockToList(node->blkend, bp);
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
               node->blkbeg->ptr = ip;
               SetVecBit(bp->ignodes, node->ignum, 1);
            }
         }
/*
 *       If it's a register being set, add it to list of live regs as well
 *       as to the regstate of all currently live ranges
 */
         else
         {
            k = vals[i] + 1;
            iv = Reg2Regstate(k);
            BitVecComb(liveregs, liveregs, iv, '|');
            for (j=0; j != nn; j++)
               if (myIG[j])
                  BitVecComb(myIG[j]->liveregs, myIG[j]->liveregs, iv, '|');
         }
      }
   }
   free(myIG);
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
}

void CombineBlockIG()

char *Int2Reg(int i)
/*
 * Translates integral encoding to machine-specific registers
 */
{
   static char ln[128];

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
   {
   fprintf(stderr, 
      "I=[%d,%d); F=[%d,%d); D=[%d,%d); ICC=[%d,%d); FCC=[%d,%d); PC=%d\n",
           IREGBEG, IREGEND,FREGBEG, FREGEND,DREGBEG, DREGEND,
           ICCBEG, ICCEND, FCCBEG, FCCEND, PCREG);
      fko_error(__LINE__, "Unknown register index %d, file=%s\n",
                i, __FILE__);
   }
   return(ln);
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
      if (!strcmp(archiregs[i], regname)) return(-i-1);
   for (i=FREGBEG; i < FREGEND; i++)
      if (!strcmp(archfregs[i], regname)) return(-i-1);
   for (i=DREGBEG; i < DREGEND; i++)
      if (!strcmp(archdregs[i], regname)) return(-i-1);
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
         SetVecBit(iv, k-FREGBEG+DREGBEG, 1);
      #elif defined(SPARC)
         SetVecBit(iv, ((k-FREGBEG)>>1)+DREGBEG, 1);
      #endif
   }
   else if (k >= DREGBEG && k < DREGEND)
   {
      #if defined(X86) || defined(PPC)
         SetVecBit(iv, k-DREGBEG+FREGBEG, 1);
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
   else fko_error(__LINE__, "Unknown register index %d, file=%s\n",
                  k, __FILE__);
   return(iv);
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
   CFU2D = CFUSETU2D = INUSETU2D = INDEADU2D = 0;
}
