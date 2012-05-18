#include "fko.h"

/* Majedul: to save the paths for speculative Vectorization. */
static LOOPPATH **PATHS = NULL;
static int NPATH = 0, TNPATH = 0, VPATH = -1;

void NewPathTable(int chunk)
{
   int i, n;
   LOOPPATH **new;
   n = TNPATH + chunk;
   new = malloc(n*sizeof(LOOPPATH*));
   assert(new);
   for (i=0; i!=TNPATH; i++)
      new[i] = PATHS[i];
   for (; i!=n ; i++)
      new[i] = NULL;
   if(PATHS)
      free(PATHS);
   PATHS = new;
   TNPATH = n;
}
int AddPath2Table(LOOPPATH *path)
{
   if (NPATH == TNPATH)
      NewPathTable(8);
   PATHS[NPATH] = path;
   path->pnum = NPATH;
   return(NPATH++); 
}

LOOPPATH *NewLoopPath(BLIST *blks)
{
   BLIST *bl;
   BBLOCK *bp;
   LOOPPATH *new;
   new = malloc(sizeof(LOOPPATH));
   assert(new);
/*
 * update the block list and info related to it
 */
#if 1
   assert(blks); /* right now, we don't expect NULL list*/
#endif
   if (blks)
   {
      new->blocks = blks;
      new->head = blks->blk;
      for (bl = blks; bl->next; bl=bl->next);
      new->tail = bl->blk;

      new->uses = NewBitVec(TNREG+32); /*by default they are 0 */
      new->defs = NewBitVec(TNREG+32);
      SetVecAll(new->uses, 0);
      SetVecAll(new->defs, 0);
/*
 *    Are you sure that uses/defs are calulated yet?? 
 */
      for (bl = blks; bl; bl=bl->next)
      {
         if (bl->blk->uses && bl->blk->defs)
         {
            BitVecComb(new->uses, new->uses, bl->blk->uses, '|');
            BitVecComb(new->defs, new->defs, bl->blk->defs, '|');
         }
      }
/*    will update after path analysis */
      new->ptrs = NULL;
      new->scal = NULL;
      new->sflag = NULL;
      new->lpflag = 0;
      new->vflag = 0;
      new->varrs = NULL;
      new->vscal = NULL;
      new->vsflag = NULL;
      new->vsoflag = NULL;
   }
   else /* is there any case where we may need this? */
   {
      new->blocks = NULL;
      new->head = NULL;
      new->tail = NULL;
      new->ptrs = NULL;
      new->scal = NULL;
      new->sflag = NULL;
      new->uses = 0;
      new->defs = 0;
      new->lpflag = 0;
      new->vflag = 0;
      new->varrs = NULL;
      new->vscal = NULL;
      new->vsflag = NULL;
      new->vsoflag = NULL;
   }
   new->pnum = AddPath2Table(new); /* though it is updated inside the func */
   return new;
}

void KillPath(LOOPPATH *path)
{
   if (path->blocks) /* insert by creating new list */
      KillBlockList(path->blocks);
   if (path->ptrs)
      KillAllPtrinfo(path->ptrs);
   if (path->scal)
      free(path->scal);
   if (path->sflag)
      free(path->sflag);
   if (path->uses) 
      KillBitVec(path->uses);
   if (path->defs)
      KillBitVec(path->defs);
   PATHS[path->pnum] = NULL;  
/* head and tail points to the bblock which should not be deleted */
   free(path);
}
void KillPathFromTable(LOOPPATH *apath)
{
   int i, j;
   for (i = 0; i < NPATH; i++)
   {
      if (PATHS[i] == apath) break;
   }
   if (i != NPATH)
   {
      KillPath(PATHS[i]);
      PATHS[i] = NULL;
      if (VPATH == i) VPATH = -1;
      for (j = i+1; j < NPATH; i++, j++)
      {
         PATHS[i] = PATHS[j];
         PATHS[i]->pnum = i;
      }
      NPATH--;
   }
}
void KillPathTableEntries()
{
   int i;
   for (i=0; i < NPATH; i++)
   {
      if (PATHS[i])
      {
         KillPath(PATHS[i]);
         PATHS[i] = NULL;
         if (VPATH == i) VPATH = -1;
      }
   }
   NPATH = 0;
}
void KillPathTable()
{
   KillPathTableEntries();
   free(PATHS);
   PATHS = NULL;
   TNPATH = 0;
}
/* 
 * Majedul: Changed for AVX
 */

int Type2Vlen(int type)
{
   if (type == T_VDOUBLE || type == T_DOUBLE)

   #if defined(X86) && defined(AVX)
       return(4);
   #else
       return(2);   
   #endif

   else if (type == T_VFLOAT || type == T_FLOAT)
   #if defined(X86) && defined(AVX)
       return(8);
   #else
       return(4);
   #endif

   else
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
#if 1
   if (pbase)
   {
      fprintf(stderr, "moving pointers!!!\n");
      for (p=pbase; p; p=p->next)
      {
          fprintf(stderr, "%s\n",STname[SToff[pbase->ptr-1].sa[0]]);
      }
   }
   else
      fprintf(stderr, "No moving pointer, already deleted!!!\n");
#endif
   for (N=0,p=pbase; p; p = p->next)
   {
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
   for (k=0,i=1; i <= n; i++) /* FIXME: should be < n */
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
     sfinsts[] = {FLD,  FST,  FMUL,  FMAC, FADD,  FSUB,  FABS,  FMOV,  FZERO},
     vfinsts[] = {VFLD, VFST, VFMUL, VFMAC, VFADD, VFSUB, VFABS, VFMOV, VFZERO},
     sdinsts[] = {FLDD, FSTD, FMULD, FMACD, FADDD, FSUBD, FABSD, FMOVD, FZEROD},
     vdinsts[] = {VDLD, VDST, VDMUL, VDMAC, VDADD, VDSUB, VDABS, VDMOV, VDZERO};
   const int nvinst=9;
   enum inst sld, vld, sst, vst, smul, vmul, smac, vmac, sadd, vadd, ssub, vsub, 
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
      #if defined(X86) && defined(AVX)
         vlen = 8;
      #else
         vlen = 4;
      #endif
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
      #if defined(X86) && defined(AVX)
         vlen = 4;
      #else
         vlen = 2;
      #endif
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
            #if defined(X86) && defined(AVX)
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1, 
                                 -r0, STiconstlookup(0x3276));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VDADD,-r0,-r0,
                                 -r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1, 
                                 -r0, STiconstlookup(0x3715));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VDADD,-r0,-r0,
                                 -r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSTS, 
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #else
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1, 
                                 -r0, STiconstlookup(0x33));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VDADD,-r0,-r0,
                                 -r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSTS, 
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #endif
         }
         else
         {
            #if defined(X86) && defined(AVX)
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF, 
                                 -r1, -r0, STiconstlookup(0x7654FEDC));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF, 
                                 -r1, -r0, STiconstlookup(0x765432BA));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF, 
                                 -r1, -r0, STiconstlookup(0x76CD3289));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSTS, 
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);         
            #else
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF, 
                                 -r1, -r0, STiconstlookup(0x3276));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF, 
                                 -r1, -r0, STiconstlookup(0x5555));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSTS, 
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #endif
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
 * Get code into standard form for analysis, it can be made isolated from
 * this function. but needed to do it before any analysis.
 */
   GenPrologueEpilogueStubs(bbbase, 0);
   NewBasicBlocks(bbbase);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
   lp = optloop;
#if 0
   fprintf(stdout,"LIL Vector Analysis \n\n");
   PrintInst(stdout,bbbase);
   exit(0);
#endif
   
/*
 * Create a bad LIL to perform vector loop analysis
 */
   KillLoopControl(lp);

   if (FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]))
   {
      fko_warn(__LINE__, "Index refs inside loop prevent vectorization!!\n");
      return(11);
   }
   
   pi0 = FindMovingPointers(lp->blocks);
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
   i = DoLoopSimdAnal(optloop);
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
#if 0
         fprintf(stdout,"LIL just Before Vectorization\n\n");
         PrintInst(stdout,bbbase);
         exit(0);
#endif
   i = SimdLoop(optloop);
   if (i)
      return(i);
   UpdateNamedLoopInfo();
   if (SVSTATE)
      SaveFKOState(3);
   return(0);
}

int IsSpeculationNeeded()
/*
 * check wether speculative vectorization is needed.
 * Returns 1 if SSV is needed, 0 otherwise
 */
{
   LOOPQ *lp;
   BLIST *bp;

   if (!optloop)
   {
      fko_warn(__LINE__, "Cannot vectorize rout without loop!!\n");
      return(0);
   }
/*
 * Get code into standard form for analysis, it can be made isolated from
 * this function. but needed to do it before any analysis.
 */
   GenPrologueEpilogueStubs(bbbase, 0);
   NewBasicBlocks(bbbase);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
   lp = optloop;

#if 0
   fprintf(stdout, "\n CFG: \n");
   ShowFlow(NULL,bbbase);
   fprintf(stdout, "\n LOOP BLOCKS: \n");
   for (bp = lp->blocks; bp ; bp = bp->next)
      fprintf(stdout, "%d ",bp->blk->bnum);
   fprintf(stdout,"\n");
#endif

/*
 * check for conditional branch inside loop other than loop itself,
 * if there is one, it is a candidate for speculative vectorization
 * NOTE: If variable use in conditional branch is loop invariant, we don't 
 * need to vectorize the branch itself. But we omit this case here because 
 * we can use some loop transformation like: loop unswitching
 * to split loop if branch condition is invariant with the loop.
 */
   for (bp = lp->blocks; bp ; bp = bp->next)
   {
      if (bp->blk->csucc && bp->blk->csucc != lp->header)
      {
#if 0
         fprintf(stderr,"%d has csucc=%d but loop header= %d\n",bp->blk->bnum,
                 bp->blk->csucc->bnum, lp->header->bnum);
#endif
         return 1;
      }
   }
   return 0;
}

void FindPaths(BBLOCK *head, BLIST *loopblocks, LOOPQ *lp, BLIST *blkstack)
/*
 * saves all the paths from header to the tail of loop in the global path 
 * data structure (PATHS).
 */
{
   BLIST *bl;
   if (head == lp->tails->blk)
   {
      blkstack = AddBlockToList(blkstack,head);
/*
 *    This will create a new list without killing the previous one but
 *    in reverse direction. I avoid killing as we need the list to track 
 *    the path when it backtracks.
 */
      NewLoopPath(NewReverseBlockList(blkstack));
#if 0
   fprintf(stderr, "PATH COMPLETE!\n");
   for (bl = blkstack; bl ; bl = bl->next)
      fprintf(stderr, "%d ",bl->blk->bnum);
   fprintf(stderr,"\n");
#endif
      return ;
   }
/*
 * this will not be the case as we skip the invalid path. only occurs
 * when there is no head at the first time.
 */
   if (!head)
   {
      fprintf(stderr, "Invalid Path!\n");
      return ;
   }
/*
 * adds a node to point the blk at the front of the list and returns that ptr
 * So, before returning recursive function, free the node; otherwise we can't 
 * find it again.
 */
   blkstack = AddBlockToList(blkstack, head);
   if (head->csucc)
   {
      if (FindBlockInList(loopblocks,head->csucc))
      {
         FindPaths(head->csucc, loopblocks, lp, blkstack);
      }
   }
   if (head->usucc)
   {
      if (FindBlockInList(loopblocks,head->usucc))
      {
         FindPaths(head->usucc, loopblocks, lp, blkstack);
      }
   }
/*
 * we create a new list (hence, allocate mem) to save the list which 
 * completes a path in reverse direction. So, when backtracks, we can free 
 * this mem. 
 */
   free(blkstack);
}

void FindLoopPaths(LOOPQ *lp)
/*
 * Majedul: works after the creation of CFG and loop info by FindLoops()
 * finds all the paths from head to tails for optloop maintaining the order
 * form head to tail.
 */
{
   BLIST *bl;
   int i;
   BBLOCK *blk;
   
   bl = NULL; /* serves as a temporary ptr in recusion */
/*
 * There must be loop and loop blocks to find the paths for it.
 */
   if (!lp)
   {
      fko_warn(__LINE__,"Must apply on a loop");
   }

/*
 * Consider only one tail of loop for this, after SSV there may be
 * multiple tails.... 
 * HERE HERE. is it a necessary? will make a conclusion after SSV.
 */
   if (!lp->tails || lp->tails->next)
   {
      fko_warn(__LINE__,"Must have only one tail of loop right now!\n");
      return ;
   }
#if 0
   fprintf(stdout, "\n CFG: \n");
   ShowFlow(NULL,bbbase);
   fprintf(stdout, "\n LOOP BLOCKS: \n");
   for (bp = lp->blocks; bp ; bp = bp->next)
      fprintf(stdout, "%d ",bp->blk->bnum);
   fprintf(stdout,"\n");
#endif

/*
 * Find all the paths from head to tail and save it in PATHS in
 * head to tail order.
 */
   FindPaths(lp->header, lp->blocks, lp, bl); /* use modified DFS algorithm */

#if 1
   fprintf(stderr, "\nBLOCK LIST IN EACH PATH: \n");
   for (i = 0; i < NPATH; i++)
   {
      fprintf(stderr, "PATH %d: ",i);
      for (bl = PATHS[i]->blocks; bl ; bl = bl->next)
      {
         fprintf(stderr, "%d ",bl->blk->bnum);
      }
      fprintf(stderr,"\n");
   }
#endif
}

void PrintVars(FILE *out, char* title, ushort iv)
/*
 * temporary for testing, will be moved to misc later 
 */ 
{
   short *sp;
   int i, N;
   sp = BitVec2Array(iv, 1-TNREG);
   fprintf(out, "%s : ", title);
   for (N=sp[0], i=1; i <= N; i++)
   {
      if (!STname[sp[i-1]])
         fprintf(out,"[ST NULL] ");
      fprintf(out, "%s(%d) ",STname[sp[i]-1]? STname[sp[i-1]]: "NULL",sp[i]-1);
   }
   fprintf(out, "\n");
}

void PrintPtrInfo(FILE *out, struct ptrinfo *ptrs)
/*
 * Temporary function, will be moved later
 */
{
   struct ptrinfo *pl;
   for (pl = ptrs; pl ; pl = pl->next)
   {
      fprintf(out, "ptr: %s, nupdates: %d, flag = %d\n",STname[pl->ptr-1],
              pl->nupdate, pl->flag);
   }
}

int CheckVarInBitvec(int vid, short iv)
/*
 * check whether this var is set in the bit vector,
 * return 0 otherwise.
 */
{
   short *sp;
   int i, n, N;
   extern short STderef;
/*
 * skip registers 
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);

   sp = BitVec2Array(iv, 1-TNREG);
   for (N=sp[0],n=0,i=1; i <= N; i++)
   {
      if (vid == (sp[i]-1))
      {
         free(sp);
         return(1);
      }
   }
   free(sp);
   return(0);
}

int PathFlowVectorAnalysis(LOOPPATH *path)
/*
 * Analyze single path of loop and stores all the info in paths,
 * returns error code if failed, otherwise 0. 
 * NOTE: don't stop the analysis when error is found, rather complete all
 * the analysis, save the data structure and return error code at last
 */
{
   extern short STderef;
   int errcode;
   short iv, iv1, blkvec;
   int i, j, k, n, N, vid;
   int vflag;
   LOOPQ *lp;
   BLIST *scope, *bl, *blTmp;
   ILIST *il, *ib;
   INSTQ *ip;
   char ln[1024];
   struct ptrinfo *pbase, *p;
/*
 * all these arrays store element count at 0 position.
 */
   short *sp, *s;        /* temporary ptrs */
/*
 * tempory for paths 
 */
   short *scal;         /* store all fp variable, N arr format */
   short *sflag;          /* store all moving pointers, N arr format */
   short lpflag;
/*
 * for vector analysis 
 */
   short *varrs, *vscal, *vsflag, *vsoflag ; /* either save or free mem */
/* 
 * initialize neccessary locals 
 */
   errcode = 0; /* by default vectorizable */
   lp = optloop;
   scope = path->blocks;

/*
 * Find variable accessed in the path and store it in path
 */
   iv = NewBitVec(32);
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
      for (bl=scope; bl; bl=bl->next)
      {
         path->uses = BitVecComb(path->uses, path->uses, bl->blk->uses, '|');
         path->defs = BitVecComb(path->defs, path->defs, bl->blk->defs, '|');
      }
   }
   iv = BitVecComb(iv, path->uses, path->defs, '|');
/*
 * right now, skip all the regs from uses, defs
 * NOTE: Need to check why there are redundent vars with diff ST index 
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);

#if 0
   fprintf(stderr,"Vars of path %d \n", path->pnum);
   PrintVars(stderr, "ALL VARS",iv);
#endif

/*
 * Skip all non-fp variables, valid entires upto n (included) 
 * NOTE: No action for INT var in vector but need to consider complex case 
 * with index var update later!!!
 * NOTE: As our vector path never uses/sets integer variable (except index)
 * we don't have to worry about this in Backup/Recovery stage. So, we just 
 * skip int here.
 */
   sp = BitVec2Array(iv, 1-TNREG);
   lpflag |= LP_OPT_LCONTROL; /* by default optimizable */
   for (N=sp[0],n=0,i=1; i <= N; i++)
   {
      if (IS_FP(STflag[sp[i]-1]))
         sp[n++] = sp[i];
/*
 *    For non-fp var, if it's not the index var, avoid vectorization
 */
      else if (sp[i] != lp->I && sp[i] != lp->end &&
               sp[i] != lp->inc)
      {
         fko_warn(__LINE__, "Bailing out Path%d on vect due to var %d,%s\n",
                  path->pnum,sp[i],STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
         errcode += 1;
      }
/*
 *    Use of index variable [need to check by condtion? ]
 *    NOTE: need to check the index var variable whether it is set outside
 *    of the loop control. Make sure loop control is killed before.
 */
      else 
      {
         if (CheckVarInBitvec(sp[i]-1, path->defs))
         {
            fko_warn(__LINE__,
                     "Index variable = %d is updated outside loop control\n",
                     STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
            lpflag &= ~(LP_OPT_LCONTROL);
         }
      }
   }   
   if (!n) 
   {
      fko_warn(__LINE__,"No fp var, Nothing to vectorize");
      errcode += 2;
   }
#if 0
/*
 * HERE HERE, format of sp is changed. skip the element count 
 */
   if (lpflag)
      fprintf(stderr, "Loop control optimizable\n");
   fprintf(stderr, "Vars: ");
   for (i=0; i < n ; i++)
   {
      fprintf(stderr,"%s[%d] ",STname[sp[i]-1],sp[i]-1);
   }
   fprintf(stderr, "\n");
#endif

/*
 * Make sure all the vars are of same type; will reconsider this limitation
 * later
 */
   j = FLAG2TYPE(STflag[sp[0]-1]); /* this is element not count now*/
   for (i=1; i < n; i++)
   {
      if (FLAG2TYPE(STflag[sp[i]-1]) != j)
      {
         fko_warn(__LINE__,
               "Mixed type %d(%s), %d(%s) prevents vectorization!!\n\n",
               j, STname[sp[0]-1] ? STname[sp[0]-1] : "NULL",
               FLAG2TYPE(sp[i]-1), STname[sp[i]-1] ? STname[sp[i]-1]
               : "NULL");
         errcode += 4;
         break;
      }
   }
/*
 * Stored the required type for vectorization, may need later
 */
   if (i == n )
   {
      if (j == T_FLOAT)
         vflag = T_VFLOAT;
      else if (j == T_DOUBLE)
         vflag = T_VDOUBLE;
   }

/*
 * Moving pointer analysis for path
 */
   pbase = FindMovingPointers(scope);
   lpflag |= LP_OPT_MOVPTR;      /* by default optimizable*/
   for (N=0,p = pbase; p; p = p->next)
   {
      if (IS_FP(STflag[p->ptr-1]))
      {
         N++;
         if ((p->flag | PTRF_CONTIG | PTRF_INC) != p->flag)
         {
            fko_warn(__LINE__, "Ptr movement prevents vectorization!!\n");
            errcode += 8;
            lpflag &= ~LP_OPT_MOVPTR;
         }
      }
      if (p->nupdate > 1)
      {
         fko_warn(__LINE__, "Multiple ptr updates prevent vectorization!!\n");
         errcode += 16;
         lpflag &= ~LP_OPT_MOVPTR;
      }
      if (!(p->flag & PTRF_CONTIG))
      {
         fko_warn(__LINE__,
                  "Non-contiguous ptr updates prevent vectorization!!\n");
         errcode += 32;
         lpflag &= ~LP_OPT_MOVPTR;
      }
   }
/*
 * Copy all moving pointers to varrs
 */
   s = malloc(sizeof(short)*(N+1));
   assert(s);
   s[0] = N;
   for (j=0,i=1,p=pbase; p; p = p->next)
      if (IS_FP(STflag[p->ptr-1]))
         s[i++] = p->ptr;
   varrs = s; /* do we need varrs later??? */

#if 0
   fprintf(stderr,"MOV PTR: ");
   for (i=1, j=varrs[0]; i <=j; i++)
      fprintf(stderr,"%s[%d] ",STname[varrs[i]-1],varrs[i]-1);
   fprintf(stderr,"\n");
#endif

/*
 * Remove the moving arrays from scalar vals.
 * NOTE: array ptr which not changed, considered as scal
 * FIXME: number of vscal is not correct always: corrected the condition 
 */
#if 0
   for (i=0; i <= n; i++)
   {
      fprintf(stderr, "sp = %d, s = %d\n",sp[i], s[i]);
   }
#endif 
   for (k=0,i=1; i < n; i++) /* BUG: why n is included??? changed to < n */
   {
      for (j=1; j <= N && s[j] != sp[i]; j++);
      if (j > N)
      {
         sp[k++] = sp[i]; /*sp elem starts with 0 pos*/
      }
   }
   n = k;   /* n is k+1 */

/*
 * Store the scals for path->scals. we will update the flags later. 
 */
   scal = malloc(sizeof(short)*(n+1));
   sflag = calloc(n+1,sizeof(short)); /* initialize by 0 */
   assert(scal && sflag);
   scal[0] = sflag[0] = n;
   for (i=1; i <= n; i++)
      scal[i] = sp[i-1];
/*
 * Set flags for those variables which are used, 
 * flags wil be set for for defs later along with vscal analysis
 */
   for (i=1; i <=n; i++)
   {
      if (CheckVarInBitvec(scal[i]-1, path->uses))
         sflag[i] |= SC_USE;
   }
   
/*
 * Here start the analysis for vector scalars 
 * copy for the vector analysis  
 */
   vscal = malloc(sizeof(short)*(n+1)); 
   assert(vscal);
/*
 * save scalar vars and flags 
 * n = number of scal vars
 */
   if (n)
   {
      vsflag = calloc(n+1, sizeof(short));
      vsoflag = calloc(n+1, sizeof(short));
      assert(vsflag && vsoflag);
   }
   vscal[0] = vsflag[0] = vsoflag[0] = n;
   for (i=1; i <= n; i++)
      vscal[i] = sp[i-1];

   free(sp);
   sp = vscal+1;
   
#if 0 
   fprintf(stderr, "Scal Vars: ");
   for (i=1, N=vscal[0]; i <= N ; i++)
   {
      fprintf(stderr,"%s[%d]",STname[vscal[i]-1],vscal[i]-1);
   }
   fprintf(stderr,"\n");
#endif

/*
 * Sort scalar vals into livein,liveout, and tmp
 */
   if (n)
   {
/*
 *    Find fp scalars set inside path
 */
      iv1 = Array2BitVec(n, sp, TNREG-1);
      SetVecAll(iv, 0);
      for (bl=scope; bl; bl = bl->next)
      {
         for (ip=bl->blk->ainst1; ip; ip = ip->next)
            if (ACTIVE_INST(ip->inst[0]))
               BitVecComb(iv, iv, ip->set, '|');
      }
      BitVecComb(iv, iv, iv1, '&');  /* filter out */
      s = BitVec2StaticArray(iv);
      for (i=1; i <= s[0]; i++)
      {
         k = s[i] - TNREG + 1;
         for (j=0; j < n; j++)
         {
            if (sp[j] == k)
            {
               vsflag[j+1] |= VS_SET;
               sflag[j+1] |= SC_SET;
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
               vsflag[j+1] |= VS_LIVEIN;
               sflag[j+1] |=SC_LIVEIN;
               break;
            }
         }
      }
/*
 *    Find vars live on loop exit, that are also accessed in post-tail
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
      BitVecComb(iv, iv, iv1, '&');     /* filter out for this path*/
      s =  BitVec2StaticArray(iv);
      for (i=1; i <= s[0]; i++)
      {
         k = s[i] - TNREG + 1;
         for (j=0; j < n; j++)
         {
            if (sp[j] == k)
            {
               vsflag[j+1] |= VS_LIVEOUT;
               sflag[j+1] |= SC_LIVEOUT;
               break;
            }
         }
      }
/*
 *    convert blocks of scope into bvec
 */
      blTmp = scope; /* need to copy pointer as the function changes pointer */
      blkvec = BlockList2BitVec(blTmp);
/*
 *    Find out how to init livein and reduce liveout vars
 */
      s = vsflag+1;
      for (i=0; i < n; i++)
      {
/*
 *       For livein variables, any access is legal, but only handled in 2 ways:
 *       If adder, init one val to 0, others to init val, if anything else
 *       (MUL or assignment), init all vals to same
 *
 */
         if (s[i] & VS_LIVEIN) /* skip private variable here */
         {
            j = FindReadUseType(lp->header->inst1, sp[i], blkvec);
            if (j == VS_ACC)
            {
               s[i] |= VS_ACC;
               sflag[i+1] |= SC_IACC;
            }
            else if (j == VS_MUL)
            {
               s[i] |= VS_MUL;
               sflag[i+1] != SC_IMUL;
            }
            else
            {
/*
 *             HERE HERE. Is there other operator we need to check?? 
 *             check the func FIndReadUseType!!!
 */
               fprintf(stderr, "j=%d, ACC=%d,MUL=%d\n", j, VS_ACC, VS_MUL);
               fko_warn(__LINE__,
                        "Mixed use of var %d(%s) prevents vectorization!!\n\n",
                        sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
               errcode += 64;
               sflag[i+1] |= SC_IMIXED;
            }
         }
/*
 *       Output scalars must be accumulators to vectorize
 */
         else if (s[i] & VS_LIVEOUT)
         {
            SetVecAll(iv, 0);
            ib = FindPrevStore(lp->posttails->blk->inst1, sp[i],blkvec, iv);
            j = 0;
            vsoflag[i+1] |= VS_ACC;
            /* by default all set, will clear accordingly */
            sflag[i+1] |= SC_OACC;
            sflag[i+1] |= SC_OMUL;
            sflag[i+1] |= SC_OMIXED;
            for (il=ib; il; il = il->next)
            {
               if (ip->prev->inst[0] == FADD || ip->prev->inst[0] == FADDD)
               {
                  sflag[i+1] &= ~SC_OMUL;
               }
               else if (ip->prev->inst[0] == FMUL || ip->prev->inst[0] == FMULD)
               {
                  fko_warn(__LINE__,
                "Non-add use of output var %d(%s) prevents vectorization!!\n\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode += 128;
                  vsoflag[i+1] &= ~VS_ACC;
                  sflag[i+1] &= ~SC_OACC;
               }
               else
               {
                  fko_warn(__LINE__,
                "Non-add use of output var %d(%s) prevents vectorization!!\n\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode += 128;
                  vsoflag[i+1] &= ~VS_ACC;
                  sflag[i+1] &= ~SC_OACC;
                  sflag[i+1] &= ~SC_OMUL;
               }
            }
            KillIlist(ib);
         }
         else /* Only Private variables are not livein and liveout */
         {
            sflag[i+1] |= SC_PRIVATE;
         }
      }
   }
/*
 * Update path data structure 
 */
   if (!errcode)
   {
      lpflag |= LP_VEC;
      path->lpflag = lpflag;
      path->ptrs = pbase;
      path->scal = scal;
      path->sflag = sflag;
/*
 *    update data for vector path
 *    NOTE: VVSCAL is updated when vector path is finalized 
 */
      path->vflag = vflag;
      path->varrs = varrs;
      path->vscal = vscal;
      path->vsflag = vsflag;
      path->vsoflag = vsoflag;
   }
   else /* Scalar Path */
   {
      path->lpflag = lpflag;
      path->ptrs = pbase;
      path->scal = scal;
      path->sflag = sflag;
      path->vflag = 0;
/*
 *    free all data for vector path 
 */
      free(varrs);
      free(vscal);
      free(vsflag);
      free(vsoflag);
   }

/*
 * Time to check all the information
 */
#if 1
   fprintf(stderr, "PATH = %d \n", path->pnum);
   fprintf(stderr, "--------------\n");
   fprintf(stderr, "Control Flag : %d\n", path->lpflag);
   fprintf(stderr, "SCALARS(FLAG) : ");
   for (N=scal[0],i=1; i <= N; i++)
   {
      //fprintf(stderr, "%s(%d) ",STname[scal[i]-1],sflag[i]);
      fprintf(stderr, "%s(%d) ",STname[path->scal[i]-1],path->sflag[i]);
   }
   fprintf(stderr, "\n");
#endif 

return errcode;
}

int SpeculativeVectorAnalysis()
/*
 * Duplicated the optloop blks, do analysis with this. 
 */
{
   int i, j, n, k, N;
   LOOPPATH *vpath;
   LOOPQ *lp;
   short *sp, *sc, *sf;
   char ln[512];

   lp = optloop;
/*
 * Find paths from optloop
 */
   CalcInsOuts(bbbase);
   CalcAllDeadVariables();
   FindLoopPaths(lp);
#if 0
   fprintf(stdout, "Symbol Table \n");
   PrintST(stdout);
#endif

/*
 * NOTE: Loop control can always be killed assuming optloop is always
 * contructed by loop statement in HIL
 */
   KillLoopControl(lp);
   CalcInsOuts(bbbase);
   CalcAllDeadVariables();

/*
 * apply analysis for each path
 * NOTE: Analysis needs to be performed on original blocks (not on duplicated)
 * because we will need the data flow anlysis (uses/defs,livein/liveout) also.
 */
   assert(PATHS);
   for (i=0; i < NPATH; i++)
   {
      PathFlowVectorAnalysis(PATHS[i]);
   }
#if 0
   fprintf(stderr, "\nFigure out all vars in each path\n");
   fprintf(stderr, "================================\n");

   for (i = 0; i < NPATH; i++)
   {
      fprintf(stderr, "PATH : %d\n", i);
      fprintf(stderr, "Control Flag: %d\n", PATHS[i]->lpflag);
      fprintf(stderr, "FP SCALAR (FLAG) : ");
      sc = PATHS[i]->scal;
      sf = PATHS[i]->sflag;
      for (j=1, N=sc[0]; j <= N; j++ )
      {
         fprintf(stderr,"%s(%d) ",STname[sc[j]-1], sf[j]);
      }
      fprintf(stderr,"\n");
   }
#endif
/*
 * Select path for vectorization. Currently, select first available vec path
 * Later we may consider special logic to minimize complexity 
 */
   for (i=0; i < NPATH; i++)
   {
      if (PATHS[i]->lpflag & LP_VEC) /* first vectorizable path */
      {
         VPATH = i;
         break;
      }
   }
   if (VPATH != -1)
   {
/*
 *    Update optloop with the vector path 
 */
      vpath = PATHS[VPATH];
      n = vpath->varrs[0];
      lp->varrs = malloc(sizeof(short)*(n+1));
      assert(lp->varrs);
      for (i=0; i <=n; i++)
      {
         lp->varrs[i] = vpath->varrs[i];
      }

      n = vpath->vscal[0];
      lp->vscal = malloc(sizeof(short)*(n+1));
      lp->vvscal = malloc(sizeof(short)*(n+1));
      lp->vsflag = malloc(sizeof(short)*(n+1));
      lp->vsoflag = malloc(sizeof(short)*(n+1));
      assert(lp->vscal && lp->vvscal && lp->vsflag && lp->vsoflag);
      for (i=0; i <= n; i++)
      {
         lp->vscal[i] = vpath->vscal[i];
         lp->vsflag[i] = vpath->vsflag[i];
         lp->vsoflag[i] = vpath->vsoflag[i];
      }
      /*
       * Create vector local for all vector scalars in loop
       */
      lp->vflag = vpath->vflag;
      k = LOCAL_BIT | FLAG2TYPE(vpath->vflag);
      lp->vvscal[0] = n;
      sp = vpath->vscal + 1;
      for (i=0; i < n; i++)
      {
         sprintf(ln, "_V%d_%s", i-1, STname[sp[i]-1] ? STname[sp[i]-1] : "");
         j = lp->vvscal[i+1] = STdef(ln, k, 0);
         SToff[j-1].sa[2] = AddDerefEntry(-REG_SP, j, -j, 0, j);
      }
      /*
       * Save fko
       */
      SaveFKOState(1);
      return(0);
   }
   else
   {
      return(1);
   }
}

int SpeculativeVecTransform(LOOPQ *lp)
/*
 * transform Vector in single path which is saved globaly as vect path. Must 
 * call before scalar Restart. Scalar Restart will messed the optloop up.
 * assume analysis updated the optloop according to the vect path only
 */
{
   short *sp;
   BLIST *bl, *scope;
/*
 * Need to add vector conditional cmp for branchs like: JEQ, JNE, JLT, JGT, JGE 
 * Corresponding vector cmp of br for single precision: 
 * VFCMPEQW, VFCMPNEW, VFCMPNLTW, VFCMPGTW, VFCMPGEW
 * For double precision:
 * VDCMPEQW, VDCMPNEW, VDCMPNLTW, VDCMPGTW, VDCMPGEW
 * NOTE: Vec CMP changes the destination vector register, 
 * 
 */
   static enum inst
      sfinsts[]= {FLD,  FST,  FMUL,  FMAC, FADD,  FSUB,  FABS,  FMOV,  FZERO},
      vfinsts[]= {VFLD, VFST, VFMUL, VFMAC, VFADD, VFSUB, VFABS, VFMOV, VFZERO},
      sdinsts[]= {FLDD, FSTD, FMULD, FMACD, FADDD, FSUBD, FABSD, FMOVD, FZEROD},
      vdinsts[]= {VDLD, VDST, VDMUL, VDMAC, VDADD, VDSUB, VDABS, VDMOV, VDZERO};
/*
 * for vector cmp... temporary solution for siamax using JGT
 * need to generalized later
 */
   static enum inst
      brinsts[] = {JEQ, JNE, JLT, JGT, JGE},
      #if defined(AVX)
         /*vfcmpinsts[] = {VFCMPEQW, VFCMPNEW, VFCMPLTW, VFCMPGTW, VFCMPGEW},*/
         vfcmpinsts[] = {VFCMPEQW, VFCMPNEW, VFCMPLTW, VFCMPLEW, VFCMPGEW},
         /*vdcmpinsts[] = {VDCMPEQW, VDCMPNEW, VDCMPLTW, VDCMPGTW, VDCMPGEW};*/
         vdcmpinsts[] = {VDCMPEQW, VDCMPNEW, VDCMPLTW, VDCMPLEW, VDCMPGEW};
      #else
         vfcmpinsts[] = {VFCMPEQW, VFCMPNEW, VFCMPLTW, VFCMPNLEW, VFCMPNLTW},
         vdcmpinsts[] = {VDCMPEQW, VDCMPNEW, VDCMPLTW, VDCMPNLEW, VDCMPNLTW};
      #endif
   const int nbr=5;

   const int nvinst=9;
   enum inst sld, vld, sst, vst, smul, vmul, smac, vmac, sadd, vadd, ssub, vsub,
             sabs, vabs, smov, vmov, szero, vzero, inst, binst, mskinst;
   short r0, r1, op, ir, vrd;
   enum inst *sinst, *vinst, *vcmpinst;
   int i, j, n, k, m, mskval, nfr=0;
   char ln[512];
   struct ptrinfo *pi0, *pi;
   INSTQ *ip, *ippu, *iph, *iptp, *iptn;
   short vlen;
   enum inst vsld, vsst, vshuf;
   short sregs[TNFR], vregs[TNFR];

/*
 * Need at least one path to vectorize
 */
   assert(VPATH!=-1);
   scope = PATHS[VPATH]->blocks;

/*
 * Figure out what type of insts to translate
 */
   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
      sinst = sfinsts;
      vinst = vfinsts;
      vcmpinst = vfcmpinsts;
      #if defined(X86) && defined(AVX)
         vlen = 8;
      #else
         vlen = 4;
      #endif
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
      vcmpinst = vdcmpinsts;
      vld = VDLD;
      #if defined(X86) && defined(AVX)
         vlen = 4;
      #else
         vlen = 2;
      #endif
      vst = VDST;
   }
   r0 = GetReg(FLAG2TYPE(lp->vflag));
   r1 = GetReg(FLAG2TYPE(lp->vflag));

/*
 * If Loop control is already not killed, kill all controls in optloop 
 * We will put back the appropriate loop control at the end of the function.
 */
   KillLoopControl(lp);
#if 0
/*
 * Generate scalar cleanup loop before simdifying loop
 */
   GenCleanupLoop(lp);
#endif

/*
 * Find all pointer updates, and remove them from vector path (leaving only
 * vectorized instructions for analysis); will put them back in loop after
 * vectorization is done.
 */

   pi0 = FindMovingPointers(scope);
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
            #if defined(X86) && defined(AVX)
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1,
                                 -r0, STiconstlookup(0x3276));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VDADD,-r0,-r0,
                                 -r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1,
                                 -r0, STiconstlookup(0x3715));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VDADD,-r0,-r0,
                                 -r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSTS,
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #else
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1,
                                 -r0, STiconstlookup(0x33));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VDADD,-r0,-r0,
                                 -r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSTS,
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #endif
         }
         else
         {
            #if defined(X86) && defined(AVX)
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x7654FEDC));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x765432BA));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x76CD3289));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSTS,
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #else
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x3276));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x5555));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL,VFADD,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSTS,
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #endif
         }
      }
   }
   GetReg(-1);
   iptp = InsNewInst(lp->posttails->blk, iptp, NULL, CMPFLAG, CF_VRED_END,
                     0, 0);
/*
 * Translate body of loop
 */
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         inst = GET_INST(ip->inst[0]);
         if (ACTIVE_INST(inst))
         {
/*
 *          check for FCMP/FCMPD instruction so that we can replace it
 *          with vector cmp. 
 */
            if (inst == FCMP || inst == FCMPD)
            {
/*
 *             Select appropriate vector cmp inst
 */
               if (inst == FCMP)
               {
                  vcmpinst = vfcmpinsts;
                  mskinst = VFSBTI;
               }
               else
               {
                  vcmpinst = vdcmpinsts;
                  mskinst = VDSBTI;
               }
/*
 *             check for the next inst. Right now assume the next instruction
 *             would always be one of the conditional branch
 */
               binst = ip->next->inst[0]; /* assume next ainst alwasy branch*/
/*
 *             find ioptsimdndex of appropriate branch and VCMP
 */
               for (m = 0; m < nbr; m++)
               {
                  if (brinsts[m] == binst)
                     break;
               }
               assert(m!=nbr); /* there must be a branch */
/*
 *             insert new instructions replacing old FCMP and BR insts.
 *             NOTE: Those two old instructions use two fregs/dregs (fcc, icc
 *             and pc), but new insts would use 3 vfregs/vdregs and 1 iregs (icc
 *             and pc) where 1 vfreg/vdreg and ireg would be overwritten.
 *             
 *             NOTE: We reserve  new ireg and vreg here, these regs are only
 *             used as destination. So, we can reuse later. Need to free those.
 *             I avoid that here right now. We consider this later.
 */
               ip->inst[0] = vcmpinst[m];
/*
 *             get new vregs for the destination, no vld as it is not uses but
 *             sets
 */
               vrd = GetReg(FLAG2TYPE(lp->vflag));
               ip->inst[1] = -vrd;
/*
 *             Find appropriate vector source regs for VCMP, here assume all the
 *             regs are of same type set by vflag, later relax this assumption
 */
               for (j = 2; j < 4; j++)
               {
                  op = ip->inst[j];
                  assert(op);
                  if ( op < 0)
                  {
                     op = -op;
                     k = FindInShortList(nfr,sregs, op);
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
/*
 *             it's time to add other instruction like mask and test
 *             ireg is set, so, get a new ireg
 */
               ir = GetReg(T_INT);
               ip = InsNewInst(bl->blk, ip, NULL, mskinst, -ir, -vrd, 0);
/*
 *             add test the iregs with const val populated by vec len
 */
               mskval = 1;
               for (i = 1; i < vlen; i++)
                  mskval = (mskval << 1) | 1;
               ip = InsNewInst(bl->blk, ip, NULL, CMP, -ICC0, -ir, 
                               STiconstlookup(mskval)) ;
/*             ip = InsNewInst(bl->blk, ip, NULL, CMPAND, -ir, -ir, 
                               STiconstLoopup(mskval)) ; */
/*
 *             Change the branch in next inst as a JEQ
 *             NOTE: changed the logic.. need to check later
 */
               /*ip->next->inst[0] = JEQ;*/
               ip->next->inst[0] = JNE;    
            }
            else /* changing other scalar inst to vector inst */
            {
               for (i=0; i < nvinst; i++)
               {
/*
 *                If the inst is scalar fp, translate to vector fp, and insist
 *                all fp ops become vector ops
 */
                  if (sinst[i] == inst)
                  {
                     ip->inst[0] = vinst[i];
/*
 *                   Change scalar ops to vector ops
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
                           if (IS_DEREF(STflag[op-1]))
                           {
                              k = STpts2[op-1];
                              if (!FindInShortList(lp->varrs[0],lp->varrs+1,k))
                              {
                                 k = FindInShortList(lp->vscal[0],lp->vscal+1,
                                                     k);
                                 assert(k);
                                 ip->inst[j] = SToff[lp->vvscal[k]-1].sa[2];
                                 assert(ip->inst[j] > 0);
                              }
                           }
                           else if (!FindInShortList(lp->varrs[0],lp->varrs+1,
                                    op))
                           {
                              k = FindInShortList(lp->vscal[0], lp->vscal+1,
                                                  op);
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

void AddBackupRecovery(BLIST *scope, LOOPQ *lp)
/*
 * Right now, we fill focus on only iamax, later generalize
 * add recovery vector inst in scalar restart
 */
{
   BLIST *bl;
   INSTQ *ip;
   int i, j, k, N;
   short sc, sf, vs;
   short vlen;
   LOOPPATH *vp, *path;
   BBLOCK *bp;
   enum inst vsld, vshuf, vst, fst;
   short r0;

   vp = PATHS[VPATH];
   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
      vsld = VFLDS;
      vshuf = VFSHUF;
      vst = VFST;
      fst = FST;     /* to check the recovery vars */
      #if defined(X86) && defined(AVX)
         vlen = 8;
      #else
         vlen = 4;
      #endif
   }
   else
   {
      vsld = VDLDS;
      vshuf = VDSHUF;
      vst = VDST;
      fst = FSTD;
      #if defined(X86) && defined(AVX)
         vlen = 4;
      #else
         vlen = 2;
      #endif
   }
#if 0
   fprintf(stderr, "\nFigure out all vars in each path\n");
   fprintf(stderr, "================================\n");

   for (i = 0; i < NPATH; i++)
   {
      fprintf(stderr, "PATH : %d\n", i);
      fprintf(stderr, "Control Flag: %d\n", PATHS[i]->lpflag);
      fprintf(stderr, "FP SCALAR (FLAG) : ");
      sc = PATHS[i]->scal;
      sf = PATHS[i]->sflag;
      for (j=1, N=sc[0]; j <= N; j++ )
      {
         fprintf(stderr,"%s(%d) ",STname[sc[j]-1], sf[j]);
      }
      fprintf(stderr,"\n");
   }
#endif
   
/*
 * Check all the scalar variables of vector path, those may need backup
 * recovery. vars which accessed in scalar path only, need no recovery.
 */

   for (i=1, N=vp->scal[0]; i <= N; i++)
   {
      sc = vp->scal[i]; /* scal and vscal are same here */
      vs = vp->vscal[i];
      sf = vp->sflag[i];
/*
 *    if it is private variable, nothing to do.
 */
      if (sf & SC_PRIVATE )
         continue;
/*
 *    simple case first: this scalar is used in vector path as vscal but 
 *    not set in this path like: amax in iamax.
 *
 *    NOTE: vscal and scal are the same for vector path as we skip integer and
 *    moving array pointer from scal. Right now, our vector path doesn't have 
 *    any integer vars (except index) and moving array pointer just have one
 *    update at most (which will be moved to the tail of loop).
 */
      if ((sf & SC_USE) && !(sf & SC_SET))
      {
/*
 *       check for all scalar paths whether this var is set.
 *       Simple Transformation: set corresponding vscal after that inst.
 *       Later can be optimized by putting this set at the last once, if 
 *       possible.
 */
         for (j=0; j < NPATH; j++)
         {
            if (j == VPATH) continue; /* skip vector path */
            path = PATHS[j];
/*
 *          NOTE: Need to count blocks once if there are some common blocks in
 *          several scalar paths. right at this point, assume no common path.
 *          NOTE: blocks common with vector path doesn't set this variable, as 
 *          we checked that first. so, don't worry check those blocks.
 */
            for (bl=path->blocks; bl; bl=bl->next)
            {
               bp = FindBlockInListByNumber(scope, bl->blk->bnum);
/*
 *             HERE HERE, need to find where the scalar is set, but dupblks
 *             doesn't have set/use info. So, we rely on the HIL format, 
 *             checking for destination of FST. 
 *             NOTE: for vector path, scal and vscal represent same vars, but
 *             vflag and sflag are different.
 *             optloop has vvscal for vscal!
 */            
               for (ip = bp->inst1; ip; ip=ip->next)
               {
                  if (ip->inst[0] == fst && ip->inst[1] == SToff[vs-1].sa[2])
                  {
                     r0 = GetReg(FLAG2TYPE(lp->vflag));
                     ip = PrintComment(bp, ip, NULL,
                     "Vector Recovery of %s", STname[vs-1]);
                     ip = InsNewInst(bp, ip, NULL, vsld, -r0,
                                     SToff[vs-1].sa[2], 0);
                     ip = InsNewInst(bp, ip, NULL, vshuf, -r0, -r0, 
                                     STiconstlookup(0));
/*                   uses optloop's vvscal */                     
                     /*ip = InsNewInst(lp->preheader, ip, NULL, vst,
                                     SToff[lp->vvscal[i]-1].sa[2], -r0, 0);*/
                     ip = InsNewInst(bp, ip, NULL, vst,
                                     SToff[lp->vvscal[i]-1].sa[2], -r0, 0);
                     GetReg(-1);
                  }
/*
 *                Find all index ref and update that with vlen-1
 *                NOTE: need to skip loop update part, will remove the loop 
 *                update at the last.
 */
                  if (ip->inst[0] == LD && ip->inst[2] == SToff[lp->I-1].sa[2]
                      && !(ip->prev->inst[0] == CMPFLAG && ip->prev->inst[1] ==
                           CF_LOOP_UPDATE))
                  {
                     k = ip->inst[1];
                     InsNewInst(bp, ip, NULL, ADD, k, k, 
                                STiconstlookup(vlen-1) );
                  }
               }
            }
         }
      }
/*
 *    if the var is only set in vpath and not used, we need to look for the
 *    use and set in scalar path. 
 *    NOTE: not consider this yet, need not for iamax.
 */
      else if (!(sf & SC_USE) && (sf & SC_SET))
      {
      }
/*
 *    vars which is neither sets nor uses cannot be in scal. so, the only
 *    option left is both SET and USE. In that case, check whether it is a
 *    reduction variable!
 *    NOTE: not implemented yet as need not for iamax
 */
      else
      {

      }
   }
   
}

void ScalarRestart(LOOPQ *lp)
{
   BBLOCK *bp0, *bp, *newCF, *bdown;
   BLIST *bl, *vbl, *dupblks, *ftheads, *delblks;
   INSTQ *ip, *ip0;
   int i, j, cflag, vlen;
   int iv, ivtails;
   char ln[512];
   extern BBLOCK *bbbase;
   extern int FKO_BVTMP;
   
/*
 * Need at least one path to vectorize
 */
   assert(VPATH!=-1);
/*
 * Find the header of loop in the bbbase
 * NOTE: after cleanup, blocks have duplicate names. don't check anything
 * by bnum.
 */
   for (bp0 = bbbase; bp0; bp0 = bp0->down )
   {
      if (bp0 == lp->header)
         break;
   }
/*
 * Find the block where we need to implement Scalar Restart stage
 * from head to tail where the two pathfirst split.
 * NOTE: we need to implement Scalar Restart in each path except the vect path.
 */
   for (i = 0; i < NPATH; i++)
   {
/*
 *    Vector path needs no scalar restart
 */
      if (i == VPATH)
         continue;
/*
 *    check with vector path, determine where to implement scalar restart
 */
      for (bl = PATHS[i]->blocks,vbl = PATHS[VPATH]->blocks; bl && vbl ;
           bl = bl->next, vbl = vbl->next)
      {
         if (bl->blk != vbl->blk)
         {
            fprintf(stderr, "Split in vpath blk = %d, spath blk = %d\n",
                    vbl->blk->bnum, bl->blk->bnum);
            break;
         }
      }
      assert(bl); /* atleast one block which is not common */
/*
 *    Add duplicated blocks after this block, will delete this path later
 */
      for ( ; bp0 != bl->blk; bp0 = bp0->down);
      assert(bp0);
/*
 *    Find all fall thru path headers in loop, copy it after the new block
 */
/*      FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec); */
      iv = FKO_BVTMP;
      SetVecAll(iv, 0);
      ivtails = BlockList2BitVec(lp->tails);
      ftheads = FindAllFallHeads(NULL, lp->blkvec, lp->header, ivtails, iv);
      ftheads = ReverseBlockList(ftheads);
/*
 *    NOTE, to avoid the nested loop, we need to copy the loop blks vec len
 *    times. but need to update the dup tails also.
 *    Right now, test with fixed length = 4, later changed it by vec length
 *    times
 */
      bdown = bp0->down;
      vlen = Type2Vlen(lp->vflag);
      for (j = 0; j < vlen ; j++)
      {
/*
 *       Need to duplicate the loop blocks before updating the blocks for
 *       scalar restart. 
 *       NOTE: header needs to be isolated from blks to use dup function.
 *       Need to iterate the Dup functions with diff label value: 0 is for
 *       cleanup.
 *       NOTE: all these codes for dupblks run vlen times, only Labels are
 *       changes. Start from 1 as 0 is used for cleanup for label
 */
         SetVecBit(lp->blkvec, lp->header->bnum-1,0);
         FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
         newCF = DupCFScope(lp->blkvec, iv, i*4+j+1, lp->header);
         assert(newCF->ilab);
         SetVecBit(lp->blkvec, lp->header->bnum-1,1);
         iv = BitVecCopy(iv, lp->blkvec);
         dupblks = CF2BlockList(NULL, iv, newCF);
/*
 *       Loop blocks are duplicated. Now add backup recovery operation.
 *       Right now, add instructions whenever it requires. Later, we can
 *       optimize it by putting those all together at the end (but keep in mind
 *       that we need to do it before loop control and ptr movement update)
 */
         AddBackupRecovery(dupblks, lp);
/*
 *       Need to kill backedge (branch inst from tail of loop), add JMP to 
 *       the start of next scalar iteration.for last iteration, need to create
 *       a block and check condition for loop.
 *       NOTE: CFG is completely messedup but don't update the CFG yet, 
 *       should update it after vect transform.
 */
         for (bl=lp->tails; bl; bl=bl->next)
         {
            bp = FindBlockInListByNumber(dupblks,bl->blk->bnum);
            assert(bp);
/*          search for the branch instruction */
            for (ip = bp->inst1; ip; ip=ip->next)
            {
               if (IS_BRANCH(ip->inst[0]))
               {
                  ip = DelInst(ip); /* recalc the inst1, instN*/
                  break;
               }
            }
/*          Add new jump to 1st block of next iteration  */
            if (ip)
            {
               sprintf(ln, "_S_%d_%d",i,j+1); /*1st block of next */
               cflag = STlabellookup(ln);
               InsNewInst(bp, NULL,ip,JMP,-PCREG,cflag,0);
            }
            else
            {
               ip = bp->instN; /*CMPFLAG isn't considered as active inst*/
               assert(ip);
               sprintf(ln, "_S_%d_%d",i,j+1); /*1st block of next */
               cflag = STlabellookup(ln);
               InsNewInst(bp, ip,NULL,JMP,-PCREG,cflag,0);
            }
            
         }
/*
 *       create new block after bp0
 */
         bp = NewBasicBlock(bp0, NULL);
         bp0->down = bp;
         bp0 = bp;
         sprintf(ln, "_S_%d_%d",i,j);
         cflag = STlabellookup(ln);
         InsNewInst(bp, NULL, NULL, LABEL, cflag,0,0);

         for (bl=ftheads; bl; bl = bl->next)
         {
            for (bp=bl->blk;bp; bp = bp->down)
            {
               if (!BitVecCheck(lp->blkvec, bp->bnum-1))
                  break;
               bp0->down = FindBlockInListByNumber(dupblks, bp->bnum);
               bp0->down->up = bp0;
               bp0 = bp0->down;
               if (BitVecCheck(ivtails, bp->bnum-1))
                  break;
            }
            bp0->down = NULL;
         }
/*
 *       need to add block of the tails of dup blocks to the loop header
 */
      }
/*
 *    create a new block at the last of scalar restart path to check and
 *    jump back to lp->header or, lp->posttail (vector reduction)
 */
      bp = NewBasicBlock(bp0, NULL);
      bp0->down = bp;
      bp->up = bp0;
      bp0 = bp;
      sprintf(ln, "_S_%d_%d",i,j);
      cflag = STlabellookup(ln);
      InsNewInst(bp, NULL, NULL, LABEL, cflag,0,0);
/*
 *    Creating two posttails would complicate the analysis later. So, need to
 *    skip that by intelligent design.
 */
#if 0
/*
 *    Need to add loop test here. assume that loop test codes are on lp->tails
 *    from CF_LOOP_TEST to CF_LOOP_END
 *    NOTE: May have only branch inst. do we need reg laod/st?
 */
      ip0 = bp->ainstN;
      ip = FindCompilerFlag(lp->tails->blk, CF_LOOP_TEST);   
      assert(ip);
      ip = ip->next;
/*    condition: de morgan's law */      
      while (ip && (ip->inst[0] != CMPFLAG || ip->inst[1] != CF_LOOP_END))
      {
         InsNewInst(bp, ip0, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                    ip->inst[3]);
         ip0 = ip0->next;
         ip = ip->next;
      }

/*
 *    Add jump to post tail of loop. only one posttail, posttail should have
 *    a label
 */
      assert(lp->posttails->blk && !lp->posttails->next && 
             lp->posttails->blk->ilab);
      InsNewInst(bp,ip0,NULL,JMP,-PCREG,lp->posttails->blk->ilab,0);
#else
/*
 *    To avoid multiple posttails (and also violation of loop normal form), 
 *    the original loop tail is splited into 2 parts here. We will jump back
 *    to the loop test from here. 
 *    NOTE: LOOP update and loop test is become isolated now. Need to check 
 *    whether it creates problem later. Need to make sure that the condition
 *    codes (status register) also don't changed by this time.
 *    
 *    HERE HERE, 
 *    We can do better: we can check all paths whether we can apply 
 *    loop control and ptr update optimization. I keep that info in
 *    path->lpflag. if it is true, we can kill all the loop comtrol and 
 *    ptr updates and jump back to original loop tail.
 *
 */
      ip0 = bp->instN; /* aisnt not considered CMPFLAG and COMMENT*/
/*
 *    split tail by adding label to the tail of original loop.
 *    NOTE: add label before CMPFLAG
 */   assert(lp->tails->blk && !lp->tails->next);
      ip = FindCompilerFlag(lp->tails->blk, CF_LOOP_TEST);
      assert(ip);
      sprintf(ln,"%s_LOOP_TEST",STname[lp->body_label-1]);
      cflag = STlabellookup(ln);
      InsNewInst(lp->tails->blk, NULL, ip, LABEL, cflag,0,0);
/*
 *    now jump back to new label in tail
 */
      InsNewInst(bp, ip0, NULL, JMP, -PCREG, cflag, 0);

#endif
/*    
 *    Link down to the cleanup code
 */
      bp0->down = bdown; /* need to correct the up link!*/
      bdown->up = bp0;
   }
/* Need to delete the inst of first block of scalar restart and delete the
 * blks which are not used by vect path or yet to come paths. This should be
 * done after the duplication, otherwise the original loop blks change.
 * NOTE: need to delete all blocks upto tail(if tail diff then including tail)
 * need to change the optloop info after that. Be sure that original loop
 * format is not needed anywhere else after this function.
 */
   for (i = 0; i < NPATH; i++)
   {
      if (i == VPATH) continue;
      for (bl = PATHS[i]->blocks,vbl = PATHS[VPATH]->blocks; bl && vbl ;
           bl = bl->next, vbl = vbl->next)
      {
         if (bl->blk != vbl->blk)
            break;

      }
      assert(bl); /* atleast one block which is not common */
/*
 *    mark the scalar restart block with a CMPFLAG, if first inst is label
 *    insert flag after that. Remove all inst of that block.
 *    HERE HERE. We need to delete all instructions upto tail block
 */
      ip = InsNewInstAfterLabel(bl->blk, CMPFLAG, CF_SCAL_RES, 0 ,0);
      ip = ip->next;
      while (ip)
         ip = DelInst(ip);
/*
 *    list blocks to delete except the 1st one.
 */
      delblks = NULL;
      bp0 = NULL;
      bp = bl->blk;
      for (bl = bl->next; bl; bl = bl->next)
      {
         if (!FindInList(PATHS[VPATH]->blocks,bl->blk))
         {
/*
 *          Need to check whether it is not part of other paths yet to test
 */
            for (j = i+1; j < NPATH; j++)
            {
               bp0 = FindBlockInList(PATHS[i]->blocks,bl->blk);
               if (bp0) break;
            }
            if (bp0) continue;
            delblks = AddBlockToList(delblks,bl->blk);
         }
      }
/*
 *    HERE HERE. Need to consider the side effect of block killing... ...
 *    Haven't checked with example yet
 */
#if 1      
      if (!delblks)
         for (bl = delblks; bl; bl=bl->next)
            KillBlock(bp, bl->blk);
#endif
   }
/*
 *    Need to re-calculate the optloop and all its info. But before that need
 *    to temporarily add the OptimizeLoopControl
 */
  /* OptimizeLoopControl(); */
}

int SpecSIMDLoop(void)
{
   LOOPQ *lp;
   INSTQ *ippu;
   struct ptrinfo *pi0;
   BLIST *bl;
   lp = optloop;
   
#if 1   
   KillLoopControl(lp);
   GenCleanupLoop(lp);
   OptimizeLoopControl(lp, 1, 0, NULL);
#endif

/*
 * ScalarRestart messes up the optloop and all its parameter. Still, we need to
 * call it before. So, don't update optloop yet. all work is done with the 
 * original optloop.
 * NOTE: may copy all the loop blocks and apply scalar restart using those
 * duplicated block.
 */
#if 0 
   fprintf(stdout, " LIL BEFORE SCALAR RESTART LOOP \n");
   PrintInst(stdout, bbbase);
#endif
   ScalarRestart(lp);
/*
 * Consider only vector path now. Need to add some analysis for various vars
 * to implement backup stages.
 */
   SpeculativeVecTransform(lp);  
#if 1
   UnrollCleanup(lp,1);
#endif
#if 0
   fprintf(stdout, " LIL AFTER SSV LOOP \n");
   PrintInst(stdout, bbbase);
   fprintf(stdout, " SYMBOL TABLE \n");
   PrintST(stdout);
#endif

   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
#if 0
   fprintf(stdout, " LIL NEW CFG \n");
   PrintInst(stdout, bbbase);
   
   ShowFlow("cfg.dot",bbbase);
#if 1
   lp = optloop;
   fprintf(stderr, "\n LOOP BLOCKS: \n");
   if(!lp->blocks) fprintf(stderr, "NO LOOP BLK!!!\n");
   for (bl = lp->blocks; bl ; bl = bl->next)
   {
      assert(bl->blk);
      fprintf(stderr, "%d ",bl->blk->bnum);
   }
   fprintf(stderr,"\n");
   fprintf(stderr, "\n LOOP BLOCKS: \n");
   if(lp->header) fprintf(stderr, "loop header: %d\n",lp->header->bnum);
   if(lp->preheader) fprintf(stderr, "loop preheader: %d\n",
                             lp->preheader->bnum);
   
   fprintf(stderr,"loop tails: ");
   for (bl = lp->tails; bl ; bl = bl->next)
   {
      assert(bl->blk);
      fprintf(stderr, "%d ",bl->blk->bnum);
   }
   fprintf(stderr,"\n");
   
   fprintf(stderr,"loop posttails: ");
   for (bl = lp->posttails; bl ; bl = bl->next)
   {
      assert(bl->blk);
      fprintf(stderr, "%d ",bl->blk->bnum);
   }
   fprintf(stderr,"\n");
#endif

#endif

#if 0
      fprintf(stdout, "Final SSV \n");
      PrintInst(stdout, bbbase);
#endif 

   return(0);
}
