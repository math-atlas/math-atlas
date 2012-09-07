#include "fko.h"

/* Majedul: to save the paths for speculative Vectorization. */
static LOOPPATH **PATHS = NULL;
static int NPATH = 0, TNPATH = 0, VPATH = -1;

/* 
 * Majedul: temporary flag for SSV which is needed in loop peeling to predict
 * max LABEL_id. I will manage that using special flag which indicates the type
 * of vectorization.
 */
int isSSV = 0;

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
#if 0
    fprintf(stderr, "%s: inst=%s, inst->next=%s\n", STname[var-1],
              instmnem[ip->inst[0]], instmnem[ip->next->inst[0]] );
#endif
/*
 *    Majedul: 
 *    1. Production: "ID PE ID * avar" this logic is OK
 *       Example: dot += x * y;
 *    2. Production: "ID PE avar" this is not OK
 *       Example: sum += x;
 *       here, the LIL:
 *          LD reg0, sum
 *          LD reg1, x
 *          FADD reg0, reg0, reg1
 *          FST sum, reg0
 *    So, I have added this while loop to skip all the lds      
 */
      while (ip->next->inst[0] == FLD || ip->next->inst[0] == FLDD)
         ip=ip->next;
      assert(ip->next);

      if (ip->next->inst[0] == FADD || ip->next->inst[0] == FADDD ||
          ip->next->inst[0] == FMAC || ip->next->inst[0] == FMACD)
         j |= VS_ACC;
      else if (IS_STORE(ip->next->inst[0])&& STpts2[ip->next->inst[2]-1] == var)
      {
         j |= FindReadUseType(ip->next, var, blkvec);
         j |= FindReadUseType(ip->next, STpts2[ip->next->inst[1]-1], blkvec);
      }
      else 
         j |= VS_MUL; /* VS_MUL represents anything but accumulator */
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
   {
      fko_warn(__LINE__, " No Loop to Vectorize\n\n");
      return(1);
   }
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
      fko_warn(__LINE__, "No fp vars : Nothing to Vectorize!\n\n");
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

#if 0
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
   for (k=0,i=1; i < n; i++) /* FIXME: should be < n... replaced from <=  */
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
#if 0            
            fprintf(stderr," V1: LIVEIN : %s[%d], sp: %s[%d], j=%d\n",
                    STname[lp->vscal[i+1]-1],lp->vscal[i+1], STname[sp[i]-1],
                    sp[i], j);
#endif
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
 *       FIXME: if liveout var is also livein, vsoflag is not updated.
 *       still it works for old SimdLoop as by default all liveouts are
 *       VS_ACC
 */
         else if (s[i] & VS_LIVEOUT)
         {
            SetVecAll(iv, 0);
            ib = FindPrevStore(lp->posttails->blk->inst1, sp[i],lp->blkvec, iv);
            j = 0;
            for (il=ib; il; il = il->next)
            {
               ip = il->inst; /* FIXED: ip is assigned here */
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

INSTQ *AddAlignTest(LOOPQ *lp, BBLOCK *bp, INSTQ *ip, int fa_label)
{
   int k;
   int r0, r1;
   int hconst; 
/*
 * we need to populate the const correctly irrespectively
 * in X86_32, ptr is 32 bit but X86_64, ptr is 64 bit
 * NOTE: right now, it is hard-coded for AVX and SSE. Later, we need to 
 * generalize it for any architecture
 */
   #ifdef AVX
      hconst = 0x1F;      
   #else
      hconst = 0x0F;
   #endif
   k = STiconstlookup(hconst);
   r0 = GetReg(T_INT); /*ptr is loaded in int reg*/
/*
 * load X and test with the const. need to check whether load ptr is ok
 * NOTE: in X8664, reg is 64 bit but int is 32 bit. As we use constant
 * the CMPAND should not create any problem.
 */
   ip = InsNewInst(bp, ip, NULL, LD, -r0, SToff[lp->varrs[1]-1].sa[2], 0);
   ip = InsNewInst(bp, ip, NULL, CMPAND, -ICC0, -r0, k );
   ip = InsNewInst(bp, ip, NULL, JNE, -PCREG, -ICC0, fa_label);
   
   GetReg(-1);

   return ip;   
}

short InsertNewLocal(char *name, int type )
{
   short k;
   k = STdef(name, type | LOCAL_BIT, 0);
   SToff[k-1].sa[2] = AddDerefEntry(-REG_SP, k, -k, 0, k);
   return k;
}

BLIST *AddLoopDupBlks(LOOPQ *lp, BBLOCK *up, BBLOCK *down, int lbNum)
/*
 * Returns pointer of block list of duplicated blocks, adding duplicated 
 * loop blocks between up and down blocks using lbNum label index extension
 */
{
   BLIST *dupblks, *ftheads, *bl;
   BBLOCK *newCF, *bp, *bp0;
   int iv, ivtails;
   extern int FKO_BVTMP; 
   
   bp0 = up;
/*
 * Duplicate original loop body
 */
   SetVecBit(lp->blkvec, lp->header->bnum-1, 0);
   FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
   newCF = DupCFScope(lp->blkvec, iv, lbNum, lp->header); 
   assert(newCF->ilab);
/*
 * Use CF to produce a block list of duped blocks
 */
   SetVecBit(lp->blkvec, lp->header->bnum-1, 1);
   iv = BitVecCopy(iv, lp->blkvec);
   dupblks = CF2BlockList(NULL, iv, newCF);

/*
 * Find all fall-thru path headers in loop; iv becomes blocks we have already
 * added
 */
   SetVecAll(iv, 0);
   ivtails = BlockList2BitVec(lp->tails);
   ftheads = FindAllFallHeads(NULL, lp->blkvec, lp->header, ivtails, iv);
   ftheads = ReverseBlockList(ftheads);
/*
 *  add loop blks one fall-thru path at a time
 */
   for (bl=ftheads; bl; bl = bl->next)
   {
      for (bp=bl->blk; bp; bp = bp->down)
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
   
   bp0->down = down;
   return dupblks;
}

void AddLoopPeeling(LOOPQ *lp, int jblabel, int falabel, short Np, 
                    short Nv)
/*
 * original loop, lp = [N, -1, 0] or, [0, 1, N]
 * 
 * After loop peeling: 
 * NINC: 
 * 1. Peel loop, lpn = [Np, -1, 0] 
 * 2. All use of induction var (i) is changed by i + N-Np
 * 3. Vector loop, lp = [N-Np, -1, 0] 
 * PNIC:
 * 1. Peel loop, lpn = [0, 1, Np]
 * 2. Vector Loop, lp = [0, 1, N-Np]
 * 3. All use of induction var (i) is changed by i + Np
 * .......................................................
 * 
 * Change in plan ... we will change the loop control, left the index intact
 * NINC
 * 1. Peel loop, lpn = [N, -1, N-Np]
 * 2. Vect loop, lp = [N-Np, -1, 0]
 * 
 * PINC
 * 1. Peel loop, lpn = [0, 1, Np]
 * 2. Vect loop, lp = [Np, 1, N]
 */
{
   BBLOCK *bp0, *bp;
   BLIST *bl, *dupblks;
   INSTQ *ip, *iip;
   ILIST *il, *iref;
   int lnum;
   short oldN;
   int k, r0, r1;
   LOOPQ *lpn;
   extern BBLOCK *bbbase;
   extern int FKO_BVTMP;

/*
 * Find last block, add loop peeling after it
 */
   for (bp0=bbbase; bp0->down; bp0=bp0->down);
   bp = NewBasicBlock(bp0, NULL);
   bp0->down = bp;
   bp0 = bp;
/*
 * Start new block with a  label
 */
   ip = InsNewInst(bp, NULL, NULL, LABEL, falabel , 0, 0);
/*
 * Add new local for loop->end and populate this var 
 * NOTE: 
 * NINC: 
 */
   k = type2shift(lp->vflag);
   r0 = GetReg(T_INT);
   r1 = GetReg(T_INT);
   ip = InsNewInst(bp, ip, NULL, LD, -r0, SToff[lp->varrs[1]-1].sa[2], 0);
   ip = InsNewInst(bp, ip, NULL, MOV, -r1, -r0, 0);
   ip = InsNewInst(bp, ip, NULL, ADD, -r0, -r0, STiconstlookup((1<<k)-1));
   ip = InsNewInst(bp, ip, NULL, SHR, -r0, -r0, STiconstlookup(k));
   ip = InsNewInst(bp, ip, NULL, SHL, -r0, -r0, STiconstlookup(k)); 
   ip = InsNewInst(bp, ip, NULL, SUB, -r0, -r0, -r1);
   
   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
      ip = InsNewInst(bp, ip, NULL, SHR, -r0, -r0, STiconstlookup(2)); 
   else /* array type is double */
      ip = InsNewInst(bp, ip, NULL, SHR, -r0, -r0, STiconstlookup(3)); 
/*
 * NOTE: depending on NINC/PINC, _Np will store N-NP or Np value
 */
   if (lp->flag & L_NINC_BIT) oldN = lp->beg;
   else oldN = lp->end;
   
   if (lp->flag & L_NINC_BIT)
   {
      ip = InsNewInst(bp, ip, NULL, LD, -r1, SToff[oldN-1].sa[2], 0);
      ip = InsNewInst(bp, ip, NULL, SUB, -r1, -r1, -r0);
      ip = InsNewInst(bp, ip, NULL, ST, SToff[Np-1].sa[2],-r1,  0); 
   }
   else
      ip = InsNewInst(bp, ip, NULL, ST, SToff[Np-1].sa[2],-r0,  0); 
   GetReg(-1);
/*
 * Test new _N, if it is greater than N, jump back to Aligned part. 
 * It will go to cleanup loop (not vector loop) anyway.
 */

   r0 = GetReg(T_INT);
   r1 = GetReg(T_INT);
   ip = InsNewInst(bp, ip, NULL, LD, -r0, SToff[Np-1].sa[2], 0);
   if (lp->flag & L_NINC_BIT) /* Np = N - np, if 0>Np jumpback*/
   {  
      ip = InsNewInst(bp, ip, NULL, CMP, -ICC0, -r0, STiconstlookup(0));
      ip = InsNewInst(bp, ip, NULL, JLE, -PCREG, -ICC0, jblabel);
   }
   else /* Np = np*/
   {
      ip = InsNewInst(bp, ip, NULL, LD, -r1, SToff[oldN-1].sa[2], 0);
      ip = InsNewInst(bp, ip, NULL, CMP, -ICC0, -r1, -r0);
      ip = InsNewInst(bp, ip, NULL, JLE, -PCREG, -ICC0, jblabel);
   }
   GetReg(-1);
/*
 * Create a new block for the loop preheader to manage the loop.
 */
   bp = NewBasicBlock(bp0, NULL);
   bp0->down = bp;
   bp0 = bp;
   ip = InsNewInst(bp, NULL, NULL, CMPFLAG, CF_LOOP_INIT, 0, 0);
/*
 * Make sure loop control is killed before copied the optloop
 * shoule be killed before ... ...
 */
   KillLoopControl(lp);
/*
 * Add duplicated loop body
 * NOTE: assumption: lp is vectorizable, either normal or SSV
 * NOTE: if unroll is used, we need to keep space for that too. 
 *
 */
   if (NPATH >  0 && isSSV) /* SSV is aplied */
   {
      lnum = (NPATH -1) * Type2Vlen(lp->vflag) + 1; /* keep space for SSV*/
   }
   else /* normal vectorization. 0 is preserve for cleanup*/
   { 
      lnum = FKO_UR + 1;  /* need to reserve label for unroll */
   }
   
   dupblks = AddLoopDupBlks(lp, bp0, bp0->down, lnum );
/*
 * Create new loop structure for peel loop
 * loop structure: [ N, -1, N-Np]
 * main loop: now [N, -1, 0] will be: [N-Np, -1, 0]
 */
   lpn = NewLoop(lp->flag);
   lpn->I = lp->I;
   if (lp->flag & L_NINC_BIT )
   {
      assert(IS_CONST(STflag[lp->end-1]) && (SToff[lp->end-1].i == 0));
      assert(IS_CONST(STflag[lp->inc-1]) && SToff[lp->inc-1].i == -1);
/*
 *    Peel loop .... Np = N - np, Nv = N yet
 */
      lpn->beg = lp->beg; /* N */
      lpn->end = Np;   /* N-Np */
      lpn->inc = lp->inc;
      /*fprintf(stderr, "NINC: lpn->beg = %s, lpn->end = %s\n", 
              STname[lpn->beg-1], STname[lpn->end-1]);*/
/*
 *    change main loop... 
 *    NOTE: This is used even if program the doen't enter peeling loop
 */
      lp->beg = Nv; /* need to change the value of Nv to Np at end of loop*/
   }
   else if (lp->flag & L_PINC_BIT )
   {
      assert(IS_CONST(STflag[lp->beg-1]) && (SToff[lp->beg-1].i == 0));
      assert(IS_CONST(STflag[lp->inc-1]) && SToff[lp->inc-1].i == 1);
      
      lpn->beg = lp->beg;
      lpn->end = Np; /* Np */
      lpn->inc = lp->inc;
/*
 *    change the original loop
 *    NOTE: not tested yet
 */
      lp->beg = Nv; /* update Nv with Np*/
   }
   else
      assert(0);
#if 0
/*
 * NOTE: if induction var (index var) is used inside loop and the loop is
 * NINC, then we need to change the index by i + N - Np
 * HERE HERE, I don't think the issue yet: it i is set inside the loop.
 */

   iref = FindIndexRef(dupblks, SToff[lpn->I-1].sa[2]);
   if (iref)
   {
      if (lp->flag | L_NINC_BIT) /* need to update all the index ref */
      {
         for (il=iref; il; il=il->next)
            UpdatePLIndexRef(il->inst, Nv, Np);
      }
   }
#endif

/*
 * NOTE: set CMPFLAG CF_LOOP_INIT at the preheader and mark the preheader
 * Now, OptimizeLoopControl can handle the loop accordingly. We don't need to
 * manage I manually, but we need to update the loop control accordingly. 
 */
   lpn->preheader = bp; /* to manage the loop control automatically */

   bp = FindBlockInListByNumber(dupblks, lp->header->bnum);
   assert(bp && bp->ilab);
   lpn->body_label = bp->ilab;
   for (bl=lp->tails; bl; bl = bl->next)
   {
      bp = FindBlockInListByNumber(dupblks, bl->blk->bnum);
      assert(bp);
      lpn->tails = AddBlockToList(lpn->tails, bp);
   }
   lpn->blocks = dupblks;
   OptimizeLoopControl(lpn, 1, 0, NULL);

/*
 * After all tails of cleanup loop, add jump back to aligned code
 * before that save Np to Nv... it will needed for main loop
 */
   for (bl=lpn->tails; bl; bl = bl->next)
   {
      bp = NewBasicBlock(bl->blk, bl->blk->down);
      bl->blk->down = bp;
      r0 = GetReg(T_INT);
      ip = InsNewInst(bp, ip, NULL, LD, -r0, SToff[Np-1].sa[2], 0);
      ip = InsNewInst(bp, ip, NULL, ST, SToff[Nv-1].sa[2], -r0, 0);
      InsNewInst(bp, ip, NULL, JMP, -PCREG, jblabel, 0);
      GetReg(-1);
   }

   KillLoop(lpn); 
}

void GenForceAlignedPeeling(LOOPQ *lp)
/*
 * Generate code of loop peeling for force aligned. works only with single 
 * moving array ptr. Force aligned is only applied when we can vectorize the
 * code.
 * NOTE: loop peeling may change the loop control of main loop. It should be
 * called before GenCleanupLoop and FinalizeVectorCleanup for consistancy
 */
{
   int i, j;
   int jBlabel, fAlabel;
   BBLOCK *bp;
   INSTQ *ip;
   short Np, Nv;
   int r0, r1;
   LOOPQ *lpn;
   extern BBLOCK *bbbase;
/*
 * Need to create 2 new var for peel and vector loop control
 */
   Np = InsertNewLocal("_Np", T_INT ); /* actually save, N-Np or Np value */
   Nv = InsertNewLocal("_Nv", T_INT ); /* 1st, N then changed to N-Np or N*/
#if 0
/*
 * Create new loop structure for peel loop
 * loop structure: [ N, -1, N-Np]
 * main loop: now [N, -1, 0] will be: [N-Np, -1, 0]
 */
   lpn = NewLoop(lp->flag);
   lpn->I = lp->I;
   if (lp->flag | L_NINC_BIT )
   {
      assert(IS_CONST(STflag[lp->end-1]) && (SToff[lp->end-1].i == 0));
      assert(IS_CONST(STflag[lp->inc-1]) && SToff[lp->inc-1].i == -1);
      
      lpn->beg = Np;
      lpn->end = lp->end;
      lpn->inc = lp->inc;
   }
   else if (lp->flag | L_PINC_BIT )
   {
      assert(IS_CONST(STflag[lp->beg-1]) && (SToff[lp->beg-1].i == 0));
      assert(IS_CONST(STflag[lp->inc-1]) && SToff[lp->inc-1].i == 1);
      
      lpn->beg = lp->beg;
      lpn->end = Np;
      lpn->inc = lp->inc;
   }
   else
      assert(0);
#endif

/*
 * find the location to add checking of alignment. 
 */
   bp = bbbase;
   ip = FindCompilerFlag(bp, CF_LOOP_INIT);
   assert(ip); 
   jBlabel = STlabellookup("_FKO_ALIGNED");
   fAlabel = STlabellookup("_FKO_FORCE_ALIGN");
   ip = InsNewInst(bp, NULL, ip, CMPFLAG, CF_FORCE_ALIGN, 0, 0);
/*
 * Now, the vector loop control is changed, so initialize it with original N
 * HERE HERE, flag may changed after loop control optimization, is it 
 * considered, here???
 */
   r0 = GetReg(T_INT);
   if (lp->flag & L_NINC_BIT)
   {
      /*fprintf(stderr, "NINC! \n\n");*/
      ip = InsNewInst(bp, ip, NULL, LD, -r0, SToff[lp->beg-1].sa[2], 0);
      ip = InsNewInst(bp, ip, NULL, ST, SToff[Nv-1].sa[2], -r0, 0);
   }
   else /* PINC, i believe, we need to make the Nv 0*/
   {
      ip = InsNewInst(bp, ip, NULL, MOV, -r0, STiconstlookup(0), 0);
      ip = InsNewInst(bp, ip, NULL, ST, SToff[Nv-1].sa[2], -r0, 0);
   }
   GetReg(-1);
/*
 * Add the checking of alignment. it is always related with the vector length
 * the system supports, eg.- for SSE vlen 128bit, for AVX, 256 bit.
 */
   ip = AddAlignTest(lp, bp, ip, fAlabel); 
   ip = InsNewInst(bp, ip, NULL, LABEL, jBlabel,0,0);   

/*
 * Add loop peeling at the end of the code after cleanup (if this function is 
 * called after cleanup, otherwise after ret)
 */
   AddLoopPeeling(lp, jBlabel, fAlabel, Np, Nv);
  /* KillLoop(lpn); */
}

int IsLoopPeelOptimizable(LOOPQ *lp)
{
   int peel, flag;
   short beg, end, inc;

   peel = 1;
   beg = lp->beg;
   end = lp->end;
   inc = lp->inc;
   flag = lp->flag;
/*
 * For now, the necessary conditions:
 * 1. Only one Moving array ptr
 * 2. Must be vectorized
 * 3. Simple loop format: [N, -1, 0] or, [0, 1, N]
 */
   if (lp->varrs[0] > 1) 
   {
      peel = 0;  
      fko_warn(__LINE__,"morethan one moving array ptr prevents align-peel!!\n");
   }
/*
 * Always call after confirming vectorization.
 */

#if 0  
   if (VPATH == -1)
   {
      peel = 0;
      fprintf(stderr,"No Vectorizable Path for SSV, no need to align-peel!!\n");
   }
#endif
   if (flag & L_NINC_BIT)
   {
      if (!IS_CONST(STflag[end-1]) || SToff[end-1].i != 0 ) 
      {
         peel = 0;
      }
      if (!IS_CONST(STflag[inc-1]) || SToff[inc-1].i != -1 ) 
      {
         peel = 0;
      }
      if (!peel) fko_warn(__LINE__, "NINC: must be [N, 0, -1] format \n");
   }
   else if (flag & L_PINC_BIT)
   {
      if ( !(IS_CONST(STflag[beg-1]) && SToff[beg-1].i == 0) ) 
      {
         peel = 0;
         fko_warn(__LINE__,"beg = %d, Const?=%d !!\n", SToff[beg-1].i, 
                 IS_CONST(STflag[lp->beg-1]));
      }
      if (!(IS_CONST(STflag[inc-1]) && SToff[inc-1].i == 1) ) 
      {
         peel = 0;
         fko_warn(__LINE__,"inc = %d, Const?=%d !!\n", SToff[inc-1].i, 
                 IS_CONST(STflag[lp->inc-1]));
      }
      if (!peel) fko_warn(__LINE__,"PINC: must be [0, N, 1] format:[%d,%d]!!\n",
                         SToff[beg-1].i, SToff[inc-1].i);
   }
   else 
   {
      peel = 0;
      fko_warn(__LINE__, "Neither NINC nor PINC, no align-peel!! \n");
   }
   
   return peel;
}
int SimdLoop(LOOPQ *lp)
{
   short *sp;
   BLIST *bl;
   static enum inst 
     sfinsts[] = {FLD,      FST,      FMUL,     FMAC,     FADD,     FSUB,    
                  FABS,     FMOV,     FZERO,    FNEG,     FCMOV1,   FCMOV2, 
                  FCMPWEQ,  FCMPWNE,  FCMPWLT,  FCMPWLE,  FCMPWGT,  FCMPWGE},

     vfinsts[] = {VFLD,     VFST,     VFMUL,    VFMAC,    VFADD,    VFSUB,   
                  VFABS,    VFMOV,    VFZERO,   VFNEG,    VFCMOV1,  VFCMOV2,
                  VFCMPWEQ, VFCMPWNE, VFCMPWLT, VFCMPWLE, VFCMPWGT, VFCMPWGE},

     sdinsts[] = {FLDD,     FSTD,     FMULD,    FMACD,    FADDD,    FSUBD, 
                  FABSD,    FMOVD,    FZEROD,   FNEGD,    FCMOVD1,  FCMOVD2, 
                  FCMPDWEQ, FCMPDWNE, FCMPDWLT, FCMPDWLE, FCMPDWGT, FCMPDWGE},

     vdinsts[] = {VDLD,     VDST,     VDMUL,    VDMAC,    VDADD,    VDSUB, 
                  VDABS,    VDMOV,    VDZERO,   VDNEG,    VDCMOV1,  VDCMOV2,
                  VDCMPWEQ, VDCMPWNE, VDCMPWLT, VDCMPWLE, VDCMPWGT, VDCMPWGE};
   
   const int nvinst=18;
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
 * temp testing 
 */

#if 1
   if (IsLoopPeelOptimizable(lp))
      GenForceAlignedPeeling(lp);
#endif   
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
/*
 *       Majedul: FIXME: There is a bug in the vector analysis...
 *       It doesn't recognize sum as accumulator to init but can recognize
 *       in reduction.
 */
#if 0         
         fprintf(stderr, "LIVEIN : %s : %d\n", 
                 STname[lp->vscal[i+1]-1], lp->vsflag[i+1]);
#endif   
/*
 *       Majedul: FIXME: for accumulator init, shouldn't we need an Xor
 *       though most of the time it works as vmoss/vmosd automatically
 *       zerod the upper element. but what if the optimization transforms it
 *       into reg-reg move. vmovss/vmosd for reg-reg doesn't make the upper 
 *       element zero!!! 
 *       NOTE: so far, it works. as temp reg normally uses a move from mem
 *       before reg-reg move, which makes the upper element zero.
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
#if 0         
         fprintf(stderr, "LIVEOUT : %s : %d\n", 
                 STname[lp->vscal[i+1]-1], lp->vsflag[i+1]);
#endif
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
   extern BBLOCK *bbbase;
/*
 * Get code into standard form for analysis
 */
   GenPrologueEpilogueStubs(bbbase, savesp);
   bbbase = NewBasicBlocks(bbbase);
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

/*
 * NOTE: As UpdateOptLoopWithMaxMinVars() is called before, Prologue Epilogue is 
 * already generated. 
 */

#if 0   
   GenPrologueEpilogueStubs(bbbase, 0);
   NewBasicBlocks(bbbase);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
   lp = optloop;
#else
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__); 
   lp = optloop; 
#endif

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

#if 0
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
      fprintf(out, "%s(%d) ",STname[sp[i]-1]? STname[sp[i]-1]: "NULL",sp[i]-1);
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
 * Analyze single path of loop and stores all the info in paths data structure,
 * returns error code if failed, otherwise 0.
 * NOTE: it doesn't change the original code but assume loopcontrol is killed.
 * NOTE: don't stop the analysis when error is found, rather complete all
 * the analysis, save the data structure and return error code at last
 */
{
   extern short STderef;
   extern int FKO_BVTMP;
   extern int VECT_FLAG;

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
 * all these arrays element count is at position 0.
 */
   short *sp, *s, *scf;        /* temporary ptrs */
/*
 * temporary for paths 
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
   /*iv = NewBitVec(32);*/
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);

   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
   for (bl=scope; bl; bl=bl->next)
   {
      path->uses = BitVecComb(path->uses, path->uses, bl->blk->uses, '|');
      path->defs = BitVecComb(path->defs, path->defs, bl->blk->defs, '|');
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
   j = FLAG2TYPE(STflag[sp[0]-1]); /* this is element, not element-count now*/
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
               sflag[j+1] |= SC_LIVEIN;
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
 *    Special treatment for max/min vars
 */
   /*lp = optloop; */
#if 0
   if (lp->maxvars)
   {
      N = lp->mmaxvars[0];
      for (i=1; i <= N; i++)
      {
         k = FindInShortList(vscal[0], vscal+1, lp->maxvars[i]);
         vsflag[k] |= VS_MAX;
         sflag[k]  |= SC_MAX;
      }
   }
#endif

/*
 *    convert blocks of scope into bvec
 */
      blTmp = scope; /* need to copy pointer as the function changes pointer */
      blkvec = BlockList2BitVec(blTmp);
/*
 *    Find out how to init livein and reduce liveout vars
 */
      s = vsflag+1;
      scf = sflag+1;

      for (i=0; i < n; i++)
      {
#if 0         
/*
 *       For livein variables, any access is legal, but only handled in 2 ways:
 *       If adder, init one val to 0, others to init val, if anything else
 *       (MUL or assignment), init all vals to same
 *
 */
/*
 *       Majedul: vars can be both LIVEIN and LIVEOUT. So, in analysis phrase
 *       why not we check for both. 
 */
         if (s[i] & VS_LIVEIN) /* skip private variable here */
         {
            /*fprintf(stderr,"%d(%s) : LIVEIN\n",
                        sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");*/
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
         if (s[i] & VS_LIVEOUT)
         {
            /*fprintf(stderr,"%d(%s) : LIVEOUT\n",
                        sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");*/
            SetVecAll(iv, 0);
/*
 *          Majedul: Check this function. Need to resolved norm2
 *          for SSQ, we need to differentiate the scal and ssq as input/ouput
 *          although both are LIVEOUT at the exit of the loop
 */
            ib = FindPrevStore(lp->posttails->blk->inst1, sp[i],blkvec, iv);
            j = 0;
            vsoflag[i+1] |= VS_ACC;
            /* by default all set, will clear accordingly */
            sflag[i+1] |= SC_OACC;
            sflag[i+1] |= SC_OMUL;
            sflag[i+1] |= SC_OMIXED;
            for (il=ib; il; il = il->next)
            {
/*
 *             Majedul: 
 *             FIXME: Figure out the ip issue. Shouldn't be it derived from
 *             ILIST pointer il !!??
 */
               ip = il->inst; /* FIXED */
               if (ip->prev->inst[0] == FADD || ip->prev->inst[0] == FADDD)
               {
                  sflag[i+1] &= ~SC_OMUL;
                  fprintf(stderr, "var = %s -> ACCUMULATOR\n",
                          STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
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
                  fprintf(stderr,"var = %s : prev_inst = %s, cur_inst = %s\n", 
                          STname[sp[i]-1] ? STname[sp[i]-1] : "NULL", 
                          instmnem[ip->prev->inst[0]],instmnem[ip->inst[0]]);
                  fprintf(stderr, "cur_inst_dest = %s\n",
                          STname[STpts2[ip->inst[1]]-1] ? 
                          STname[STpts2[ip->inst[1]]-1] : "NULL"); 

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
/*       Only private variables are neither livein nor liveout */               
         if (!(s[i] & VS_LIVEIN) && ! (s[i] & VS_LIVEOUT)) 
         {
            /*fprintf(stderr,"%d(%s) : PRIVATE\n",
                        sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");*/
            sflag[i+1] |= SC_PRIVATE;
         }
#else
/*
 *       Majedul: Re-write the analysis for livein/liveout var to be acc/mul
 *       NOTE: for livein var, we can handle both accumulator and MUL/DIV/ASSIGN
 *       for liveout, we can handle accumulator when it is reduction var and 
 *       if it is not set inside the loop/path, we can handle MUL/DIV/ASSIGN.
 *       NOTE: Right now, we consider VS_ACC and VS_MAX both are LIVEIN and 
 *       LIVEOUT
 */
         if (scf[i] & SC_LIVEIN) /* need to check for the vector init */
         {
/*
 *          There are three options: 
 *          a) Accumulator: both use and set, but use only with ADD
 *          d) Max var: can be either use or set depend on path  
 *          b) Not set/ only used: used for MUL/DIV.... ADD? skip right now
 */
/*
 *          NOTE: for SSV, even though a var is max/min, reduction is not
 *          dependent on that. Reduction is dependant on the analysis of 
 *          vector path.
 */
            if (VECT_FLAG & VECT_SV)  /* not consider max/min for SSV */
            {
                j = FindReadUseType(lp->header->inst1, sp[i], blkvec);
            }
            else /* consider max/min var for reduction otherwise */
            {
               if (lp->maxvars && 
                   FindInShortList(lp->maxvars[0], lp->maxvars+1, sp[i]))
                  j = VS_MAX;
               else if (lp->minvars && 
                        FindInShortList(lp->minvars[0], lp->minvars+1, sp[i]))
                  j = VS_MIN;
            else
                j = FindReadUseType(lp->header->inst1, sp[i], blkvec);
            }

            if ( (scf[i] & SC_SET) && (scf[i] & SC_USE) ) /*mustbe used as acc*/
            {
               if (j == VS_ACC)
               {
                  s[i] |= VS_ACC;
                  scf[i] |= SC_ACC;
                  /*vsoflag[i+1] |= VS_ACC;*/
               }
               else if (j == VS_MAX)
               {
                  s[i] |= VS_MAX;
                  scf[i] |= SC_MAX;
                  /*vsoflag[i+1] |= VS_MAX;*/
               }
               else if (j == VS_MIN)
               {
                  s[i] |= VS_MIN;
                  scf[i] |= SC_MIN;
                  /*vsoflag[i+1] |= VS_MIN;*/
               }
               else
               {
                  fko_warn(__LINE__,
                          "var %d(%s) must be Accumulator !!\n\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode += 64;
                  scf[i] |= SC_MIXED;
                  /*vsoflag[i+1] &= ~(VS_ACC | VS_MAX);*/
               }
            }
            else if (scf[i] & SC_USE)
            {
               if (j == VS_MAX) /* can be used depends on path choice */
               {
                  s[i] |= VS_MAX;
                  scf[i] |= SC_MAX;
                  /*vsoflag[i+1] |= VS_MAX;*/
               }
               else if (j == VS_MIN) /* can be used depends on path choice */
               {
                  s[i] |= VS_MIN;
                  scf[i] |= SC_MIN;
                  /*vsoflag[i+1] |= VS_MIN;*/
               }
               else if (j == VS_MUL) /*includes div */
               {
                  s[i] |= VS_MUL;
                  scf[i] |= SC_MUL;
                  /*vsoflag[i+1] |= VS_MUL;*/
               }
               else
               {
                  fko_warn(__LINE__,
                           "Mixed use of var %d(%s) prevents vectorization!!\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode += 64;
                  scf[i] |= SC_MIXED;
               }
            }
         }

         if (scf[i] & SC_LIVEOUT) /* if it is set, it must be acc*/
         {
            if (scf[i] & SC_SET)
            {
/*
 *             Now, we need to check whether it is used as accumulator!
 *             it is already checked in LIVEIN part as accumulator must also
 *             be LIVEIN... right now
 *             NOTE: we consider that all liveout must be also livein here.
 */   
               if (s[i] & VS_ACC)
                  vsoflag[i+1] |= VS_ACC;
               else if (s[i] & VS_MAX)
               {
                  vsoflag[i+1] |= VS_MAX;
                  /*fprintf(stderr, "%s is a max var which is set in path: %d\n",
                          STname[sp[i]-1], path->pnum);*/
               }
               else if (s[i] & VS_MIN)
               {
                  vsoflag[i+1] |= VS_MIN;
                  /*fprintf(stderr, "%s is a max var which is set in path: %d\n",
                          STname[sp[i]-1], path->pnum);*/
               }
               else
               {
                  fko_warn(__LINE__,
                     "Liveout var %d(%s) must be Accumulator/max when set!!\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode +=128;
               }

            }
            else if (scf[i] & SC_USE)
            {
/*
 *             if the variable is only used, it can be vectorized. 
 *             Need to check whether it is only used for MUL/DIV
 *             NOTE: not consider yet!!!
 */
               if (s[i] & VS_MUL)
                  vsoflag[i+1] |= VS_MUL;
               else if (s[i] & VS_MAX)
               {
                  vsoflag[i+1] |= VS_MAX;
                  /*fprintf(stderr, 
                        "%s is a max var which is only used in path:%d\n",
                          STname[sp[i]-1], path->pnum);*/
               }
               else if (s[i] & VS_MIN)
               {
                  vsoflag[i+1] |= VS_MIN;
                  /*fprintf(stderr, 
                        "%s is a min var which is only used in path:%d\n",
                          STname[sp[i]-1], path->pnum);*/
               }
               else
               {
                  fko_warn(__LINE__,
                     "Liveout var %d(%s) must be mul/max when only use!!\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode +=256;
               }

            }
         }
/*
 *       Private vars, nothing to worry
 */
         if (!(scf[i] & SC_LIVEIN) && ! (scf[i] & SC_LIVEOUT)) 
         {
            scf[i] |= SC_PRIVATE;
         }
#endif
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
#if 0
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
/*  print flags of each scalar*/
   for (N=scal[0], i=1; i <= N; i++)
   {
      fprintf(stderr, "%s : ",STname[path->scal[i]-1]);
      if (path->sflag[i] & SC_LIVEIN) fprintf(stderr, "LIVEIN ");
      if (path->sflag[i] & SC_LIVEOUT) fprintf(stderr, "LIVEOUT ");
      if (path->sflag[i] & SC_PRIVATE) fprintf(stderr, "PRIVATE ");
      if (path->sflag[i] & SC_SET) fprintf(stderr, "SET ");
      if (path->sflag[i] & SC_USE) fprintf(stderr, "USE ");
      if (path->sflag[i] & SC_ACC && path->sflag[i] & SC_LIVEIN) 
         fprintf(stderr, "IN_ACC ");
      if (path->sflag[i] & SC_ACC && path->sflag & SC_LIVEOUT) 
         fprintf(stderr, "OUT_ACC ");
      if (path->sflag[i] & SC_MUL && path->sflag & SC_LIVEIN) 
         fprintf(stderr, "IN_MUL ");
      if (path->sflag[i] & SC_MUL && path->sflag & SC_LIVEOUT) 
         fprintf(stderr, "OUT_MUL ");
      if (path->sflag[i] & SC_MAX && path->sflag & SC_LIVEOUT) 
         fprintf(stderr, "OUT_MAX ");
      if (path->sflag[i] & SC_MIN && path->sflag & SC_LIVEOUT) 
         fprintf(stderr, "OUT_MIN ");
      fprintf(stderr, "\n");
   }

   fprintf(stderr, "ERR CODE: %d\n",errcode);
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
/*
 * Loop Control should be re-inserted 
 */

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
 * 
 * NOTE: First path often not the fall thru path. We will choose fall thru path
 */
   for (i=0; i < NPATH; i++)
   {
      if (PATHS[i]->lpflag & LP_VEC) /* first vectorizable path */
      {
         VPATH = i;
#if 0
         break;
#else    /* if all the blocks of this path are in fall thru, choose it */
         
#endif
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
 *
 * NOTE: This vector transform should work for Redundant Vector operation also
 * as we have done if-conversion before this. We don't have normal FCMP and 
 * branches after that. So, it can be done trivially.
 *
 *
 */
{
   short *sp;
   BLIST *bl, *scope;
   static enum inst
      sfinsts[]= {FLD,  FST,  FMUL, FDIV,  FMAC, FADD,  FSUB,  FABS,  FMOV,  
                  FZERO},
      vfinsts[]= {VFLD, VFST, VFMUL, VFDIV, VFMAC, VFADD, VFSUB, VFABS, VFMOV, 
                  VFZERO},
      sdinsts[]= {FLDD, FSTD, FMULD, FDIVD, FMACD, FADDD, FSUBD, FABSD, FMOVD, 
                  FZEROD},
      vdinsts[]= {VDLD, VDST, VDMUL, VDDIV, VDMAC, VDADD, VDSUB, VDABS, VDMOV, 
                  VDZERO};
/*****************************************************************************/
/*
 * Generalization of Vector Compare for SSV. 
 *
 * Case-1: (Any condition, fall through vector path )
 * -------------------------------------------------
 * if_condition -> true -> scalar_path
 * scalar: if (condition) GOTO SCALAR_PATH
 * vector: if (condition_any_vector_element) GOTO SCALAR_PATH
 * Comment: "if condition is true (to goto scalar path) for any of the vector 
 * element, goto scalar_path."
 * 
 * Implementation: 
 *
 * V_CMP__W ... ... ...
 * V_SBTI $ireg, ...
 * CMP $ireg, $0,
 * JNE ...
 *
 * Case-2: (All Condition, fall through scalar path)
 * if_condition -> true -> vector_path
 * scalar: if (condition) GOTO VECTOR_PATH
 * vector: if (condition_all_vector_element) GOTO VECTOR_PATH
 * comment: "jump to vector path only if all the elements follow the condition."
 *
 * Implementation:
 *
 * V_CMP__W ... ... ...
 * V_SBTI $ireg, ...
 * CMP $ireg, $0x0F  # for avx and double
 * JEQ ...
 * 
 * NOTE: only case-1 is implemented right now.
 */
/*****************************************************************************/   
#if 0
static enum inst
      brinsts[] = {JEQ, JNE, JLT, JLE, JGT, JGE},
      #if defined(AVX)
         vfcmpinsts[] = {VFCMPWEQ, VFCMPWNE, VFCMPWLT, VFCMPWLE, VFCMPWGT, 
                         VFCMPWGE},
         vdcmpinsts[] = {VDCMPWEQ, VDCMPWNE, VDCMPWLT, VDCMPWLE, VDCMPWGT, 
                         VDCMPWGE};
      #else
/*
 *    SSE supports : EQ, NE, LT, LE, NLT, NLE
 *    not supports : GT, GE, NGT, NGE
 *    So, we need to replace GT and GE with NLE and NLT
 */
         vfcmpinsts[] = {VFCMPWEQ, VFCMPWNE, VFCMPWLT, VFCMPWLE, VFCMPWNLE, 
                         VFCMPWNLT},
         vdcmpinsts[] = {VDCMPWEQ, VDCMPWNE, VDCMPWLT, VDCMPWLE, VDCMPWNLE, 
                         VDCMPWNLT};
      #endif
#endif
/*
 * NOTE: We have 6 conditional branches. So, we only need to keep 6 VCMPWXX inst
 * for SSE, GT and GE are replaced with the NLE and NLT inside the l2a while
 * converting lil to assembly
 */
   static enum inst
      brinsts[] = {JEQ, JNE, JLT, JLE, JGT, JGE},
         vfcmpinsts[] = {VFCMPWEQ, VFCMPWNE, VFCMPWLT, VFCMPWLE, VFCMPWGT, 
                         VFCMPWGE},
         vdcmpinsts[] = {VDCMPWEQ, VDCMPWNE, VDCMPWLT, VDCMPWLE, VDCMPWGT, 
                         VDCMPWGE};
   const int nbr = 6;

   const int nvinst = 10;
   enum inst sld, vld, sst, vst, smul, vmul, sdiv, vdiv, smac, vmac, sadd, vadd,
             ssub, vsub, sabs, vabs, smov, vmov, szero, vzero, inst, binst, 
             mskinst;
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
 *    Majedul: output var doesn't have to be accumulator. We will 
 *    Handle var after checking the VS_ACC flag
 *
 */
      if (VS_LIVEOUT & lp->vsflag[i+1])
      {
         j++;
         /*assert((lp->vsoflag[i+1] & (VS_MUL | VS_EQ | VS_ABS)) == 0);*/
         if (lp->vsoflag[i+1] & VS_ACC)
         {

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
         else /* VS_MUL/ VS_DIV*/
         {
            iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                  "Reduce vector for %s", STname[lp->vscal[i+1]-1]);
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vld, -r0,
                              SToff[lp->vvscal[i+1]-1].sa[2], 0);
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vsst,
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
#ifdef AVX
/*
 *             get new vregs for the destination, no vld as it is not uses but
 *             sets
 */
               vrd = GetReg(FLAG2TYPE(lp->vflag));
               ip->inst[1] = -vrd;
#else
/*
 *             NOTE: for SSE, vcmpwXX has 3 operand, des and src1 is aliased
 */            
               op = ip->inst[2];
               if (op < 0 )
               {
                  vrd = -op;
                  ip->inst[1] = op;
               }
               else
                  assert(0); /* src1 is deref: will consider this case later */

#endif
/*
 *             it's time to add other instruction like mask and test
 *             ireg is set, so, get a new ireg
 */
               ir = GetReg(T_INT);
               ip = InsNewInst(bl->blk, ip, NULL, mskinst, -ir, -vrd, 0);
/*
 *             add test the iregs with const val populated by vec len
 */
#if 0               
               mskval = 1;
               for (i = 1; i < vlen; i++)
                  mskval = (mskval << 1) | 1;
#else          /* consider only vcmp_any case.. fall through vector path */
               mskval = 0;
#endif 
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
/*
 *             Majedul: NOTE: here ends a block. Shouldn't we re-initialize
 *             the reg with GetReg(-1). !!!!! 
 *             FIXME: 
 */
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

int RedundantVectorTransform(LOOPQ *lp)
/*
 * NOTE: This is similar of normal vector transformation DoSimdLoop except max
 * is need to be recognized. As I have changes lot in vector analysis in 
 * Path based analysis, I will create separate vector xform here. 
 * NOTE: this vector transform can be and will be merge with Speculative vector
 * transfer later. 
 */
{
   short *sp;
   BLIST *bl;
   static enum inst 
     sfinsts[] = {FLD,      FST,      FMUL,     FMAC,     FADD,     FSUB,    
                  FABS,     FMOV,     FZERO,    FNEG,     FCMOV1,   FCMOV2, 
                  FCMPWEQ,  FCMPWNE,  FCMPWLT,  FCMPWLE,  FCMPWGT,  FCMPWGE,
                  FMAX,     FMIN },

     vfinsts[] = {VFLD,     VFST,     VFMUL,    VFMAC,    VFADD,    VFSUB,   
                  VFABS,    VFMOV,    VFZERO,   VFNEG,    VFCMOV1,  VFCMOV2,
                  VFCMPWEQ, VFCMPWNE, VFCMPWLT, VFCMPWLE, VFCMPWGT, VFCMPWGE,
                  VFMAX,    VFMIN },

     sdinsts[] = {FLDD,     FSTD,     FMULD,    FMACD,    FADDD,    FSUBD, 
                  FABSD,    FMOVD,    FZEROD,   FNEGD,    FCMOVD1,  FCMOVD2, 
                  FCMPDWEQ, FCMPDWNE, FCMPDWLT, FCMPDWLE, FCMPDWGT, FCMPDWGE,
                  FMAXD,    FMIND },

     vdinsts[] = {VDLD,     VDST,     VDMUL,    VDMAC,    VDADD,    VDSUB, 
                  VDABS,    VDMOV,    VDZERO,   VDNEG,    VDCMOV1,  VDCMOV2,
                  VDCMPWEQ, VDCMPWNE, VDCMPWLT, VDCMPWLE, VDCMPWGT, VDCMPWGE,
                  VDMAX,    VDMIN };
   
   const int nvinst=20;
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

#if 0
   fprintf(stdout, "LIL before Peeling\n");
   PrintInst(stdout, bbbase);
#endif
/*
 * Loop peeling for single moving ptr alignment 
 */

#if 1
   if (IsLoopPeelOptimizable(lp))
      GenForceAlignedPeeling(lp);
#endif   

#if 0
   fprintf(stdout, "LIL After Peeling\n");
   PrintInst(stdout, bbbase);
#endif
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
/*
 *       Majedul: FIXME: There is a bug in the vector analysis...
 *       It doesn't recognize sum as accumulator to init but can recognize
 *       in reduction.
 */
#if 0         
         fprintf(stderr, "LIVEIN : %s : %d\n", 
                 STname[lp->vscal[i+1]-1], lp->vsflag[i+1]);
#endif   
/*
 *       Majedul: FIXME: for accumulator init, shouldn't we need an Xor
 *       though most of the time it works as vmoss/vmosd automatically
 *       zerod the upper element. but what if the optimization transforms it
 *       into reg-reg move. vmovss/vmosd for reg-reg doesn't make the upper 
 *       element zero!!! 
 *       NOTE: so far, it works. as temp reg normally uses a move from mem
 *       before reg-reg move, which makes the upper element zero.
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
         /*assert((lp->vsoflag[i+1] & (VS_MUL | VS_EQ | VS_ABS)) == 0);*/
         if (lp->vsoflag[i+1] & VS_ACC || lp->vsoflag[i+1] & VS_MAX || 
             lp->vsoflag[i+1] & VS_MIN)
         {
            if (lp->vsoflag[i+1] & VS_ACC)
            {
               iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                  "Reduce accumulator vector for %s", STname[lp->vscal[i+1]-1]);
            }
            else if (lp->vsoflag[i+1] & VS_MAX)
            {
               iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                  "Reduce max vector for %s", STname[lp->vscal[i+1]-1]);

            }
            else if (lp->vsoflag[i+1] & VS_MIN)
            {
               iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                  "Reduce min vector for %s", STname[lp->vscal[i+1]-1]);
            }
            else assert(0);

            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vld, -r0,
                              SToff[lp->vvscal[i+1]-1].sa[2], 0); 
            if (vld == VDLD)
            {
               if (lp->vsoflag[i+1] & VS_ACC)
                  inst = VDADD;
               else if (lp->vsoflag[i+1] & VS_MAX)
                  inst = VDMAX;
               else if (lp->vsoflag[i+1] & VS_MIN)
                  inst = VDMIN;
               else
                  assert(0);

            #if defined(X86) && defined(AVX)
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1,
                                 -r0, STiconstlookup(0x3276));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, inst,-r0,-r0,
                                 -r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1,
                                 -r0, STiconstlookup(0x3715));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, inst,-r0,-r0,
                                 -r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSTS,
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #else
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1,
                                 -r0, STiconstlookup(0x33));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, inst,-r0,-r0,
                                 -r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSTS,
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #endif
            }
            else
            {
               if (lp->vsoflag[i+1] & VS_ACC)
                  inst = VFADD;
               else if (lp->vsoflag[i+1] & VS_MAX)
                  inst = VFMAX;
               else if (lp->vsoflag[i+1] & VS_MIN)
                  inst = VFMIN;
               else
                  assert(0);

            #if defined(X86) && defined(AVX)
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x7654FEDC));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, inst,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x765432BA));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, inst,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x76CD3289));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, inst,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSTS,
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #else
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x3276));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, inst,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
                                 -r1, -r0, STiconstlookup(0x5555));
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, inst,
                                 -r0,-r0,-r1);
               iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSTS,
                                 SToff[lp->vscal[i+1]-1].sa[2], -r0, 0);
            #endif
            }
         }
         else /* VS_MUL/ VS_DIV*/
         {
            iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                  "Reduce vector for %s", STname[lp->vscal[i+1]-1]);
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vld, -r0,
                              SToff[lp->vvscal[i+1]-1].sa[2], 0);
            iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vsst,
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

void AddInstBackupRecovery(BLIST *scope, LOOPQ *lp)
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
 *                can be done before after duplicating the blocks
 */
#if 0                  
                  if (ip->inst[0] == LD && ip->inst[2] == SToff[lp->I-1].sa[2]
                      && !(ip->prev->inst[0] == CMPFLAG && ip->prev->inst[1] ==
                           CF_LOOP_UPDATE))
                  {
                     k = ip->inst[1];
                     InsNewInst(bp, ip, NULL, ADD, k, k, 
                                STiconstlookup(vlen-1) );
                  }
#endif                  
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

void AddBackupRecover(LOOPQ *lp, BBLOCK *bp0, int spath)
/*
 * adds back in vector path and recovery at the first of scalar path
 */
{

}

void AddScalarUpdate(LOOPQ *lp, BBLOCK *bp0, int spath)
/*
 * Vector-to-scalar reduction to operate on scalar in Scalar Restart
 */
{
   int i, j, k, N;
   short r0, r1, op;
   short scal, spsflag, vpsflag, vscal, vvflag;
   INSTQ *ip;
   BLIST *bl;
   LOOPPATH *vp, *sp;

   vp = PATHS[VPATH];
   sp = PATHS[spath];

/*
 * Note: variable that is set on vector path needs vector to scalar reduction.
 * If there are other scalar paths, we may need vector to scalar reduction 
 * for that also, as at the end of scalar path, scalar is updated to vector.
 * NOTE: Right now, we just consider 2 paths!!!
 */
   assert(NPATH <= 2); /* we will handle more paths later*/
   
   for (i=1, N=vp->vscal[0]; i <= N; i++)
   {
      vscal = vp->vscal[i];
      vpsflag = vp->sflag[i]; /* flags for the vars */
/*
 *    if it is private vector variable, nothing to do.
 */
      if (vpsflag & SC_PRIVATE )  
         continue; 
   
      if ( (vpsflag & SC_USE) && !(vpsflag & SC_SET))
         continue; /* not modified in vpath, so use scalar directly */

      else if ( vpsflag & SC_SET )
      {
/*
 *       This var is updated in vpath as vector. So, reduce the updated one 
 *       into scalar
 */
         vvflag = vp-> vsflag[i];
         if (vvflag & VS_ACC ) /* accumulator! so, reduce it*/
         {
           ip = bp0->instN;
           r0 = GetReg(FLAG2TYPE(lp->vflag));
           r1 = GetReg(FLAG2TYPE(lp->vflag));
/*
 *         Reduce this vector in scalar, vvscal has the vector of corresponding
 *         scalar of vscal in vector path
 */         
           if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
           {
              ip = PrintComment(bp0, ip, NULL, "Reduce accumulator vector for%s"
                                ,STname[vscal-1]);
              ip = InsNewInst(bp0, ip, NULL, VFLD, -r0, 
                              SToff[lp->vvscal[i]-1].sa[2], 0);
               #if defined(X86) && defined(AVX)
                  ip = InsNewInst(bp0, ip, NULL, VFSHUF, -r1, -r0, 
                                    STiconstlookup(0x7654FEDC));
                  ip = InsNewInst(bp0, ip, NULL,VFADD,-r0,-r0,-r1);
                  ip = InsNewInst(bp0, ip, NULL, VFSHUF, -r1, -r0, 
                                    STiconstlookup(0x765432BA));
                  ip = InsNewInst(bp0, ip, NULL,VFADD,-r0,-r0,-r1);
                  ip = InsNewInst(bp0, ip, NULL, VFSHUF, -r1, -r0, 
                                    STiconstlookup(0x76CD3289));
                  ip = InsNewInst(bp0, ip, NULL,VFADD,-r0,-r0,-r1);
                  ip = InsNewInst(bp0, ip, NULL, VFSTS, 
                                    SToff[lp->vscal[i]-1].sa[2], -r0, 0);
               #else
                  ip = InsNewInst(bp0, ip, NULL, VFSHUF, -r1, -r0, 
                                    STiconstlookup(0x3276));
                  ip = InsNewInst(bp0, ip, NULL,VFADD,-r0,-r0,-r1);
                  ip = InsNewInst(bp0, ip, NULL, VFSHUF, -r1, -r0, 
                                    STiconstlookup(0x5555));
                  ip = InsNewInst(bp0, ip, NULL,VFADD,-r0,-r0,-r1);
                  ip = InsNewInst(bp0, ip, NULL, VFSTS,
                                    SToff[lp->vscal[i]-1].sa[2], -r0, 0);
               #endif
           }
           else  /* type is double */
           {
              ip = PrintComment(bp0, ip, NULL, "Reduce accumulator vector for%s"
                                ,STname[vscal-1]);
              ip = InsNewInst(bp0, ip, NULL, VDLD, -r0, 
                              SToff[lp->vvscal[i]-1].sa[2], 0);
               #if defined(X86) && defined(AVX)
                  ip = InsNewInst(bp0, ip, NULL, VDSHUF, -r1, -r0, 
                                    STiconstlookup(0x3276));
                  ip = InsNewInst(bp0, ip, NULL,VDADD,-r0,-r0,-r1);
                  ip = InsNewInst(bp0, ip, NULL, VDSHUF, -r1, -r0, 
                                    STiconstlookup(0x3715));
                  ip = InsNewInst(bp0, ip, NULL,VDADD,-r0,-r0,-r1);
                  ip = InsNewInst(bp0, ip, NULL, VDSTS,
                                    SToff[lp->vscal[i]-1].sa[2], -r0, 0);
               #else
                  ip = InsNewInst(bp0, ip, NULL, VDSHUF, -r1, -r0, 
                                    STiconstlookup(0x33));
                  ip = InsNewInst(bp0, ip, NULL,VDADD,-r0,-r0,-r1);
                  ip = InsNewInst(bp0, ip, NULL, VDSTS,
                                    SToff[lp->vscal[i]-1].sa[2], -r0, 0);
               #endif
           }

           GetReg(-1);

         }
         else /* not accumulator. So, must be MUL/DIV. just broadcast */
         {

         }
      }
   }
}

void AddVectorUpdate(LOOPQ *lp, BBLOCK *blk, int spath)
/*
 * updates necessary vars for the next speculative vector iteration
 */
{
   BLIST *bl;
   INSTQ *ip;
   int i, j, k, N, N1, found;
   short scal, spsflag, vpsflag, vscal, vsflag;
   short vlen;
   LOOPPATH *vp, *sp;
   BBLOCK *bp;
   enum inst vsld, vshuf, vst, fst;
   short r0;

   vp = PATHS[VPATH];
   sp = PATHS[spath];
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
 * Check all the vectors of vector path, those may need to update for
 * next iteration. vars which accessed in scalar path only, need no recovery.
 */

   for (i=1, N=vp->vscal[0]; i <= N; i++)
   {
      //scal = vp->scal[i];   /* scal and vscal are same for vector path */
      vscal = vp->vscal[i];
      vpsflag = vp->sflag[i]; /* flags for the vars */
      vsflag = vp->vsflag[i]; /* lp->vsflag[i]*/
/*
 *    if it is private vector variable, nothing to do.
 */
      if (vpsflag & SC_PRIVATE )
         continue;
/*
 *    simple case first: this var is used in vector path as vscal but 
 *    not set in this path like: amax in iamax.b
 *    NOTE: vscal and scal are the same for vector path as we skip integer and
 *    moving array pointer from scal.
 *    HERE. If a variable is used in vector path but set in scalar path, we 
 *    need to update the appropriate vector at the update stage.
 */
      if ( (vpsflag & SC_USE) && !(vpsflag & SC_SET)) 
      {
/*
 *       check the scal flag for the scalar path 
 *       If it is set here, update the appropriate vector.
 */
         found = 0;
         for (j=1, N1=sp->scal[0]; j <= N1; j++)
         {
            if (vscal == sp->scal[j])
            {
               found = 1;
               spsflag = sp->sflag[j];
               break;
            }
         }
         if (found)
         {
            if (spsflag & SC_SET)
            {
               ip = FindCompilerFlag(blk, CF_SSV_VUPDATE);
               if (!ip)
                  ip = InsNewInst(blk, NULL, NULL, CMPFLAG, CF_SSV_VUPDATE, 
                                  0, 0);
               ip = PrintComment(blk, ip, NULL,"Vector Update of %s", 
                                 STname[vscal-1]);
               r0 = GetReg(FLAG2TYPE(lp->vflag));
               ip = InsNewInst(blk, ip, NULL, vsld, -r0, 
                               SToff[vscal-1].sa[2], 0);
               ip = InsNewInst(blk, ip, NULL, vshuf,-r0, -r0, 
                               STiconstlookup(0));
               ip = InsNewInst(blk, ip, NULL, vst, 
                               SToff[lp->vvscal[i]-1].sa[2], -r0, 0);
               GetReg(-1);
            }
         }
       }
/*
 *    although a var is not use in vector path but set, we need to check
 *    whether it is live out at the exit of loop. If so, we need to update
 *    the vector. not implemented yet as not needed for iamax
 */
      else if ( !(vpsflag & SC_USE) && (vpsflag & SC_SET))
      {

      }
/*
 *    We need to further check whether it is reduction variable. in case so,
 *    we will need to update the vector accordingly. 
 *    Not implemented yet.
 */
      else if ( (vpsflag & SC_USE) && (vpsflag & SC_SET) )
      {
         r0 = GetReg(FLAG2TYPE(lp->vflag));
         ip = FindCompilerFlag(blk, CF_SSV_VUPDATE);
         if (!ip)
            ip = InsNewInst(blk, NULL, NULL, CMPFLAG, CF_SSV_VUPDATE, 0, 0);      
/*
 *       Majedul: FIXME: for accumulator init, shouldn't we need an VXOR
 *       though most of the time it works as vmoss/vmosd automatically
 *       zerod the upper element. but what if the optimization transforms it
 *       into reg-reg move. vmovss/vmosd for reg-reg doesn't make the upper 
 *       element zero!!! 
 *       NOTE: so far, it works. as temp reg normally uses a move from mem
 *       before reg-reg move, which makes the upper element zero.
 */
         if (VS_ACC & lp->vsflag[i])
            ip = PrintComment(blk, ip, NULL,
               "Init accumulator vector for %s", STname[lp->vscal[i]-1]);
         else
            ip = PrintComment(blk, ip, NULL,
               "Init vector equiv of %s", STname[lp->vscal[i]-1]);
         ip = InsNewInst(blk, ip, NULL, vsld, -r0,
                    SToff[lp->vscal[i]-1].sa[2], 0);
         if (!(VS_ACC & lp->vsflag[i]))
            ip = InsNewInst(blk, ip, NULL, vshuf, -r0, -r0, STiconstlookup(0));
         ip = InsNewInst(blk, ip, NULL, vst,
                    SToff[lp->vvscal[i]-1].sa[2], -r0, 0);
         GetReg(-1);
      }

   }

}

void SetScalarRestartOptFlag(LOOPQ *lp, int *lpOpt, int *usesIndex, int *usesPtr)
{
   int i, j;
   int lo, ui, up;
   ILIST *il;
   struct ptrinfo *pi;
/*
 * by default, all are set. 
 */
   lo = 1; ui = 1; up = 1;
/*
 * Check all paths of the loop is optimizable for loop control and loop_mov_ptr
 * Right now, consider both yes or no. Later can be optimized for either one
 */
   for (i = 0; i< NPATH; i++ )
   {
      if ( !(PATHS[i]->lpflag & LP_OPT_LCONTROL) || 
           !(PATHS[i]->lpflag & LP_OPT_MOVPTR) )
      {
         lo = 0;
         break;
      }
   }
/*
 * Check whether index is used inside the optloop
 */
   if (lo)
   {
      KillLoopControl(lp);
      il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
      if (il)
      {
         lp->flag |= L_IREF_BIT;
         KillIlist(il);
      }
      else ui = 0;
      pi = FindMovingPointers(lp->blocks);
      if (!pi)
         up = 0;
   }

/*
 * Update the flags
 */
   *lpOpt = lo;
   *usesIndex = ui;
   *usesPtr = up;
}

void DupBlkPathWithOpt(LOOPQ *lp, BLIST **dupblks, int islpopt, int UsesIndex,
                       int UsesPtrs, int path)
/*
 * duplicates loop blks vector lenth's time with loop index and ptr update
 * optimization if valid
 */
{
   int i, j;
   int vlen, URbase, cflag;
   int iv;
   BBLOCK *newCF, *bp;
   INSTQ *ip, *ipn;
   BLIST *bl;
   char ln[512];
   struct ptrinfo *pi;
   extern int FKO_BVTMP;
   static int fid = 1;

   vlen = Type2Vlen(lp->vflag);
   URbase = (lp->flag & L_FORWARDLC_BIT) ? 0 :vlen-1;
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);

   for ( i=0; i < vlen; i++)
   {
      SetVecBit(lp->blkvec, lp->header->bnum-1, 0);
      FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
      newCF = DupCFScope(lp->blkvec, iv, fid++, lp->header);
      assert(newCF->ilab);
      SetVecBit(lp->blkvec, lp->header->bnum-1,1);
      iv = BitVecCopy(iv, lp->blkvec);
      dupblks[i] = CF2BlockList(NULL, iv, newCF);   
#if 0
      fprintf(stderr, "dupblks : ");
      for (bl = dupblks[i]; bl; bl=bl->next)
         fprintf(stderr, "%d ", bl->blk->bnum);
      fprintf(stderr, "\n");
      ShowFlow(NULL, newCF);      
#endif
/*
 *    if all paths are loop control and moving ptr optimizable, then delete
 *    loop control and moving ptr form duplicated blocks and update the load
 *    of the pointer accordingly.
 *    NOTE: don't kill loop control and moving pointer from the optloop. it 
 *    may be needed in vector transform stage. so, delete those from copied
 *    blocks
 */
      if (islpopt)
      {
/*
 *       Assuming loop control is deleted before the scalar Restart.
 *       Now, find moving pointer and update pointer loads accordingly
 *       in dublicated blocks
 */
         pi = FindMovingPointers(dupblks[i]);
         assert(pi);
         ip = KillPointerUpdates(pi, i);
         for (; ip; ip = ipn)
         {
            ipn = ip->next;
            free(ip);
         }
         UpdatePointerLoads(dupblks[i], pi, i);
         KillAllPtrinfo(pi);
/*
 *       Need to check the loop for forwarding or not!
 */
         if (UsesPtrs)
            UpdateUnrolledIndices(dupblks[i], lp->I, (lp->flag & L_NINC_BIT)?
                                  URbase-i:URbase+i);
      }
/*
 *    Need to kill backedge (branch inst from tail of loop), add JMP to 
 *    the start of next scalar iteration pro-actively.
 *    for last iteration, need to create a block and check condition for loop.
 *    NOTE: HERE HERE. assuming each iteration is started with a label 
 *    S_path_iter
 *
 *    NOTE: CFG is completely messedup but don't update the CFG yet, 
 *    should update it after vect transform.
 */
      for (bl=lp->tails; bl; bl=bl->next)
      {
         bp = FindBlockInListByNumber(dupblks[i],bl->blk->bnum);
         assert(bp);
/*       search for the branch instruction */
         for (ip = bp->inst1; ip; ip=ip->next)
         {
            if (IS_BRANCH(ip->inst[0]))
            {
               ip = DelInst(ip); /* recalc the inst1, instN*/
               break;
            }
         }
/*       Add new jump to 1st block of next iteration  */
         if (ip)
         {
            sprintf(ln, "_S_%d_%d",path,i+1); /*1st block of next */
            cflag = STlabellookup(ln);
            InsNewInst(bp, NULL,ip,JMP,-PCREG,cflag,0);
         }
         else
         {
            ip = bp->instN; /*CMPFLAG isn't considered as active inst*/
            assert(ip);
            sprintf(ln, "_S_%d_%d",path,i+1); /*1st block of next */
            cflag = STlabellookup(ln);
            InsNewInst(bp, ip,NULL,JMP,-PCREG,cflag,0);
         }
            
      }
      
   }
   
}

BBLOCK *GenScalarRestartPathCode(LOOPQ *lp, int path, int vlen,BLIST *ftheads, 
                                 int ivtails)
{
   int i, j;
   char ln[512];
   int cflag;
   BBLOCK *bp0, *bp, *bptop, *bpi;
   BLIST **dupblks;
   BLIST *bl;
   INSTQ *ip, *ip0;
   int islpopt, UsesIndex, UsesPtrs;


   dupblks = malloc(sizeof(BLIST*)*vlen);
   assert(dupblks);
/*
 * Set all flags to optimized the duplicated blocks
 */
   SetScalarRestartOptFlag(lp, &islpopt, &UsesIndex, &UsesPtrs);

/*
 * dublicates blocks with required optimization
 */
   DupBlkPathWithOpt(lp, dupblks, islpopt, UsesIndex, UsesPtrs, path);

/*
 * create a new BBLOCK structure for the scalar restart of this path
 */
   bptop = bp0 = NewBasicBlock(NULL,NULL);
   sprintf(ln, "_Scalar_Restart_%d",path);
   cflag = STlabellookup(ln);
   InsNewInst(bp0, NULL,NULL,LABEL,cflag,0,0);
/*
 * Here starts the backup recovery operation.
 * Right now, it is dummy function. 
 */
   AddBackupRecover(lp, bp0, path);
/*
 * Here, we start vector to scalar update
 */
   AddScalarUpdate(lp, bp0, path);

/*
 * copy duplicated blocks 
 */
   for (i = 0; i < vlen; i++)
   {
/*
 *    create new block for each iteration
 */
      bp = NewBasicBlock(bp0, NULL);
      bp0->down = bp;
      bp->up = bp0;
      bp0 = bp;
      sprintf(ln, "_S_%d_%d",path,i);
      cflag = STlabellookup(ln);
      ip = InsNewInst(bp, NULL, NULL, LABEL, cflag,0,0);
/*
 *    copy duplicated blocks for each iteration
 */
      for (bl=ftheads; bl; bl = bl->next)
      {
         for (bp=bl->blk;bp; bp = bp->down)
         {
            if (!BitVecCheck(lp->blkvec, bp->bnum-1))
               break;
            bp0->down = FindBlockInListByNumber(dupblks[i], bp->bnum);
            bp0->down->up = bp0;
            bp0 = bp0->down;
            if (BitVecCheck(ivtails, bp->bnum-1))
               break;
         }
         bp0->down = NULL;
      }
   }
/*
 *    create a new block at the last of scalar restart path to implement vector
 *    update stage and to check and jump back to tail of loop
 */
   bp = NewBasicBlock(bp0, NULL);
   bp0->down = bp;
   bp->up = bp0;
   bp0 = bp;
   sprintf(ln, "_S_%d_%d",path,i);
   cflag = STlabellookup(ln);
   ip = InsNewInst(bp, NULL, NULL, LABEL, cflag,0,0);
/*
 * Create a label to indicate the vector update stage
 */
   sprintf(ln, "_IFKO_VECTOR_UPDATE_%d",path);
   cflag = STlabellookup(ln);
   ip = InsNewInst(bp, ip, NULL, LABEL, cflag,0,0);
/*
 * Add vector update for this path
 */
   AddVectorUpdate(lp, bp, path);

/*
 * To avoid multiple posttails (and also violation of loop normal form), 
 * the original loop tail is splited into 2 parts here. We will jump back
 * to the loop test from here. 
 * NOTE: LOOP update and loop test is become isolated now. Need to check 
 * whether it creates problem later. Need to make sure that the condition
 * codes (status register) also don't changed by this time.
 *    
 * HERE HERE, 
 * We can do better: we can check all paths whether we can apply 
 * loop control and ptr update optimization. I keep that info in
 * path->lpflag. if it is true, we can kill all the loop comtrol and 
 * ptr updates and jump back to original loop tail. 
 *
 * FIXED: JUMP back to tail will not work if tail contains other statements
 * than just the ptr and lc update!!!! 
 * HERE HERE, to solve the issue add LOOP_TEST for all the cases before 
 * loop control..... 
 * Test Case : modified asum: 
 *       if (x < 0.0)
 *          x = -x;
 *       sum += x;
 *
 */
   ip0 = bp->instN; /* aisnt not considered CMPFLAG and COMMENT*/
      
   if (islpopt)
   {
      /*
       *       Jump back to loop tail. assume only one tail now
       */
      assert(lp->tails->blk && !lp->tails->next);
      bpi = lp->tails->blk;
#if 0
      if (bpi->ainst1->inst[0] == LABEL)
         cflag = bpi->ainst1->inst[1];
      else
      {
         sprintf(ln,"%s_LOOP_TEST",STname[lp->body_label-1]);
         cflag = STlabellookup(ln);
         InsNewInst(bpi, NULL, bp->ainst1, LABEL, cflag,0,0);
      }
#endif
/*
 *    We need to jump back before the ptr update but after any other 
 *    instruction. So, insert LABEL accordingly.
 */
      ip = FindCompilerFlag(bpi, CF_LOOP_PTRUPDATE);
      if (!ip) ip = FindCompilerFlag(bpi, CF_LOOP_UPDATE);
      assert(ip);
      sprintf(ln,"%s_LOOP_TEST",STname[lp->body_label-1]);
      cflag = STlabellookup(ln);
      InsNewInst(bpi, NULL, ip, LABEL, cflag,0,0);
   }
   else
   {
/*
 *    split tail by adding label to the tail of original loop.
 *    NOTE: add label before CMPFLAG
 */
      assert(lp->tails->blk && !lp->tails->next);
      ip = FindCompilerFlag(lp->tails->blk, CF_LOOP_TEST);
      assert(ip);
      sprintf(ln,"%s_LOOP_TEST",STname[lp->body_label-1]);
      cflag = STlabellookup(ln);
      InsNewInst(lp->tails->blk, NULL, ip, LABEL, cflag,0,0);
   }
/*
 * now jump back to new label in tail
 */
   InsNewInst(bp, ip0, NULL, JMP, -PCREG, cflag, 0);
   bp->down = NULL;

/*
 * Return the base pointer 
 */
   return bptop;
}

void ScalarRestart(LOOPQ *lp)
/*
 * changing in plan: 1st generate the whole code structure for each path 
 * which can be explored by down/up pointer, then update the original code
 */
{
   int i, j, k;
   int iv, ivtails, vlen;
   BBLOCK **bpaths; /* hold complete code structure for each path*/
   BBLOCK *bp0, *bdown, *bp;
   BLIST *ftheads, *bl, *vbl;
   INSTQ *ip;
   extern BBLOCK *bbbase;
   extern int FKO_BVTMP;

   assert(VPATH!=-1);
   bpaths = malloc(sizeof(BBLOCK*)*NPATH);
   vlen = Type2Vlen(lp->vflag);
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
 *    Find all fall thru path headers in loop which will be needed at code 
 *    generation
 */
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);
   ivtails = BlockList2BitVec(lp->tails);
   ftheads = FindAllFallHeads(NULL, lp->blkvec, lp->header, ivtails, iv);
   ftheads = ReverseBlockList(ftheads);

/*
 * Generate scalar restart code for each path first before any update
 */
   for (i=0; i < NPATH; i++)
   {
      if ( i == VPATH)
      {
         bpaths[i] = NULL; /* nothing for vector path*/ 
         continue;
      }
/*
 *    Generate code for Scalar Restart before any update of original code
 */
      bpaths[i] = GenScalarRestartPathCode(lp, i, vlen, ftheads, ivtails);
#if 0
      fprintf(stdout, "SCALAR RESTART CODE :\n");
      PrintInst(stdout, bpaths[i]);
      exit(0);
#endif           
   }

/*
 * Now check each path to figure out where to copy 
 */
   for (i=0; i < NPATH; i++)
   {
      if (i == VPATH) continue;
/*
 *    check with vector path, determine where to implement scalar restart
 *    Requirement: blocklist blocks in path must be in sequential according
 *    to control flow
 */
      for (bl = PATHS[i]->blocks,vbl = PATHS[VPATH]->blocks; bl && vbl ;
           bl = bl->next, vbl = vbl->next)
      {
         if (bl->blk != vbl->blk)
         {
            /*fprintf(stderr, "Split in vpath blk = %d, spath blk = %d\n",
                    vbl->blk->bnum, bl->blk->bnum);*/
            break;
         }
      }
      assert(bl); /* atleast one block which is not common */
/*
 *    Add duplicated blocks after this block, will delete this path later
 */
      for ( ; bp0 != bl->blk; bp0 = bp0->down);
      assert(bp0); /* starting of scalar */
/*
 *    add the generated scalar restart code at the appropriate position
 */
#if 0
      bdown = bp0->down;
      ip = InsNewInstAfterLabel(bp0, CMPFLAG, CF_SCAL_RES,0,0);
      while(ip) ip = DelInst(ip);
      bp0->down = bpaths[i];
      bpaths[i]->up = bp0;
      
      for (bp0 = bpaths[i]; bp0->down; bp0 = bp0->down);
      bp0->down = bdown;
      bdown->up = bp0;
#else
/*
 *    bp0 is the starting block. figure out the block upto which we will 
 *    kill. 
 */
      bdown = bp0->down;
      for (; bdown; bdown=bdown->down)
      {
         if (!FindInList(PATHS[VPATH]->blocks, bdown) 
             && FindInList(lp->blocks,bdown))
         {
            //bdown->up = KillBlock(bdown->up, bp);
         }
         else
            break;
      }
      assert(bdown); /* right now, it should be NULL. atleast tail blk! */
/*
 *    delete instruction from bp0. NOTE: don't delete bp0 itself. 
 */
      ip = InsNewInstAfterLabel(bp0, CMPFLAG, CF_SCAL_RES,0,0);
      while(ip) ip = DelInst(ip);
      
/*
 *    link blks of scalar restart
 */
      bp0->down = bpaths[i];
      bpaths[i]->up = bp0;
/*
 *    link the common blks down.
 *    NOTE: There may be several special case which is not handle yet. 
 *    if there are multiple if statement, we may need to change this simple
 *    logic
 */
      for (bp0 = bpaths[i]; bp0->down; bp0 = bp0->down);
      bp0->down = bdown;
      bdown->up = bp0;

#endif

   }
}

void ScalarRestart0(LOOPQ *lp)
/*
 * This is an old implementation. It does have issue. It only handles the code
 * structure exactly like iamax. Still, I've kept it for the reference of 
 * various implementation consideration.
 *
 */
{
   BBLOCK *bp, *bp0, *bpi, *newCF, *bdown;
   BLIST *bl, *vbl, *dupblks, *ftheads, *delblks;
   INSTQ *ip, *ip0, *ipn;
   ILIST *il;
   struct ptrinfo *pi, *pi0;
   int i, j, cflag, vlen, URbase;
   int islpopt, UsesIndex, UsesPtrs;
   int iv, ivtails;
   char ln[512];
   extern BBLOCK *bbbase;
   extern int FKO_BVTMP;
   
/*
 * Need at least one path to vectorize
 */
   assert(VPATH!=-1);

/*
 * All the parameter settings for scalar restart and to optimize the structure
 */

   vlen = Type2Vlen(lp->vflag);
   URbase = (lp->flag & L_FORWARDLC_BIT) ? 0 :vlen-1;
/*
 * Check all paths of the loop is optimizable for loop control and loop_mov_ptr
 * Right now, consider both yes or no. Later can be optimized for either one
 */
   islpopt = 1;
   UsesIndex = 1;
   UsesPtrs = 1;
   for (i = 0; i< NPATH; i++ )
   {
      if ( !(PATHS[i]->lpflag & LP_OPT_LCONTROL) || 
           !(PATHS[i]->lpflag & LP_OPT_MOVPTR) )
      {
         islpopt = 0;
         break;
      }
   }
   /*fprintf(stderr, "is loop optimizable = %d\n",islpopt);*/
/*
 * Check whether index is used inside the optloop
 */
   if (islpopt)
   {
      KillLoopControl(lp);
      il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
      if (il) KillIlist(il);
      else UsesIndex = 0;
      pi0 = FindMovingPointers(lp->blocks);
      if (!pi0)
         UsesPtrs = 0;
   }
#if 0
   fprintf(stderr, "Optloop tail: \n");
   PrintThisBlockInst(stderr, lp->tails->blk);
#endif
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
            /*fprintf(stderr, "Split in vpath blk = %d, spath blk = %d\n",
                    vbl->blk->bnum, bl->blk->bnum);*/
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
#if 1
         fprintf(stderr, "dupblks : ");
         for (bl = dupblks; bl; bl=bl->next)
         {
            fprintf(stderr, "%d ", bl->blk->bnum);
         }
         fprintf(stderr, "\n");
      
         ShowFlow(NULL, newCF);     
#endif      
/*
 *       if all paths are loop control and moving ptr optimizable, then delete
 *       loop control and moving ptr form duplicated blocks and update the load
 *       of the pointer accordingly.
 *       NOTE: don't kill loop control and moving pointer from the optloop. it 
 *       may be needed in vector transform stage. so, delete those from copied
 *       blocks
 */
         if (islpopt)
         {
/*
 *          Assuming loop control is deleted before the scalar Restart.
 *          Now, find moving pointer and update pointer loads accordingly
 *          in dublicated blocks
 */
            pi = FindMovingPointers(dupblks);
            assert(pi);
            ip = KillPointerUpdates(pi, j);
            for (; ip; ip = ipn)
            {
               ipn = ip->next;
               free(ip);
            }
            UpdatePointerLoads(dupblks, pi, j);
            KillAllPtrinfo(pi);
/*
 *          Need to check the loop for forwarding or not!
 */
            if (UsesPtrs)
               UpdateUnrolledIndices(dupblks, lp->I, (lp->flag & L_NINC_BIT)?
                                     URbase-j:URbase+j);
         }
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
 *    create a new block at the last of scalar restart path to implement vector
 *    update stage and to check and jump back to tail of loop
 */
      bp = NewBasicBlock(bp0, NULL);
      bp0->down = bp;
      bp->up = bp0;
      bp0 = bp;
      sprintf(ln, "_S_%d_%d",i,j);
      cflag = STlabellookup(ln);
      ip = InsNewInst(bp, NULL, NULL, LABEL, cflag,0,0);
/*
 *    Create a label to indicate the vector update stage
 */
      sprintf(ln, "_IFKO_VECTOR_UPDATE_%d",i);
      cflag = STlabellookup(ln);
      ip = InsNewInst(bp, ip, NULL, LABEL, cflag,0,0);
/*
 *    Add vector update for this path
 */
      AddVectorUpdate(lp, bp, i);

/*
 *    Creating two posttails would complicate the analysis later. So, need to
 *    skip that by intelligent design.
 */
      
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
      
      if (islpopt)
      {
/*
 *       Jump back to loop tail. assume only one tail now
 */
         assert(lp->tails->blk && !lp->tails->next);
         bpi = lp->tails->blk;
         
         if (bpi->ainst1->inst[0] == LABEL)
            cflag = bpi->ainst1->inst[1];
         else
         {
            sprintf(ln,"%s_LOOP_TEST",STname[lp->body_label-1]);
            cflag = STlabellookup(ln);
            InsNewInst(bpi, NULL, bp->ainst1, LABEL, cflag,0,0);
         }

      }
      else
      {
/*
 *       split tail by adding label to the tail of original loop.
 *       NOTE: add label before CMPFLAG
 */
         assert(lp->tails->blk && !lp->tails->next);
         ip = FindCompilerFlag(lp->tails->blk, CF_LOOP_TEST);
         assert(ip);
         sprintf(ln,"%s_LOOP_TEST",STname[lp->body_label-1]);
         cflag = STlabellookup(ln);
         InsNewInst(lp->tails->blk, NULL, ip, LABEL, cflag,0,0);
      }
/*
 *    now jump back to new label in tail
 */
      InsNewInst(bp, ip0, NULL, JMP, -PCREG, cflag, 0);
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
 *          Need to check whether it is not part of other paths yet to test.
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

   KillLoopControl(lp);
/*
 * NOTE: Loop peeling must be called before cleanup, as it may chnage the loop
 * control structure of main loop, hence the cleanup loop. 
 * all the loop control should be killed before loop peeling or, cleanup loop.
 */
   isSSV = 1;
#if 1   
   if (IsLoopPeelOptimizable(lp))
      GenForceAlignedPeeling(lp);
#endif

/*
 * Generate cleanup loop before speculative vector transformation, as it will
 * change the optloop structure
 */
   GenCleanupLoop(lp);
#if 0
   fprintf(stdout, "After Cleanup\n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif   
/*
 * Add loop peeling to force align with the vector alignemnt requirement.
 * will work when only there is only one moving array pointer. 
 * Of course, it should be done before vector trnasformation, before changing
 * the optloop structure.
 * NOTE: checking of force alignment is done before cleanup but I added this
 * after cleanup to take advantages of the analysis of cleanup directly!
 */
#if 0
   fprintf(stdout, "LIL after cleanup generation\n");
   PrintInst(stdout,bbbase);
   exit(0);
#endif
/* skip the loop controls for now*/
#if 0
/*
 * FIXME: LOOP_PTR_UPDATE is not introduced yet at 1st. ptr_update is located 
 * before anyother loop control flag. So, KillPtrUpdate is necessary before the
 * OptimizeLoopControl. 
 * We can kill the ptr updates for loop tails only and pass it with the func,
 * Still need to check the function.
 */
   pi0 = FindMovingPointers(lp->tails);
   ippu = KillPointerUpdates(pi0,1);
   /*OptimizeLoopControl(lp, 1, 0, NULL);*/
   OptimizeLoopControl(lp, 1, 0, ippu);
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

#if 0
   UnrollCleanup(lp,1);
#endif

#if 0
   fprintf(stdout, " LIL AFTER SSV LOOP \n");
   PrintInst(stdout, bbbase);
   //fprintf(stdout, " SYMBOL TABLE \n");
   //PrintST(stdout);
   exit(0);
#endif
/*
 * It's time to kill the data structure for paths. 
 * If path info is needed later, skip this deletion.
 */
   KillPathTable();

/*
 * Update prefetch information
 * NOTE: according to new program states, it will be updated in STATE 3.
 * So, we no longer need this here.
 */
#if 0   
   UpdateNamedLoopInfo();
#endif

#if 0   
/*
 * Everything is messed up already. So, re-make the control flow and check it 
 */
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);

/*
 * Update prefetch information
 */
   UpdateNamedLoopInfo();

#endif

#if 0
   int i,N;
   lp = optloop; 
   fprintf(stderr, "vars info:[%d] \n",lp->varrs[0]);
   for (i=1,N=lp->varrs[0]; i<=N; i++)
   {
      fprintf(stderr, "vars = %d ",lp->varrs[i]);
   }
   fprintf(stderr, "Prefetch info: \n");
   for (i=1,N=lp->pfarrs[0]; i<=N; i++)
   {
      fprintf(stderr, "Arr = %d ",lp->pfarrs[i]);
   }
#endif

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
      PrintST(stdout);
      exit(0);
#endif 

   return(0);
}

short *UpdateBlkWithNewVar(BBLOCK *bp0, int vid, short *sp)
/*
 * take a var list (format: N, v1, v2,..) and blk; rename and update with
 * new var and returns the newvar list in same format
 */
{
   int i, j, k, N;
   short sv, nv, op;
   short *newvars; 
   char *strnvar;
   INSTQ *ip;
   
/*
 * allocate space for newvars
 */
#if 0
   newvars = calloc(sp[0]+1, sizeof(short));
#else
   newvars = malloc(sizeof(short)*(sp[0]+1));
   for (i=0; i < (sp[0]+1); i++)
      newvars[i] = 0;
#endif
/*
 * Findout the first set of var, update the destination and update all later
 * use of the var as dest/src
 */

   for ( N=sp[0], i=1; i <= N; i++ )
   {
      sv = sp[i];
      for (ip = bp0->ainst1, j=1; ip; ip=ip->next, j++)
      {
         op = ip->inst[1];
         if (op > 0)
         {
            k = SToff[op-1].sa[1]; /* index of original ST */
            /*fprintf(stderr, "%d: %d\n", j, k);*/
            if (k == sv)
            {
               /*fprintf(stderr, " got 1st set for %s at inst %d\n", 
                       STname[sv-1], j);*/
               strnvar = malloc(sizeof(char)*(strlen(STname[sv-1])+4));
               sprintf(strnvar,"%s_%d",STname[sv-1],vid);
               nv = InsertNewLocal(strnvar,STflag[sv-1]);
               newvars[i] = nv; /* update the param */
               /*ip->inst[1] = nv;*/
               ip->inst[1] = SToff[nv-1].sa[2];
               break;
            }
         }
      }
      /* findout the use of this var in remaining instruction */
      if (!ip) continue;
      for (ip=ip->next; ip; ip=ip->next)
      {
         for (j=1; j < 4; j++)
         {
            op = ip->inst[j];
            if (op > 0)
            {
               k = SToff[op-1].sa[1]; /* index of original ST */
               if (k == sv)
               {
                  /*ip->inst[j] = nv;*/
                  ip->inst[j] = SToff[nv-1].sa[2];
               }
            }
         }
      }
   }
   return newvars;
}

short RemoveBranchWithMask(BBLOCK *sblk)
/*
 * removes conditional branches from a split blk with appropriate CMP and mask 
 */
{
   int i, j, k;
   INSTQ *ip, *ip0;
   short mask;
   static int maskid = 0;
   char *cmask;
   int type;
   int freg0;
   enum inst fst;
/*
 * NOTE: Although SSE doesn't support GT and GE, we can implement those with
 * NLT, NLE
 */
   static enum inst
      brinsts[] = {JEQ, JNE, JLT, JLE, JGT, JGE},
      fcmpwinsts[] = {FCMPWEQ, FCMPWNE, FCMPWLT, FCMPWLE, FCMPWGT, FCMPWGE},
      dcmpwinsts[] = {FCMPDWEQ, FCMPDWNE, FCMPDWLT,FCMPDWLE, FCMPDWGT, 
                      FCMPDWGE};
#if 0      
      #if defined(AVX)
         fcmpwinsts[] = {FCMPWEQ, FCMPWNE, FCMPWLT, FCMPWLE, FCMPWGT, FCMPWGE},
         dcmpwinsts[] = {FCMPDWEQ, FCMPDWNE, FCMPDWLT,FCMPDWLE, FCMPDWGT, 
                         FCMPDWGE};
      #else
/*
 *    SSE supports : EQ, NE, LT, LE, NLT, NLE
 *    not supports : GT, GE, NGT, NGE
 *    So, we need to replace GT and GE with NLE and NLT
 */
         fcmpwinsts[] = {FCMPWEQ, FCMPWNE, FCMPWLT, FCMPWLE, FCMPWNLE, 
                         FCMPWNLT},
         dcmpwinsts[] = {FCMPDWEQ, FCMPDWNE, FCMPDWLT, FCMPDWLE, FCMPDWNLE, 
                         FCMPDWNLT};
      #endif
#endif

   enum inst *cmpinsts;      
   int nbr;

   nbr = 6;
   
   for (ip=sblk->ainst1; ip; ip=ip->next)
   {
      if (IS_COND_BRANCH(ip->inst[0]))
      {
/*
 *       Right now, we don't expect cmp without float/double
 *       format: 
 *                CMP -FCC0, -freg0, -freg1
 *                BR -PCREG, -FCC0, LABEL
 */
         assert((ip->inst[2] == -FCC0));
         ip0 = ip->prev;
/*
 *       Assuming that this conditional branch is not optimized (unlike 
 *       loopcontrol). So, there must be a CMP inst. We need that to modify 
 *       this to CMPW, writing result on mask. 
 */
         assert(IS_CMP(ip0->inst[0]) && (ip0->inst[1]==-FCC0));
         if (ip0->inst[0] == FCMP)
         {
            type = T_FLOAT;
            cmpinsts = fcmpwinsts;
            fst = FST;
         }
         else if (ip0->inst[0] == FCMPD)
         {
            type = T_DOUBLE;
            cmpinsts = dcmpwinsts;
            fst = FSTD;
         }
         else assert(0);
/*
 *       creating mask local variable to store the result of CMP
 */
         cmask = (char*)malloc(sizeof(char)*10);
         sprintf(cmask,"mask_%d",++maskid);
         mask = InsertNewLocal(cmask,type);
/*
 *       changing the CMP with CMPW, remove the branch
 *       NOTE: of course, it will messed up the CFG  but we will change the CFG
 *       anyway after this translation.
 */
         for (k=0; k < nbr; k++)
            if (brinsts[k] == ip->inst[0])
               break;
         assert(k!=nbr);
         
         ip0->inst[0] = cmpinsts[k];
/*
 *       NOTE: we keep op1 and op2 same which will work for both AVX and SSE
 */
         /*ip0->inst[1] = SToff[mask-1].sa[2];*/
         ip0->inst[1] = ip0->inst[2]; /* update value in a reg */
         ip0 = InsNewInst(sblk, ip0, NULL, fst, SToff[mask-1].sa[2], 
                          ip0->inst[1], 0);
#if 1 
         DelInst(ip); 
#else
/*
 *       Changing the conditional branch into unconditinal
 *       format: 
 *             JMP PCREG, LABEL, 0
 *             BR  PCREG, cc#, LABEL
 */
         ip->inst[0] = JMP;
         ip->inst[2] = ip->inst[3];
         ip->inst[3] = 0;

#endif
/*
 *       HERE HERE, we consider only one conditinal branch in a block
 *       So, no need to check other instructions after getting that.
 */
         break;
      }
   }
   return mask;
}

short *FindVarsCMOVNeeded(BBLOCK *bp0)
/*
 * returns array of vars in standard format (N,s1,s2...) which needed the
 * CMOV for redundant vector transformation.
 */
{
   int i;
   short iv;
   short *sp;
   INSTQ *ip;
   extern int FKO_BVTMP;
   extern short STderef;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;   /* avoiding init of temp vec rather reuse */
   SetVecAll(iv, 0);
/*
 * Recalculate ins and outs, dead vars if not updated yet.
 */
   if (!CFUSETU2D )
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
/*
 * figure out all vars and regs which is set in this blk
 * Take those which is liveout at the exiting of this blk
 * NOTE: only fully tested but works for the default testcase.
 */
   for (ip = bp0->inst1; ip; ip=ip->next)
   {
      iv = BitVecComb(iv, iv, ip->set, '|');
   }
   iv = BitVecComb(iv, iv, bp0->outs, '&');
/*
 * clear all the regs in bvec to find out vars only
 */
   for (i=0; i < TNREG; i++)
   {
      SetVecBit(iv, i, 0);
   }
   SetVecBit(iv, STderef+TNREG-1,0);
/*
 * create array of vars (st-index) which needed CMOV
 */
   sp = BitVec2Array(iv, 1-TNREG);
   return sp;
}

short *FindVarsSetInBlock(BBLOCK *bp0)
{
   int i;
   short iv;
   short *sp;
   INSTQ *ip;
   extern int FKO_BVTMP;
   extern short STderef;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;   /* avoiding init of temp vec rather reuse */
   SetVecAll(iv, 0);
/*
 * Recalculate ins and outs, dead vars if not updated yet.
 */
   if (!CFUSETU2D )
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
/*
 * figure out all vars and regs which is set in this blk
 */
   for (ip = bp0->inst1; ip; ip=ip->next)
   {
      iv = BitVecComb(iv, iv, ip->set, '|');
   }
/*
 * clear all the regs in bvec to find out vars only
 */
   for (i=0; i < TNREG; i++)
   {
      SetVecBit(iv, i, 0);
   }
   SetVecBit(iv, STderef+TNREG-1,0);
/*
 * create array of vars (st-index) which needed CMOV
 */
   sp = BitVec2Array(iv, 1-TNREG);
   return sp;
}

void MovInstFromBlkToBlk(BBLOCK *fromblk, BBLOCK *toblk)
/*
 * delete all inst and move inst except JMP and LABEL from that blk to a another
 * blk (at the end but before JMP). 
 * HERE HERE, don't consider conditional branch yet
 */
{
   BBLOCK *bp;
   INSTQ *ip, *ip0;

   bp = toblk;
   ip0 = toblk->instN;
   if (ip0->inst[0] == JMP) ip0 = ip0->prev; /* JMP, keep that at the end */
   
   ip = fromblk->inst1; /* inst to be moved */

   while (ip)
   {
      if (ip->inst[0] != JMP && ip->inst[0] != LABEL) 
      {
         ip0 = InsNewInst(bp, ip0, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                          ip->inst[3]);
         ip = RemoveInstFromQ(ip);
      }
      else
         /*ip = ip->next;*/
         ip = RemoveInstFromQ(ip); /* delete all */
   }
}

void RedundantScalarComputation(LOOPQ *lp)
/*
 * Assuming CFG is already constructed.
 */
{
   int i, j, k, n, N;
   int type;
   int *vpos;
   short iv, iv1;
   short *sp;
   
   short *isetvars, *esetvars, *icmvars, *ecmvars; /* i=if, e=else */
   short *inewvars, *enewvars;

   char *nvar;
   short nv, sv, nv1, nv2, mask;
   short freg0, freg1, freg2;
   enum inst cmov1, cmov2, fld, fst; 
   BBLOCK *bp, *bp0;
   INSTQ *ip, *ip0;
   BLIST *ifblks, *elseblks, *splitblks, *mergeblks;
   BLIST *bl;
   extern short STderef;
   extern int FKO_BVTMP;

   splitblks = NULL;
   ifblks = NULL;
   elseblks = NULL;
   mergeblks = NULL;
/*
 * identify the ifblks, elseblks and common blks
 * NOTE: loopcontrol is killed already. So, there should not be any loop branch
 * Only branch should be the branches responsiblefor  the control flow inside 
 * loop.
 * 
 * Right now, I just consider the single if-else block
 */

/*
 * Finding all split blks inside the optloop
 * NOTE: call KillLoopControl before it, otherwise catch that one also 
 */
   for (bl = lp->blocks; bl; bl = bl->next  )
   {
      bp = bl->blk;
      for (ip = bp->ainst1; ip; ip=ip->next) 
      {
         if (IS_COND_BRANCH(ip->inst[0]))
         {
            splitblks = AddBlockToList(splitblks, bp);
         }
      }
   }
/*
 * Right now, we consider single if-else block, will generalize later
 */
   assert(!splitblks->next);
/*
 * find out if and corresponding else block
 * NOTE: right now, ifblks / elseblks are the single blk. need to traverse
 * the bbbase otherwise.
 */
   ifblks = AddBlockToList(ifblks, splitblks->blk->csucc); 

   if (ifblks->blk->usucc != splitblks->blk->usucc)
      elseblks = AddBlockToList(elseblks, splitblks->blk->usucc);
   
   if (elseblks)
      bp = elseblks->blk->usucc;
   else
      bp = splitblks->blk->usucc;
   assert(bp);
   
   if (bp == ifblks->blk->usucc)
      mergeblks = AddBlockToList(mergeblks, bp);
   else assert(0);

#if 0
   fprintf(stderr, "Split Blocks: \n");
   for (bl = splitblks; bl; bl = bl->next)
      fprintf(stderr, "%d ", bl->blk->bnum);
   fprintf(stderr, "\n");
   fprintf(stderr, " if-block = %d\n", ifblks->blk->bnum);
   fprintf(stderr, " else-block = %d\n", elseblks->blk->bnum);
   fprintf(stderr, " commonblks = %d\n", mergeblks->blk->bnum);
#endif

/*
 * NOTE: We need to rename all the variables which are set inside the blks
 * even if it is not a candidate for select operation: 
 * Example: 
 * 
 * [using if-else]
 *
 * if (x < 0.0)
 * { 
 *    x = -x;
 *    sum +=x;
 * }
 * else
 *    sum+=x;
 *
 * Transformation:
 *
 * x = X[0];
 * mask1 = x < 0.0 
 *    x1 = -x;          # if x is not renamed, it doesn't work
 *    sum1 = sum + x1;
 *    sum2 = sum + x
 *    sum = mask1 ? sum1: sum2
 *
 * RULE: rename if it is set in both blks or, it is live out
 */

/*
 * finding all the variables which are set inside blks
 */
   if (ifblks) isetvars = FindVarsSetInBlock(ifblks->blk);
   if (elseblks) esetvars = FindVarsSetInBlock(elseblks->blk);
/*
 * identify all vars which need select operation
 * 1) which is set inside if/else blks
 * 2) which is liveout at the exit of the block
 */
   if (ifblks) icmvars = FindVarsCMOVNeeded(ifblks->blk);
   if (elseblks) ecmvars = FindVarsCMOVNeeded(elseblks->blk);
/*
 * Update with new vars, return new vars' st index in corresponding location
 * that means, if x var is in pos 2 in setvars, new var st will be found on the
 * same index of nvars
 */
   if(ifblks) inewvars = UpdateBlkWithNewVar(ifblks->blk, 1, isetvars);
   if (elseblks) enewvars = UpdateBlkWithNewVar(elseblks->blk, 2, esetvars);

#if 0
   sp = isetvars;
   fprintf(stderr, "vars set [ifblk]: ");
   for (N=sp[0], i=1; i <=N; i++)
      fprintf(stderr, "%s[%d] ", STname[sp[i]-1], sp[i]-1);
   fprintf(stderr, "\n");
#endif

/*
 * Update the conditional branch of split blk with VCMPW updating a mask. 
 * Conditional branch is deleted.
 */
   mask = RemoveBranchWithMask(splitblks->blk);  
/*
 * copy the if/else block at end of split block but before JMP, if any
 */
   if (ifblks) MovInstFromBlkToBlk(ifblks->blk, splitblks->blk);
   if (elseblks) MovInstFromBlkToBlk(elseblks->blk, splitblks->blk);

/*
 * find out the union of reguired vars form if/else blks
 * NOTE: avoid creating new bvec rather use tmp
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;   /* avoiding init of temp vec rather reuse */
   SetVecAll(iv, 0); 
/*
 * NOTE: BitVec2Array returns formated array[N,s1,s2...] but Array2BitVec
 * takes unformatted array without the count.
 */
   iv = BitVecCopy(iv, Array2BitVec(icmvars[0], icmvars+1, TNREG-1));
   if (elseblks)
      iv = BitVecComb(iv, iv, Array2BitVec(ecmvars[0], ecmvars+1, TNREG-1),'|');
   sp = BitVec2Array(iv, 1-TNREG);

#if 0
   fprintf(stderr, "vars set [union]: ");
   for (N=sp[0], i=1; i <=N; i++)
      fprintf(stderr, "%s[%d] ", STname[sp[i]-1], sp[i]-1);
   fprintf(stderr, "\n");
#endif
/*
 * Figure out which var is in which blk
 */
   N = sp[0]; 
   vpos = malloc(sizeof(int)*(N+1));
   vpos[0] = N;
   
   for (i=1; i <= N; i++)
   {
      if (FindInShortList(icmvars[0], icmvars+1, sp[i]))
      {
         if (elseblks)
         {
            if (FindInShortList(ecmvars[0], ecmvars+1, sp[i]))
               vpos[i] = 3;
            else
               vpos[i] = 1;
         }
         else
            vpos[i] = 1;
      }
      else
         vpos[i] = 2;
   }

#if 0
   fprintf(stderr, "vars set [union]: ");
   for (N=sp[0], i=1; i <=N; i++)
      fprintf(stderr, "%s[%d] = %d ", STname[sp[i]-1], sp[i]-1, vpos[i]);
   fprintf(stderr, "\n");
#endif

/*
 * Add select operations. two cases:
 * CASE-1: var is set in both if and else blks
 * CASE-2: var is set either of the blks but not in both
 *
 * for case-1, add select instruction at the first of the common blocks.
 * for case-2, add select inst at the end of the working blk.
 * 
 * NOTE: before inserting the select inst, the conditinal jump should be change
 * update the mask variable.....!!!!!!!!!
 *
 * KEEP IN MIND: 
 * convention of select: 
 * dest = (mask==0) ? src1 : src2   
 * that means, dest = mask? scr1: src2
 *
 * LIL:
 * FCMOV1   :  dest = (mask==0) ? dest : src   # dest is aliased with src1 
 * FCMOV2   :  dest = (mask==0) ? src : dest   # dest is aliased with src2
 *
 */

/*
 * NOTE: We should merge everything into single blk so that other optimization
 * can be performed, like: AE 
 */
            
   bp = splitblks->blk;
   ip = bp->ainstN;
   if (ip->inst[0] == JMP) ip=ip->prev;

   for (N=sp[0], i=1; i <= N; i++)
   {
      if (IS_FLOAT(STflag[sp[i]-1]))
      {
         cmov1 = FCMOV1;
         cmov2 = FCMOV2;
         fld = FLD;
         fst = FST;
         type = T_FLOAT;
      }
      else
      {
         cmov1 = FCMOVD1;
         cmov2 = FCMOVD2;
         fld = FLDD;
         fst = FSTD;
         type = T_DOUBLE;
      }
      switch(vpos[i])
      {
         case 1:  /* if blks*/ 
/*
 *          findout corrsponding new vars
 */
            k = FindInShortList(isetvars[0], isetvars+1, sp[i]);
            nv1 = inewvars[k];
/*
 *          Now time to insert select inst
 *          new var in if: nv1 
 */
            freg0 = GetReg(type);
            freg1 = GetReg(type);
            freg2 = GetReg(type);
            ip = InsNewInst(bp, ip, NULL, fld, -freg0, SToff[sp[i]-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg1, SToff[nv1-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg2, SToff[mask-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, cmov1, -freg0, -freg1, -freg2 );
            ip = InsNewInst(bp, ip, NULL, fst, SToff[sp[i]-1].sa[2], -freg0, 0);
            GetReg(-1);
            break;
         case 2:  /* else blks.... only else possible??? */
            /*bp = elseblks->blk;*/
/*
 *          findout corrsponding new vars
 */
            k = FindInShortList(esetvars[0], esetvars+1, sp[i]);
            nv2 = inewvars[k];
/*
 *          Now time to insert select inst
 *          New var in else: nv2
 */
            freg0 = GetReg(type);
            freg1 = GetReg(type);
            freg2 = GetReg(type);
            ip = InsNewInst(bp, ip, NULL, fld, -freg0, SToff[sp[i]-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg1, SToff[nv2-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg2, SToff[mask-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, cmov1, -freg0, -freg1, -freg2 );
            ip = InsNewInst(bp, ip, NULL, fst, SToff[sp[i]-1].sa[2], -freg0, 0);
            GetReg(-1);
            break;
         case 3:  /* both in if and else blk */
            
            /*bp = mergeblks->blk;
            ip = bp->ainst1;
            if (ip->inst[0] == LABEL) ip=ip->next;
            */
/*
 *          findout corrsponding new vars
 */
            k = FindInShortList(isetvars[0], isetvars+1, sp[i]);
            nv1 = inewvars[k];
            k = FindInShortList(esetvars[0], esetvars+1, sp[i]);
            nv2 = enewvars[k];
/*
 *          Now time to insert select inst
 *          new var in if: nv1, New var in else: nv2
 */
            freg0 = GetReg(type);
            freg1 = GetReg(type);
            freg2 = GetReg(type);
            /*ip = InsNewInst(bp, NULL, ip, fld, -freg0, SToff[nv1-1].sa[2],0);*/
            ip = InsNewInst(bp, ip, NULL, fld, -freg0, SToff[nv1-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg1, SToff[nv2-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg2, SToff[mask-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, cmov2, -freg0, -freg1, -freg2 );
            ip = InsNewInst(bp, ip, NULL, fst, SToff[sp[i]-1].sa[2], -freg0, 0);
            GetReg(-1);
            break;
         default: ;
     }
   }
}

int CheckMaxMinConstraints(BLIST *scope, short var, short label)
/*
 * 
 */
{
   int i, j;
   int isetcount, osetcount;
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip;
   enum inst st;

   i = FLAG2TYPE(STflag[var-1]);
   if (i == T_FLOAT)
      st = FST;
   else if (i == T_DOUBLE)
      st = FSTD;
   else if (i == T_INT)
      st = ST;
   else
      fko_error(__LINE__,"Unknown type=%d, file=%s", i, __FILE__);

#if 0   
   switch(i)
   {
   case T_FLOAT:
      st = FST;
      break;
   case T_DOUBLE:
      st = FSTD;
      break;
   case T_INT:
      st = ST;
      break;
   default:;
   }
#endif   
   
/*
 * check in the if blk, found by label. var should be set only once
 */
   isetcount = 0;
   osetcount = 0;

   for (bl = scope; bl; bl=bl->next)
   {
      bp = bl->blk;
      ip = bp->ainst1;
/*
 *    NOTE: assuming this if blk begins with ainst LABEL
 */
      if (ip->inst[0] == LABEL && ip->inst[1] == label)
      {
         for ( ; ip; ip=ip->next)
         {
            /*fprintf(stderr," %s %d \n", instmnem[ip->inst[0]], ip->inst[1]);*/
/*
 *          Figure out when to use STpts2 and SToff[].sa[2] ??? 
 */
            if (ip->inst[0] == st && STpts2[ip->inst[1]-1] == var)
               isetcount++;
         }
      }
      else
      {
         for ( ; ip; ip=ip->next)
         {
            if (ip->inst[0] == st && STpts2[ip->inst[1]-1] == var)
               osetcount++;
         }
      }
   }
/*
 * condition to be max is: update only once inside if-blk, not in other blk
 */
   if (isetcount == 1 && !osetcount)
      return (1);

   return (0);
}

int VarIsMax(BLIST *scope, short var)
/*
 * Figure out whether a var is Max varibale
 * shifted in appropriate location later.
 */
{
   int i, j;
   short reg0, reg1, regv;
   enum inst inst, ld, st, br, cmp;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl;

   i = FLAG2TYPE(STflag[var-1]);
   switch(i)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      cmp = FCMP;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      cmp = FCMPD;
      break;
   case T_INT:
      ld = LD;
      st = ST;
      cmp = CMP;
      break;
   default:
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__,"Unknown type=%d, file=%s. Should be done before Vect",
                i, __FILE__);
   }

   for (bl = scope; bl; bl = bl->next)
   {
      bp = bl->blk;
      for (ip = bp-> ainst1; ip; ip = ip->next)
      {
/*
 *       FORMAT: cmp fcc, reg0, reg1
 *               JLT/JGT PC, fcc, LABEL
 *       There would be exactly two ld of 2 vars/const
 */
         if (IS_COND_BRANCH(ip->inst[0]) && (ip->prev->inst[0]== cmp))
         {
            reg0 = ip->prev->inst[2];
            reg1 = ip->prev->inst[3];
            br = ip->inst[0];

            ip0 = ip->prev->prev->prev;     /* 1st ld */
            assert((ip0->inst[0] == ld));
            ip1 = ip->prev->prev;           /* 2nd ld */
            assert((ip1->inst[0] == ld));
/*
 *          for load, inst[2] must be a var 
 */
            if (STpts2[ip0->inst[2]-1] == var)
               regv = ip0->inst[1];
            else if (STpts2[ip1->inst[2]-1] == var)
               regv = ip1->inst[1];
            else 
               regv = 0;
            if ( ((regv == reg0) && (br == JLT)) ||  
                 ((regv == reg1) && (br == JGT)) )
            {
               /* check for single set inside if-blk and not set where else*/
               if (CheckMaxMinConstraints(scope, var, ip->inst[3]))
                  return (1);
            }
         }
      }
   }
   return (0);
}

int VarIsMin(BLIST *scope, short var)
/*
 * Figure out whether a var is Min varibale
 * shifted in appropriate location later.
 * NOTE: this function is same as the VarIsMax, it can be merged with that
 */
{
   int i, j;
   short reg0, reg1, regv;
   enum inst inst, ld, st, br, cmp;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl;

   i = FLAG2TYPE(STflag[var-1]);
   switch(i)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      cmp = FCMP;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      cmp = FCMPD;
      break;
   case T_INT:
      ld = LD;
      st = ST;
      cmp = CMP;
      break;
   default:
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__,"Unknown type=%d, file=%s. Should be done before Vect",
                i, __FILE__);
   }

   for (bl = scope; bl; bl = bl->next)
   {
      bp = bl->blk;
      for (ip = bp-> ainst1; ip; ip = ip->next)
      {
/*
 *       FORMAT: cmp fcc, reg0, reg1
 *               JLT/JGT PC, fcc, LABEL
 *       There would be exactly two ld of 2 vars/const
 */
         if (IS_COND_BRANCH(ip->inst[0]) && (ip->prev->inst[0]== cmp))
         {
            reg0 = ip->prev->inst[2];
            reg1 = ip->prev->inst[3];
            br = ip->inst[0];

            ip0 = ip->prev->prev->prev;     /* 1st ld */
            assert((ip0->inst[0] == ld));
            ip1 = ip->prev->prev;           /* 2nd ld */
            assert((ip1->inst[0] == ld));
/*
 *          for load, inst[2] must be a var
 *          STpts2[] vs. SToff[].sa[2] 
 */
            if (STpts2[ip0->inst[2]-1] == var)
               regv = ip0->inst[1];
            else if (STpts2[ip1->inst[2]-1] == var)
               regv = ip1->inst[1];
            else 
               regv = 0;
/*
 *          NOTE: cbr is opposit from Max
 */
            if ( ((regv == reg0) && (br == JGT)) ||  
                 ((regv == reg1) && (br == JLT)) )
            {
               /* check for single set inside if-blk and not set where else*/
               if (CheckMaxMinConstraints(scope, var, ip->inst[3]))
                  return (1);
            }
         }
      }
   }
   return (0);
}

short *FindAllScalarVars(BLIST *scope)
{
   struct ptrinfo *pbase, *p;
   short *sp, *s, *scal;
   int i, j, k, n, N;
   int iv;
   BLIST *bl;
   extern int FKO_BVTMP;
   extern short STderef;
/*
 * Find variable accessed in the path and store it in path
 */
   /*iv = NewBitVec(32);*/
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);

   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
   for (bl=scope; bl; bl=bl->next)
   {
      iv = BitVecComb(iv, iv, bl->blk->uses, '|');
      iv = BitVecComb(iv, iv, bl->blk->defs, '|');
   }
   
/*
 * right now, skip all the regs from uses, defs
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);

/*
 * Skip all non-fp variables, valid entires upto n (included) 
 * NOTE: No action for INT var in vector but need to consider complex case 
 * with index var update later!!!
 * NOTE: As our vector path never uses/sets integer variable (except index)
 * we don't have to worry about this in Backup/Recovery stage. So, we just 
 * skip int here.
 */
   sp = BitVec2Array(iv, 1-TNREG);
   for (N=sp[0],n=0,i=1; i <= N; i++)
   {
      if (IS_FP(STflag[sp[i]-1]))
         sp[n++] = sp[i];
   }   

/*
 * Moving pointer analysis for path
 */
   pbase = FindMovingPointers(scope);
   for (N=0, p=pbase; p; p = p->next)
      if (IS_FP(STflag[p->ptr-1]))
         N++;
/*
 * Copy all moving pointers to varrs
 */
   s = malloc(sizeof(short)*(N+1));
   assert(s);
   s[0] = N;
   for (j=0,i=1,p=pbase; p; p = p->next)
      if (IS_FP(STflag[p->ptr-1]))
         s[i++] = p->ptr;

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
   assert(scal );
   scal[0] =  n;
   for (i=1; i <= n; i++)
      scal[i] = sp[i-1];
   
   if (s) free(s);
   if (sp) free(sp);

   return scal;

}

void UpdateOptLoopWithMaxMinVars()
{
   int i, j, N, n, m;
   short *scal, *spmax, *spmin;
   LOOPQ *lp;
   
#if 0   
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__); 
   lp = optloop; 
#else
   GenPrologueEpilogueStubs(bbbase, 0);
   NewBasicBlocks(bbbase);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
   lp = optloop;
#endif
   
   scal = FindAllScalarVars(lp->blocks);   
   N = scal[0];
   spmax = malloc(sizeof(short)*(N+1));
   assert(spmax);
   spmin = malloc(sizeof(short)*(N+1));
   assert(spmin);

   for (i=1, m=0, n=0; i <= N; i++)
   {
      /*fprintf(stderr, " var = %s(%d)\n",STname[scal[i]-1], scal[i] );*/
      if (VarIsMax(lp->blocks, scal[i]))
      {
         spmax[++m] = scal[i];
         /*fprintf(stderr, " Max var = %s\n",STname[scal[i]-1] );*/
      }
      else if (VarIsMin(lp->blocks, scal[i]))
      {
         spmin[++n] = scal[i];
         /*fprintf(stderr, " Min var = %s\n",STname[scal[i]-1] );*/
      }
   }
/*
 * Update with max vars
 */
   if (m)
   {
      if (lp->maxvars) free(lp->maxvars);
      lp->maxvars = malloc(sizeof(short)*(m+1));
      assert(lp->maxvars);
      lp->maxvars[0] = m;
      for (i=1; i <=m ; i++)
         lp->maxvars[i] = spmax[i];
   }
   else
   {
      if (lp->maxvars) free(lp->maxvars);
      lp->maxvars = NULL;
   }
/*
 * update with min vars
 */
   if (n)
   {
      if (lp->minvars) free(lp->minvars);
      lp->minvars = malloc(sizeof(short)*(n+1));
      assert(lp->minvars);
      lp->minvars[0] = n;
      for (i=1; i <=n ; i++)
         lp->minvars[i] = spmin[i];
   }
   else
   {
      if (lp->minvars) free(lp->minvars);
      lp->minvars = NULL;
   }
/*
 * free all temporaries
 */
   if (scal) free(scal);
   if (spmax) free(spmax);
   if (spmin) free(spmin);
}

void UpdateOptLoopWithMaxMinVars1()
/*
 * This function is used in state1 to keep track of all the max/min vars
 * there is an other version of it which is kept for backward compability and
 * will be deleted later.
 */
{
   int i, j, N, n, m;
   short *scal, *spmax, *spmin;
   LOOPQ *lp;
   
   lp = optloop;
   
   scal = FindAllScalarVars(lp->blocks);   
   N = scal[0];
   spmax = malloc(sizeof(short)*(N+1));
   assert(spmax);
   spmin = malloc(sizeof(short)*(N+1));
   assert(spmin);

   for (i=1, m=0, n=0; i <= N; i++)
   {
      /*fprintf(stderr, " var = %s(%d)\n",STname[scal[i]-1], scal[i] );*/
      if (VarIsMax(lp->blocks, scal[i]))
      {
         spmax[++m] = scal[i];
         /*fprintf(stderr, " Max var = %s\n",STname[scal[i]-1] );*/
      }
      else if (VarIsMin(lp->blocks, scal[i]))
      {
         spmin[++n] = scal[i];
         /*fprintf(stderr, " Min var = %s\n",STname[scal[i]-1] );*/
      }
   }
/*
 * Update with max vars
 */
   if (m)
   {
      if (lp->maxvars) free(lp->maxvars);
      lp->maxvars = malloc(sizeof(short)*(m+1));
      assert(lp->maxvars);
      lp->maxvars[0] = m;
      for (i=1; i <=m ; i++)
         lp->maxvars[i] = spmax[i];
   }
   else
   {
      if (lp->maxvars) free(lp->maxvars);
      lp->maxvars = NULL;
   }
/*
 * update with min vars
 */
   if (n)
   {
      if (lp->minvars) free(lp->minvars);
      lp->minvars = malloc(sizeof(short)*(n+1));
      assert(lp->minvars);
      lp->minvars[0] = n;
      for (i=1; i <=n ; i++)
         lp->minvars[i] = spmin[i];
   }
   else
   {
      if (lp->minvars) free(lp->minvars);
      lp->minvars = NULL;
   }
/*
 * free all temporaries
 */
   if (scal) free(scal);
   if (spmax) free(spmax);
   if (spmin) free(spmin);
}

void VectorRedundantComputation()
/*
 * This function implements another approaches for Vectorization. 
 * Redundant computation: 
 * compute both blocks and use a select operation to select the correct one.
 */
{
   int i, j, N, n;
   short *sc, *sf;
   short *scal;
   short *sp;
   LOOPQ *lp;

   INSTQ *ippu;
   struct ptrinfo *pi0;
   BLIST *bl;
   extern BBLOCK *bbbase;

   lp = optloop;

/*
 * Redundant Scalar Computation Transformation using CMOV 
 */
   KillLoopControl(lp);
   RedundantScalarComputation(lp);

#if 1
   pi0 = FindMovingPointers(lp->tails);
   ippu = KillPointerUpdates(pi0,1);
   /*OptimizeLoopControl(lp, 1, 0, NULL);*/
   OptimizeLoopControl(lp, 1, 0, ippu);
#endif

#if 1
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
#endif

   lp = optloop;
#if 0
   lp = optloop;
   fprintf(stderr, "\nLOOP BLOCKS: ");
   if(!lp->blocks) fprintf(stderr, "NO LOOP BLK!!!\n");
   for (bl = lp->blocks; bl ; bl = bl->next)
   {
      assert(bl->blk);
      fprintf(stderr, "%d ",bl->blk->bnum);
   }
   fprintf(stderr,"\n");
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

#if 0 
   fprintf(stdout, "LIL Before Vectorization: SRC \n");
   PrintInst(stdout,bbbase);
   //ShowFlow("cfg-sc.dot",bbbase); /* checked for scalar, it's ok */
   //exit(0);
#endif

/*
 * Apply normal vectorization
 * NOTE: VectorizeStage1 and VectorizeSatge3 don't work; as stage1 
 * restore FKOstate0 code. So, redundant code transformation is lost. 
 * need to update the analysis.
 * Saving current state as State0 would not work. SaveFKOState(0) doesn't 
 * save bit vector. So, it would create a segfault with RestoreFKOstate
 *
 */
#if 0
   /*SaveFKOState(0);*/
   assert(!VectorizeStage1());
   assert(!VectorizeStage3(0,0));
#endif
  
#if 1 
   assert(!SpeculativeVectorAnalysis());
#endif


/*
 * restate mvptr and loop control 
 */
#if 0
 pi0 = FindMovingPointers(lp->tails);
 ippu = KillPointerUpdates(pi0,1);
 /*OptimizeLoopControl(lp, 1, 0, NULL);*/
 OptimizeLoopControl(lp, 1, 0, ippu);
 assert(!VectorizeStage3(0,0));
#endif

 assert(!RedundantVectorTransform(lp));
#if 0 
   fprintf(stdout, "LIL-Vector \n");
   PrintInst(stdout,bbbase);
   //ShowFlow("cfg-vrc.dot",bbbase); /* checked for scalar, it's ok */
   //exit(0);
#endif
#if 1
   KillPathTable();
#endif
 UpdateNamedLoopInfo();

#if 0
   UnrollCleanup(optloop,1);
   fprintf(stdout, "LIL After Vectorization \n");
   PrintInst(stdout,bbbase);
/*
 * Everything is messed up already. So, re-make the control flow and check it 
 */
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
#endif
}


int CheckMaxReduceConstraints(BLIST *scope, short maxvar, short xvar, 
                              short label)
/*
 * this function checks the ifblk whether, 
 */
{
   int i, j; 
   enum inst st;
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip, *ip0;

   i = FLAG2TYPE(STflag[maxvar-1]);
   switch(i)
   {
   case T_FLOAT:
      st = FST;
      break;
   case T_DOUBLE:
      st = FSTD;
      break;
   case T_INT:
      st = ST;
      break;
   default:
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__,"Unknown type=%d, file=%s. Should be done before Vect",
                i, __FILE__);
   }

 /*fprintf(stderr, "check for maxvar=%s, lable=%d\n", STname[maxvar-1],label);*/

   for (bl=scope; bl; bl=bl->next)
   {
      bp = bl->blk;
/*
 *    Assumption: if blk starts with active inst LABEL
 */
      ip0 = bp->ainst1;
      if (ip0->inst[0] == LABEL && ip0->inst[1] == label)
      {
        for (ip = ip0; ip; ip=ip->next)
        {
           if (ip->inst[0] == st && STpts2[ip->inst[1]-1]!= maxvar)
              return 0;
        }
      }
   }
   return 1;

}

void RemoveInstFromLabel2Br(BLIST *scope, short label)
{
   int i, j;
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip;

   for (bl=scope; bl; bl=bl->next)
   {
      bp = bl->blk;
      for (ip=bp->inst1; ip; ip=ip->next)
      {
         if (ip->inst[0] == LABEL && ip->inst[1] == label)
         {
            while (ip && ip->inst[0]!=JMP)
            {  
               assert(!IS_COND_BRANCH(ip->inst[0]));
               ip = RemoveInstFromQ(ip);
            }
            assert(ip->inst[0] == JMP);
            RemoveInstFromQ(ip);
         }
      }
   }
   
}

int CheckElimIFBlk(short maxvar)
/*
 * Assuming single occurrance first 
 */
{
   int i, j;
   short reg0, reg1, regv, regx;
   short xvar, label;
   enum inst inst, ld, st, br, cmp, max;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl, *scope;

   i = FLAG2TYPE(STflag[maxvar-1]);
   switch(i)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      cmp = FCMP;
      max = FMAX;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      cmp = FCMPD;
      max = FMAXD;
      break;
   default:
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__,"Unknown type=%d, file=%s. Should be done before Vect",
                i, __FILE__);
   }
/*
 * Primarily, we are concern about the optloop blks, but need to extend to
 * all blocks in bbbase later
 */   
   scope = optloop->blocks;

   for (bl = scope; bl; bl = bl->next)
   {
      bp = bl->blk;
      for (ip = bp-> ainst1; ip; ip = ip->next)
      {
/*
 *       FORMAT: cmp fcc, reg0, reg1
 *               JLT/JGT PC, fcc, LABEL
 *       There would be exactly two ld of 2 vars/const
 */
         if (IS_COND_BRANCH(ip->inst[0]) && (ip->prev->inst[0]== cmp))
         {
            reg0 = ip->prev->inst[2];
            reg1 = ip->prev->inst[3];
            br = ip->inst[0];

            ip0 = ip->prev->prev->prev;     /* 1st ld */
            assert((ip0->inst[0] == ld));
            ip1 = ip->prev->prev;           /* 2nd ld */
            assert((ip1->inst[0] == ld));
/*
 *          for load, inst[2] must be a var 
 */
            if (STpts2[ip0->inst[2]-1] == maxvar)
            {
               regv = ip0->inst[1];
               xvar = ip1->inst[2];
               regx = ip1->inst[1];
            }
            else if (STpts2[ip1->inst[2]-1] == maxvar)
            {
               regv = ip1->inst[1];
               xvar = ip0->inst[2];
               regx = ip0->inst[1];
            }
            else 
               regv = 0;
            if ( ((regv == reg0) && (br == JLT)) ||  
                 ((regv == reg1) && (br == JGT)) )
            {
               label = ip->inst[3];
               /* check for single set inside if-blk and not set where else*/
               if (CheckMaxMinConstraints(scope, maxvar, label) && 
                   CheckMaxReduceConstraints(scope, maxvar, xvar, label))
               {
                  // Now, it's time to eliminiate ifblk inserting max inst
                  /*fprintf(stderr, "elim blks for max var = %s\n", 
                          STname[maxvar-1]);*/
                  assert(ip0->prev->inst[0] != ld);
                  ip1 = InsNewInst(bp, ip1, NULL, max, regv, regv, regx);
                  ip1 = InsNewInst(bp, ip1, NULL, st, SToff[maxvar-1].sa[2],
                                   regv, 0);
                  ip1 = ip1->next;
                  while (ip1 && !IS_COND_BRANCH(ip1->inst[0])) 
                     ip1 = RemoveInstFromQ(ip1);
                  assert(IS_COND_BRANCH(ip1->inst[0]));
                  ip1 = RemoveInstFromQ(ip1); /* delete the branch itself */
                  // it's time to delete the if blk
                  RemoveInstFromLabel2Br(scope, label);
               }
            }
         }
      }
   }
   return (0);
   
}

void VectorizeElimIFwithMaxMin()
{
   int i, j, N;
   short *scal;
   LOOPQ *lp;

   lp = optloop;
   scal = FindAllScalarVars(lp->blocks);   
   for (N = scal[0], i=1; i <= N; i++)
   {
      if (VarIsMax(lp->blocks, scal[i]))
      {
         /*fprintf(stderr, "Max var = %s\n", STname[scal[i]-1]);*/
         CheckElimIFBlk(scal[i]);
      }
   }
/*
 * re-construct the CFG 
 */
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
/*
 * analyze and vectorize 
 */
   //assert(!IsSpeculationNeeded());

   lp = optloop;
#if 1 
   assert(!SpeculativeVectorAnalysis());
   assert(NPATH==1); /* still multiple path!! */
#endif

   assert(!RedundantVectorTransform(lp));
#if 0 
   fprintf(stdout, "LIL-Vector \n");
   PrintInst(stdout,bbbase);
   //ShowFlow("cfg-vrc.dot",bbbase); /* checked for scalar, it's ok */
   //exit(0);
#endif
#if 1   
   KillPathTable();
#endif

   UpdateNamedLoopInfo();

}

/*=============================================================================
 * Functions for new program flow. 
 * Many of the functions of this file will be obsolete after the completion 
 * of this section.
 *============================================================================*/
int VectorAnalysis()
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
   assert(!PathFlowVectorAnalysis(PATHS[0]));

   if (PATHS[0]->lpflag & LP_VEC)
         VPATH = 0;
   
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

int Vectorization()
/*
 * So far RedundantVectorTransform is the most general vector Transform. 
 * will re-write later.
 */
{
   LOOPQ *lp;
   lp = optloop;
   assert(!RedundantVectorTransform(lp));
   KillPathTable();
   return (0);
}
