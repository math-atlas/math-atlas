ILIST *FindPrevStore(INSTQ *ipstart, short var, int blkvec)
/*
 * Finds prev store of var starting with inst ipstart, stopping search if
 * pred block is not in blkvec.
 * RETURNS : list of INSTQ that mark the prev store of var (multiple in the
 *           case where prev load is in preceeding blocks)
 */
{
   INSTQ *ip;
   ILIST *il=NULL, il2;
   BLIST *bl;
   if (ipstart)
   {
      for (ip=ipstart; ip; ip = ip->prev)
         if (IS_STORE(ip->inst[0] && STpts2[ip->inst[1]] == var)
            return(NewIlist(ip, NULL));
      for (bl=ipstart->myblk->preds; bl; bl = bl->next)
      {
         if (BitVecCheck(blkvec, bl->blk->bnum-1) &&
             BitVecCheck(bl->blk->outs, var+TNREG-1))
         {
            il2 = FindPrevStore(bl->blk->ainstN, var, blkvec);
            if (il)
               il2->next = il;
            il = il2;
         }
      }
   }
   return(il);
}

ILIST *FindNextLoad(INSTQ *ipstart, short var, int blkvec)
/*
 * Finds next load of var starting with inst ipstart
 * RETURNS : list of INSTQ that mark the next load of var (multiple in the
 *           case where 1st load is in succeeding blocks)
 */
{
   INSTQ *ip;
   ILIST *il=NULL, il2;

   if (ipstart)
   {
      for (ip=ipstart; ip; ip = ip->next)
      {
         if (IS_LOAD(ip->inst[0] && STpts2[ip->inst[1]] == var)
            return(NewIlist(ip, NULL));
      }
      assert(ipstart->myblk);
      if (ipstart->myblk->usucc && 
          BitVecCheck(blkvec, ipstart->myblk->usucc->bnum-1) &&
          BitVecCheck(ipstart->myblk->usucc->ins, var+TNREG-1))
         il = FindNextLoad(ipstart->myblk->usucc->ainst1, var);
      if (ipstart->myblk->csucc && 
          BitVecCheck(blkvec, ipstart->myblk->csucc->bnum-1) &&
          BitVecCheck(ipstart->myblk->csucc->ins, var+TNREG-1))
         il2 = FindNextLoad(ipstart->myblk->csucc->ainst1, var);
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

   ib = FindNextLoad(ip, var, blkvec);
   for (il=ib; il; il = il->next)
   {
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
int DoInitLoopVecAnal(LOOPQ *lp)
/*
 * Does some analysis on unoptimized code to determine how to vectorize loop
 * This will be later thrown away, and we will start again with either
 * vectorized/unrolled loop, or unvectorized code.
 * RETURNS: 0 if loop cannot be vectorized, 1 if it can
 * NOTE: supported vectorizable operations are assignment, add and mul.
 */
{
   struct ptrinfo *pbase, *p;
   short *sp, *s;
   int iv, iv1;
   int i, j, k, n, N;
   extern int FKO_BVTMP;
   ILIST *il, *ib;
   if (!lp)
      return(0);
/*
 * Get code into standard form for analysis
 */
   GenPrologueEpilogueStubs(bbbase, 0);
   NewBasicBlocks(bbbase);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
/*
 * Require one and only one post-tail to simplify analysis
 */
   if (!lp->posttail || lp->posttail->next)
   {
      fko_warn(__LINE__, 
      "Must have one and only one posttail for simdification!\n\n");
      return(0);
   }
   CalcInsOuts(bbbase);
   CalcAllDeadVariables();
/*
 * Find all variables accessed in the loop
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   for (bl=blocks; bl; bl = bl->next)
   {
      iv = BitVecComb(iv, bl->blk->uses, '|');
      iv = BitVecComb(iv, bl->blk->defs, '|');
   }
/*
 * Subtract off all registers
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
/*
 * Allow only fp ops & index ops
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
         fko_warn(__LINE__, "Bailing on vect due to var %d,%s\n", sp[i]
                  STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
         free(sp);
         return(0);
      }
   }
/*
 * If no fp vars, nothing to vectorize
 */
   if (!n)
   {
      free(sp);
      return(0);
   }
   else
   {
   /*
    * Make sure all vars are of same type
    */
      j = FLAG2TYPE(sp[0]-1);
      for (i=1; i < n; i++)
      {
         if (FLAG2TYPE(sp[i]-1) != j)
         {
             fko_warn(__LINE__, 
                      "Mixed type %d(%s), %d(%s) prevents vectorization!!\n\n,
                      j, STname[sp[0]-1] ? STname[sp[0]-1] : "NULL", 
                      FLAG2TYPE(sp[i]-1), STname[sp[i]-1] ? STname[sp[i]-1] 
                      : "NULL");
             return(0);
         }
      }
      lp->vflag = j;
   }
/*
 * Find arrays to vectorize
 */
   pbase = FindMovingPointers(lp->blocks);
   for (N=0,p=pbase; p; p = p->next)
   {
      if (IS_FP(STflag[p->ptr-1]))
      {
         N++;
         if ((p->flag | PTRF_CONTIG | PTRF_INC) != p->flag)
         {
            fko_warn(__LINE__, "Ptr movement prevents vectorization!!\n");
            free(sp);
            return(0);
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
   for (k=i=0; i < n; i++)
   {
      for (j=0; j < N && s[j] != sp[i]; j++);
      if (j == N)
         sp[k++] = sp[i];
   }
   n -= k;
   assert(n >= 0);
   lp->vscal = malloc(sizeof(short)*(n+1));
   assert(lp->vscal)
   if (n)
   {
      lp->vsflag = calloc(n, sizeof(short));
      assert(lp->vsflag);
   }
   lp->vscal[0] = n;
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
               lp->vsflag[j] |= VS_SET;
               break;
            }
         }
      }
/*
 *    Find fp scalars live on loop input
 */
      iv1 = Array2BitVec(n, sp, TNREG-1);
      BitVecComb(iv1, lp->header->ins, '&');
      s = BitVec2StaticArray(iv1);
      for (i=1; i <= s[0]; i++)
      {
         k = s[i] - TNREG + 1;
         for (j=0; j < n; j++)
         {
            if (sp[j] == k)
            {
               lp->vsflag[j] |= VS_LIVEIN;
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
      for (bl=tails; bl; bl = bl->next)
      {
         BitVecDup(iv1, bl->blk->outs, '=');
         if (bl->blk->usucc && 
             !BitVecCheck(lp->blkvec, bl->usucc->blk->bnum-1))
            BitVecComb(iv1, bl->blk->usucc->ins, '&');
         else
         {
            assert(bl->blk->csucc &&
                   !BitVecCheck(lp->blkvec, bl->csucc->blk->bnum-1));
            BitVecComb(iv1, bl->blk->csucc->ins, '&');
         }
         BitVecComb(iv, iv1, '|');
      }
      iv1 = Array2BitVec(n, sp, TNREG-1);
      BitVecComb(iv, iv1, '&');
      s =  BitVec2StaticArray(iv);
      for (i=1; i <= s[0]; i++)
      {
         k = s[i] - TNREG + 1;
         for (j=0; j < n; j++)
         {
            if (sp[j] == k)
            {
               lp->vsflag[j] |= VS_LIVEOUT;
               break;
            }
         }
      }
/*
 *    Find out how to init livein and reduce liveout vars
 *    NOTE: right now, we have an error, because we don't consider what happens
 *          when the first op of a scalar is to copy it to another scalar,
 *          in which case you must examine the use of both vars.
 */
      s = lp->vsflag;
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
               fko_warn(__LINE__, 
                        "Mixed use of var %d(%s) prevents vectorization!!\n\n",
                        sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
               return(0);
            }
         }
/*
 *       Output scalars must be accumulators to vectorize
 */
         else if (s[i] & VS_LIVEOUT)
         {
            ib = FindPrevStore(lp->posttail->inst1, sp[i],lp->blkvec);
            j = 0;
            for (il=ib; il; il = il->next)
            {
               if (ip->prev->inst[0] != FADD || ip->prev->inst[0] != FADDD)
               {
                  fko_warn(__LINE__, 
                "Non-add use of output var %d(%s) prevents vectorization!!\n\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  return(0);
               }
            }
            KillIlist(ib);
            s[i] |= VS_ACC;
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
   return(1);
}

void VecUnrollLoop(LOOPQ *lp, int unroll)
{
   enum inst sld, vld, sst, vst, smul, vmul, sadd, vadd, ssub, vsub, 
             sabs, vabs, smov, vmov, szero, vzero;

   if (!DoInitLoopVecAnal(lp))
     exit(-1);
   if (IS_FLOAT(lp->vflag))
   {
      sld   = FLD;
      sst   = FST;
      smul  = FMUL;
      sadd  = FADD;
      ssub  = FSUB;
      sabs  = FABS;
      smov  = FMOV;
      szero = FZERO;
      vld   = VFLD;
      vst   = VFST;
      vmul  = VFMUL;
      vadd  = VFADD;
      vsub  = VFSUB;
      vabs  = VFABS;
      vmov  = VFMOV;
      vzero = VFZERO;
   }
   else
   {
      sld   = FLDD;
      sst   = FSTD;
      smul  = FMULD;
      sadd  = FADDD;
      ssub  = FSUBD;
      sabs  = FABSD;
      smov  = FMOVD;
      szero = FZEROD;
      vld   = VDLD;
      vst   = VDST;
      vmul  = VDMUL;
      vadd  = VDADD;
      vsub  = VDSUB;
      vabs  = VDABS;
      vmov  = VDMOV;
      vzero = VDZERO;
   }
}
