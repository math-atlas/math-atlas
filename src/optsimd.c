#include "fko.h"

int Type2Vlen(int type)
{
   if (type == T_VDOUBLE || type == T_DOUBLE)
      return(2);
   else if (type == T_VFLOAT || type == T_FLOAT)
      return(4);
   return(1);
}

ILIST *FindPrevStore(INSTQ *ipstart, short var, int blkvec, int ivseen)
/*
 * Finds prev store of var starting with inst ipstart, stopping search if
 * pred block is not in blkvec.
 * RETURNS : list of INSTQ that mark the prev store of var (multiple in the
 *           case where prev load is in preceeding blocks)
 */
{
   INSTQ *ip;
   ILIST *il=NULL, *il2;
   BLIST *bl;
   if (ipstart)
   {
      SetVecBit(ivseen, ipstart->myblk->bnum-1, 1);
      for (ip=ipstart; ip; ip = ip->prev)
         if (IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]] == var)
            return(NewIlist(ip, NULL));
      for (bl=ipstart->myblk->preds; bl; bl = bl->next)
      {
         if (BitVecCheck(blkvec, bl->blk->bnum-1) &&
             !BitVecCheck(ivseen, bl->blk->bnum-1) &&
             BitVecCheck(bl->blk->outs, var+TNREG-1))
         {
            il2 = FindPrevStore(bl->blk->ainstN, var, blkvec, ivseen);
            if (il)
               il2->next = il;
            il = il2;
         }
      }
   }
   return(il);
}

ILIST *FindNextLoad(INSTQ *ipstart, short var, int blkvec, int ivseen)
/*
 * Finds next load of var starting with inst ipstart
 * RETURNS : list of INSTQ that mark the next load of var (multiple in the
 *           case where 1st load is in succeeding blocks)
 */
{
   INSTQ *ip;
   ILIST *il=NULL, *il2=NULL;

   if (ipstart)
   {
      SetVecBit(ivseen, ipstart->myblk->bnum-1, 1);
      for (ip=ipstart; ip; ip = ip->next)
      {
         if (IS_LOAD(ip->inst[0]) && STpts2[ip->inst[2]-1] == var)
            return(NewIlist(ip, NULL));
      }
      assert(ipstart->myblk);

      if (ipstart->myblk->usucc && 
          !BitVecCheck(ivseen, ipstart->myblk->usucc->bnum-1) &&
          BitVecCheck(blkvec, ipstart->myblk->usucc->bnum-1) &&
          BitVecCheck(ipstart->myblk->usucc->ins, var+TNREG-1))
         il = FindNextLoad(ipstart->myblk->usucc->ainst1, var, blkvec, ivseen);
      if (ipstart->myblk->csucc &&  
          !BitVecCheck(ivseen, ipstart->myblk->csucc->bnum-1) &&
          BitVecCheck(blkvec, ipstart->myblk->csucc->bnum-1) &&
          BitVecCheck(ipstart->myblk->csucc->ins, var+TNREG-1))
         il2 = FindNextLoad(ipstart->myblk->csucc->ainst1, var, blkvec, ivseen);
      if (!il)
         il = il2;
      else 
         il->next = il2;
   }
   return(il);
}

short FindReadUseType(INSTQ *ip, short var, int blkvec)
/*
 * Find first use of var, including 1st use of any var it is copied to,
 * if a move is the first op
 */
{
   short j=0;
   ILIST *ib, *il;
   static int iv=0;

   if (!iv)
      iv = NewBitVec(32);
   else
      SetVecAll(iv, 0);
   ib = FindNextLoad(ip, var, blkvec, iv);
   for (il=ib; il; il = il->next)
   {
      ip = il->inst;
      if (ip->next->inst[0] == FADD || ip->next->inst[0] == FADDD)
         j |= VS_ACC;
      else if (IS_STORE(ip->next->inst[0])&& STpts2[ip->next->inst[2]-1] == var)
      {
         j |= FindReadUseType(ip->next, var, blkvec);
         j |= FindReadUseType(ip->next, STpts2[ip->next->inst[1]-1], blkvec);
      }
      else
         j |= VS_MUL;
   }
   KillIlist(ib);
   return(j);
}

int DoLoopSimdAnal(LOOPQ *lp)
/*
 * Does some analysis on unoptimized code to determine how to vectorize loop
 * This will be later thrown away, and we will start again with either
 * vectorized/unrolled loop, or unvectorized code.
 * RETURNS: error code if loop cannot be vectorized, 0 if it can
 * NOTE: supported vectorizable operations are assignment, add and mul.
 */
{
   struct ptrinfo *pbase, *p;
   short *sp, *s;
   int iv, iv1;
   int i, j, k, n, N;
   extern int FKO_BVTMP;
   ILIST *il, *ib;
   BLIST *bl;
   INSTQ *ip;
   extern short STderef;

   if (!lp)
      return(1);
/*
 * Require one and only one post-tail to simplify analysis
 */
   if (!lp->posttails || lp->posttails->next)
   {
      fko_warn(__LINE__, 
      "Must have one and only one posttail for simdification!\n\n");
      return(2);
   }
   CalcInsOuts(bbbase);
   CalcAllDeadVariables();
/*
 * Find all variables accessed in the loop
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   for (bl=lp->blocks; bl; bl = bl->next)
   {
      iv = BitVecComb(iv, iv, bl->blk->uses, '|');
      iv = BitVecComb(iv, iv, bl->blk->defs, '|');
   }
/*
 * Subtract off all registers, and ptr deref warning
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);
/*
 * Allow only fp ops & index ops
 * HERE HERE HERE
 * NOTE: need also array incrementing.  Probably need to delete all this crap
 *       before doing analysis
 */
   sp = BitVec2Array(iv, 1-TNREG);
   for (N=sp[0],n=0,i=1; i <= N; i++)
   {
      if (IS_FP(STflag[sp[i]-1]))
         sp[n++] = sp[i];
/*
 *    For non-fp var, if it's not the index var, give up
 */
      else if (sp[i] != lp->I && sp[i] != lp->end && 
               sp[i] != lp->inc)
      {
         fko_warn(__LINE__, "Bailing on vect due to var %d,%s\n", sp[i],
                  STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
         free(sp);
         return(3);
      }
   }
/*
 * If no fp vars, nothing to vectorize
 */
   if (!n)
   {
      free(sp);
      return(4);
   }
   else
   {
   /*
    * Make sure all vars are of same type
    */
      j = FLAG2TYPE(STflag[sp[0]-1]);
      for (i=1; i < n; i++)
      {
         if (FLAG2TYPE(STflag[sp[i]-1]) != j)
         {
             fko_warn(__LINE__, 
                      "Mixed type %d(%s), %d(%s) prevents vectorization!!\n\n",
                      j, STname[sp[0]-1] ? STname[sp[0]-1] : "NULL", 
                      FLAG2TYPE(sp[i]-1), STname[sp[i]-1] ? STname[sp[i]-1] 
                      : "NULL");
             return(4);
         }
      }
      if (j == T_FLOAT)
         lp->vflag = T_VFLOAT;
      else if (j == T_DOUBLE)
         lp->vflag = T_VDOUBLE;
   }
/*
 * Find arrays to vectorize
 */
   pbase = FindMovingPointers(lp->blocks);
   for (N=0,p=pbase; p; p = p->next)
   {
fprintf(stderr, "moving ptr = %s\n", STname[p->ptr-1]);
      if (IS_FP(STflag[p->ptr-1]))
      {
         N++;
         if ((p->flag | PTRF_CONTIG | PTRF_INC) != p->flag)
         {
            fko_warn(__LINE__, "Ptr movement prevents vectorization!!\n");
            free(sp);
            return(5);
         }
      }
   }
/*
 * Copy all moving pointers to lp->varrs
 */
   s = malloc(sizeof(short)*(N+1));
   assert(s);
   s[0] = N;
   for (j=0,i=1,p=pbase; p; p = p->next)
      if (IS_FP(STflag[p->ptr-1]))
         s[i++] = p->ptr;
   lp->varrs = s;
/*
 * Remove the moving arrays from scalar vals
 */
   for (k=0,i=1; i <= n; i++)
   {
      for (j=1; j <= N && s[j] != sp[i]; j++);
      if (j > N)
         sp[k++] = sp[i];
   }
   n = k;
   assert(n >= 0);
   lp->vscal = malloc(sizeof(short)*(n+1));
   assert(lp->vscal)
   if (n)
   {
      lp->vsflag = calloc(n+1, sizeof(short));
      lp->vsoflag = calloc(n+1, sizeof(short));
      assert(lp->vsflag && lp->vsoflag);
   }
   lp->vscal[0] = lp->vsflag[0] = lp->vsoflag[0] = n;
   for (i=1; i <= n; i++)
      lp->vscal[i] = sp[i-1];

   free(sp);
   sp = lp->vscal+1;
/*
 * Sort scalar vals into livein,liveout, and tmp
 */
   if (n)
   {
/*
 *    Find fp scalars set inside loop
 */
      iv1 = Array2BitVec(n, sp, TNREG-1);
      SetVecAll(iv, 0);
      for (bl=lp->blocks; bl; bl = bl->next)
      {
         for (ip=bl->blk->ainst1; ip; ip = ip->next)
            if (ACTIVE_INST(ip->inst[0]))
               BitVecComb(iv, iv, ip->set, '|');
      }
      BitVecComb(iv, iv, iv1, '&');
      s = BitVec2StaticArray(iv);
      for (i=1; i <= s[0]; i++)
      {
         k = s[i] - TNREG + 1;
         for (j=0; j < n; j++)
         {
            if (sp[j] == k)
            {
               lp->vsflag[j+1] |= VS_SET;
               break;
            }
         }
      }
/*
 *    Find fp scalars live on loop input
 */
      iv1 = Array2BitVec(n, sp, TNREG-1);
      BitVecComb(iv1, iv1, lp->header->ins, '&');
      s = BitVec2StaticArray(iv1);
      for (i=1; i <= s[0]; i++)
      {
         k = s[i] - TNREG + 1;
         for (j=0; j < n; j++)
         {
            if (sp[j] == k)
            {
               lp->vsflag[j+1] |= VS_LIVEIN;
               break;
            }
         }
      }
/*
 *    Find vars live on loop exit, that are also accessed in post-tail
 *    NOTE: if a var is live on one posttail, we'll write it to all posttail,
 *          but later phase of dead assignment elim can clean this up
 *
 */
      SetVecAll(iv1, 0);
      SetVecAll(iv, 0);
      for (bl=lp->tails; bl; bl = bl->next)
      {
         BitVecDup(iv1, bl->blk->outs, '=');
         if (bl->blk->usucc && 
             !BitVecCheck(lp->blkvec, bl->blk->usucc->bnum-1))
            BitVecComb(iv1, iv1, bl->blk->usucc->ins, '&');
         else
         {
            assert(bl->blk->csucc &&
                   !BitVecCheck(lp->blkvec, bl->blk->csucc->bnum-1));
            BitVecComb(iv1, iv1, bl->blk->csucc->ins, '&');
         }
         BitVecComb(iv, iv, iv1, '|');
      }
      iv1 = Array2BitVec(n, sp, TNREG-1);
      BitVecComb(iv, iv, iv1, '&');
      s =  BitVec2StaticArray(iv);
      for (i=1; i <= s[0]; i++)
      {
         k = s[i] - TNREG + 1;
         for (j=0; j < n; j++)
         {
            if (sp[j] == k)
            {
               lp->vsflag[j+1] |= VS_LIVEOUT;
               break;
            }
         }
      }
/*
 *    Find out how to init livein and reduce liveout vars
 */
      s = lp->vsflag+1;
      for (i=0; i < n; i++)
      {
/*
 *       For livein variables, any access is legal, but only handled in 2 ways:
 *       If adder, init one val to 0, others to init val, if anything else
 *       (MUL or assignment), init all vals to same
 *
 */
         if (s[i] & VS_LIVEIN)
         {
            j = FindReadUseType(lp->header->inst1, sp[i], lp->blkvec);
            if (j == VS_ACC)
               s[i] |= VS_ACC;
            else if (j == VS_MUL)
               s[i] |= VS_MUL;
            else
            {
               fprintf(stderr, "j=%d, ACC=%d,MUL=%d\n", j, VS_ACC, VS_MUL);
               fko_warn(__LINE__, 
                        "Mixed use of var %d(%s) prevents vectorization!!\n\n",
                        sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
               return(6);
            }
         }
/*
 *       Output scalars must be accumulators to vectorize
 */
         else if (s[i] & VS_LIVEOUT)
         {
            SetVecAll(iv, 0);
            ib = FindPrevStore(lp->posttails->blk->inst1, sp[i],lp->blkvec, iv);
            j = 0;
            for (il=ib; il; il = il->next)
            {
               if (ip->prev->inst[0] != FADD || ip->prev->inst[0] != FADDD)
               {
                  fko_warn(__LINE__, 
                "Non-add use of output var %d(%s) prevents vectorization!!\n\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  return(7);
               }
            }
            KillIlist(ib);
            lp->vsoflag[i+1] |= VS_ACC;
         }
      }
   }
/*
 * HERE HERE HERE
 * Now, need to scope optloop->varrs to make sure they are always operated on
 * by '=', '+' or '*'
 * NOTE: might fix this by waiting until the actual simidification is done,
 *       and simply backing up if it is not the case; otherwise, should 
 *       probably keep info around, so we know how to make the transforms
 *       when it is time
 * NOTE: array vals always loaded to scalars by HIL, so if we determine
 *       the operation performed on scalars, we will know the op being used
 *       by the arrays.
 *     
 */
   return(0);
}


int SimdLoop(LOOPQ *lp)
{
   short *sp;
   BLIST *bl;
   static enum inst 
      sfinsts[] = {FLD,  FST,  FMUL,  FADD,  FSUB,  FABS,  FMOV,  FZERO},
      vfinsts[] = {VFLD, VFST, VFMUL, VFADD, VFSUB, VFABS, VFMOV, VFZERO},
      sdinsts[] = {FLDD, FSTD, FMULD, FADDD, FSUBD, FABSD, FMOVD, FZEROD},
      vdinsts[] = {VDLD, VDST, VDMUL, VDADD, VDSUB, VDABS, VDMOV, VDZERO};
   const int nvinst=8;
   enum inst sld, vld, sst, vst, smul, vmul, sadd, vadd, ssub, vsub, 
             sabs, vabs, smov, vmov, szero, vzero, inst;
   short r0, r1, op;
   enum inst *sinst, *vinst;
   int i, j, n, k, nfr=0;
   char ln[512];
   struct ptrinfo *pi0, *pi;
   INSTQ *ip, *ippu, *iph, *iptp, *iptn;
   short vlen;
   enum inst vsld, vsst, vshuf;
   short sregs[TNFR], vregs[TNFR];

/*
 * Figure out what type of insts to translate
 */
   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
      sinst = sfinsts;
      vinst = vfinsts;
      vlen = 4;
      vsld = VFLDS;
      vsst = VFSTS;
      vshuf = VFSHUF;
      vld = VFLD;
      vst = VFST;
   }
   else
   {
      vsld = VDLDS;
      vsst = VDSTS;
      vshuf = VDSHUF;
      sinst = sdinsts;
      vinst = vdinsts;
      vld = VDLD;
      vlen = 2;
      vst = VDST;
   }
   r0 = GetReg(FLAG2TYPE(lp->vflag));
   r1 = GetReg(FLAG2TYPE(lp->vflag));
/*
 * Remove loop control logic from loop, and disallow simdification if
 * index is used in loop
 * NOTE: could allow index ref, but then need special case of non-vectorized
 *       op, so don't.  Can transform loops that use I to loops that use
 *       ptr arith instead.
 */
   KillLoopControl(lp);
/*
 * Generate scalar cleanup loop before simdifying loop
 */
   GenCleanupLoop(lp);

/* 
 * Find all pointer updates, and remove them from body of loop (leaving only
 * vectorized instructions for analysis); will put them back in loop after
 * vectorization is done.
 */

   pi0 = FindMovingPointers(lp->blocks);
   ippu = KillPointerUpdates(pi0, vlen);
/* 
 * Find inst in preheader to insert scalar init before; want it inserted last
 * unless last instruction is a jump, in which case put it before the jump.
 * As long as loads don't reset condition codes, this should be safe
 * (otherwise, need to move before compare as well as jump)
 */
   iph = lp->preheader->ainstN;
   if (iph && IS_BRANCH(iph->inst[0]))
      iph = iph->prev;
   else
      iph = NULL;
/*
 * Find inst in posttail to insert reductions before; if 1st active inst
 * is not a label, insert at beginning of block, else insert after label
 */
   if (lp->posttails->blk->ilab)
   {
      iptp = lp->posttails->blk->ainst1;
      iptn = NULL;
   }
   else
   {
      iptp = NULL;
      iptn = lp->posttails->blk->ainst1;
      if (!iptn)
         iptn = lp->posttails->blk->inst1;
   }

/*
 * Insert scalar-to-vector initialization in preheader for vars live on entry
 * and vector-to-scalar reduction in post-tail for vars live on exit
 */
   j = 0;
   k = STiconstlookup(0);
   for (n=lp->vscal[0],i=0; i < n; i++)
   {
      if (VS_LIVEIN & lp->vsflag[i+1])
      {
/*
 *       ADD-updated vars set v[0] = scalar, v[1:N] = 0
 */
         if (VS_ACC & lp->vsflag[i+1])
            PrintComment(lp->preheader, NULL, iph, 
               "Init accumulator vector for %s", STname[lp->vscal[i+1]-1]);
         else
            PrintComment(lp->preheader, NULL, iph, 
               "Init vector equiv of %s", STname[lp->vscal[i+1]-1]);
         InsNewInst(lp->preheader, NULL, iph, vsld, -r0,
                    SToff[lp->vscal[i+1]-1].sa[2], 0);
         if (!(VS_ACC & lp->vsflag[i+1]))
            InsNewInst(lp->preheader, NULL, iph, vshuf, -r0, -r0, k);
         InsNewInst(lp->preheader, NULL, iph, vst, 
                    SToff[lp->vvscal[i+1]-1].sa[2], -r0, 0);
      }
/*
 *    Output vars are known to be updated only by ADD
 */
      if (VS_LIVEOUT & lp->vsflag[i+1])
      {
         j++;
         assert((lp->vsoflag[i+1] & (VS_MUL | VS_EQ | VS_ABS)) == 0);
         iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                  "Reduce accumulator vector for %s", STname[lp->vscal[i+1]-1]);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vld, -r0,
                           SToff[lp->vvscal[i+1]-1].sa[2], 0);
         if (vld == VDLD)
         {
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1, -r0,
                              STiconstlookup(0x33));
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VDADD,-r0,-r0,-r1);
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSTS, 
                              SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
         }
         else
         {
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF, -r1, -r0,
                              STiconstlookup(0x3276));
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,-r0,-r0,-r1);
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF, -r1, -r0,
                              STiconstlookup(0x5555));
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,-r0,-r0,-r1);
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSTS, 
                              SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
         }
      }
   }
   GetReg(-1);
   iptp = InsNewInst(lp->posttails->blk, iptp, NULL, CMPFLAG, CF_VRED_END,
                     0, 0);
/*
 * Translate body of loop
 */
   for (bl=lp->blocks; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         inst = GET_INST(ip->inst[0]);
         if (ACTIVE_INST(inst))
         {
            for (i=0; i < nvinst; i++)
            {
/*
 *             If the inst is scalar fp, translate to vector fp, and insist
 *             all fp ops become vector ops
 */
               if (sinst[i] == inst)
               {
                  ip->inst[0] = vinst[i];
/*
 *                Change scalar ops to vector ops
 */
                  for (j=1; j < 4; j++)
                  {
                     op = ip->inst[j];
                     if (!op) continue;
                     else if (op < 0)
                     {
                        op = -op;
                        k = FindInShortList(nfr, sregs, op);
                        if (!k)
                        {
                           nfr = AddToShortList(nfr, sregs, op);
                           k = FindInShortList(nfr, sregs, op);
                           vregs[k-1] = GetReg(FLAG2TYPE(lp->vflag));
                        }
                        ip->inst[j] = -vregs[k-1];
                     }
                     else
                     {
fprintf(stderr, "scoping %s (%d)\n", STname[op-1], op);
                        if (IS_DEREF(STflag[op-1]))
                        {
                           k = STpts2[op-1];
                           if (!FindInShortList(lp->varrs[0],lp->varrs+1,k))
                           {
                              k = FindInShortList(lp->vscal[0],lp->vscal+1,k);
                              assert(k);
                              ip->inst[j] = SToff[lp->vvscal[k]-1].sa[2];
                              assert(ip->inst[j] > 0);
                           }
                        }
                        else if (!FindInShortList(lp->varrs[0],lp->varrs+1,op))
                        {
                           k = FindInShortList(lp->vscal[0], lp->vscal+1, op);
                           assert(k);
                           ip->inst[j] = lp->vvscal[k];
                        }
                     }
                  }
                  break;
               }
            }
         }
      }
   }
   GetReg(-1);
/*
 * Put back loop control and pointer updates
 */
   OptimizeLoopControl(lp, vlen, 0, ippu);
   CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = CFLOOP = 0;
#if 0
   InvalidateLoopInfo();
   NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
#endif
   return(0);
}

int VectorizeStage1(void)
/*
 * Assuming we have a correct scalar LIL (stage 0), creates a scalar LIL,
 * but with all vector locals declared, and init prol/epil not called.
 * Returns 0 on success, error code on failure.
 */
{
   short *varrs, *vscal, *vsflag, *vsoflag;
   short *sp;
   LOOPQ *lp;
   INSTQ *ip, *ipn;
   struct ptrinfo *pi0, *pi;
   int flag, k, i, n, j;
   char ln[1024];

   if (!optloop)
   {
      fko_warn(__LINE__, "Cannot vectorize rout without loop!!\n");
      return(15);
   }
/*
 * Get code into standard form for analysis
 */
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
   GenPrologueEpilogueStubs(bbbase, 0);
   NewBasicBlocks(bbbase);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
   lp = optloop;
/*
 * Create a bad LIL to perform vector loop analysis
 */
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
   KillLoopControl(lp);
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
   if (FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]))
   {
      fko_warn(__LINE__, "Index refs inside loop prevent vectorization!!\n");
      return(11);
   }
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
   pi0 = FindMovingPointers(lp->blocks);
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
   for (pi=pi0; pi; pi = pi->next)
   {
      if (pi->nupdate > 1)
      {
         fko_warn(__LINE__, "Multiple ptr updates prevent vectorization!!\n");
         return(12);
      }
      if (!(pi->flag & PTRF_CONTIG))
      {
         fko_warn(__LINE__, 
                  "Non-contiguous ptr updates prevent vectorization!!\n");
         return(13);
      }
   }
   ip = KillPointerUpdates(pi, Type2Vlen(FLAG2TYPE(lp->vflag)));
   KillAllPtrinfo(pi);
   for (; ip; ip = ipn)
   {
      ipn = ip->next;
      free(ip);
   }
   CFUSETU2D = 0;
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
   i = DoLoopSimdAnal(optloop);
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
   if (i)
      return(i);
/*
 * Save vector info, and then restore good scalar state
 */
   varrs = optloop->varrs;
   vscal = optloop->vscal;
   vsflag = optloop->vsflag;
   vsoflag = optloop->vsoflag;
   optloop->varrs = optloop->vscal = optloop->vsflag = optloop->vsoflag = NULL;
   flag = lp->vflag;
   RestoreFKOState(0);
   lp = optloop;
   optloop->varrs = varrs;
   optloop->vscal = vscal;
   optloop->vsflag = vsflag;
   optloop->vsoflag = vsoflag;
   optloop->vflag = flag;
/*
 * Create vector locals for all vector scalars in loop, and their derefs
 */
   k = LOCAL_BIT | FLAG2TYPE(flag);
   n = lp->vscal[0];
   lp->vvscal = malloc(sizeof(short)*(n+1));
   assert(lp->vvscal);
   lp->vvscal[0] = n;
   sp = vscal + 1;
   for (i=0; i < n; i++)
   {
      sprintf(ln, "_V%d_%s", i-1, STname[sp[i]-1] ? STname[sp[i]-1] : "");
      j = lp->vvscal[i+1] = STdef(ln, k, 0);
      SToff[j-1].sa[2] = AddDerefEntry(-REG_SP, j, -j, 0, j);
   }
   SaveFKOState(1);
   return(0);
}

int VectorizeStage3(int savesp, int SVSTATE)
/*
 * Assuming Stage 1 vect done, create Stage 3, which includes vectorizing
 * the loop and generating cleanup (though jump to cleanup left to unroll)
 * This stage has already gen prol/epil, so must begin from Stage 1 if
 * a transform changes frame layout
 */
{
   int i;
/*
 * Get code into standard form for analysis
 */
   GenPrologueEpilogueStubs(bbbase, savesp);
   NewBasicBlocks(bbbase);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
   i = SimdLoop(optloop);
   if (i)
      return(i);
   UpdatePrefetchInfo();
   if (SVSTATE)
      SaveFKOState(3);
   return(0);
}
