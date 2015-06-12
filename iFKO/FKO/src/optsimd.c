/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#include "fko.h"

/* Majedul: to save the paths for speculative Vectorization. */
static LOOPPATH **PATHS = NULL;
static int NPATH = 0, TNPATH = 0, VPATH = -1;
/* 
 * Majedul: temporary flag for SSV which is needed in loop peeling to predict
 * max LABEL_id. I will manage that using special flag which indicates the type
 * of vectorization.
 * NOTE: NO NEED NOW, SHOULD BE DELETED!
 */
/*int isSSV = 0;*/

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
         assert(bl->blk);
#if 0         
         if (bl->blk->uses && bl->blk->defs)
         {
            BitVecComb(new->uses, new->uses, bl->blk->uses, '|');
            BitVecComb(new->defs, new->defs, bl->blk->defs, '|');
         }
#else
         if (bl->blk->uses)
            BitVecComb(new->uses, new->uses, bl->blk->uses, '|');
         if (bl->blk->defs)
            BitVecComb(new->defs, new->defs, bl->blk->defs, '|');
#endif
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
/*
 * Kill all others but not those which point bblock
 * All of them are copied in optloop. 
 */
   if (path->varrs)
      free(path->varrs);
   if (path->vscal)
      free(path->vscal);
   if (path->vsflag)
      free(path->vsflag);
   if (path->vsoflag)
      free(path->vsoflag);
   
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

ILIST *FindPrevStore(INSTQ *ipstart, short var, INT_BVI blkvec, INT_BVI ivseen)
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

ILIST *FindNextLoad(INSTQ *ipstart, short var, INT_BVI blkvec, INT_BVI ivseen)
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

short FindReadUseType(INSTQ *ip, short var, INT_BVI blkvec)
/*
 * Find first use of var, including 1st use of any var it is copied to,
 * if a move is the first op
 * Majedul: updated to re-init the iv if ip is NULL which is not the normal 
 * case. Still, it will not harm anything as we return 0 which is valid result
 */
{
   short j=0;
   ILIST *ib, *il;
   static INT_BVI iv=0;

   if (ip)
   {
      if (!iv)
         iv = NewBitVec(32);
      else
         SetVecAll(iv, 0);
/*
 *    finds first use of loads of the var.
 *    for examble: 
 *    1. Not vectorizable: (first use of sum is not ACC)
 *       x1 = sum;
 *       sum += x;
 *
 */
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
         /*while (ip->next->inst[0] == FLD || ip->next->inst[0] == FLDD)*/
         while(IS_LOAD(ip->next->inst[0])) /* consider int ld too */
            ip=ip->next;
         assert(ip->next);
#if 0
         fprintf(stderr, "%s : %d %d %d\n", instmnem[ip->next->inst[0]], 
                 ip->next->inst[1], ip->next->inst[2], ip->next->inst[3]);
#endif
      if (ip->next->inst[0] == FADD || ip->next->inst[0] == FADDD ||
          ip->next->inst[0] == FMAC || ip->next->inst[0] == FMACD)
         j |= VS_ACC;
/*
 * FIXED: inst[2] always be a register for store. 
 * So, STpts2[ip->next->inst[2]-1] should be invalid. If inst[1] and var are
 * same, no need to recurse again! 
 * NOTE: logic of this recursion should be checked !
 */
      else if (IS_SELECT_OP(ip->next->inst[0])) /* is cmov instruction ? */
      {
#if 0
         fprintf(stderr, "%s : %d %d %d\n", instmnem[ip->next->inst[0]], 
                 ip->next->inst[1], ip->next->inst[2], ip->next->inst[3]);
#endif
         j |= VS_SELECT;  /* SC_SELECT */
      }
      else if (IS_STORE(ip->next->inst[0])&& STpts2[ip->next->inst[1]-1] == var)
      {
         j |= FindReadUseType(ip->next, var, blkvec);
       /*j |= FindReadUseType(ip->next, STpts2[ip->next->inst[1]-1], blkvec);*/
      }
      else 
         j |= VS_MUL; /* VS_MUL represents anything but accumulator */
      }
      KillIlist(ib);
      return(j);
   }
   else
   {
      if (iv) KillBitVec(iv);
      iv = 0;
      return(0);
   }
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
   INT_BVI iv, iv1;
   int i, j, k, n, N;
   extern INT_BVI FKO_BVTMP;
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
   for (k=0,i=1; i < n; i++) /* FIXED: should be < n... replaced from <=  */
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

INSTQ *AddAlignTest(LOOPQ *lp, BBLOCK *bp, INSTQ *ip, short fptr, int fa_label)
{
   int k;
   int r0;
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
   ip = InsNewInst(bp, ip, NULL, LD, -r0, SToff[fptr-1].sa[2], 0);
   ip = InsNewInst(bp, ip, NULL, CMPAND, -ICC0, -r0, k );
   ip = InsNewInst(bp, ip, NULL, JNE, -PCREG, -ICC0, fa_label);
   
   GetReg(-1);

   return ip;   
}
#if 0
/*
 * NOTE: this function is shifted to symtab.c file
 */
short InsertNewLocal(char *name, int type )
/*
 * add new local var in symbol table. We should always use this function 
 * to insert any var for compiler's internal purpose.
 */
{
   short k, j;
/*
 * NOTE: name string is copied and stored in Symbol Table
 * So, free the string from the caller function
 * FIXED: SToff is a global pointer and it can be updated from AddDerefEntry()
 * so, splited the statement.
 */
   k = STdef(name, type | LOCAL_BIT, 0);
   j =  AddDerefEntry(-REG_SP, k, -k, 0, k);
   SToff[k-1].sa[2] = j;
   return k;
}
#endif
BLIST *AddLoopDupBlks(LOOPQ *lp, BBLOCK *up, BBLOCK *down)
/*
 * Returns pointer of block list of duplicated blocks, adding duplicated 
 * loop blocks between up and down blocks using lbNum label index extension
 * NOTE: lbNum is not used anymore. A static variable is kept inside DupCFScope
 * to manage the label. so, it's deleted
 */
{
   BLIST *dupblks, *ftheads, *bl;
   BBLOCK *newCF, *bp, *bp0;
   INT_BVI iv, ivtails;
   extern INT_BVI FKO_BVTMP; 
   
   bp0 = up;
/*
 * Duplicate original loop body
 */
#if 0
   SetVecBit(lp->blkvec, lp->header->bnum-1, 0);
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
   /*newCF = DupCFScope(lp->blkvec, iv, lbNum, lp->header);*/
   newCF = DupCFScope(lp->blkvec, iv, lp->header); 
   assert(newCF->ilab);
   SetVecBit(lp->blkvec, lp->header->bnum-1, 1);
#else
/*
 * NOTE: first blk scope should include the header, because ivscp0 in 
 * DupScope0 only is used to change the branch target. It was not necessary 
 * before as we always duplicate the loop after killing the loop control. 
 * But we may need to duplicate the loop with the loop control. In that case,
 * it won't change the target of back edge of the loop. 
 * Here, we want to add the header as the ivscp0 but skip that in ivscp
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
   SetVecBit(iv, lp->header->bnum-1, 0);
   newCF = DupCFScope(lp->blkvec, iv, lp->header); 
   assert(newCF->ilab);
#endif
/*
 * Use CF to produce a block list of duped blocks
 */
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
/*
 *       FIXED: NewBasicBlock with create new block with bnum 1
 */
         if (!BitVecCheck(lp->blkvec, bp->bnum-1))
            break;
         bp0->down = FindBlockInListByNumber(dupblks, bp->bnum);
         bp0->down->up = bp0;
         bp0 = bp0->down;
         if (BitVecCheck(ivtails, bp->bnum-1))
            break;
/*
 *       if bp->down is not a succesor of bp, we don't need to explore that
 *       NOTE: it's a safe guard. We ensure that everytime, only one path is
 *       copied. ftheads contains the head of each path.
 */
         if (bp->usucc != bp->down && bp->csucc != bp->down)
            break;
      }
      bp0->down = NULL;
   }
   
   bp0->down = down;
   return dupblks;
}

void AddLoopPeeling(LOOPQ *lp, int jblabel, int falabel, short Np, 
                    short Nv, short fptr)
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
   INSTQ *ip;
   short oldN;
   int k, r0, r1;
   LOOPQ *lpn;
   extern BBLOCK *bbbase;
   extern INT_BVI FKO_BVTMP;
   /*int lnum;*/
   /*ILIST *il, *iref;*/

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
   ip = InsNewInst(bp, ip, NULL, LD, -r0, SToff[fptr-1].sa[2], 0);
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
 * NOTE: NO need now as controlled by static variable
 *
 */
#if 0
   if (NPATH >  0 && isSSV) /* SSV is aplied */
   {
      lnum = (NPATH -1) * Type2Vlen(lp->vflag) + 1; /* keep space for SSV*/
   }
   else /* normal vectorization. 0 is preserve for cleanup*/
   { 
      lnum = FKO_UR + 1;  /* need to reserve label for unroll */
   }
#endif   
   dupblks = AddLoopDupBlks(lp, bp0, bp0->down);
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

short FindMostAccessedPtr(short *ptrs, BLIST *scope)
{
   int i, k, n;
   int maxA;
   short op;
   INSTQ *ip;
   BLIST *bl;
   short ptm;   /* most accessed*/
   short *ptar; /* number of read access inside loop */
   short *ptaw; /* number of write access inside loop */
/*
 * allocate pta to track the number of access. pta[0] = number of ptr
 * position of each element represents each ptr in ptrs
 */
   n = ptrs[0];
   ptar = calloc((n+1),sizeof(short)); /* calloc can be used*/
   ptaw = calloc((n+1),sizeof(short)); /* calloc can be used*/
   assert(ptar && ptaw);
   ptar[0] = ptaw[0] = n;
/*
 * findout the read and write access of the ptr inside the scope
 * NOTE: for each memory access, the ptr may be read 2 times in initial LIL
 * it will result more number of access but it doesn't hurt; because it is
 * true for all ptrs.
 */
   for (bl=scope; bl; bl=bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip=ip->next)
      {
         for (i=1; i<4; i++) 
         {
            op = ip->inst[i];
            if (op > 0)
            {
               k = STpts2[op-1];
               k = FindInShortList(ptrs[0], ptrs+1, k);
               if (k)
               {
                  if (i==1) /* destination*/
                  {
                     ptaw[k]++;
                  }
                  else
                  {
                     ptar[k]++;
                  }
               }
            }
         }
      }
   }
#if 0
   for (n=ptrs[0], i=1; i<=n; i++)
   {
      fprintf(stderr, "%s: ", STname[ptrs[i]-1]);
      fprintf(stderr, "read=%d write=%d\n", ptar[i], ptaw[i]);
   }
   exit(0);
#endif
/*
 * find max read+write access; 
 * NOTE: if they are same, we should consider  max write. not implemented yet
 */
   maxA=0;
   ptm=1;
   for (n=ptrs[0],i=1; i<=n; i++)
   {
      if ((ptar[i]+ptaw[i]) > maxA)
      {
         maxA = ptar[i]+ptaw[i];
         ptm = i;
      }
   }
   ptm = ptrs[ptm];
#if 0  
   fprintf(stderr, "%s is selected\n", STname[ptm-1]);
   exit(0);
#endif
   return ptm;  
}

short FindPtrToForceAlign(LOOPQ *lp)
/*
 * return ST index of ptr to force align
 */
{
   short ptr;
/*
 * if there exist a ptr from FORCE_ALIGN markup
 */
   if (lp->faalign)  /* if exist, return the 1st one */
   {
      lp->fa2vlen = lp->faalign[1];
      /*return (lp->faalign[1]);*/
   }
   else if (!lp->faalign && lp->fbalign)
   {
      lp->fa2vlen = lp->varrs[1];
      /*return (lp->varrs[1]);*/
   }
/*
 * if no ptr is specified in markup, find out best candidate
 * here is strategy to do that:
 * 1. ptr which is mutually aligned with other ptr
 * 2. ptr which has max read/write access
 */   
   else
   {
      ptr = lp->varrs[1];
      if (lp->maaligned)
         ptr = lp->maaligned[1];
      else if (!lp->maaligned && lp->mbalign) /* all malign, select anyone */
         ptr = lp->varrs[1];
      else /* no falign and no malign ptr, find the most accessed one */
         ptr = FindMostAccessedPtr(lp->varrs, lp->blocks);
      lp->fa2vlen = ptr;
   }

   return(lp->fa2vlen);
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
   int jBlabel, fAlabel;
   BBLOCK *bp;
   INSTQ *ip;
   short Np, Nv;
   short fptr;    /* array which needs to be forced aligned */
   int r0;
   extern BBLOCK *bbbase;
/*
 * Need to create 2 new var for peel and vector loop control
 */
   Np = InsertNewLocal("_Np", T_INT ); /* actually save, N-Np or Np value */
   Nv = InsertNewLocal("_Nv", T_INT ); /* 1st, N then changed to N-Np or N*/
/*
 * find the location to add checking of alignment. 
 */
   /*bp = bbbase;*/
/*
 * FIXED: we need to traverse all the blocks to findout the flag  
 */
   for (bp = bbbase; bp; bp=bp->down)
   {
      ip = FindCompilerFlag(bp, CF_LOOP_INIT);
      if (ip) break;
   }
   assert(bp);
#if 1
   assert(ip); 
#else
   if (!ip)
   {
      fprintf(stdout, "LIL before alignment check\n");
      PrintInst(stdout, bbbase);
      //PrintST(stdout);
      ShowFlow("cfg-ag.dot", bbbase);
      fko_error(__LINE__, "Error in file = %s, line = %d\n",__FILE__,__LINE__);
   }
#endif
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
 * find out the array which needs to be forced aligned
 * FIXME: apply new design!!
 */
#if 0  
   if (lp->faalign) 
     fptr = lp->faalign[1];    /* the ptr which is specified in markup */
  else
     fptr = lp->varrs[1];     /* 1st availbale ptr */
#else
  fptr = FindPtrToForceAlign(lp);
#if 0
  fprintf(stderr, "FPTR=%s\n", STname[fptr-1]);
#endif
#endif
/*
 * Add the checking of alignment. it is always related with the vector length
 * the system supports, eg.- for SSE vlen 128bit, for AVX, 256 bit.
 */
   ip = AddAlignTest(lp, bp, ip, fptr, fAlabel); 
   ip = InsNewInst(bp, ip, NULL, LABEL, jBlabel,0,0);   

/*
 * Add loop peeling at the end of the code after cleanup (if this function is 
 * called after cleanup, otherwise after ret)
 */
   AddLoopPeeling(lp, jBlabel, fAlabel, Np, Nv, fptr);
}

int IsSIMDalignLoopPeelable(LOOPQ *lp)
{
   int i, k, n;
   int flag;
   short beg, end, inc;

   beg = lp->beg;
   end = lp->end;
   inc = lp->inc;
   flag = lp->flag;
/*
 * Redesigned: For now, the necessary conditions:
 * 
 * 1. Must have atleast one moving ptr 
 * 2. Must be vectorized
 * 3. optloop must be in simple format: [N,-1,0] or, [0,1,N]
 * 4. Not all array ptr are aligned to vector length
 *    a) using markup : ALIGNED(32) :: *;
 *    b) no array/ptr which is not aligned
 */
/*
 * 1. NO moving ptr !!!, not peelable 
 * FIXED: varrs may not be null but element count is zeor
 */
   if (!lp->varrs || !lp->varrs[0]) /* FIXED */
   {
      fko_warn(__LINE__, "NOT PEELABLE: NO moving ptr!!! \n");
      return 0; 
   }
/*
 * 2. Must be vectorized
 * Note: This is normally called inside vectorization itself; so, by default 
 * vectorization flag is on. Now, let's see whether any vectorizable path 
 * exists as we call the same analyzer for all type of vectorization 
 */
   if (VPATH == -1)
   {
      fko_warn(__LINE__,"NOT PEELABLE: No Vectorizable Path exists!!!\n");
      return 0;
   }
/*
 * 3. optloop must be in simple format: [N,-1,0] or [0,1,N]
 * NoTE: N can be variable or const, but make sure it has sufficient iterations
 */
   if (flag & L_NINC_BIT) // should be [N,-1,0]
   {
      if (!IS_CONST(STflag[end-1]) || SToff[end-1].i != 0 || 
            !IS_CONST(STflag[inc-1]) || SToff[inc-1].i != -1)
      {
         fko_warn(__LINE__, "NOT PEELABLE: NINC not in [N,-1,0] format");
         return 0;
      }
   }
   else if (flag & L_PINC_BIT) // should be [0,1,N]
   {
      if (!IS_CONST(STflag[beg-1]) || SToff[beg-1].i != 0 || 
            !IS_CONST(STflag[inc-1]) || SToff[inc-1].i != 1 )
      {
         fko_warn(__LINE__, "NOT PEELABLE: PINC not in [0,1,N] format(%d,%d)",
                  SToff[beg-1].i, SToff[inc-1].i);
         return 0;
      }
   }
   else
   {
      fko_warn(__LINE__, "NOT PEELABLE: Unknown optloop control format!!!");
      return 0;
   }
/*
 * 4. Not all array ptr are aligned to vector length
 *    a) using markup : ALIGNED(32) :: *;
 *    b) no array/ptr which is not aligned
 */
   if (!lp->aaligned && lp->abalign)  
   {

      if (lp->abalign[1] >= type2len(lp->vflag)) // ALIGNED(VB) :: *;
      {
         fko_warn(__LINE__, "NO PEELING: all are aligned to vlen or greater");
         return 0;
      }
   }
   else if (lp->aaligned) /* alignment is specified for some ptr */
   {
      for (n=lp->varrs[0], i=1; i <= n; i++)
      {
         k = lp->varrs[i];
         if (!(k = FindInShortList(lp->aaligned[0], lp->aaligned+1, k)))
            break;
         else if (lp->abalign[k] < type2len(lp->vflag)) /*k = index of aptr */
            break;
      }
      if (i>n) /* no moving ptr which is not aligned to vlen */
      {
         fko_warn(__LINE__, "NO PEELING: all are aligned to vlen or greater");
         return 0;
      }
   }

   return(1); /* no known case which prohibits loop peeling; so do it */
}
#if 0
/*
 * old SIMD vectorization implementation
 */
int SimdLoop(LOOPQ *lp)
{
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
   enum inst vst, vld, inst;
   short r0, r1, op;
   enum inst *sinst, *vinst;
   int i, j, n, k, nfr=0;
   struct ptrinfo *pi0;
   INSTQ *ip, *ippu, *iph, *iptp, *iptn;
   short vlen;
   enum inst vsld, vshuf;
   short sregs[TNFR], vregs[TNFR];
   /*enum inst vsst, vzero, szero, vmov, smov, vabs, sabs, vsub, vadd, sadd,
    *          vmac, smac, vmul, smul, sst, sld;*/
   /*short *sp;*/

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
      /*vsst = VFSTS;*/
      vshuf = VFSHUF;
      vld = VFLD;
      vst = VFST;
   }
   else
   {
      vsld = VDLDS;
      /*vsst = VDSTS;*/
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
   if (IsSIMDalignLoopPeelable(lp))
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
#endif
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
   extern int FKO_MaxPaths;

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
/*
 *    FIXED: possible memory leak !!!
 *    When backtracks, we will not find the blkstack
 */
      free(blkstack);
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
/*
 * if we have argument for a threshold of max paths
 */   
   if (!FKO_MaxPaths || NPATH < FKO_MaxPaths)
   {
/*
 *    NOTE: if we choose usucc first, we will find fall through path 1st. 
 *    We may use that later.
 */
      if (head->usucc)
      { 
         if (FindBlockInList(loopblocks,head->usucc))
         {
            FindPaths(head->usucc, loopblocks, lp, blkstack);
         }
      }
      if (head->csucc)
      {
         if (FindBlockInList(loopblocks,head->csucc))
         {
            FindPaths(head->csucc, loopblocks, lp, blkstack);
         }
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
   for (bl = lp->blocks; bl ; bl = bl->next)
      fprintf(stdout, "%d ",bl->blk->bnum);
   fprintf(stdout,"\n");
#endif
/*
 * NOTE: Make sure all paths in pathtable are already deleted before
 * new analysis. It should be killed after all path related analysis is done.
 * Killing paths in pathtable here will not work as it may contain info of 
 * previous bitvec (if we already re-initialize the bit vector)
 */
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

int FindNumPaths(LOOPQ *lp)
{
   int i;
   
/*
 * Only one tails right now
 */
   assert(lp->tails && !lp->tails->next);
   
   CalcInsOuts(bbbase);
   CalcAllDeadVariables();
   
   FindLoopPaths(lp); 
   i = NPATH;
   KillPathTable();
   return i;
}



void PrintVars(FILE *out, char* title, INT_BVI iv)
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
      //if (!STname[sp[i-1]])
      if (!STname[sp[i]-1])
         fprintf(out,"[ST NULL:%d] ",sp[i]-1);
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

int IsRefOnlyInArray(BLIST *scope, short vdt) 
{
   int macc;
   BLIST *bl;
   INSTQ *ip, *ip1;
   short ireg, dt;
   macc = 0; 

   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainstN; ip; ip = ip->prev)
      {
         if (ip->inst[0] == LD && ip->inst[2] == vdt)
         {
            macc = 0;
            ireg = ip->inst[1];
            ip1 = ip->next;
            while(IS_LOAD(ip1->inst[0]))
            {
               dt = ip1->inst[2]; 
               if (NonLocalDeref(dt) && ireg == SToff[dt-1].sa[1]) 
                     macc = 1;
               ip1 = ip1->next;
            }
            if (!macc)
               return(0);
         }
      }
   }
   return(1);
}

int CheckVarInBitvec(int vid, INT_BVI iv)
/*
 * check whether this var is set in the bit vector,
 * return 0 otherwise.
 */
{
   short *sp;
   int i, N;
   extern short STderef;
/*
 * skip registers 
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);

   sp = BitVec2Array(iv, 1-TNREG);
   for (N=sp[0],i=1; i <= N; i++)
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
   extern INT_BVI FKO_BVTMP;
   extern int VECT_FLAG;

   int errcode;
   INT_BVI iv, iv1, blkvec;
   int i, j, k, n, N;
   int vflag;
   LOOPQ *lp;
   BLIST *scope, *bl, *blTmp;
   INSTQ *ip;
   struct ptrinfo *pbase, *p;
   ILIST *il;
   /*ILIST *il, *ib;*/
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
   lpflag = 0;
   lp = optloop;
   assert(path);
   scope = path->blocks;
   assert(scope);
/*
 * Find variable accessed in the path and store it in path
 */
   /*iv = NewBitVec(32);*/
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);

   if (!CFUSETU2D )
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#if 0
   fprintf(stderr, "[p:%d] p->uses=%d, p->defs=%d\n", path->pnum, 
           path->uses, path->defs);
#endif   
   for (bl=scope; bl; bl=bl->next)
   {
#if 0
      fprintf(stderr, "[b:%d] b->uses=%d, b->defs=%d\n", bl->blk->bnum, 
              bl->blk->uses, bl->blk->defs);
#endif      
      path->uses = BitVecComb(path->uses, path->uses, bl->blk->uses, '|');
      path->defs = BitVecComb(path->defs, path->defs, bl->blk->defs, '|');
   }
   
   iv = BitVecComb(iv, path->uses, path->defs, '|');
/*
 * right now, skip all the regs from uses, defs
 * NOTE: Need to check why there are redundant vars with diff ST index 
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);

#if 0
   fprintf(stderr,"Vars of path %d \n", path->pnum);
   PrintVars(stderr, "ALL VARS",iv);
   PrintInst(stdout,bbbase);
   PrintLoop(stderr, loopq);
   exit(0);
#endif

/*
 * Skip all non-fp variables, valid entries upto n (included) 
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
/*
 *       it's okay if the variable is not set and only used to calculate
 *       the address of array access (like: lda); otherwise not vectorizable
 */
         if ( CheckVarInBitvec(sp[i]-1, path->defs) || 
              !IsRefOnlyInArray(scope, SToff[sp[i]-1].sa[2]) )
         {
            fko_warn(__LINE__, "Bailing out Path%d on vect due to var %d,%s\n",
                  path->pnum,sp[i],STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
            errcode += 1;
         }
      }
/*
 *    NOTE: need to check the index var variable whether it is set outside
 *    of the loop control. Make sure loop control is killed before.
 *    NOTE: use of loop control variable is okay as long as they are not set 
 *    and not used to set any other integer variable.
 */
      else 
      {
         if (CheckVarInBitvec(sp[i]-1, path->defs))
         {
            fko_warn(__LINE__,
                 "Loop Control variable = %d is updated outside loop control\n",
                     STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
            lpflag &= ~(LP_OPT_LCONTROL);
/*
 *          FIXED: if loop control is updated inside loop, loop is not countable.
 *          we cann't vectorize loop in that case. we have to return this and 
 *          skip any kind of vectorization in that case. Right now, we just 
 *          stop executiong the code.
 */
            errcode += 1;
         }
         else if (sp[i] == lp->I)
         {
/*
 *          NOTE: use of index variable inside loop (out of loop control zone)
 *          and to access memory (like: A[i]) prohibits current vectorization 
 *          method. It can easily be vectorized after adding an additional 
 *          transformation stage which would transform those access into A[0]
 *          / *A and update the pointer by A + inc.
 *          Not implemented yet!!!
 */
            if ( ( il=FindIndexRef(scope, SToff[lp->I-1].sa[2]) ) )
            {
               fko_warn(__LINE__, "Use of index variable out of loop control");
               errcode += 1;
               KillIlist(il);
            }
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
 * FIXED: number of vscal is not correct always: corrected the condition 
 */
#if 0
   for (i=0; i <= n; i++)
   {
      fprintf(stderr, "sp = %d, s = %d\n",sp[i], s[i]);
   }
#endif 
   for (k=0,i=0; i < n; i++) /* sp starts from 0  */
   {
      for (j=1; j <= N && s[j] != sp[i]; j++); /* s[0] has the count */
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
/*
 *       Majedul: Re-writen the analysis for livein/liveout var to be acc/mul
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
 *          b) Not set/ only used: used for MUL/DIV.... 
 *             ADD? skip right now
 *          NOTE: if it is only used, ADD should not create any problem!
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
/*
 *          Line in var which is set and used in vector path 
 */
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
                          "LIVE IN: var %d(%s) must be Accumulator !!\n\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode += 64;
                  scf[i] |= SC_MIXED;
                  /*vsoflag[i+1] &= ~(VS_ACC | VS_MAX);*/
               }
            }
/*
 *          Live in var which is only used
 *          NOTE: only used variable should be vectorizable
 */
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
#if 0
                  fko_warn(__LINE__,
                           "LIVE IN: Mixed use of var %d(%s) "
                           "prevents vectorization!!\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode += 128;
                  scf[i] |= SC_MIXED;
#else
/*
 *                testing ... ... 
 *                Only used variable can be treated like the VS_MUL
 */
                  s[i] |= VS_MUL;
                  scf[i] |= SC_MUL;
#endif
               }
            }
         }
/*
 *       Live out variables
 */
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
/*
 *                HERE HERE
 *                NOTE: for speculative vectorization, a global max var may
 *                not be considered as max at all!
 */
                  fko_warn(__LINE__, "Liveout var %d(%s) must be Accumulator/"
                           "max(nonVs) when set!\n", sp[i], 
                           STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode +=256;
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
                  errcode +=512;
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
      lpflag &= ~LP_VEC; /* look here, don't confuse ~ with ! */
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
      if ((path->sflag[i] & SC_ACC) && (path->sflag[i] & SC_LIVEOUT)) 
         fprintf(stderr, "OUT_ACC ");
      if ((path->sflag[i] & SC_MUL) && (path->sflag[i] & SC_LIVEIN)) 
         fprintf(stderr, "IN_MUL ");
      if ((path->sflag[i] & SC_MUL) && (path->sflag[i] & SC_LIVEOUT)) 
         fprintf(stderr, "OUT_MUL ");
      if ((path->sflag[i] & SC_MAX) && (path->sflag[i] & SC_LIVEOUT)) 
         fprintf(stderr, "OUT_MAX ");
      if ((path->sflag[i] & SC_MIN) && (path->sflag[i] & SC_LIVEOUT)) 
         fprintf(stderr, "OUT_MIN ");
      fprintf(stderr, "\n");
   }
   fprintf(stderr, "ERR CODE: %d\n",errcode);
#endif 
return errcode;
}

int PathVectorizable(int pnum)
/*
 * assuming that all path based analysis is done. Now check whether a path is
 * vectorizable.
 * NOTE: path data structure is satic in this source so that it can't be 
 * directly accessed from other src files
 */
{
   return(PATHS[pnum-1]->lpflag & LP_VEC);
}


int SpeculativeVectorAnalysis()
/*
 * Duplicated the optloop blks, do analysis with this.
 * NOTE: Assuming the number of branches inside loop is few. finding all loop
 * paths is costly, but not worried with that now. possible to  implement 
 * incremental algorithm without explicitly figuring out all the paths
 */
{
   int i, j, n, k;
   LOOPPATH *vpath;
   LOOPQ *lp;
   short *sp;
   char ln[512];
   int *err;
   extern int path;
   /*short *sc, *sf;*/

   lp = optloop;
   VPATH = -1;
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
 * To store the error code of each path
 */
   if (NPATH)
   {
      err = malloc(sizeof(int) * NPATH);
      assert(err);
   }

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
      err[i] = PathFlowVectorAnalysis(PATHS[i]);
      if (err[i]) 
         fko_warn(__LINE__, "Path-%d not vectorizable (error-code=%d)\n",
                  i+1, err[i]);
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
      fprintf(stderr, "Blocks : %s\n",PrintBlockList(PATHS[i]->blocks));
      fprintf(stderr, "Control Flag: %d\n", PATHS[i]->lpflag);
      j = PATHS[i]->lpflag & LP_VEC;
      fprintf(stderr, "Vectorizable: %d\n",j);
      if (!j)
      {
         fprintf(stderr, "   Error Code=%d\n", err[i]);
      }
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
 * If path is specify in command line, try to use it;otherwise, bydefault choose
 * the last vectorizable path.
 * NOTE: First path often not the fall thru path. We will choose fall thru path
 */
/*
 * NOTE: BUG in spec vec when not in fall-thru path... so, enforce the fall-thru
 * path by disabling the if code.
 */

#if 0   
   if (path !=-1)
   {
      assert(path <= NPATH);
      if (PATHS[path-1]->lpflag & LP_VEC)
         VPATH = path-1;
      else
      {
         fprintf(stderr, "Path not vectorizable\n");
         assert(0);
      }
   }
   else
   {
      for (i=0; i < NPATH; i++)
      {
         if (PATHS[i]->lpflag & LP_VEC) /* first vectorizable path */
         {
            VPATH = i;
            break; /* if all paths are vec, then 0 would be the fall-thru */
         }
      }
   }
#else
/*
 * As we apply fallthru transformation, we can just check for the fall-thru
 * path for vectorization!
 */
   for (i=0; i < NPATH; i++)
   {
      if (PATHS[i]->lpflag & LP_VEC) /* first vectorizable path */
      {
         VPATH = i;
         break; /* if all paths are vec, then 0 would be the fall-thru */
      }
   }
/*
 * NOTE: if we apply fall-thru transformation, we must apply it for vector path
 * If default fall-thru path is not vectorization, need to apply fall-thru 
 * transformation.
 * It can't be applied here... see FeedBackLoopInfo()... should apply on vect!
 */
#if 0   
   if (path == -1)
   {
      if (VPATH)
      {
         fprintf(stderr, "\nDEFAULT PATH IS NOT VECTORIZABLE! " 
                         "APPLY FALL-THRU ON VECTOR PATH\n");
         assert(!VPATH);
      }
   }
   else
   {
      /*fprintf(stderr, "Vector path=%d\n", VPATH);*/
      assert(!VPATH);
   }
#else
/*
 * if fall-thru is applied, path becomes the PATH[0] or, fall-thru
 */
   if (path != -1)
   {
      assert(!VPATH);
   }
#endif

#endif
/*
 * free the err code
 */
   if(err) free(err);
   
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
      //SaveFKOState(1);
      return(0);
   }
   else
   {
      return(1);
   }
}

void AddVectorInitReduction(LOOPQ *lp)
{
   int i, j, k, n;
   INSTQ *iph, *iptp, *iptn;
   short r0, r1;    
   enum inst vld, vsld, vst, vsst, vshuf;
/*
 * Figure out what type of insts to translate
 */
   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
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
      vld = VDLD;
      vst = VDST;
   }

   r0 = GetReg(FLAG2TYPE(lp->vflag));
   r1 = GetReg(FLAG2TYPE(lp->vflag));
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
}

void AddVectorBackup(LOOPQ *lp)
{
   int i, N;
   BBLOCK *bp;
   INSTQ *ip;
   short vreg;
   enum inst vld, vst;

/*
 * no var to backup, return. 
 * NOTE: Scalar Restart is called before the VecXform. So, bvvscal is already
 * populated, if any
 */ 
   if (!lp->bvvscal) return; 

   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
      vld = VFLD;
      vst = VFST;
   }
   else
   {
      vld = VDLD;
      vst = VDST;
   }

   bp = lp->header;
   assert(bp->ainst1->inst[0] == LABEL); /* 1st inst should be LABEL*/
   ip = bp->ainst1;
   ip = InsNewInst(bp, ip, NULL, CMPFLAG, CF_SSV_VBACKUP, 0, 0);
   for (N=lp->bvvscal[0], i=1; i <= N; i++)
   {
      if (lp->bvvscal[i])
      {
         vreg = GetReg(FLAG2TYPE(lp->vflag)); 
         ip = PrintComment(bp, ip, NULL, "Backing up vector %s to %s",
                           STname[lp->vvscal[i]-1], STname[lp->bvvscal[i]-1]);
         ip = InsNewInst(bp, ip, NULL, vld, -vreg, 
                           SToff[lp->vvscal[i]-1].sa[2], 0);
         ip = InsNewInst(bp, ip, NULL, vst, SToff[lp->bvvscal[i]-1].sa[2],
                           -vreg, 0);
      }
   }

   ip = InsNewInst(bp, ip, NULL, CMPFLAG, CF_SSV_VBACKUP_END, 0, 0);

}
void SpecVecPathXform(BLIST *scope, LOOPQ *lp)
{
   int i, j, k, m, mskval, nfr;
   INSTQ *ip;
   BLIST *bl;
   static enum inst
      sfinsts[]= {FLD,  FST,  FMUL, FDIV,  FMAC, FADD,  FSUB,  FABS,  FMOV,  
                  FZERO, FNEG},
      vfinsts[]= {VFLD, VFST, VFMUL, VFDIV, VFMAC, VFADD, VFSUB, VFABS, VFMOV, 
                  VFZERO, VFNEG},
      sdinsts[]= {FLDD, FSTD, FMULD, FDIVD, FMACD, FADDD, FSUBD, FABSD, FMOVD, 
                  FZEROD, FNEGD},
      vdinsts[]= {VDLD, VDST, VDMUL, VDDIV, VDMAC, VDADD, VDSUB, VDABS, VDMOV, 
                  VDZERO, VDNEG};
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
#if 0
/*
 * vector memory aligned and unaligned load/store
 * NOTE: not needed in updated implementation
 */
   static enum inst 
      valign[] = {VFLD, VDLD, VLD, VFST, VDST, VST},
      vualign[] = {VFLDU, VDLDU, VLDU, VFSTU, VDSTU, VSTU};
   const int nvalign = 6; /* number of align/unalign inst */
#endif
   enum inst inst, binst, mskinst;
   short sregs[TNFR], vregs[TNFR];
   short op, ir, vrd;
   enum inst *sinst, *vinst, *vcmpinst;
   const int nbr = 6;
   const int nvinst = 11;

   nfr = 0;
/*
 * Figure out what type of insts to translate
 */
   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
      sinst = sfinsts;
      vinst = vfinsts;
      vcmpinst = vfcmpinsts;
   }
   else
   {
      sinst = sdinsts;
      vinst = vdinsts;
      vcmpinst = vdcmpinsts;
   }
/*
 * Translate body of loop
 */
   for (bl=scope; bl; bl = bl->next)
   {
/*
 *    skip the code of vector backup, if exists
 */
      if (bl->blk == lp->header)
      {
         ip = FindCompilerFlag(bl->blk, CF_SSV_VBACKUP_END);
         if (ip)
            ip = ip->next;
         else ip = bl->blk->ainst1;
      }
      else ip = bl->blk->ainst1;
/*
 *    transform the rest of the code
 */
      for ( ; ip; ip = ip->next)
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
 *             find index of appropriate branch and VCMP
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
 *             NOTE: in vector path, no integer is used. So, we can safely
 *             use a int reg.
 */
               ir = GetReg(T_INT);
               ip = InsNewInst(bl->blk, ip, NULL, mskinst, -ir, -vrd, 0);
/*
 *             NOTE: 
 *             By default, we always vectorize the fall-thru case, our 
 *             intension is to get the maxium perf. So, making the scalar 
 *             path as fall-thru is meaningless!! Still, you can see the 
 *             code in the old implementation. 
 */
               mskval = 0;
               ip = InsNewInst(bl->blk, ip, NULL, CMP, -ICC0, -ir, 
                               STiconstlookup(mskval)) ;
/*             ip = InsNewInst(bl->blk, ip, NULL, CMPAND, -ir, -ir, 
                               STiconstLoopup(mskval)) ; */
               ip->next->inst[0] = JNE;   
/*
 *             Majedul: NOTE: here ends a block. Shouldn't we re-initialize
 *             the reg with GetReg(-1). !!!!! 
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
#if 0                              
/*
 *                         otherwise, it is a memory load/store
 *                         check for appropriate alignment using alignment 
 *                         markups
 *                         NOTE: if we have markup for force aligned, rest of
 *                         the arrays are unaligned... we may still have no 
 *                         force aligned array by the markup mutually unaligned 
 *                         is specifies that they are not mutually aligned. We
 *                         use 1st array in varrs to align in loop peeling..
 */
                              else
                              {  
                                 if ((lp->falign && k != lp->falign[1]) || 
                                     (lp->malign == -1 && !lp->falign && 
                                        k != lp->varrs[1]) )
                                 {
                                    k = FindInstIndex(nvalign, valign, 
                                                      ip->inst[0]);
                                    if (k != -1)
                                       ip->inst[0] = vualign[k];
                                 }
                              }
#endif                              
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
}

INSTQ *DupVecPathInst(BLIST *scope, BBLOCK *bp0, LOOPQ *lp)
{
   BLIST *bl;
   INSTQ *ip, *ipN, *ip0;
   
/*
 * NOTE: can be set with a CMPFLAG
 */
   ip0 = ipN = PrintComment(bp0, NULL, NULL, "Duplicable Instructions begin!!!");

   for (bl=scope; bl; bl=bl->next)
   {
/*
 *    skip the code of vector backup, if exists
 */
      if (bl->blk == lp->header)
      {
         ip = FindCompilerFlag(bl->blk, CF_SSV_VBACKUP_END);
         if (ip)
            ip = ip->next;
         else ip = bl->blk->ainst1;
      }
      else ip = bl->blk->ainst1;
      
      for ( ; ip; ip=ip->next)
      { 
/*
 *       skip all LABEL and FLAG
 */
         if (ip->inst[0] == LABEL || ip->inst[0] == CMPFLAG) continue;
/*
 *       Copy the remaining instruction.
 */
         ip0 = InsNewInst(bp0, ip0, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                          ip->inst[3]);
      }
   }
   return ipN;
}

BLIST *UnrollLargerBet(BBLOCK *bp0, LOOPQ *lp, struct ptrinfo *pi,  int unroll)
{
   int i, nbr;
   short imask;
   short r0, r1;
   INSTQ *ip, *ip1, *ipCB, *ipnext;
   BBLOCK *bp; 
   BLIST *bl, *dupblks;

   dupblks = NULL;
/*
 * NOTE:
 * --Assumption: only one conditional branch to handle, no more than 2 paths.
 *  This limitation is relaxed!
 * Note about the duplication:
 * 1. vector path can be xformed as a single basic block after unrolling.
 * 2. As it will be a single block, all labels can be deleted.
 * 3. like the normal unrolling, ptr lds need to be updated
 * 4. unlike the normal unrolling, no need to consider index var update; 
 *    vector path ensure that there would be no integer vars used/set here.
 * 5. issue: 
 *    need extra int reg to preserve the mask val:
 *    - can be kept an int reg live on through the whole unrolled blk
 *    - can be used an int ver to store the mask val and use ld/st format.
 *    NOTE: choose 2nd option now. 
 */
   /*assert(NPATH==2);*/ /* morethan 2 paths are not consider right now */

/*
 * Now, it's time to do it for multiple paths with multiple branches in vector
 * path.
 * NOTE: same imask can be used to store the result after ORing for all the 
 * branches and for all unrolled code, as only one failure would lead to the
 * Scalar Restart.
 */
   
   if (unroll < 2) return(NULL); /* no need to duplicate */
   if (!bp0) return(NULL);       /* nothing to duplicate */
/*
 * update the integer mask and remove the conditional branch.
 */
   imask = InsertNewLocal("_imask", T_INT);
/*
 * Figure out the number of conditional branches inside the main vector 
 * path. If there are more than one branches, we need to update the main
 * vector path with _imask also. But, we will do that after duplicating.
 */
/*
 * Need to point a branch sothat we can copy it at the end
 * NOTE: interesting point: After vectorization, all conditional branches share
 * the same br-inst (JNE), even the destination is also same, points to the 
 * Scalar Restart. So, we can use any of them
 */
   ipCB = NULL;
   nbr = 0;
   for (ip=bp0->inst1; ip; ip=ip->next)
   {
      if (IS_COND_BRANCH(ip->inst[0]))
      {
         nbr++;
         ipCB = ip; /* will overwrite each time, still point valid one */
      }
   }
   assert(ipCB); /* there must be atleast one branch */
/*
 * duplicate inst but change the mask and delete the branch
 */
   for (i=1; i < unroll; i++)
   {
      bp = NewBasicBlock(NULL, NULL);
      bl = NULL;
      bl = AddBlockToList(bl, bp);
      ip1 = PrintComment(bp, NULL, NULL, "Dup-%d begins!!!", i);
/*
 *    copy all instructions
 */
      for (ip=bp0->inst1; ip; ip=ip->next)
         ip1 = InsNewInst(bp, ip1, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                          ip->inst[3]);
/*
 *    update the ptr lds. Assumption: ptr updated already being killed at SV
 *    HERE HERE ptr is already deleted from this path!!! how can we figure out
 *    the moving ptr now!!! Must be passed from outside!!!
 */
      if (pi)
         UpdatePointerLoads(bl, pi, i*Type2Vlen(FLAG2TYPE(lp->vflag)));
/*
 *    change the mask inst and deleted the branch    
 */
      for (ip=bp->ainst1; ip; ip=ip->next)
      {
         if (IS_CMPW(ip->inst[0]) && 
             (ip->next->inst[0]==VFSBTI || ip->next->inst[0] == VDSBTI))
         {
            r0 = GetReg(T_INT);
            r1 = GetReg(T_INT);
            InsNewInst(bp, NULL, ip, LD, -r0, SToff[imask-1].sa[2], 0);
            ip = ip->next; /* mask inst */
/*
 *          update the mask inst with new int reg and insert a store in imask
 */
            ip->inst[1] = -r1;
            ip = InsNewInst(bp, ip, NULL, OR, -r0, -r0, -r1);
            ip = InsNewInst(bp, ip, NULL, ST, SToff[imask-1].sa[2],-r0, 0);
            ip = ip->next;
            assert(IS_CMP(ip->inst[0]) && IS_COND_BRANCH(ip->next->inst[0]));
            ipnext = ip->next;  /* save the next inst*/
            DelInst(ip);
            ip = DelInst(ipnext);
            GetReg(-1);
         }
      }
/*
 *    Insert branch at the end of the for last duplication
 */
      if (i == (unroll-1) )
      {
         r0 = GetReg(T_INT);
         ip = bp->ainstN;
         ip = InsNewInst(bp, ip, NULL, LD, -r0, SToff[imask-1].sa[2], 0);
         ip = InsNewInst(bp, ip, NULL, CMP, -ICC0, -r0, STiconstlookup(0));
         ip = InsNewInst(bp, ip, NULL, ipCB->inst[0], ipCB->inst[1], 
                         ipCB->inst[2], ipCB->inst[3]);
         GetReg(-1);
      }
#if 0
      fprintf(stderr, "Dup #%d: \n", i);
      fprintf(stderr, "================\n");
      PrintThisBlockInst(stderr, bp);
#endif
/*
 *    add blks in list
 */
      dupblks = AddBlockToList(dupblks, bp);
      //KillBlockList(bl);
      //KillAllInst(bp);
      //KillAllBasicBlocks(bp);
   }
/*
 * update the main loop blk
 * NOTE: If there are more than one one branches, update the later branches 
 * as well
 */
   for (ip=bp0->ainst1; ip; ip=ip->next)
   {
      if (IS_CMPW(ip->inst[0]) && 
         (ip->next->inst[0]==VFSBTI || ip->next->inst[0] == VDSBTI))
      { 
         r0 = GetReg(T_INT);
         ip = ip->next; /* mask inst */
/*
 *       update the mask inst with new int reg and insert a store in imask
 */
         ip->inst[1] = -r0;
         ip = InsNewInst(bp, ip, NULL, ST, SToff[imask-1].sa[2], -r0, 0);
         ip = ip->next;
         assert(IS_CMP(ip->inst[0]) && IS_COND_BRANCH(ip->next->inst[0]));
         ipnext = ip->next;  /* save the next inst*/
         DelInst(ip);
         ip = DelInst(ipnext);
         GetReg(-1);
         break;   /* this is to ensure for the 1st branch */
      }
   }
/*
 * If there are multiple branches, update the later branches with imask
 */
   if (nbr > 1)
   {
      assert(ip); /* DelInst returns the next inst */
      for (; ip; ip=ip->next)
      {
         if (IS_CMPW(ip->inst[0]) && 
             (ip->next->inst[0]==VFSBTI || ip->next->inst[0] == VDSBTI))
         {
            r0 = GetReg(T_INT);
            r1 = GetReg(T_INT);
            InsNewInst(bp, NULL, ip, LD, -r0, SToff[imask-1].sa[2], 0);
            ip = ip->next; /* mask inst */
/*
 *          update the mask inst with new int reg and insert a store in imask
 */
            ip->inst[1] = -r1;
            ip = InsNewInst(bp, ip, NULL, OR, -r0, -r0, -r1);
            ip = InsNewInst(bp, ip, NULL, ST, SToff[imask-1].sa[2],-r0, 0);
            ip = ip->next;
            assert(IS_CMP(ip->inst[0]) && IS_COND_BRANCH(ip->next->inst[0]));
            ipnext = ip->next;  /* save the next inst*/
            DelInst(ip);
            ip = DelInst(ipnext);
            GetReg(-1);
         }
      }
   }

/*
 * reverse the list to maintain the sequence, add the original blk
 */
   dupblks = ReverseBlockList(dupblks);
   dupblks = AddBlockToList(dupblks, bp0);
#if 0
   for (bl=dupblks; bl; bl=bl->next)
   {
      PrintThisBlockInst(stderr, bl->blk);
   }
#endif   

   return(dupblks);
}

void InsUnrolledCodeToVpath(LOOPQ *lp, BLIST *scope, BLIST *dupblks)
{
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ipn;
   
   if (!dupblks) return;
/*
 * delete all inst after backup to tails (upto the label before CF_LOOP_TEST)
 * Assumption: always jumpback from scalar_resatrt to label before CF_LOOP_TEST
 * which follows the current implementation.
 */
   for (bl=scope; bl; bl=bl->next)
   {
/*
 *    skip the code of vector backup, if exists
 */
      if (bl->blk == lp->header)
      {
         ip = FindCompilerFlag(bl->blk, CF_SSV_VBACKUP_END);
         if (ip)
            ip = ip->next;
         else
         {
            assert(bl->blk->ainst1->inst[0] == LABEL); /* atleast has a label*/
            ip = bl->blk->ainst1->next;
         }
      }
      else ip = bl->blk->inst1;
/*
 *    If it is tail blk, delete inst upto the LABEL inserted by Scalar Restart 
 */
      if (FindBlockInList(lp->tails, bl->blk))
      {
         for ( ; ip; ip=ipn)
         {
/*
 *          HERE HERE, need to identify the LABEL which is inserted by the
 *          Scalar Resart. 
 *          NOTE: kept all labels. So, now just need to check whether The 
 *          CMPFLAG is one them which are used to update loopcontrol
 */
            if (ip->inst[0] == CMPFLAG && 
                (ip->inst[1] == CF_LOOP_PTRUPDATE 
                 || ip->inst[1] == CF_LOOP_UPDATE))
               break;
            else
            {
               if (ip->inst[0] == LABEL || ip->inst[0] == CMPFLAG)
                  ipn = ip->next;
               else
                  ipn = DelInst(ip);
            }
         }
      }
/*
 *    for other blks, delete all instructions
 */
      else 
      {
         while (ip)
            ip = DelInst(ip);
      }
   }

/*
 * Now, add instructions from duplicated blks at the header of lp
 */
   bp = lp->header;
   ip0 = FindCompilerFlag(bp, CF_SSV_VBACKUP_END);
   if (!ip0) 
   {
      ip0=bp->ainst1;
      assert(ip0->inst[0] == LABEL);
   }
   for (bl=dupblks; bl; bl=bl->next)
   {
      for (ip=bl->blk->inst1; ip; ip=ip->next)
      {
         ip0 = InsNewInst(bp, ip0, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                          ip->inst[3]);
      }
   }
/*
 * delete all temporary blks and inst
 */
   for (bl=dupblks; bl; bl=bl->next)
      KillAllBasicBlocks(bl->blk);
   KillBlockList(bl);

#if 0
   fprintf(stdout, "Final SV with Unroll\n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif   
}


int SpecVecXform(LOOPQ *lp, int unroll)
/*
 * supports larger bet for unrolling of speculative vectorization
 */
{
   BBLOCK *bp;
   BLIST *scope, *dupblks;
   INSTQ *ippu;
   struct ptrinfo *pi0;
   short vlen, URbase;
   /*INSTQ *ip0, *ip;*/ 
/*
 * Need at least one path to vectorize
 */
   assert(VPATH!=-1);
   scope = PATHS[VPATH]->blocks;

   vlen = Type2Vlen(lp->vflag);
   URbase = unroll? vlen*unroll: vlen;

/*
 * If Loop control is already not killed, kill all controls in optloop 
 * We will put back the appropriate loop control at the end of the function.
 */
   KillLoopControl(lp);

/*
 * Find all pointer updates, and remove them from vector path (leaving only
 * vectorized instructions for analysis); will put them back in loop after
 * vectorization is done.
 */
   pi0 = FindMovingPointers(scope);
   ippu = KillPointerUpdates(pi0, URbase);
/*
 * It's time to generate the vector initialization and vector reduction
 */
   AddVectorInitReduction(lp);
/*
 * add vector backup there based on lp->bvvscal. Recovery code is inserted in
 * Scalar Restart.
 */
   AddVectorBackup(lp);
/*
 * Transform the vector path 
 */
   SpecVecPathXform(scope, lp);
/*
 * Now, we need to duplicate the vector path 
 * ***********************************************************
 */
   bp = NewBasicBlock(NULL, NULL);
#if 0
   ip0 = DupVecPathInst(scope, bp, lp); 
   dupblks = UnrollLargerBet(bp, lp, pi0,  unroll);
   PrintThisInstQ(stderr, ip0);
   PrintThisBlockInst(stderr, bp);
#else
   DupVecPathInst(scope, bp, lp); 
   dupblks = UnrollLargerBet(bp, lp, pi0,  unroll);
#endif 
   KillAllPtrinfo(pi0); /* pi0 is needed in unroll. So, kill after that */

/*
 * Put back loop control and pointer updates
 */
   OptimizeLoopControl(lp, URbase, 0, ippu);
/*
 * Now, we can update with the duplicated blks, deleting the previous insts 
 */
   if (dupblks) 
      InsUnrolledCodeToVpath(lp, scope, dupblks);

   CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = CFLOOP = 0;
   
   return(0);
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
   BLIST *bl, *scope;
   static enum inst
      sfinsts[]= {FLD,  FST,  FMUL, FDIV,  FMAC, FADD,  FSUB,  FABS,  FMOV,  
                  FZERO, FNEG},
      vfinsts[]= {VFLD, VFST, VFMUL, VFDIV, VFMAC, VFADD, VFSUB, VFABS, VFMOV, 
                  VFZERO, VFNEG},
      sdinsts[]= {FLDD, FSTD, FMULD, FDIVD, FMACD, FADDD, FSUBD, FABSD, FMOVD, 
                  FZEROD, FNEGD},
      vdinsts[]= {VDLD, VDST, VDMUL, VDDIV, VDMAC, VDADD, VDSUB, VDABS, VDMOV, 
                  VDZERO, VDNEG};
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

   const int nvinst = 11;
   enum inst vld, vst, inst, binst, mskinst;
   /*enum inst vzero, szero, vmov, smov, vabs, sabs, vsub, ssub, vadd, sadd, vmac,
             smac, vdiv, sdiv, vmul, smul, sst, sld;*/
   short r0, r1, op, ir, vrd;
   enum inst *sinst, *vinst, *vcmpinst;
   int i, j, n, k, m, mskval, nfr=0;
   struct ptrinfo *pi0;
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
/*
   r0 = GetReg(FLAG2TYPE(lp->vflag));
   r1 = GetReg(FLAG2TYPE(lp->vflag));
*/
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

   pi0 = FindMovingPointers(scope); /* mem leak, need to kill */
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
   
   r0 = GetReg(FLAG2TYPE(lp->vflag));
   r1 = GetReg(FLAG2TYPE(lp->vflag));
   
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
 *             find index of appropriate branch and VCMP
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
/*============================================================================
 *             Under Construction !!!!!
 *             Just for tesing ... .... .... 
 *============================================================================*/
#if 0
/*
 *             NOTE: consider the 'path =1' is case for vector fall thru.
 *             Later, we will need a tester to test it !!!!
 */
               extern int path;

               if (path==1 && VPATH==0)
               {
/*
 *                vector path is the fall thru 
 */
                  mskval = 0;
                  ip = InsNewInst(bl->blk, ip, NULL, CMP, -ICC0, -ir, 
                                  STiconstlookup(mskval)) ;
/*                ip = InsNewInst(bl->blk, ip, NULL, CMPAND, -ir, -ir, 
                                  STiconstLoopup(mskval)) ; */
                  ip->next->inst[0] = JNE;   
               }
               else
               {
/*
 *                if the scalar is the fall thru
 */
                  fprintf(stderr, "SCALAR FALL-TRU SELECTED!\n\n");

                  mskval = 1;
                  for (i = 1; i < vlen; i++)
                     mskval = (mskval << 1) | 1;
                  ip = InsNewInst(bl->blk, ip, NULL, CMP, -ICC0, -ir, 
                                  STiconstlookup(mskval)) ;
                  ip->next->inst[0] = JEQ;
               }

#else          /* consider vcmp_any case.. only fall through vector path */
/*
 *             NOTE: 
 *             By default, we always vectorize the fall-thru case, our 
 *             intension is to get the maxium perf. So, making the scalar 
 *             path as fall-thru is meaningless!!
 */
               mskval = 0;
               ip = InsNewInst(bl->blk, ip, NULL, CMP, -ICC0, -ir, 
                               STiconstlookup(mskval)) ;
/*             ip = InsNewInst(bl->blk, ip, NULL, CMPAND, -ir, -ir, 
                               STiconstLoopup(mskval)) ; */
               ip->next->inst[0] = JNE;   
#endif 
/*
 *             Majedul: NOTE: here ends a block. Shouldn't we re-initialize
 *             the reg with GetReg(-1). !!!!! 
 *             FIXME: 
 */
/*=========================================================================*/
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
   enum inst vld, vst, inst;
   /*enum inst vzero, szero, vmov, smov, vabs, sabs, vsub, ssub, vadd, sadd, vmac,
             smac, vmul, smul, sst, sld;*/
   short r0, r1, op;
   enum inst *sinst, *vinst;
   int i, j, n, k, nfr=0;
   struct ptrinfo *pi0;
   INSTQ *ip, *ippu, *iph, *iptp, *iptn;
   short vlen;
   enum inst vsld, vsst, vshuf;
   short sregs[TNFR], vregs[TNFR];
   /*short *sp;*/

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
   if (IsSIMDalignLoopPeelable(lp))
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
   
   r0 = GetReg(FLAG2TYPE(lp->vflag));
   r1 = GetReg(FLAG2TYPE(lp->vflag));
   
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

#if 0
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
#endif

int IsBackupCandidate(short var, short vpflag, int chcom)
/*
 * check whether this var is a candidate for back and recovery, 
 * when chcom is set, check only the common blks upto cond branch
 * vpflag represent the flag got from vector path
 */
{
   int i, nbr;
   BLIST *bl, *scope;
   INSTQ *ip;
/*
 * private var need no backup/recovery
 */
   if (vpflag & SC_PRIVATE) return(0);
   if (vpflag & SC_SET)
   {
/*
 * if chcom is set, only the common blks before 1st conditional branch is 
 * checked. 
 */
      if (chcom)
      {
         scope = PATHS[VPATH]->blocks;
/*
 *       We may have multiple conditional branches when we deals with several
 *       paths. 
 *       Algorithm: we will check until the last branch arrives. As after that 
 *       we don't need to jump to Scalar Restart, we don't need to consider inst
 *       beyond that. 
 */
/*
 *       Figure out the number of branches in vector path
 */
         nbr = 0;
         for (bl=scope; bl; bl=bl->next)
            for (ip=bl->blk->inst1; ip; ip=ip->next)
               if (IS_COND_BRANCH(ip->inst[0]))
                  nbr++;
/*
 *       assuming blks are consequtive in path, FindLoopPath algo suggests 
 *       that
 */
         for (bl=scope; bl; bl=bl->next)
         {
            for (ip=bl->blk->inst1, i=1; ip; ip=ip->next, i++)
            {
/*
 *             can be checked whether a var is set using bitvec, but 
 *             I use here directly the inst format as it maintains the ld
 *             st syntax yet.
 *             NOTE: Now, there can be multiple conditional branches. 
 *             we need to check until the last branch ... 
 */
               if (IS_COND_BRANCH(ip->inst[0]) && i == nbr)
               {
                  return(0);
               }
               /*else if (IS_STORE(ip->inst[0]) && ip->inst[1] == var)*/
               else if(ip->inst[1] == var)
               {
                  return(1);
               }
            }
         }
         return(0);
      }
      else
         return(1);
   }
   else return(0); /* if not set in vpath, no need to backup it */
}

void AddVectorRecover(LOOPQ *lp, BBLOCK *bp0, int chcom)
/*
 * adds backup in vector path and recovery at the first of scalar path
 * if chcom (check common) is set, check only common path before the conditional
 * branch.
 */
{
   int i, j, N, n, k;
   short *sc, *vvsc, *sf, *bvvscal;
   INSTQ *ip;
   enum inst vld, vst;
   short vreg;
   char ln[512];
/*
 * temporary for backup variables
 */
   n = 0;
   assert(lp->vvscal);
   bvvscal = calloc(lp->vvscal[0]+1, sizeof(short));
   assert(bvvscal);
/*
 * we need to check only the vector path for vector backup and recovery
 * NOTE: What happens for Memory write!!! There is no recopvery machanism. 
 * FIXME: right now, we don't consider memory write in speculation. we
 * should add an assert to exit if there is a memory write in our scope.
 */
   vvsc = lp->vvscal;
/*
 * NOTE: in vpath, scal and vscal represent the same. There should not be any
 * scal in vector path which is not vectorizatable. So, in vector path, I use
 * scal and vscal interchangeably. But for scalar path, it would not be the 
 * case. vscal should be NULL. 
 * HERE HERE, but sflag and vsflag can't be used interchangeably. 
 * FIXME: Then why both scal and vscal are used in optloop !!! 
 */
   sc = PATHS[VPATH]->vscal; /* check only the scalar of vpath */
   sf = PATHS[VPATH]->sflag;

   for (i=1, N=sc[0]; i <= N; i++) /* */
   {
      if (IsBackupCandidate(sc[i], sf[i], chcom))
      {
/*
 *       accroding to current implementation of PathFlowVectorAnalysis, 
 *       #elem of scal, vscal are same. 
 *       inserted a assert about the assumption.
 */
         assert(lp->vscal[i] == PATHS[VPATH]->vscal[i] 
                && lp->vscal[i] == PATHS[VPATH]->scal[i]) ;
/*
 *       create new vector var for backup ... ... ... 
 */   
         sprintf(ln, "_B%s", STname[vvsc[i]-1]);
         k = bvvscal[i] = STdef(ln, LOCAL_BIT | FLAG2TYPE(lp->vflag), 0);
         j = AddDerefEntry(-REG_SP, k, -k, 0, k);
         SToff[k-1].sa[2] = j; /* AddDerefEntry may change the SToff itself */
         n++;
#if 0         
         fprintf(stderr, "%s: %s(%s) needs backup!\n", STname[bvvscal[i]-1], 
                 STname[vvsc[i]-1], STname[sc[i]-1]);
#endif         
      }
   }
/*
 * has backup var? update optloop
 */
   if (n)
   {
      bvvscal[0] = N;
      lp->bvvscal = bvvscal;
/*
 *    Add code to restore the original vector from the backup at the begining of
 *    scalar restart. This func is call right after the creation of the scalar
 *    restart blk. so, add inst at the end to keep the necessary labels intack.
 *    HERE HERE, backup of this variable should be at the beginning of vector 
 *    path.
 */
/*
 *    set appropriate inst
 */
      if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
      {
         vld = VFLD;
         vst = VFST;
      }
      else
      {
         vld = VDLD;
         vst = VDST;
      }

      ip = bp0->instN;
      ip = InsNewInst(bp0, ip, NULL, CMPFLAG, CF_SSV_VRECOVERY, 0, 0);
      for (N=lp->bvvscal[0], i=1; i <= N; i++)
      {
        if(lp->bvvscal[i])
        {
           vreg = GetReg(FLAG2TYPE(lp->vflag)); 
           ip = PrintComment(bp0, ip, NULL, "Restoring vector %s form %s",
                             STname[lp->vvscal[i]-1], STname[lp->bvvscal[i]-1]);
           ip = InsNewInst(bp0, ip, NULL, vld, -vreg, 
                           SToff[lp->bvvscal[i]-1].sa[2], 0);
           ip = InsNewInst(bp0, ip, NULL, vst, SToff[lp->vvscal[i]-1].sa[2],
                           -vreg, 0);
           GetReg(-1);
        }
      }
   }
   else
   {
      lp->bvvscal = NULL;
      free(bvvscal);
   }

#if 0
   fprintf(stderr, "\n Vaiable set in vector path\n");
   fprintf(stderr, "================================\n");

    //fprintf(stderr, "Control Flag: %d\n", PATHS[i]->lpflag);
   fprintf(stderr, "SET: ");
   sc = PATHS[VPATH]->scal;
   sf = PATHS[VPATH]->sflag;
   for (j=1, N=sc[0]; j <= N; j++ )
   {
      if (sf[j] & SC_PRIVATE) continue;
      if (sf[j] & SC_SET)
         fprintf(stderr,"%s(%d) ",STname[sc[j]-1], sf[j]);
   }
   fprintf(stderr,"\n");
#endif
   
}

void AddScalarUpdate(LOOPQ *lp, BBLOCK *bp0)
/*
 * Vector-to-scalar reduction to operate on scalar in Scalar Restart
 * inst is added at the end of the block bp0
 */
{
   int i, N;
   short r0, r1;
   short vpsflag, vscal, vvflag;
   INSTQ *ip;
   LOOPPATH *vp;
   /*short scal, spsflag;*/

   vp = PATHS[VPATH];

/*
 * NOTE: variable that is set on vector path needs vector to scalar reduction.
 * So, the Scalar update doesn't depend on scalar path at all!!!
 */
   for (i=1, N=vp->vscal[0]; i <= N; i++)
   {
      vscal = vp->vscal[i];
      vpsflag = vp->sflag[i]; /* SC flags for the vars */
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
              ip = PrintComment(bp0, ip, NULL,"Reduce accumulator vector for "
                                "%s", STname[vscal-1]);
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
              ip = PrintComment(bp0, ip, NULL, "Reduce accumulator vector for " 
                                "%s",STname[vscal-1]);
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
/*
 *          NOTE: currently we only supports accumulator as a set var!
 *          Will implement when we expands the limitation
 */
            assert(0); /* should be filter by the analysis itself */
         }
      }
   }
}

int isVarSetInScope(short var, BLIST *scope)
{
   int isSet;
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip;

   isSet = 0;
   
   for (bl=scope; bl; bl=bl->next)
   {
      bp = bl->blk;
      for (ip=bp->inst1; ip; ip=ip->next)
      {
         if (IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]-1] == var)
         {
            isSet = 1;
            break;
         }
      }
      if (isSet)
         break;
   }
   return isSet;
}

void AddVectorUpdate(LOOPQ *lp, BBLOCK *blk)
/*
 * updates necessary vars for the next speculative vector iteration
 * let's get rid of spath!!!
 */
{
   INSTQ *ip;
   int i, N;
   int isScSet;
   short vpsflag, vscal;
   LOOPPATH *vp;
   enum inst vsld, vshuf, vst;
   short r0;
   extern short STderef;
   extern INT_BVI FKO_BVTMP;
   /*INT_BVI iv;*/
   /*enum inst fst;*/
   /*LOOPPATH *sp;*/
   /*short vlen;*/
   /*short vsflag, spsflag, scal;*/

   vp = PATHS[VPATH];

   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
      vsld = VFLDS;
      vshuf = VFSHUF;
      vst = VFST;
      /*fst = FST;*/     /* to check the recovery vars */
#if 0
   #if defined(X86) && defined(AVX)
         vlen = 8;
      #else
         vlen = 4;
      #endif
#endif
   }
   else
   {
      vsld = VDLDS;
      vshuf = VDSHUF;
      vst = VDST;
      /*fst = FSTD;*/
#if 0
      #if defined(X86) && defined(AVX)
         vlen = 4;
      #else
         vlen = 2;
      #endif
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
      isScSet = 0;            /* is set in scalar path ? */
      /*scal = vp->scal[i];*/   /* scal and vscal are same for vector path */
      vscal = vp->vscal[i];
      vpsflag = vp->sflag[i]; /* flags for the vars */
      /*vsflag = vp->vsflag[i];*/ /* used lp->vsflag[i]*/
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
      else if ( (vpsflag & SC_USE) && !(vpsflag & SC_SET)) 
      {
/*
 *       check the scal flag for the scalar path 
 *       If it is set here, update the appropriate vector.
 */
#if 0
         for (k=0; k < NPATH; k++)
         { 
            if (k == VPATH) continue;
            sp = PATHS[k];
            for (j=1, N1=sp->scal[0]; j <= N1; j++)
            {
               if (vscal == sp->scal[j] && (sp->sflag[j] & SC_SET))
               {
                  //found = 1;
                  //spsflag = sp->sflag[j];
                  isScSet = 1;
                  break;
               }
            }
         }
#else
/* 
 *       checking all path may become exponential. so, we just
 *       check the loop blocks for the set info.
 *       NOTE: don't use bvec, they may not be updated 
 */
         isScSet=isVarSetInScope(vscal,lp->blocks);
         /*fprintf(stdout, "vscal = %s[%d], set=%d\n", 
                   STname[vscal-1], vscal-1, isScSet);*/

#endif

/*
 *       if it's a candidate, go ahead to add the vector update
 */
         if (isScSet)
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
/*
 *    although a var is not use in vector path but set, we need to check
 *    whether it is live out at the exit of loop. If so, we need to update
 *    the vector. not implemented yet as not needed for iamax
 */
      else if ( !(vpsflag & SC_USE) && (vpsflag & SC_SET))
      {
/*
 *       NOTE: as we only permit accumulator to be set in vector path right now,
 *       we will not face this issue. will extends when remove this limitation
 */
         assert(0);
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
/*
 * update flags based on the loop. if loop is not lpOpt, we don't consider other
 * flags at all.
 */
{
   int i, ptested;
   int lo, ui, up;
   ILIST *il;
   struct ptrinfo *pi, *p;
   extern int FKO_MaxPaths;
/*
 * by default, all are set. 
 */
   lo = 1; ui = 1; up = 1;
   ptested = 0;
   pi = NULL;

#if 0   
/*
 * Check all paths of the loop is optimizable for loop control and loop_mov_ptr
 * Right now, consider both yes or no. Later can be optimized for either one
 * LP_OPT_MOVPTR = PTRF_CONTIG | PTRF_INC + Single update 
 * LO_OPT_LCONTROL = index defs outside loop control!... loop becomes 
 *    uncountable, hence not vectorizable at all...
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
#else
/*
 * update all flags without analyzing all paths
 * NOTE: if !LP_OPT_LCONTROL... loop becomes uncountable.. so, not vectorizable
 * So, just consider LP_OPT_MOVPTR... it ensures that ptr update can be deleted
 * and kept at the end of loop before loop control
 */
   if (!FKO_MaxPaths || NPATH < FKO_MaxPaths) /* not all paths traverse!!! */
   {
      for (i = 0; i< NPATH; i++ )
      {
         if ( !(PATHS[i]->lpflag & LP_OPT_LCONTROL) || 
               !(PATHS[i]->lpflag & LP_OPT_MOVPTR) )
         {
            lo = 0;
            break;
         }
      }
   }
   else /* check moving paths .... */
   {
      pi = FindMovingPointers(lp->blocks); 
      ptested = 1;
      for (p = pi; p; p=p->next)
      {
         if (IS_FP(STflag[p->ptr-1]))
         {
            if ((p->flag | PTRF_CONTIG | PTRF_INC) != p->flag)
            {
               lo = 0;
               break;
            }
            else if (p->nupdate > 1)
            {
               lo = 0;
               break;
            }
            else if (!(p->flag & PTRF_CONTIG))
            {
               lo = 0;
               break;
            }
            else;
         }
         else assert(0); /* we only consider fp ptr now */
      }
   }
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
      if (!ptested)
         pi = FindMovingPointers(lp->blocks);
      if (!pi)
         up = 0;
   }
#endif
/*
 * Update the flags
 */
   *lpOpt = lo;
   *usesIndex = ui;
   *usesPtr = up;
}

void DupBlkPathWithOpt(LOOPQ *lp, BLIST **dupblks, int islpopt, int UsesIndex,
                       int UsesPtrs, int dups)
/*
 * duplicates loop blks vector lenth's time with loop index and ptr update
 * optimization if valid
 */
{
   int i;
   int URbase, cflag;
   INT_BVI iv;
   BBLOCK *newCF, *bp;
   INSTQ *ip, *ipn;
   BLIST *bl;
   char ln[512];
   struct ptrinfo *pi;
   extern INT_BVI FKO_BVTMP;
   /*int vlen;*/
   /*static int fid = 1;*/
#if 0
   vlen = Type2Vlen(lp->vflag);
   URbase = (lp->flag & L_FORWARDLC_BIT) ? 0 :vlen-1;
#else
   URbase = (lp->flag & L_FORWARDLC_BIT) ? 0 :dups-1;
#endif
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);

   for ( i=0; i < dups; i++)
   {
      SetVecBit(lp->blkvec, lp->header->bnum-1, 0);
      FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
      /*newCF = DupCFScope(lp->blkvec, iv, fid++, lp->header);*/
      newCF = DupCFScope(lp->blkvec, iv, lp->header);
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
         if (UsesPtrs)
         {
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
         }
/*
 *       Need to check the loop for forwarding or not!
 */
         if (UsesIndex)
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
            sprintf(ln, "_S_%d",i+1); /*1st block of next */
            cflag = STlabellookup(ln);
            InsNewInst(bp, NULL,ip,JMP,-PCREG,cflag,0);
         }
         else
         {
            ip = bp->instN; /*CMPFLAG isn't considered as active inst*/
            assert(ip);
            sprintf(ln, "_S_%d",i+1); /*1st block of next */
            cflag = STlabellookup(ln);
            InsNewInst(bp, ip,NULL,JMP,-PCREG,cflag,0);
         }
            
      }
      
   }
   
}

BBLOCK *GenScalarRestartPathCode(LOOPQ *lp, int path, int dups, BLIST *ftheads, 
                                 INT_BVI ivtails)
{
   int i;
   char ln[512];
   int cflag;
   BBLOCK *bp0, *bp, *bptop, *bpi;
   BLIST **dupblks;
   BLIST *bl;
   INSTQ *ip, *ip0;
   int islpopt, UsesIndex, UsesPtrs;


   dupblks = malloc(sizeof(BLIST*)*dups);
   assert(dupblks);
/*
 * Set all flags to optimized the duplicated blocks
 */
   SetScalarRestartOptFlag(lp, &islpopt, &UsesIndex, &UsesPtrs);

/*
 * dublicates blocks with required optimization
 */
   DupBlkPathWithOpt(lp, dupblks, islpopt, UsesIndex, UsesPtrs, dups);

/*
 * create a new BBLOCK structure for the scalar restart of this path
 */
   bptop = bp0 = NewBasicBlock(NULL,NULL);
   sprintf(ln, "_Scalar_Restart");
   cflag = STlabellookup(ln);
   InsNewInst(bp0, NULL,NULL,LABEL,cflag,0,0);
/*
 * Here starts the backup recovery operation.
 * Right now, it is dummy function.
 * NOTE: We will need to check the whole vector path for backup/recovery 
 * if we want to unroll it, otherwise we need to check only the blks upto
 * the conditional branch to scalar_restart
 */
/*
 * HERE HERE, why dasum (with if clause) with vector backup/recovery 
 * provides better performance!!!
 */
  AddVectorRecover(lp, bp0, dups > Type2Vlen(lp->vflag)? 0 : 1);
/*
 * Here, we start vector to scalar update
 */
   AddScalarUpdate(lp, bp0);

/*
 * copy duplicated blocks 
 */
   for (i = 0; i < dups; i++)
   {
/*
 *    create new block for each iteration
 */
      bp = NewBasicBlock(bp0, NULL);
      bp0->down = bp;
      bp->up = bp0;
      bp0 = bp;
      sprintf(ln, "_S_%d",i);
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
/*
 *          if bp->down is not a succesor of bp, we don't need to explore that
 *          NOTE: it's a safe guard. We ensure that everytime, only one path is
 *          copied. ftheads contains the head of each path.
 */
            if (bp->usucc != bp->down && bp->csucc != bp->down)
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
   sprintf(ln, "_S_%d",i);
   cflag = STlabellookup(ln);
   ip = InsNewInst(bp, NULL, NULL, LABEL, cflag,0,0);
/*
 * Create a label to indicate the vector update stage
 */
   sprintf(ln, "_IFKO_VECTOR_UPDATE");
   cflag = STlabellookup(ln);
   ip = InsNewInst(bp, ip, NULL, LABEL, cflag,0,0);
/*
 * Add vector update for this path
 */
   AddVectorUpdate(lp, bp);

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
#else
/*
 *    We need to jump back before the ptr update but after any other 
 *    instruction. So, insert LABEL accordingly.
 */
      ip = FindCompilerFlag(bpi, CF_LOOP_PTRUPDATE);
      if (!ip) ip = FindCompilerFlag(bpi, CF_LOOP_UPDATE);
      assert(ip);
      sprintf(ln,"%s_LOOP_TEST",STname[lp->body_label-1]);
      cflag = STlabellookup(ln);
      /*fprintf(stderr,"L:%s = %d\n\n",ln,cflag);*/
      InsNewInst(bpi, NULL, ip, LABEL, cflag,0,0);
#endif      
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

BBLOCK *CreateSclResBlk(BBLOCK *fromBlk, BBLOCK *toBlk, int path)
/*
 * create starting blk for scalar restart at the end of RET blk
 */
{
   int label;
   BBLOCK *bp;
   char ln[512];
   extern BBLOCK *bbbase;
   for (bp=bbbase; bp; bp=bp->down)
   {
      if (bp->ainstN->inst[0] == RET)
         break;
   }
   assert(bp);
/*
 * create new blk for scalar restart
 */
   bp->down = NewBasicBlock(bp, bp->down); 
   bp = bp->down;
   if(bp->down) bp->down->up = bp;
   sprintf(ln,"_Start_Scal_%d",path);
   label = STlabellookup(ln);
   InsNewInst(bp, NULL, NULL, LABEL, label ,0,0);
/*
 * correct the label of from blk
 */
   assert(IS_COND_BRANCH(fromBlk->ainstN->inst[0]));
   fromBlk->ainstN->inst[3] = label;
/*
 * add jump to the to blk. LABEL is an active inst... will be deleted anyway.
 */
#if 0   
   assert(toBlk->ilab);
   InsNewInst(bp, bp->ainstN, NULL, JMP, -PCREG, toBlk->ilab, 0);
#endif
   return bp;
}

/*=============================================================================
 * Rewriting the Scalar Restart so that it can be applied for loop paths more
 * than two. This design depends on the following assertions:
 *
 *    1. After SV, there would two parts of the loop structure: vector path and
 *    scalar restart. 
 *    
 *    2. We will need only one Scalar Restart and jump to that as soon as
 *    any of the condition for vector path fails. So, there would be multiple
 *    entry points of scalar restart but only one exit (like: superblock but 
 *    here Scalar Restart has many blocks not a single super blk)
 *
 *    3. After generating the Scalar Restart, we will delete all blocks of all
 *    scalar paths which are not common with the vector path
 *
 *    4. Scalar Restart will be generated at the end of Return(RET) block (where
 *    is the epilogue of the function) always for the shake of generality.
 *
 *============================================================================*/

void AddScalarRestart(BBLOCK *base)
/*
 * this function will add the scalar restar code at the end of epilogue
 */
{
   BBLOCK *bp, *bpdown;
   extern BBLOCK *bbbase;

   for (bp=bbbase; bp; bp=bp->down)
   {
      if (bp->ainstN->inst[0] == RET)
         break;
   }
   assert(bp);          /* there must be a epilogue */
   bpdown = bp->down;
/*
 * add scalar restart code
 */
   bp->down = base;
   base->up = bp;

   for (bp=base; bp->down; bp=bp->down)
      ;                 /* find out the last block of scalar restart */
   bp->down = bpdown;   /* append the previous down blk of epilogue */
   bpdown->up = bp;
}

int ScalarRestart(LOOPQ *lp, int ndups)
/*
 * This function will generate the scalar restart structure and added it at the
 * end of Epilogue Block. ndups represents the number of duplication of main 
 * loop needed. returns the ST index of the label
 */
{
   INT_BVI iv, ivtails;
   int srlb;
   BBLOCK *srblks;   /* will point the CFG of scalar restart */
   BLIST *ftheads;
   extern INT_BVI FKO_BVTMP;

/*
 * Generate codes for Scalar Restart
 * ----------------------------------
 * NOTE: right now, GenScalarResartPathCode() takes a paratemer to indicate 
 * the scalar restart index. It is not necessary now on, will change this later
 */
   iv = FKO_BVTMP;
   if (!FKO_BVTMP) 
      iv = FKO_BVTMP = NewBitVec(32);
   else
      SetVecAll(iv, 0);
   ivtails = BlockList2BitVec(lp->tails);
   ftheads = FindAllFallHeads(NULL, lp->blkvec, lp->header, ivtails, iv);
   ftheads = ReverseBlockList(ftheads);
   srblks = GenScalarRestartPathCode(lp, 0, ndups, ftheads, ivtails);   
#if 0
   fprintf(stderr, "Scalar Restart\n");
   PrintInst(stderr, srblks);
   exit(0);
#endif

/*
 * Place the Scalar Restart code at the end of Epilogue blk 
 * --------------------------------------------------------
 */
   AddScalarRestart(srblks);
#if 0
   fprintf(stderr, "From epilogue \n");
   PrintInst(stderr, srblks->up);
   exit(0);
#endif
   assert(srblks->ainst1->inst[0] == LABEL);
   srlb = srblks->ainst1->inst[1];
   return srlb;
}

void RepairLoopPaths(int srlb, LOOPQ* lp)
/*
 * this function will update the loop structure by deleting all the blks of 
 * scalar paths (which is not common with vector path) and will update all 
 * the destination of vector path.
 * NOTE: make sure that after calling this function, lp->blocks will not use
 * anywhere.
 */
{
   BBLOCK *bp;
   BLIST *bl, *delblks, *pbl;
   INSTQ *ip;

   delblks = NULL;

#if 0   
   for (i=0; i < NPATH; i++)
   {
      if (i == VPATH) /* vector path*/
      {
/*
 *       update all destination of braches to the scalar restart
 *       LOOP control is deleted. so, conditional br need to fwd to sr
 */
         for (bl = PATHS[i]->blocks; bl; bl=bl->next)
         {
            bp = bl->blk;
            for (ip=bp->ainst1; ip; ip=ip->next)
            {
               if (IS_COND_BRANCH(ip->inst[0]))
               {
                  ip->inst[3] = srlb;  /* jump to scalar restart */
               }
            }
         }
      }
      else /* scalar path */
      {
/*
 *       update dellist: add the blk which is not common to vector path
 */
         for (bl = PATHS[i]->blocks; bl; bl=bl->next)
         {
            bp = bl->blk;
            if (!FindBlockInList(PATHS[VPATH]->blocks, bp))
            {
/*
 *             if it is not already inserted in delblk lists
 */
               if (!FindBlockInList(delblks, bp))
                  delblks = AddBlockToList(delblks, bp);
            }
         }
      }
   }
#else
/*
 * Update all destination of branches to the scalar restart in vpath    
 */
   for (bl=PATHS[VPATH]->blocks; bl; bl=bl->next)
   {
      bp = bl->blk;
      for (ip=bp->ainst1; ip; ip=ip->next)
      {   
         if (IS_COND_BRANCH(ip->inst[0]))
         {
            ip->inst[3] = srlb;  /* jump to scalar restart */
         }
      }
   }
/*
 * all loop blocks which are not part of vpath need to be deleted
 */
   for (bl=lp->blocks; bl; bl=bl->next)
   {
      bp = bl->blk;
      if (!FindBlockInList(PATHS[VPATH]->blocks,bp))
      {
/*
 *       if it is not already inserted in delblk lists
 */
         if (!FindBlockInList(delblks, bp))
            delblks = AddBlockToList(delblks, bp);
      }
   }
#endif

/*
 * kill blks from delblks list
 * NOTE: need to make a complete function in flow.c to completely delete a blk
 */
#if 0
   fprintf(stderr, "delblks = %s\n", PrintBlockList(delblks));
#endif   
   for (bl=delblks; bl; bl=bl->next)
   {
      bp = bl->blk;
#if 0
      fprintf(stderr, "\nbp=%d : up=%d, down=%d, usucc=%d, csucc=%d\n",bp->bnum,
              bp->up?bp->up->bnum:-1, bp->down?bp->down->bnum:-1 ,
              bp->usucc?bp->usucc->bnum:-1, bp->csucc?bp->csucc->bnum:-1 );
#endif      
#if 1
/*
 *    We always deals with intermediate blk (loop blk), not the first blk
 */
      assert(bp->up);
/*
 *    remove the blk from CFG
 */
      if (bp->up) 
      {
         bp->up->down = bp->down;
         if (bp->down)
            bp->down->up = bp->up;
      }
/*
 *    If this block usucc of up blk, change it to down
 */
      if (bp->up && bp->up->usucc == bp)
      {
         assert(GET_INST(bp->up->ainstN->inst[0] != JMP));
         bp->up->usucc = bp->down;
      }
/*
 *    with multiple paths, down blk can be the csucc when this blk doesn't
 *    belong to fall-thru path. usucc of this blk may be part of other path,
 *    hence placed before.
 */
      else if (bp->up && bp->up->csucc == bp)
      {
/*
 *       make the csucc NULL, semantic is incorrect. but it may be intermediate
 *       steps and we will check the flow after xfrom anyway
 */
         bp->up->csucc = NULL; 
      }
/*
 *    FIXED: Still this block may be a csucc/usucc of other blks. 
 *    check the preds of this blk. Make the csucc/usucc NULL to avoid dangling
 *    ptr.
 */
      for (pbl = bp->preds; pbl; pbl=pbl->next)
      {
         if (pbl->blk->csucc == bp)
            pbl->blk->csucc = NULL;
         else if (pbl->blk->usucc == bp)
            pbl->blk->usucc = NULL;
      }
/*
 *    Update preds list of other blks
 */
      if (bp->usucc)
         bp->usucc->preds = RemoveBlockFromList(bp->usucc->preds, bp);
      if (bp->csucc)
         bp->csucc->preds = RemoveBlockFromList(bp->csucc->preds, bp);
/*
 *    NOTE: need to update other global data structure like: optloop->tails,
 *    optloop->blocks!!!
 */
/*
 *    now, we can delete the blk
 */
#if 0
      fprintf(stderr, "Blk deleted = %d\n", bp->bnum);
#endif
      KillAllInst(bp->inst1);
      KillBlockList(bp->preds);
      free(bp);
#else
      DelBlock(bp); /* not updated yet */
#endif
   }
   KillBlockList(delblks);
}

int SpecSIMDLoop(int SB_UR)
{
   int unroll;    /* stronger bet unroll */
   LOOPQ *lp;
   extern int path;
   lp = optloop;
   int lbSclRes;
   /*INSTQ *ippu;*/
/*
 * see our accepted paper in PACT'13 for steps
 */

/*==========================================================================
 * Step 1: Speculated path formulation
 *==========================================================================*/
/*
 * NOTE: this is done by calling Fall through transformation at the begining
 * of main function. After this transformation fall-through path should always
 * be VPATH (speculated path, spath in paper). 
 * if fall-thru is not applied and VPATH is not fall-thru vectorization can't
 * be applied
 */
   if (path == -1)
   {
      if (VPATH)
      {
         fprintf(stderr,"\nDefault path is not vectorizable, specify path(-p)\n");
         assert(!VPATH);
      }
   }
#if 0
   PrintLoop(stderr, lp);
#endif

/*=============================================================================
 * Step 2 : Vectorization alignment and cleanup
 *============================================================================*/

   KillLoopControl(lp);
/*
 * NOTE: Loop peeling must be called before cleanup, as it may chnage the loop
 * control structure of main loop, hence the cleanup loop. 
 * all the loop control should be killed before loop peeling or, cleanup loop.
 */
   /*isSSV = 1;*/
#if 1   
   if (IsSIMDalignLoopPeelable(lp))
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

/*=============================================================================
 * Step 3: Scalar Restart Generation
 *============================================================================*/
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
   exit(0);
#endif
  
   unroll = (SB_UR < 2)? 1: SB_UR;
   lbSclRes = ScalarRestart(lp, Type2Vlen(lp->vflag)*unroll);
/*
 * Consider only vector path now. Need to add some analysis for various vars
 * to implemnt backup stages.
 */
#if 0 
   fprintf(stdout, " LIL AFTER SCALAR RESTART LOOP \n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif

/*=============================================================================
 * Step 4: Branch repair and non-speculated block removal 
 *============================================================================*/
   
   RepairLoopPaths(lbSclRes, lp);
   

/*=============================================================================
 * Step 5: path vectorization 
 *============================================================================*/
#if 0   
   SpeculativeVecTransform(lp);  
#else
   SpecVecXform(lp, unroll);
#endif
/*
 * It's time to kill the data structure for paths. 
 * If path info is needed later, skip this deletion.
 */
   KillPathTable();

#if 0
   fprintf(stdout, " LIL AFTER SSV LOOP \n");
   PrintInst(stdout, bbbase);
   //fprintf(stdout, " SYMBOL TABLE \n");
   //PrintST(stdout);
   exit(0);
#endif

#if 0 
/*
 * Everything is messed up already. So, re-make the control flow and check it 
 */
   UnrollCleanup(lp,1);
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
   
   fprintf(stdout, " LIL NEW CFG \n");
   PrintInst(stdout, bbbase);
   ShowFlow("cfg.dot",bbbase);
   exit(0);
#endif

#if 0
/*
 * Update prefetch information
 */
/*
 * Update prefetch information
 * NOTE: according to new program states, it will be updated in STATE 3.
 * So, we no longer need this here.
 */
   UpdateNamedLoopInfo();
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


#if 0
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
#endif


#if 0
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
/*
 * Need to kill/free the scal!!! shouldn't we? check details. 
 */
   UpdateNamedLoopInfo();

}
#endif

/*=============================================================================
 * Functions for new program flow. 
 * Many of the functions of this file will be obsolete after the completion 
 * of this section.
 *============================================================================*/
#if 0
/*
 * NOT USED ANYMORE 
 */
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
#endif
/*
 * Normal vectorization with single path
 */
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

/*=============================================================================
 *                            RC VECTORIZATION 
 *                            ----------------
 * NOTE: what's new: special support for integer 
 *============================================================================*/ 


/*
 * Another vector analyzer for the scalar after RC applied.
 * Right now, we assumed there would be single path after RC applied.
 */
int RcPathVectorAnalysis(LOOPPATH *path)
/*
 * Analyze single path of loop and stores all the info in paths data structure,
 * returns error code if failed, otherwise 0.
 * NOTE: it doesn't change the original code but assume loopcontrol is killed.
 * NOTE: don't stop the analysis when error is found, rather complete all
 * the analysis, save the data structure and return error code at last
 */
{
   extern short STderef;
   extern INT_BVI FKO_BVTMP;
   extern int VECT_FLAG;

   int errcode;
   INT_BVI iv, iv1, blkvec;
   int i, j, k, n, N;
   int vflag;
   LOOPQ *lp;
   BLIST *scope, *bl, *blTmp;
   INSTQ *ip;
   struct ptrinfo *pbase, *p;
   ILIST *il;
   /*ILIST *il, *ib;*/
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
   lpflag = 0;
   lp = optloop;
   assert(path);
   scope = path->blocks;
   assert(scope);
/*
 * Find variable accessed in the path and store it in path
 */
   /*iv = NewBitVec(32);*/
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);

   if (!CFUSETU2D )
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#if 0
   fprintf(stderr, "[p:%d] p->uses=%d, p->defs=%d\n", path->pnum, 
           path->uses, path->defs);
#endif   
   for (bl=scope; bl; bl=bl->next)
   {
#if 0
      fprintf(stderr, "[b:%d] b->uses=%d, b->defs=%d\n", bl->blk->bnum, 
              bl->blk->uses, bl->blk->defs);
#endif      
      path->uses = BitVecComb(path->uses, path->uses, bl->blk->uses, '|');
      path->defs = BitVecComb(path->defs, path->defs, bl->blk->defs, '|');
   }
   
   iv = BitVecComb(iv, path->uses, path->defs, '|');
/*
 * right now, skip all the regs from uses, defs
 * NOTE: Need to check why there are redundant vars with diff ST index 
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);

#if 0
   fprintf(stderr,"Vars of path %d \n", path->pnum);
   PrintVars(stderr, "ALL VARS",iv);
#endif

/*
 * Skip all non-fp variables, valid entries upto n (included) 
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
#if 0
      fprintf(stderr,
            " variable = %s \n",
                     STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
#endif
      if (IS_FP(STflag[sp[i]-1]))
      {
         sp[n++] = sp[i];
      }
/*
 *    For non-fp var, if it's not the index var, avoid vectorization
 */
      else if (sp[i] != lp->I && sp[i] != lp->end &&
               sp[i] != lp->inc && sp[i] != lp->beg)
      {
/*
 *       NOTE: need to support integer scalars too. like: fp, we will support 
 *       ACC/MUL. special case: assigned by lp->I or, lp->end, lp->inc. But 
 *       of course after RC.
 *       Right now, we will only support special case... 
 *       Example: 
 *       --------
 *                imax_1 = N - i; // imax_1 = i;
 *                imask_1 = CVT(mask_1);
 *                imax = select(imax, imax_1, imask_1);
 *
 *       NOTE: we don't need to vectorize index variable of any DT entry
 *       This is scoped out lated  
 */
         sp[n++] = sp[i];
         /*fko_warn(__LINE__, "NON FP Scalar: %d,%s\n",
                  sp[i],STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");*/
      }
/*
 *    set of index variable [need to check by condtion? ]
 *    NOTE: need to check the index var variable whether it is set outside
 *    of the loop control. Make sure loop control is killed before.
 */
      else /* lp->I or, lp->end, lp->inc, lp->beg */ 
      {
         if (CheckVarInBitvec(sp[i]-1, path->defs))
         {
            fko_warn(__LINE__,
                     "Index variable = %s is updated outside loop control\n",
                     STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
            lpflag &= ~(LP_OPT_LCONTROL);
/*
 *          NOTE: if loop control is updated inside loop, loop is not countable.
 *          we can't vectorize loop in that case. 
 */
            errcode += 1;
         }
         else if (sp[i] == lp->I)
         {
/*
 *          NOTE: use of index variable to access memory (like: A[i]) prohibits
 *          current vectorization method. It can easily be vectorized after 
 *          adding an additional transformation stage which would transform 
 *          those access into A[0] / *A and update the pointer by A + inc.
 *          Not implemented yet!!!
 */
            if ( ( il=FindIndexRefInArray(scope, SToff[lp->I-1].sa[2]) ) )
            {
               fko_warn(__LINE__, "Use of index variable to access array");
               errcode += 1;
               KillIlist(il);
            }
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
   i = 0;
   j = FLAG2TYPE(STflag[sp[i]-1]); /* this is element, not element-count now*/
/*
 * skip INT type here.
 */
   while (!IS_FP(j) && i < n)
   {
      i++;
      j = FLAG2TYPE(STflag[sp[i]-1]); /* this is element, not element-count now*/
   }
   for (; i < n; i++)
   {
      k = FLAG2TYPE(STflag[sp[i]-1]);
      if (k != j && !IS_INT(k)) /* permit INT type */
      {
         fko_warn(__LINE__,
               "Mixed floating point type %d(%s), %d(%s) prevents " 
               "vectorization!\n\n",
               j, STname[sp[0]-1] ? STname[sp[0]-1] : "NULL",
               FLAG2TYPE(sp[i]-1), STname[sp[i]-1] ? STname[sp[i]-1]
               : "NULL");
         errcode += 4;
         break;
      }
   }
/*
 * Stored the required type for vectorization, may need later
 * NOTE: we will support vectorization with multiple types... just need to 
 * keep same type in a single instruction (except conversion instruction). 
 * Just kept as it is for now.
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
 * FIXED: number of vscal is not correct always: corrected the condition
 * Source of the problem: 
 *    here, var starts from 0 index for sp but for s, it starts form 1!!!
 */
#if 0
   for (i=0; i <= n; i++)
   {
      fprintf(stderr, "sp = %s(%d), s = %d\n",STname[sp[i]-1],i, s[i]);
   }
#endif
   for (k=0,i=0; i < n; i++) /* i,k is for sp which starts from 0 index  */
   {
      for (j=1; j <= N && s[j] != sp[i]; j++); /* s[0] has the count */
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
/*
 *       Majedul: Re-writen the analysis for livein/liveout var to be acc/mul
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
 *          b) Not set/ only used: used for MUL/DIV.... 
 *             ADD? skip right now
 *          NOTE: if it is only used, ADD should not create any problem!
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
/*
 *          Line in var which is set and used in vector path 
 */
            if ( (scf[i] & SC_SET) && (scf[i] & SC_USE) ) /*mustbe used as acc*/
            {
               if (j == VS_ACC)
               {
                  s[i] |= VS_ACC;
                  scf[i] |= SC_ACC;
                  /*vsoflag[i+1] |= VS_ACC;*/
               }
               else if (j == VS_SELECT) /* recognizing CMOV operation */
               {
                  s[i] |= VS_SELECT;
                  scf[i] |= SC_SELECT;
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
                          "LIVE IN: var %d(%s) must be Accumulator !!\n\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode += 64;
                  scf[i] |= SC_MIXED;
                  /*vsoflag[i+1] &= ~(VS_ACC | VS_MAX);*/
               }
            }
/*
 *          Live in var which is only used
 *          NOTE: only used variable should be vectorizable
 */
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
#if 0
                  fko_warn(__LINE__,
                           "LIVE IN: Mixed use of var %d(%s) "
                           "prevents vectorization!!\n",
                           sp[i], STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode += 128;
                  scf[i] |= SC_MIXED;
#else
/*
 *                Only used variable can be treated like the VS_MUL
 */
                  s[i] |= VS_MUL;
                  scf[i] |= SC_MUL;
#endif
               }
            }
         }
/*
 *       Live out variables
 */
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
               else if (s[i] & VS_SELECT)
                  vsoflag[i+1] |= VS_SELECT;
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
/*
 *                HERE HERE
 *                NOTE: for speculative vectorization, a global max var may
 *                not be considered as max at all!
 */
                  fko_warn(__LINE__, "Liveout var %d(%s) must be Accumulator/"
                           "max(nonSV)/select(RC) when set!\n", sp[i], 
                           STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
                  errcode +=256;
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
                  errcode +=512;
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
      lpflag &= ~LP_VEC; /* look here, don't confuse ~ with ! */
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
   fprintf(stderr, "RC VECTOR ANALYSIS\n");
   fprintf(stderr, "====================\n");
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
      if (path->sflag[i] & SC_MAX) fprintf(stderr, "MAX ");
      if (path->sflag[i] & SC_SELECT) fprintf(stderr, "SELECT ");
      if (path->sflag[i] & SC_ACC && path->sflag[i] & SC_LIVEIN) 
         fprintf(stderr, "IN_ACC ");
      if ((path->sflag[i] & SC_ACC) && (path->sflag[i] & SC_LIVEOUT)) 
         fprintf(stderr, "OUT_ACC ");
      if ((path->sflag[i] & SC_MUL) && (path->sflag[i] & SC_LIVEIN)) 
         fprintf(stderr, "IN_MUL ");
      if ((path->sflag[i] & SC_MUL) && (path->sflag[i] & SC_LIVEOUT)) 
         fprintf(stderr, "OUT_MUL ");
      if ((path->sflag[i] & SC_MAX) && (path->sflag[i] & SC_LIVEOUT)) 
         fprintf(stderr, "OUT_MAX ");
      if ((path->sflag[i] & SC_MIN) && (path->sflag[i] & SC_LIVEOUT)) 
         fprintf(stderr, "OUT_MIN ");
      fprintf(stderr, "\n");
   }
   fprintf(stderr, "ERR CODE: %d\n",errcode);
   fprintf(stderr, "====================\n");
#endif 
return errcode;
}

int IsOnlyUseAsDTindex(short var, short flag, BLIST *scope)
{
   int ret;
   short op, fl;
   INSTQ *ip, *ipDT;
   BLIST *bl;
   BBLOCK *bp;

   ret = 0;
#if 0   
   fprintf(stderr, "%s [%d] : ", 
                    STname[var-1], flag);
            if (flag & SC_SET) fprintf(stderr, "SET ");
            if (flag & SC_USE) fprintf(stderr, "USE ");
            if (flag & SC_MAX) fprintf(stderr, "MAX ");
            if (flag & SC_SELECT) fprintf(stderr, "SELECT ");
            fprintf(stderr, "\n");
   
#endif
/*
 * if the integer variable is set, it can't be skipped as DT index
 */
   if (flag & SC_SET)
      return (0);
/*
 * Analyze the instructions, check whether it is only used as DT index 
 * variable
 */
   for (bl=scope; bl; bl=bl->next)
   {
      bp = bl->blk;
      for (ip=bp->ainst1; ip; ip=ip->next)
      {
         if (IS_LOAD(ip->inst[0]) && STpts2[ip->inst[2]-1] == var )
         {
            /*PrintThisInst(stderr, ip);*/
            ipDT = ip->next;
            while (IS_LOAD(ipDT->inst[0]))
            {
               if (ipDT->inst[0] == FLD || ipDT->inst[0] == FLDD) 
               {
                  op = ipDT->inst[2];
                  fl = STflag[op-1];
                  if (IS_DEREF(fl) && !IS_LOCAL(fl))
                  {
/*
 *                   FIXME: there should be a way to check the DT entry 
 *                   whether it uses the var as index
 */
                     /*fprintf(stderr, "DT: %d [%d, %d, %d, %d]\n", op, 
                             SToff[op-1].sa[0], SToff[op-1].sa[1],
                             SToff[op-1].sa[2], SToff[op-1].sa[3] );*/
                     ret = 1;
                  }

               }
               ipDT=ipDT->next;
            }
/*
 *          if the variable is used in any arithmatic operation, but not just
 *          load of any float/double, it doesn't meet the constrain
 */
            if (!IS_STORE(ipDT->inst[0]))
               return(0);
            /*else
               ret = 1;*/
         }
      }
   }

   return ret;
}

int isUpByInduction(LOOPQ *lp, BLIST *scope, short var)
{
   BLIST *bl;
   INSTQ *ip, *ip0;
   
   //check =1;
   for (bl = scope; bl; bl = bl->next)
   {
      for (ip = bl->blk->ainst1; ip; ip = ip->next)
      {
         if (IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]-1] == var)
         {
            //PrintThisInst(stderr, 0, ip);
            ip0 = ip->prev;
            while(!IS_STORE(ip0->inst[0]) && !IS_BRANCH(ip0->inst[0]))
            {
/*
 *          FIXED: there can be a constant, like: imax = i+1. It still does
 *          satisfy, but need to keep track of that. we need that in vector
 *          initialiation stage.
 */
#if 0
               assert(ip0);
               PrintThisInst(stderr,ip0);
#endif
               if (IS_LOAD(ip0->inst[0]))
               {
                  if (STpts2[ip0->inst[2]-1] != var)
                  {
/*
 *                   NOTE: to make it simple, I only consider imax = i + const 
 *                   case, don't support even imax = N - i;
 */
#if 0                  
                     if ( !IS_CONST(STflag[lp->beg-1]) 
                        && STpts2[ip0->inst[2]-1] == lp->beg 
                     || !IS_CONST(STflag[lp->end-1]) 
                        && STpts2[ip0->inst[2]-1] == lp->end 
                     || !IS_CONST(STflag[lp->I-1]) 
                        && STpts2[ip0->inst[2]-1] == lp->I )
#else
                     if ( STpts2[ip0->inst[2]-1] == lp->I) 

#endif
                     {
                        //fprintf(stderr, "INDUCTION!!!\n");
                        //check = 1
                     }
                     else
                     {
                        //fprintf(stderr, "NOT INDUCTION\n!!!");
                        //check = 0;
                        //break;
                        fko_warn(__LINE__, "Shadow VRC not possible: "
                                 "use of %s to update %s\n", 
                                 STname[STpts2[ip0->inst[2]-1]-1], STname[var-1]);
                     
                        return(0);
                     }
                  }
               }
               else if (ip0->inst[0]!=ADD && ip0->inst[0]!=SUB)
               {
                  fko_warn(__LINE__, "Shadow VRC not possible: "
                           "Use of instruction %s with shadow var\n", 
                            instmnem[ip0->inst[0]]);
                  return 0;
                     
               }
               ip0 = ip0->prev;
            }
         }
      }
   }
   return 1;
}

short FindReadVarCMOV(BLIST *scope, short var)
/*
 * provided a destination var of select operation and block list, this func 
 * returns other var which is used in select/cmov op
 */
{
   short reg, rdvar;
   INSTQ *ip, *ip0;
   BLIST *bl;

   rdvar = 0;  /* init with no var */
   for (bl = scope; bl; bl = bl->next)
   {
      for (ip = bl->blk->ainst1; ip; ip = ip->next)
      {
         if (ip->inst[0] == CMOV1 || ip->inst[0] == CMOV2)
         {
            assert(ip->inst[2] < 0); /* 2nd opn is reg*/
            reg = ip->inst[2];
            ip0 = ip->prev; /* check backward */
            while (IS_LOAD(ip0->inst[0]))
            {
               if (ip0->inst[1] == reg)
               {
                  rdvar = STpts2[ip0->inst[2]-1];
                  break;
               }
               ip0 = ip0->prev;
            }
         }
         if (rdvar) break;
      }
      if (rdvar) break;
   }
   return (rdvar);
}

int isShadowPossible(LOOPQ *lp)
/*
 * identify the shadow element (imax = i or N-i) and update lp->vvscal 
 * accordingly: Vimax_1, Vvlen. it will only work when we have max cmp like: 
 * x > amax
 */
{
/*
 * NOTE: we need to recognize pattern for shadowing...
 *       
 *       RESTRICTION: if there is a variable which is asigned with 
 *       index/induction variable and used in CMOV operation.. we can use 
 *       shadow trick. No other integer operation is supported inside loop 
 *       right now - (relax it later!)
 *
 *       CONDITION:
 *          1. an integer variable is live out (imax)
 *          2. an integer variable is assigned with index/induction varible (imax_1)
 *          3. first variable is updated with a CMOV operation using these two
 *             varaible ....
 *          4. No other integer operations!!! (without updating loop index)
 */
   int i,j,n;
   int isPos, count;
   short scal, rdvar;

   isPos = 1; count = 0;
   for (n=lp->vscal[0],i=0; i < n; i++)
   {
      scal = lp->vscal[i+1];
      if (IS_INT(STflag[scal-1]))
      {
         count++;
         if (lp->vsflag[i+1] & VS_LIVEOUT) /* var liveout ? */
         {
            /*fprintf(stderr, "Live Out scal = %s\n", STname[scal-1]);*/
            if (lp->vsflag[i+1] & VS_SELECT) /* updated in select/cmov inst ? */
            {
               /*fprintf(stderr, "SEL scal = %s\n", STname[scal-1]);*/
               rdvar = FindReadVarCMOV(lp->blocks, scal); /* find other src */
               /*assert(rdvar);*/
               /*fprintf(stderr, "RDVAR = %s\n", STname[rdvar-1]);*/
               if (!rdvar || !FindInShortList(lp->vscal[0], lp->vscal+1, rdvar))
               {
                  isPos = 0;
                  fko_warn(__LINE__, "Shadow VRC not possible: "
                           "Liveout Int must be updated using CMOV\n");
               }
               else /* if exist, must be updated by induction variable */
               {
                  if (!isUpByInduction(lp, lp->blocks, rdvar))
                  {
                     isPos = 0;
                     fko_warn(__LINE__, "Shadow VRC not possible: "
                              "Int scalar updated by other than index var\n");
                  }
                  else /* rdvar is our shadow variable */
                  {
                     j = FindInShortList(lp->vscal[0], lp->vscal+1, rdvar);
                     lp->vsflag[j] = lp->vsflag[j] | VS_SHADOW;
                  }
                  /*else fprintf(stderr, "possible by induction check!!\n");*/
               }
            }
         }
      }
   }
/*
 * expect exactly 2 int vars
 */
   if (count != 2)
   {
      fko_warn(__LINE__, "Shadow VRC not possible: "
               "must have two integers in RC \n");
      isPos = 0;
   }

   return(isPos);
}

/*
 * Need to make this Vector analysis general for all VRC
 */
int RcVectorAnalysis()
{
   int i, j, k, n;
   int *vpindx;
   int errcode, isIscal;
   char ln[512];
   short *sp;
   LOOPPATH *vpath;
   LOOPQ *lp;
   extern LOOPQ *optloop;
   extern BBLOCK *bbbase;
   extern int VECT_FLAG;


   lp = optloop;
   isIscal = 0; /* is there any INT scalar for vectorization */
/*
 * redo data flow analysis
 */
   CalcInsOuts(bbbase);
   CalcAllDeadVariables();
/*
 * find paths
 */
   FindLoopPaths(lp);
/*
 * NOTE: currently we only consider only one path for RC vectorization 
 */
   assert(NPATH==1);
/*
 * Kill loop control for vector analysis
 */
   KillLoopControl(lp);
   CalcInsOuts(bbbase);
   CalcAllDeadVariables();
/*
 * apply analysis
 */
   errcode = RcPathVectorAnalysis(PATHS[0]);
/*
 * if path is vectorizable, save analysis
 */
   if (!errcode) 
   {
/*
 *    Update optloop with the vector path 
 */
      VPATH = 0;
      vpath = PATHS[VPATH];
      n = vpath->varrs[0];
      lp->varrs = malloc(sizeof(short)*(n+1));
      assert(lp->varrs);
      for (i=0; i <=n; i++)
      {
         lp->varrs[i] = vpath->varrs[i];
      }
/*
 *    skip all integer variables which are used as DT index
 *    vpidx saves the vpath->vscal index of other variables 
 */
      vpindx = malloc(sizeof(int)*(vpath->vscal[0]+1));
      assert(vpindx);
      j=1;
      sp = vpath->vscal;
      for (i=1; i<=sp[0]; i++)
      {
         if (IS_INT(STflag[sp[i]-1]) && 
             IsOnlyUseAsDTindex(sp[i],vpath->sflag[i], lp->blocks))
         {
/*
 *          skip the variable, not need to vectorize it
 *          NOTE: vpindx saves the index of the list
 */
            /*fprintf(stderr, "USE only as DT index\n");*/

         }
         else
            vpindx[j++] = i;
      }
      n = j-1;
      vpindx[0]=n;
#if 0      
      fprintf(stderr, "n(%d)=%d\n",vpath->vscal[0],n);
      for (i=0; i<=n; i++)
         fprintf(stderr, " %d ", vpindx[i]);
      fprintf(stderr, "\n");
      exit(0);
#endif
      lp->vscal = malloc(sizeof(short)*(n+1));
      lp->vvscal = malloc(sizeof(short)*(n+1));
      lp->vsflag = malloc(sizeof(short)*(n+1));
      lp->vsoflag = malloc(sizeof(short)*(n+1));
      lp->vvinit = calloc((n+1), sizeof(short)); /* for special vector init  */
      assert(lp->vscal && lp->vvscal && lp->vsflag && lp->vsoflag && lp->vvinit);
/*
 *    set the count
 */
      lp->vscal[0] = lp->vvscal[0] = lp->vsflag[0] = lp->vsoflag[0] 
                   = lp->vvinit[0] = n;
      for (i=1; i <= n; i++)
      {
         lp->vscal[i] = vpath->vscal[vpindx[i]];
         lp->vsflag[i] = vpath->vsflag[vpindx[i]];
         lp->vsoflag[i] = vpath->vsoflag[vpindx[i]];
      }
   
#if 0
      n = vpath->vscal[0];
      lp->vscal = malloc(sizeof(short)*(n+1));
      lp->vvscal = malloc(sizeof(short)*(n+1));
      lp->vsflag = malloc(sizeof(short)*(n+1));
      lp->vsoflag = malloc(sizeof(short)*(n+1));
      lp->vvinit = calloc((n+1), sizeof(short)); /* for special vector init  */
      assert(lp->vscal && lp->vvscal && lp->vsflag && lp->vsoflag);
      for (i=0; i <= n; i++)
      {
         lp->vscal[i] = vpath->vscal[i];
         lp->vsflag[i] = vpath->vsflag[i];
         lp->vsoflag[i] = vpath->vsoflag[i];
      }
#endif 
/*
 * Create vector local for all vector scalars in loop
 */
      lp->vflag = vpath->vflag;
      lp->vvscal[0] = n;
      sp = lp->vscal + 1;
      for (i=0; i < n; i++)
      {
         sprintf(ln, "_V%d_%s", i, STname[sp[i]-1] ? STname[sp[i]-1] : "");
#if 0
         fprintf(stderr, "....%s->%s\n", STname[sp[i]-1], ln);
#endif
         if (IS_INT(STflag[sp[i]-1]))
         {
/*
 *          FIXME: we skip index variable. So, we will not get that here. use of
 *          index variable may complicated the vectorization itself. We only 
 *          support certain pattern for vectorization. use of index variable 
 *          even as DT index prohibits vectorization now, which is caught later
 */
            k = T_VINT;
            isIscal = 1;
         }
         else  /* mixed fp is not allowed. */
            k = FLAG2TYPE(vpath->vflag);
         lp->vvscal[i+1] = InsertNewLocal(ln,k);
      }
   }
/*
 * check whether integer scalar meets the required constrain for shadow VRC
 * AVX2 is required for shadow VRC
 */
#if defined(AVX2)
   if (isIscal) 
   {
      if (isShadowPossible(lp))
      {
         VECT_FLAG |= VECT_SHADOW_VRC;  /* shadow vrc will be applied */
         fko_warn(__LINE__, "Shadow RC possible!\n");
      }
      else
         errcode = 1024; /* code for unmanagable Int Scalar */
   }
/*#elif defined(AVX)*/
#else
   if (isIscal)
   {
      errcode = 1024;
   }
#endif
/*
 * return error code 
 */
   return(errcode);
}

void AddCodeAdjustment(LOOPQ *lp)
/*
 * in this function, we will change scalar code so that our vectorizer can
 * vectorize it directly. scalar code after the adjustment will not produce 
 * correct result. so, use it directly inside the vectorizer.
 */
{
   int i,n,count;
   int initconst;
   INSTQ *ip, *ip0;
   BLIST *bl;
   short sivlen;
   short svar; /* shadow int var */
   short reg0, reg1;
   short *vscal, *vvscal, *vsflag, *vsoflag, *vvinit;
   char ln[512];
   /*short vlen;, dt*/
/*
 * checking for shadow variable, there should be only one such variable
 */
   count = 0;
   for (i=0, n=lp->vscal[0]; i < n; i++)
   {
      if (lp->vsflag[i+1] & VS_SHADOW)
      {
         svar = lp->vscal[i+1];
         count++;
      }
   }
#if 0
   fprintf(stderr, "shadow variable = %s, count = %d\n", STname[svar-1], count);
#endif
   assert(count==1);
/*
 *    Format: 
 *    imax_1 = i
 *
 *    MASKTEST fcc0, mask
 *    imax = CMOV(imax, imax_1, fcc0)
 *
 *    converted:
 *    imax_1 = imax_1 + vlen; // add imax_1 AS LIVE _IN shadow & , init vlen
 *
 *    MASKTEST fcc0, mask
 *    imax = CMOV(imax, imax_1, fcc0)
 */
/*
 * insert new constant variable vlen with the appropriate vlen value  
 */
   sivlen = InsertNewLocal("_vlen", T_INT);
/*
 * findout shadow variable
 */
   initconst = 0;
   for (bl = lp->blocks; bl; bl = bl->next)
   {
      for (ip = bl->blk->inst1; ip; ip = ip->next)
      {
         if (IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]-1] == svar)
         {
/*
 *          delete the code backward until it hit another st/br assuming
 *          that there are no other update for this variable here
 */
            ip0 = ip->prev;
            while (!IS_STORE(ip0->inst[0]) && !IS_BRANCH(ip0->inst[0]) )
            {
               //PrintThisInst(stderr, 0, ip0); 
               if (ip0->inst[0] == ADD)
               {
                  if (IS_CONST(STflag[ip0->inst[3]-1]))
                  {
                     //PrintThisInst(stderr, 0, ip0);
                     //fprintf(stderr, "value=%d\n", SToff[ip0->inst[3]-1].i);
                     initconst = SToff[ip0->inst[3]-1].i;
                  }
                  else fko_error(__LINE__,"Can only be updated with const");
               }
               else if (ip0->inst[0] == SUB)
               {
                  if (IS_CONST(STflag[ip0->inst[3]-1]))
                  {
                     //PrintThisInst(stderr, 0, ip0);
                     //fprintf(stderr, "value=%d\n", -SToff[ip0->inst[3]-1].i);
                     initconst = -SToff[ip0->inst[3]-1].i; 
                  }
                  else fko_error(__LINE__,"Can only be updated with const");
               }
               ip0 = ip0->prev;
               DelInst(ip0->next);
            }
            ip = DelInst(ip0->next);
            //PrintThisInst(stderr, 0, ip); 
/*
 *          insert new instruction for this candidate
 *          imax_1 = imax_1 + vlen
 *          FIXME: need to support imax = i + const. need to keep track that. 
 */
            ip = ip->prev; /* point back and inst after this*/
            //PrintThisInst(stderr, 0, ip);
            reg0 = GetReg(T_INT);
            reg1 = GetReg(T_INT);
            //assert(FLAG2TYPE(STflag[svar-1]) & (T_INT | T_VINT));
                    //SToff[lp->vvscal[i+1]-1].sa[2], -r0, 0);
            ip = InsNewInst(bl->blk,ip, NULL, LD, -reg0, SToff[svar-1].sa[2], 0);
            ip = InsNewInst(bl->blk, ip, NULL, LD, -reg1, SToff[sivlen-1].sa[2],
                            0);
            ip = InsNewInst(bl->blk, ip, NULL,  ADD, -reg0, -reg0, -reg1);
            ip = InsNewInst(bl->blk, ip, NULL,  ST, SToff[svar-1].sa[2], -reg0,
                            0);
            GetReg(-1);
/*
 *          Now imax_1 is live in.. so, update the flag to VS_LIVEIN
 */
            i = FindInShortList(lp->vscal[0], lp->vscal+1, svar);
            lp->vsflag[i] = lp->vsflag[i] | VS_LIVEIN;
            lp->vvinit[i] = STiconstlookup(initconst);
/*
 *          ADD vlen as a int scal 
 */
#if 1            
            n = lp->vscal[0];
            vscal = lp->vscal;
            vvscal = lp->vvscal;
            vsflag = lp->vsflag;
            vsoflag = lp->vsoflag;
            vvinit = lp->vvinit;
            
            lp->vscal = calloc(n+2, sizeof(short));
            lp->vvscal = calloc(n+2, sizeof(short));
            lp->vsflag = calloc(n+2, sizeof(short));
            lp->vsoflag = calloc(n+2, sizeof(short));
            lp->vvinit = calloc(n+2, sizeof(short));
            assert(lp->vscal && lp->vvscal && lp->vsflag && lp->vsoflag);
            //fprintf(stderr, "%p -> %p\n\n", vscal, lp->vscal);
            lp->vscal[0] = n+1;
            lp->vvscal[0] = n+1;
            lp->vsflag[0] = n+1;
            lp->vsoflag[0] = n+1;
            lp->vvinit[0] = n+1;
            
            for (i=1; i < n+1; i++)
            {
               lp->vscal[i]=vscal[i];
               lp->vvscal[i]=vvscal[i];
               lp->vsflag[i]=vsflag[i];
               lp->vsoflag[i]=vsoflag[i];
               lp->vvinit[i]=vvinit[i];
            }
            lp->vscal[n+1] = sivlen;
            lp->vsflag[n+1] = VS_LIVEIN | VS_VLEN;
            sprintf(ln, "_V%d_%s", n+2, STname[sivlen-1] );
            //fprintf(stderr, "%s\n", ln);
            lp->vvscal[n+1] = InsertNewLocal(ln,T_VINT);
            
            free(vscal);
            free(vvscal);
            free(vsflag);
            free(vsoflag);
            free(vvinit);
#endif
         }
      }
   }
}

INSTQ *AddIntShadowPrologue(LOOPQ *lp, BBLOCK *bp0, INSTQ *iph, short scal, 
      int index)
/*
 * generate instructions for vector initialization for index
 */
{
/*
 * New Plan: 
 * ==========
 *          We will consider double and single implementation seperately. We 
 *          will need two set of vector-integer instruction instructionss 
 * Double: 
 *         For double, we consider integer as 64 bit inside vector. FKO 
 *         treats integer as 64 bit anyway. 
         
   Initialization:  
      Vimax_1 = [i-4, i-3, i-2, i-1] // as we update vimax before cmov
      Vimax = [imax, imax, imax, imax]
      Vvlen = [4, 4, 4, 4]
            
 * Single: 
 *       For single precision, we will consider integer as 32 bit. Since FKO 
 *       considers integer as 64 bit in X64, we will need some conversion. 
 *       We will use CVTSI at the epilogue of the vector to reduce the Vimax
 *       into imax.
       
   Initialization: (we need to use archsregs here ) 
      Vimax_1 = [i-8, i-7, i-6, i-5, i-4, i-3, i-2, i-1]
      Vimax = [imax, imax, imax, imax, imax, imax, imax, imax]
      Vvlen = [8, 8, 8, 8, 8, 8, 8, 8]
   FIXME: it will work only when 'imax = i'. If imax =i+1, we need to initialize
   Vimax_1 = [i+1-8, i+1-7,...] etc. we need to initialize based on the 
   initialization of imax.
 *
 */
   int vlen, sinit;
   short flag;
   short ireg, r1, r2 ;
   enum inst vst, vgr2vr, vshuf;
   /*enum inst vld;*/

   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
      /*vld = VLD;*/
      vst = VST;
      vshuf = VSSHUF;
      vgr2vr = VGR2VR32;
      vlen = 8;
   }
   else if (IS_DOUBLE(lp->vflag) || IS_VDOUBLE(lp->vflag))
   {
      /*vld = VLD;*/
      vst = VST;
      vshuf = VISHUF;
      vgr2vr = VGR2VR64;
      vlen = 4;
   }
   else
      fko_error(__LINE__,"Unsupported type for vectorization!\n");
  /*
   *  vector init based on variable type
   */
   
   flag = lp->vsflag[index];

#if 0         
         fprintf(stderr, "LIVEIN : %s : %d", 
                 STname[scal-1], flag);
         if (flag & VS_ACC) fprintf(stderr, " VS_ACC");
         if (flag & VS_MAX) fprintf(stderr, " VS_MAX");
         if (flag & VS_SHADOW) fprintf(stderr, " VS_SHADOW");
         if (flag & VS_SELECT) fprintf(stderr, " VS_SELECT");
         if (flag & VS_VLEN) fprintf(stderr, " VS_VLEN");
         fprintf(stderr, "\n");
#endif  

   ireg = GetReg(T_INT);
   r1 = GetReg(T_VINT);
   r2 = GetReg(T_VINT);

   if (flag & (VS_SELECT | VS_VLEN)) 
   {
      
      PrintComment(bp0, NULL, iph, 
                   "Init vector equiv of %s", STname[lp->vscal[index]-1]);
      if (flag & VS_VLEN)
      {
            InsNewInst(bp0, NULL, iph, MOV, -ireg, STiconstlookup(vlen), 0);
      }
      else
         InsNewInst(bp0, NULL, iph, LD, -ireg,
                    SToff[lp->vscal[index]-1].sa[2], 0);
      
      InsNewInst(bp0, NULL, iph, vgr2vr, -r1, -ireg, STiconstlookup(0));
      InsNewInst(bp0, NULL, iph, vshuf, -r1, -r1, STiconstlookup(0));
      InsNewInst(bp0, NULL, iph, vst, SToff[lp->vvscal[index]-1].sa[2], -r1, 0);

   }
   else if (flag & VS_SHADOW) /* shadow variable */
   {
/*
 *    FIXED: for double, replace vlen by 4, otherwise by 8 
 *          Vimax1 = [i-vlen, i-vlen, i-vlen+1, i-vlen+1, ... ... ]
 *    FIXME: need to update by vlen +/- const if imax = i +/- 1
 */
      PrintComment(bp0, NULL, iph, 
                   "Init vector equiv of %s", STname[lp->vscal[index]-1]);
      InsNewInst(bp0, NULL, iph, LD, -ireg, SToff[lp->I-1].sa[2], 0);
#ifdef AVX2
/*
 *    vpinsrw/q ==> VGR2VR32/64 only works on XMM register. So, we populate
 *    2 XMM register and combine them later in YMM
 */
/*
 *    populate the 1st XMM register
 */
      //fprintf(stderr, "const adjustment = %d\n", SToff[lp->vvinit[index]-1].i);
      sinit = vlen - SToff[lp->vvinit[index]-1].i;
      InsNewInst(bp0, NULL, iph, SUB, -ireg, -ireg, STiconstlookup(sinit));
      InsNewInst(bp0, NULL, iph, vgr2vr, -r1, -ireg, STiconstlookup(0));
      InsNewInst(bp0, NULL, iph, ADD, -ireg, -ireg, STiconstlookup(1));
      InsNewInst(bp0, NULL, iph, vgr2vr, -r1, -ireg, STiconstlookup(1));
      if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
      {
         InsNewInst(bp0, NULL, iph, ADD, -ireg, -ireg, STiconstlookup(1));
         InsNewInst(bp0, NULL, iph, vgr2vr, -r1, -ireg, STiconstlookup(2));
         InsNewInst(bp0, NULL, iph, ADD, -ireg, -ireg, STiconstlookup(1));
         InsNewInst(bp0, NULL, iph, vgr2vr, -r1, -ireg, STiconstlookup(3));
      }
/*
 *    populate the 2nd XMM register
 */     
      InsNewInst(bp0, NULL, iph, ADD, -ireg, -ireg, STiconstlookup(1));
      InsNewInst(bp0, NULL, iph, vgr2vr, -r2, -ireg, STiconstlookup(0));
      InsNewInst(bp0, NULL, iph, ADD, -ireg, -ireg, STiconstlookup(1));
      InsNewInst(bp0, NULL, iph, vgr2vr, -r2, -ireg, STiconstlookup(1));
      if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
      {
         InsNewInst(bp0, NULL, iph, ADD, -ireg, -ireg, STiconstlookup(1));
         InsNewInst(bp0, NULL, iph, vgr2vr, -r2, -ireg, STiconstlookup(2));
         InsNewInst(bp0, NULL, iph, ADD, -ireg, -ireg, STiconstlookup(1));
         InsNewInst(bp0, NULL, iph, vgr2vr, -r2, -ireg, STiconstlookup(3));
      }
/*
 *    combine the two XMM into YMM
 */
      if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
         InsNewInst(bp0, NULL, iph, VSSHUF, -r1, -r2, 
                    STiconstlookup(0xBA983210));
      else
         InsNewInst(bp0, NULL, iph, VISHUF, -r1, -r2, 
                    STiconstlookup(0x5410));
#else
      assert(0); // will implement later
#endif
      InsNewInst(bp0, NULL, iph, VST, SToff[lp->vvscal[index]-1].sa[2], -r1, 0);
   }
   else
      assert(0); /* no other integer operation is supported now */
   GetReg(-1);

   return iph;
}
INSTQ *AddIntShadowEpilogue(LOOPQ *lp, BBLOCK *bp0, INSTQ *iptp, INSTQ *iptn, 
      short scal, int index)
{
/*
 *    REDUCTION : live out imax
 *    =========================
 *    1. amax = HMAX(Vamax)  ==> based on max/min
 *    2. Vmask2 = Vamax == [amax, amax, amax,....]
 *    3. Vimax = CMOV(Vimax, VmaxInt, Vmask)  ==> based on amax .. max/min
 *    4. imax = HMIN(Vimax)  ===> based on PTR movement, index, condition..
 */
   int i, n;
   short flag;
   short mvar, vmvar, vmask; 
   short r0, r1, ireg, vireg0, vireg1,vireg2, type;
   enum inst mfinst, rminst, vld, vst, vsld, vshuf, vfcmpweq;
   enum inst vild, vist, vists, vgr2vr, vishuf, vicmov2, vimin, vimax;
   /*enum inst vicomv1, vilds, vsst;*/

   flag = lp->vsflag[index];
/*
 * select inst according to type
 */
   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
      vsld = VFLDS;
      /*vsst = VFSTS;*/
      vld = VFLD;
      vst = VFST;
      vshuf = VFSHUF;
      vfcmpweq = VFCMPWEQ;
      type = T_VFLOAT;
/*
 *    for float, we will consider 32 bit INT inside vector
 */
      vild = VLD;
      /*vilds = VSLDS;*/
      vist = VST;
      vists = VSSTS;
      vishuf = VSSHUF;
      vgr2vr = VGR2VR32;
      vimin = VSMIN;
      vimax = VSMAX;
      /*vicmov1 = VSCMOV1;*/
      vicmov2 = VSCMOV2;
   }
   else
   {
      vsld = VDLDS;
      /*vsst = VDSTS;*/
      vld = VDLD;
      vst = VDST;
      vshuf = VDSHUF;
      vfcmpweq = VDCMPWEQ;
      type = T_VDOUBLE;
/*
 *    for float, we will consider 32 bit INT inside vector
 */
      vild = VLD;
      /*vilds = VILDS;*/
      vist = VST;
      vists = VISTS;
      vishuf = VISHUF;
      vgr2vr = VGR2VR64;
      vimin = VIMIN;
      vimax = VIMAX;
      /*vicmov1 = VICMOV1;*/
      vicmov2 = VICMOV2;
   }

/*
 * find the max var
 */
   for (i=0, n=lp->vscal[0]; i < n; i++)
   {
      if (lp->vsflag[i+1] & VS_MAX)
      {
         mvar = lp->vscal[i+1];
         vmvar = lp->vvscal[i+1];
         if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
            mfinst = VFMAX;
         else
            mfinst = VDMAX;
      }
      else if (lp->vsflag[i+1] & VS_MIN)
      {
         mvar = lp->vscal[i+1];
         vmvar = lp->vvscal[i+1];
         if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
            mfinst = VFMIN;
         else
            mfinst = VDMIN;
      }
   }
#if 0   
   fprintf(stderr, "Max var = %s, Vmax = %s\n", 
         STname[mvar-1], STname[vmvar-1]);
#endif
   
   r0 = GetReg(FLAG2TYPE(lp->vflag));
   r1 = GetReg(FLAG2TYPE(lp->vflag));

   if (flag & VS_SELECT) /* only sel scal should be liveout */
   {
      iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                          "Reduce vector for %s", 
                           STname[scal-1]);
/*   step : 1  amax = HMAX(Vamax) --- floating point */
      if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag) ) 
      {
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFLD, -r0,
                                 SToff[vmvar-1].sa[2], 0); 
      #if defined(X86) && defined(AVX)
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
              -r1, -r0, STiconstlookup(0x7654FEDC));
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, mfinst,
              -r0,-r0,-r1);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
              -r1, -r0, STiconstlookup(0x765432BA));
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, mfinst,
              -r0,-r0,-r1);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
              -r1, -r0, STiconstlookup(0x76CD3289));
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, mfinst,
              -r0,-r0,-r1);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSTS,
              SToff[mvar-1].sa[2], -r0, 0);
      #else
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
              -r1, -r0, STiconstlookup(0x3276));
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, mfinst,
              -r0,-r0,-r1);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSHUF,
              -r1, -r0, STiconstlookup(0x5555));
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, mfinst,
              -r0,-r0,-r1);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VFSTS,
              SToff[mvar-1].sa[2], -r0, 0);
      #endif
      }
      else 
      {
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDLD, -r0,
                                 SToff[vmvar-1].sa[2], 0); 
      #if defined(X86) && defined(AVX)
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1,
              -r0, STiconstlookup(0x3276));
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, mfinst,-r0,-r0,
              -r1);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1,
              -r0, STiconstlookup(0x3715));
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, mfinst,-r0,-r0,
              -r1);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSTS,
              SToff[mvar-1].sa[2], -r0, 0);
      #else
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSHUF, -r1,
              -r0, STiconstlookup(0x33));
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, mfinst,-r0,-r0,
              -r1);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VDSTS,
              SToff[mvar-1].sa[2], -r0, 0);
      #endif
      }
      //GetReg(-1);
/*    step : 2   Vmask2 = Vamax == [amax, amax, amax,....] ---floating point */
/*    3. Vimax = CMOV(Vimax, VmaxInt, Vmask2)  ==> based on amax ..max/min -vint
 */
      vmask = InsertNewLocal("_r_vmask", type);
      iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vsld, -r0,
                        SToff[mvar-1].sa[2], 0);
      iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vld, -r1,
                        SToff[vmvar-1].sa[2], 0); 
      iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vshuf, -r0, 
                        -r0, STiconstlookup(0));     
      iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vfcmpweq, -r0, -r0,-r1);  
      iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vst,
                        SToff[vmask-1].sa[2], -r0, 0);

/*
 *    generating VmaxInt ... Normally compiler generates this by storing the 
 *    value into stack!!!
 *    NOTE: here we need to use 
 *             VCMOV2: dest = (mask==0)? src: dest
 *    FIXME: consider both max and min 
 */
      ireg = GetReg(T_INT);
      vireg0 = GetReg(T_VINT); /* not used but keep space for r0*/
      vireg1 = GetReg(T_VINT);
      vireg2 = GetReg(T_VINT);
#if 0
      fprintf(stderr, "r0=%d, r1=%d, ireg=%d, vireg0=%d, vireg1=%d, vreg2=%d\n",
              r0, r1, ireg, vireg0, vireg1, vireg2);
#endif
      iptp = InsNewInst(bp0, iptp, NULL, vld, -r0, SToff[vmask-1].sa[2], 0);
      iptp = InsNewInst(bp0, iptp, NULL, vild, -vireg1, 
                        SToff[lp->vvscal[index]-1].sa[2], 0);
/*
 *    HERE HERE, should we change the constatnt for 64 bit int???
 */
      iptp = InsNewInst(bp0, iptp, NULL, MOV, -ireg, 
                        STiconstlookup(0x7FFFFFFF), 0);

      iptp = InsNewInst(bp0, iptp, NULL, vgr2vr, -vireg2, -ireg, 
                        STiconstlookup(0));
      iptp = InsNewInst(bp0, iptp, NULL, vishuf, -vireg2, -vireg2, 
                        STiconstlookup(0));
      iptp = InsNewInst(bp0, iptp, NULL, vicmov2, -vireg1, -vireg2, -r0); 
      iptp = InsNewInst(bp0, iptp, NULL, vist, SToff[lp->vvscal[index]-1].sa[2], 
                 -vireg1, 0);
/*
 *    step 4: imax = HMIN(Vimax) ... vint 
 *    FIXME: need to check the condition... whether it is > or >=
 */
      if (mfinst == VFMAX || mfinst == VDMAX)
      {
         rminst = vimin;
      }
      else
      {
         rminst = vimax;
      }
      iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VLD, -vireg1,
                                 SToff[lp->vvscal[index]-1].sa[2], 0); 
   #if defined(X86) && defined(AVX2)
   
      if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
      {      
/*
 *    NOTE: 
 *    Here is the actual shuffle for reduction (X means don't care):
 *          0x XXXXFEDC
 *          0x XXXXXXBA
 *          0x XXXXXXX9
 *    To implement that in AVX easily, I choose following sequence:
 *          0x 7654 FEDC
 *          0x 7654 BABA
 *          0x 7654 BA99
 */
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vishuf,
                 -vireg2, -vireg1, STiconstlookup(0x7654FEDC));
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, rminst,
                 -vireg1,-vireg1,-vireg2);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vishuf,
                 -vireg2, -vireg1, STiconstlookup(0x7654BABA)); //0x 7654 32BA
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, rminst,
                 -vireg1,-vireg1,-vireg2);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vishuf,
                 -vireg2, -vireg1, STiconstlookup(0x7654BA99)); // 0x 76CD 3289
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, rminst,
                 -vireg1,-vireg1,-vireg2);
/*
 *    FIXED: all integer vars/regs are 64 bit in FKO. But we treated the 
 *    vector integer 8x32 bit. So, before storing, we need to upgarde the single
 *    element into 64 bit
 */
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VSMOVS,
                 -ireg,-vireg1,0);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, CVTSI,
                 -ireg,-ireg,0);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, ST,
                 SToff[lp->vscal[index]-1].sa[2], -ireg, 0);
      /*iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VSTS,
              SToff[lp->vscal[index]-1].sa[2], -vireg1, 0);*/
      }
      else /* double */
      {
/*
 *    NOTE: 
 *    Here is the actual shuffle for reduction (X means don't care):
 *          0x XX76
 *          0x XXX5
 *    To implement that in AVX easily, I choose following sequence:
 *          0x 7676
 *          0x 7655
 *    FIXED: we don't have max/min instruction for 64 bit int, not even in
 *    avx2. so, we have to simulate that using vcmp and cmov instruction!!
 *    don't find VICMPWLT... using VICMPGT.
 */
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vishuf,
                 -vireg2, -vireg1, STiconstlookup(0x7676));
         
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VICMPWGT,
                 -vireg0, -vireg1, -vireg2);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VICMOV1,
                 -vireg1, -vireg2, -vireg0);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vishuf,
                 -vireg2, -vireg1, STiconstlookup(0x7655)); 
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VICMPWGT,
                 -vireg0, -vireg1, -vireg2);
         iptp = InsNewInst(lp->posttails->blk, iptp, NULL, VICMOV1,
                 -vireg1, -vireg2, -vireg0);
      iptp = InsNewInst(lp->posttails->blk, iptp, NULL, vists,
              SToff[lp->vscal[index]-1].sa[2], -vireg1, 0);         
      }
   #else
/*
 *    will need AVX2 anyway
 */
      fko_error(__LINE__, "need avx2 for int vector arithmatic!!!\n");
   #endif
       

   }
   else assert(0);
   GetReg(-1);
   return iptp;
}

void AddVectorPrologueEpilogue(LOOPQ *lp)
{
   int i, j, k, n;
   INSTQ *iph, *iptp, *iptn;
   short r0, r1;
   enum inst inst;
   enum inst vld, vst, vsld, vsst, vshuf;
   /*int vlen;*/
   
/*
 * Figure out what type of insts to translate
 */
   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
#if 0       
      #if defined(X86) && defined(AVX)
         vlen = 8;
      #else
         vlen = 4;
      #endif
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
      vld = VDLD;
#if 0     
      #if defined(X86) && defined(AVX)
         vlen = 4;
      #else
         vlen = 2;
      #endif
#endif
      vst = VDST;
   }
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
#if 0
      fprintf(stderr, "%s->%s \t Type=%d\n", STname[lp->vscal[i+1]-1], 
              STname[lp->vvscal[i+1]-1], STflag[lp->vscal[i+1]-1]);
      if (IS_DOUBLE(STflag[lp->vscal[i+1]-1]))
         fprintf(stderr, "%s --> double\n", STname[lp->vscal[i+1]-1]);
#endif
      if (VS_LIVEIN & lp->vsflag[i+1])
      {
/*
 *       ADD-updated vars set v[0] = scalar, v[1:N] = 0
 */
#if 0    
         int flag;
         flag = lp->vsflag[i+1];
         fprintf(stderr, "LIVEIN : %s : %d", 
                 STname[lp->vscal[i+1]-1], flag);
         if (flag & VS_ACC) fprintf(stderr, " VS_ACC");
         if (flag & VS_MAX) fprintf(stderr, " VS_MAX");
         if (flag & VS_SHADOW) fprintf(stderr, " VS_SHADOW");
         if (flag & VS_SELECT) fprintf(stderr, " VS_SELECT");
         if (flag & VS_VLEN) fprintf(stderr, " VS_VLEN");
         fprintf(stderr, "\n");
#endif  
         if (IS_INT(FLAG2TYPE(STflag[lp->vscal[i+1]-1]))) /* special case */ 
         {
            iph = AddIntShadowPrologue(lp, lp->preheader, iph, lp->vscal[i+1], i+1);
         }
         else 
         {
            r0 = GetReg(FLAG2TYPE(lp->vflag));
            r1 = GetReg(FLAG2TYPE(lp->vflag));
/*
 *          Majedul: FIXME: for accumulator init, shouldn't we need an Xor
 *          though most of the time it works as vmoss/vmosd automatically
 *          zerod the upper element. but what if the optimization transforms it
 *          into reg-reg move. vmovss/vmosd for reg-reg doesn't make the upper
 *          element zero!!! 
 *          NOTE: so far, it works. as temp reg normally uses a move from mem
 *          before reg-reg move, which makes the upper element zero.
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
            GetReg(-1);
         }
      }
/*
 *    Output vars are known to be updated only by ADD
 */
      if (VS_LIVEOUT & lp->vsflag[i+1])
      {
         j++;
         /*assert((lp->vsoflag[i+1] & (VS_MUL | VS_EQ | VS_ABS)) == 0);*/
         if (IS_INT(FLAG2TYPE(STflag[lp->vscal[i+1]-1]))) 
         {
            iptp = AddIntShadowEpilogue(lp, lp->posttails->blk, iptp, iptn, 
                                 lp->vscal[i+1], i+1);
            //fprintf(stderr, "LIVEOUT INT!!!\n");
         }
         else
         {
            r0 = GetReg(FLAG2TYPE(lp->vflag));
            r1 = GetReg(FLAG2TYPE(lp->vflag));
            if (lp->vsoflag[i+1] & VS_ACC || lp->vsoflag[i+1] & VS_MAX || 
                lp->vsoflag[i+1] & VS_MIN)
            {
               if (lp->vsoflag[i+1] & VS_ACC)
               {
                  iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                                      "Reduce accumulator vector for %s", 
                                      STname[lp->vscal[i+1]-1]);
               }
               else if (lp->vsoflag[i+1] & VS_MAX)
               {
                  iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                        "Reduce max vector for %s", STname[lp->vscal[i+1]-1]);

               }
               else if (lp->vsoflag[i+1] & VS_MIN)
               {
                  iptp = PrintComment(lp->posttails->blk, iptp, iptn,
                                      "Reduce min vector for %s", 
                                      STname[lp->vscal[i+1]-1]);
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
            GetReg(-1);
         }
      }
   }
   iptp = InsNewInst(lp->posttails->blk, iptp, NULL, CMPFLAG, CF_VRED_END,
            0, 0);

}

int FindInstIndex(int N, enum inst *inst, enum inst ainst )
{
   int i, index;
   for (i=0; i < N; i++)
   {
      if (inst[i] == ainst)
         break;
   }
   if (i == N)
      index = -1;
   else
      index = i;

   return index;
}
int isCodeAdjustmentNeeded(LOOPQ *lp)
/*
 * if there is some integer scalar other than loop control variable, we will 
 * need code adjustment.
 */
{
   int i, n;

   for (i=0, n=lp->vscal[0]; i < n; i++)
   {
      if (IS_INT(STflag[lp->vscal[i+1]-1]))
      {
         return(1);
      }
   }
   return(0);
}
int RcVecTransform(LOOPQ *lp)
/*
 * NOTE: This is similar of normal vector transformation DoSimdLoop except max
 * is need to be recognized. As I have changes lot in vector analysis in 
 * Path based analysis, I will create separate vector xform here. 
 * NOTE: this vector transform can be and will be merge with Speculative vector
 * transfer later. 
 */
{
   BLIST *bl;
   static enum inst 
     sfinsts[] = {FLD,      FST,      FMUL,     FMAC,     FADD,     FSUB,    
                  FABS,     FMOV,     FZERO,    FNEG,     FCMOV1,   FCMOV2, 
                  FCMPWEQ,  FCMPWNE,  FCMPWLT,  FCMPWLE,  FCMPWGT,  FCMPWGE,
                  FMAX,     FMIN,     FDIV },

     vfinsts[] = {VFLD,     VFST,     VFMUL,    VFMAC,    VFADD,    VFSUB,   
                  VFABS,    VFMOV,    VFZERO,   VFNEG,    VFCMOV1,  VFCMOV2,
                  VFCMPWEQ, VFCMPWNE, VFCMPWLT, VFCMPWLE, VFCMPWGT, VFCMPWGE,
                  VFMAX,    VFMIN,    VFDIV },

     sdinsts[] = {FLDD,     FSTD,     FMULD,    FMACD,    FADDD,    FSUBD, 
                  FABSD,    FMOVD,    FZEROD,   FNEGD,    FCMOVD1,  FCMOVD2, 
                  FCMPDWEQ, FCMPDWNE, FCMPDWLT, FCMPDWLE, FCMPDWGT, FCMPDWGE,
                  FMAXD,    FMIND,    FDIVD },

     vdinsts[] = {VDLD,     VDST,     VDMUL,    VDMAC,    VDADD,    VDSUB, 
                  VDABS,    VDMOV,    VDZERO,   VDNEG,    VDCMOV1,  VDCMOV2,
                  VDCMPWEQ, VDCMPWNE, VDCMPWLT, VDCMPWLE, VDCMPWGT, VDCMPWGE,
                  VDMAX,    VDMIN,    VDDIV };
/*
 *    Majedul: instruction selection for V_INT
 */
   static enum inst
      siinsts[] = {LD, ST, ADD, SUB, CMOV1, CMOV2, CVTBDI,CVTBFI,BTC},
      vs_insts[] = {VLD, VST, VSADD, VSSUB, VSCMOV1, VSCMOV2, CVTBDI, 
                   CVTFI, BTC},
      vi_insts[] = {VLD, VST, VIADD, VISUB, VICMOV1, VICMOV2, CVTBDI, 
                   CVTFI, BTC};
/*
 *    vector memory aligned and unaligned memory load
 *    NOTE: Not needed in updated implementation!
 */
#if 0   
   static enum inst 
      valign[] = {VFLD, VDLD, VLD, VFST, VDST, VST},
      vualign[] = {VFLDU, VDLDU, VLDU, VFSTU, VDSTU, VSTU};
   const int nvalign = 6; /* number of align/unalign inst */
#endif
   const int nvinst=21;   /* number of scalar to vector float inst */
   const int nivinst = 10;
   enum inst inst, vcmov1;
   /*enum inst vzero, szero, vmov, smov, vabs, sabs, vsub, ssub, vadd, sadd, vmac,
             smac, vmul, smul, vst, sst, vld, sld;*/
   /*enum inst vcmov2;*/
   short op;
   short vmask;
   enum inst *sinst, *vinst, *viinsts;
   int i, j, k, nfr=0, nir=0;
   struct ptrinfo *pi0;
   INSTQ *ip, *ip0, *ippu;
   short vlen;
   short sregs[TNFR], vregs[TNFR];
   short siregs[TNIR], viregs[TNIR];
   short ldvir1, ldvir2;

   if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
   {
      sinst = sfinsts;
      vinst = vfinsts;
      viinsts = vs_insts;
      vcmov1 = VSCMOV1;
      /*vcmov2 = VSCMOV2;*/
#if defined (X86) && defined(AVX)
      vlen = 8;
#else
      vlen = 4;
#endif
   }
   else
   {
      sinst = sdinsts;
      vinst = vdinsts;
      viinsts = vi_insts;
      vcmov1 = VICMOV1;
      /*vcmov2 = VICMOV2;*/
#if defined (X86) && defined(AVX)
      vlen = 4;
#else
      vlen = 2;
#endif
   }
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
   if (IsSIMDalignLoopPeelable(lp))
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

#if 0
   fprintf(stdout, "LIL After cleanup\n");
   PrintInst(stdout, bbbase);
   ShowFlow("cfg.dot",bbbase);
   //exit(0);
#endif

/*
 * code adjustment for shadow trick. As it will invalidate the code, we should 
 * add this after clean up and loop alignment 
 */
   if (isCodeAdjustmentNeeded(lp))
      AddCodeAdjustment(lp);
   
/**************************************************************************/
/* separating vector prologue and epilogue...... these are going to be more 
 * and more complex considering the RC for index var...... 
 */
#if 0
   PrintInst(stdout, bbbase);
#endif
/* 
 * Find all pointer updates, and remove them from body of loop (leaving only
 * vectorized instructions for analysis); will put them back in loop after
 * vectorization is done.
 */

   pi0 = FindMovingPointers(lp->blocks);
   ippu = KillPointerUpdates(pi0, vlen);
/*
 * Add prologue epilogue for vectorization
 */
   AddVectorPrologueEpilogue(lp);
#if 0
   fprintf(stdout, "LIL After Prologue epilogue\n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif
/*
 * Translate body of loop
 */
#if 1
/*
 * Re-organized the implementation 
 */
   for (bl=lp->blocks; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         inst = GET_INST(ip->inst[0]);
         if (ACTIVE_INST(inst))
         {
/*
 *          check whether it is integer operations
 */
            if ( ( i = FindInstIndex(nivinst, siinsts, inst)) != -1)
            {
               switch(inst)
               {
#if 0
                  case MASKTEST:
                     vmask = STpts2[ip->prev->inst[2]-1];
                     DelInst(ip->prev);
                     break;
#else
                  case CVTBDI: case CVTBFI:
                     assert(ip->next->inst[0] == BTC);
                     vmask = STpts2[ip->prev->inst[2]-1];
                     DelInst(ip->prev);
                     break;
                  case BTC:
                     assert(ip->prev->inst[0] == CVTBFI || 
                            ip->prev->inst[0] == CVTBDI);
                     DelInst(ip->prev);
                     break;
#endif
                  case LD: case ST:

                     if (inst == LD)
                        k = STpts2[ip->inst[2]-1]; 
                     else
                        k = STpts2[ip->inst[1]-1]; 

                     if (!FindInShortList(lp->varrs[0], lp->varrs+1, k))
                     { 
                        //fprintf(stderr, "Not arr = %s\n", STname[k-1]);
#if 0                        
                        if (inst == LD)
                           ip->inst[0] = VLD;
                        else
                           ip->inst[0] = VST;
                        for (j=1; j < 4; j++)
                        {
                           op = ip->inst[j];
                           if (!op) continue;
                           else if (op < 0) // register
                           {
                              op = -op;
                              k = FindInShortList(nir, siregs, op);
                              if (!k)
                              {
                                 nir = AddToShortList(nir, siregs, op);
                                 k = FindInShortList(nir, siregs, op);
                                 viregs[k-1] = GetReg(T_VINT);
                              }
                              ip->inst[j] = -viregs[k-1];
                           }
                           else // scalars 
                           {
                              op = STpts2[op-1];
                              k = FindInShortList(lp->vscal[0], lp->vscal+1, op);
                              #if 0
                                 assert(k);
                              #else
/*
 * FIXME: if we use index variable inside loop, we don't know how to vectorize 
 * that. So, memory access like: a0 = A[i] can't be vectorize which should be
 * easily vectorizable. Right now, we may consider to rewrite the kernel...
 */
                                 if (!k)
                                 {
                                    fprintf(stderr, "scalar = %s[%d] not a vscal\n",
                                           STname[op-1], op);
                                    fko_error(__LINE__, "unknown scalar!!\n");
                                 }
                              #endif
                              //ip->inst[j] = lp->vvscal[k];
                              ip->inst[j] = SToff[lp->vvscal[k]-1].sa[2];
                           }
                        }
#else
/*
 *    new policy: if a int variable is not found in vscal, it must be a DT 
 *    index. So, skip that. we can add additional checking here too
 */                     
                        k = FindInShortList(lp->vscal[0], lp->vscal+1, k);
                        if (k)
                        {
                           if (inst == LD)
                           {
                              ip->inst[0] = VLD;
                              ip->inst[2] = SToff[lp->vvscal[k]-1].sa[2];
                              op = -ip->inst[1];
                              j = 1;
                           }
                           else /* ST */
                           {
                              ip->inst[0] = VST;
                              ip->inst[1] = SToff[lp->vvscal[k]-1].sa[2];
                              op = -ip->inst[2];
                              j = 2;
                           }
                           k = FindInShortList(nir, siregs, op);
                           if (!k)
                           {
                              nir = AddToShortList(nir, siregs, op);
                              k = FindInShortList(nir, siregs, op);
                              viregs[k-1] = GetReg(T_VINT);
                           }
                           ip->inst[j] = -viregs[k-1];
                        }
                        else /* do we need to check again for DT index */
                        {
                           if (lp->I == STpts2[ip->inst[2]-1])
                           {
                              #if IFKO_DEBUG_LEVEL > 1
                                 fko_warn(__LINE__, "use of index variable " 
                                          "prohibits vectorization\n");
                              #endif
                           /*fprintf(stderr, "######### %s \n", 
                                 STname[STpts2[ip->inst[2]-1]-1]);*/
                              return(-1);
                           }
                        }
#endif
                     }
                     break;

                  case CMOV1: case CMOV2:
                     ip0 = ip->prev;
#if 0                     
                     while (ip0->inst[0] != MASKTEST) ip0 = ip0->prev;
#else
                     while (ip0->inst[0] != BTC) ip0 = ip0->prev;
#endif
                     assert(ip0);
                     DelInst(ip0);
/*
 *                   updates cmov with vcmov
 *                   new to load mask and use it in cmov
 */   
                     assert(IS_LOAD(ip->prev->inst[0]) && 
                        IS_LOAD(ip->prev->prev->inst[0]));
                     ldvir2 = ip->prev->inst[1]; 
                     ldvir1 = ip->prev->prev->inst[1]; 

                     if (FLAG2TYPE(STflag[vmask-1]) == T_FLOAT 
                        || FLAG2TYPE(STflag[vmask-1]) == T_VFLOAT )
                     { 
                        k = GetReg(T_VFLOAT);
                        InsNewInst(bl->blk, NULL, ip, VFLD, -k, 
                                   SToff[vmask-1].sa[2], 0);
                     }
                     else
                     {
                        k = GetReg(T_VDOUBLE);
                        InsNewInst(bl->blk, NULL, ip, VDLD, -k, 
                              SToff[vmask-1].sa[2], 0);
                     }
                     /*ip->inst[0] = VCMOV1;*/
                     ip->inst[0] = vcmov1;
                     ip->inst[1] = ldvir1; 
                     ip->inst[2] = ldvir2; 
                     ip->inst[3] = -k;
                     assert(ip->next->inst[0] == ST);
                     ip->next->inst[0] = VST;
                     k = STpts2[ip->next->inst[1]-1];
                     k = FindInShortList(lp->vscal[0],lp->vscal+1,k);
                     assert(k);
                     ip->next->inst[1] = SToff[lp->vvscal[k]-1].sa[2];
                     ip->next->inst[2] = ldvir1;
                     ip= ip->next; 
                     break;

                  default:
                     if (siinsts[i] == inst)
                     { 
                        ip->inst[0] = viinsts[i];
                        for (j=1; j < 4; j++)
                        {
                           op = ip->inst[j];
                           if (!op) continue;
                           else if (op < 0)
                           {
                              op = -op;
                              k = FindInShortList(nir, siregs, op);
                              if (!k)
                              {
                                 nir = AddToShortList(nir, siregs, op);
                                 k = FindInShortList(nir, siregs, op);
                                 viregs[k-1] = GetReg(FLAG2TYPE(T_VINT));
                              }
                              ip->inst[j] = -viregs[k-1];
                           }
                           else
                           {
                              if (IS_DEREF(STflag[op-1]))
                              {
                                 k = STpts2[op-1];
                                 if (!FindInShortList(lp->varrs[0],
                                          lp->varrs+1,k))
                                 {
                                    k = FindInShortList(lp->vscal[0],lp->vscal+1,
                                          k);
                                    assert(k);
                                    ip->inst[j] = SToff[lp->vvscal[k]-1].sa[2];
                                    assert(ip->inst[j] > 0);
                                 }
                              }
                              else if (!FindInShortList(lp->varrs[0],
                                       lp->varrs+1,op))
                              {
                                 k = FindInShortList(lp->vscal[0], 
                                       lp->vscal+1, op);
                                 assert(k);
                                 ip->inst[j] = lp->vvscal[k]; /* ???? */
                              }
                           }
                        }
                     }
                     break;
               }
            }
/*
 *          Floating point operation
 */
            else if ( ( i = FindInstIndex(nvinst, sinst, inst)) != -1) 
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
/*
 *                         variable load/store, not a memory load/store
 */
                           if (!FindInShortList(lp->varrs[0],lp->varrs+1,k))
                           {
                              k = FindInShortList(lp->vscal[0],lp->vscal+1,k);
/*
 *                            not found in vscal!!!
 */
                              if (!k)
                              {
                                 PrintST(stderr);
                                 fprintf(stderr, "NOT FOUND = (%d)!!!\n",
                                          op);
                                 assert(k);
                              }
                              ip->inst[j] = SToff[lp->vvscal[k]-1].sa[2];
                              assert(ip->inst[j] > 0);
                           }
#if 0
/*
 *                         otherwise, it is a memory load/store
 *                         check for appropriate alignment using alignment 
 *                         markups
 *                         NOTE: if we have markup for force aligned, rest of
 *                         the arrays are unaligned... we may still have no 
 *                         force aligned array by the markup mutually unaligned 
 *                         is specifies that they are not mutually aligned. We
 *                         use 1st array in varrs to align in loop peeling..
 */
                           else
                           {
                              if ((lp->falign && k != lp->falign[1]) || 
                                  (lp->malign == -1 && !lp->falign && 
                                     k != lp->varrs[1]) )
                              {
                                 k = FindInstIndex(nvalign, valign, ip->inst[0]);
                                 if (k != -1)
                                    ip->inst[0] = vualign[k];
                              }
                           }
#endif
                        }
                        else if (!FindInShortList(lp->varrs[0],lp->varrs+1,op))
                        {
                           k = FindInShortList(lp->vscal[0], lp->vscal+1, op);
                           assert(k);
                           ip->inst[j] = lp->vvscal[k];
                        }
                     }
                  }
               }

            }
            else if ( IS_BRANCH(inst) || inst == LABEL || inst == PREFR 
                      || inst == PREFW)
            {
               // do nothing !
            }
            else 
            {
/*
 *             FIXED: VDIVD instruction added for cos/sin ... ... ... 
 */
               PrintThisInst(stderr, 0, ip);
               assert(0);
            }
         }
      }
   }
#else
   for (bl=lp->blocks; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         inst = GET_INST(ip->inst[0]);
         if (ACTIVE_INST(inst))
         {
#if 0
            PrintThisInst(stderr, 0, ip);
#endif
/*
 *          For integer instructions
 */
            if (inst == MASKTEST)
            {
               vmask = STpts2[ip->prev->inst[2]-1];
#if 0               
               fprintf(stderr, "mask...= %s\n", STname[vmask-1]);
#endif
               DelInst(ip->prev);
            }
/*
 *          there can be two type of load, scal load or arr load 
 *          Need to transform only the scalar load here 
 */
            else if (inst == LD || inst == ST)
            {
               k = STpts2[ip->inst[2]-1]; 
               if (!FindInShortList(lp->varrs[0], lp->varrs+1, k))
               {
                  //fprintf(stderr, "Not arr = %s\n", STname[k-1]);
                  if (inst == LD)
                     ip->inst[0] = VLD;
                  else
                     ip->inst[0] = VST;

                  for (j=1; j < 4; j++)
                  {
                     op = ip->inst[j];
                     if (!op) continue;
                     else if (op < 0) // register
                     {
                        op = -op;
                        k = FindInShortList(nir, siregs, op);
                        if (!k)
                        {
                           nir = AddToShortList(nir, siregs, op);
                           k = FindInShortList(nir, siregs, op);
                           viregs[k-1] = GetReg(T_VINT);
                        }
                        ip->inst[j] = -viregs[k-1];
                     }
                     else // scalars 
                     {
                        op = STpts2[op-1];
                        k = FindInShortList(lp->vscal[0], lp->vscal+1, op);
                        assert(k);
                        ip->inst[j] = lp->vvscal[k];
                     }
                  }
               } /* no support for vector integer unaligned mem yet */
            }
            else if (inst == CMOV1 || inst == CMOV2)
            {
#if 0               
               fprintf(stderr, "using mask...= %s\n", STname[vmask-1]);
#endif
                 
               ip0 = ip->prev;
               while (ip0->inst[0] != MASKTEST) ip0 = ip0->prev;
               assert(ip0);
               DelInst(ip0);
/*
 *             updates cmov with vcmov
 *             new to load mask and use it in cmov
 */   
               assert(IS_LOAD(ip->prev->inst[0]) && 
                      IS_LOAD(ip->prev->prev->inst[0]));
               ldvir2 = ip->prev->inst[1]; 
               ldvir1 = ip->prev->prev->inst[1]; 

               if (FLAG2TYPE(STflag[vmask-1]) == T_FLOAT 
                   || FLAG2TYPE(STflag[vmask-1]) == T_VFLOAT )
               {
                  k = GetReg(T_VFLOAT);
                  InsNewInst(bl->blk, NULL, ip, VFLD, -k, SToff[vmask-1].sa[2],
                              0);
               }
               else
               {
                  k = GetReg(T_VDOUBLE);
                  InsNewInst(bl->blk, NULL, ip, VDLD, -k, SToff[vmask-1].sa[2],
                              0);
               }
               ip->inst[0] = VCMOV1;
               ip->inst[1] = ldvir1; 
               ip->inst[2] = ldvir2; 
               ip->inst[3] = -k;
               assert(ip->next->inst[0] == ST);
               ip->next->inst[0] = VST;
               k = STpts2[ip->next->inst[1]-1];
               k = FindInShortList(lp->vscal[0],lp->vscal+1,k);
               assert(k);
               ip->next->inst[1] = SToff[lp->vvscal[k]-1].sa[2];
               ip->next->inst[2] = ldvir1;
               ip= ip->next; 
            }
            else
            { 
               for (i=0; i < nivinst; i++)
               {
                  if (siinsts[i] == inst)
                  {
                     ip->inst[0] = viinsts[i];
/*
 *                   change the regs from scalar to vector  
 */
                     for (j=1; j < 4; j++)
                     {
                        op = ip->inst[j];
                        if (!op) continue;
                        else if (op < 0)
                        {
                           op = -op;
                           k = FindInShortList(nir, siregs, op);
                           if (!k)
                           {
                              nir = AddToShortList(nir, siregs, op);
                              k = FindInShortList(nir, siregs, op);
                              viregs[k-1] = GetReg(FLAG2TYPE(T_VINT));
                           }
                           ip->inst[j] = -viregs[k-1];
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
                     //PrintThisInst(stderr, 0, ip);
                     break;
                  }
               }
            }
/*
 *          for floating point instructions
 */
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
#endif


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

int RcVectorization()
{
   int fstat; 
   LOOPQ *lp;
   extern LOOPQ *optloop;

   lp = optloop;
   fstat = 1;
#if 0
   //PrintLoopInfo();
   fprintf(stdout, "Before RC VEC\n");
   PrintInst(stdout,bbbase);
   //exit(0);
#endif
/*
 * we may fail to vectorize the loop after RC. Due to shadow RC, we relax our
 * analysis. Be sure to check error after transformation. 
 */
   if (!RcVecTransform(lp))
      fstat = 0;
   KillPathTable();
#if 0
/*
 * To print code here, should call FinalizeVectorCleanup first
 */
   FinalizeVectorCleanup(lp, 1);
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__,__LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
   //PrintST(stdout);
   PrintInst(stdout,bbbase);
   //fprintf(stderr, "\n\n OL_NEINC=%d\n", Get_OL_NEINC());

   exit(0);
#endif
   return (fstat);
}

/*============================================================================
 * Loop specialization for memory alignment after SIMD vectorization
 *    
 *===========================================================================*/

static void ConvertAlign2Unalign(short *aptrs, BLIST *scope)
/*
 * provided list of aligned ptr and the loop scope, this function converts 
 * aligned instruction into unaligned for those who are not in aptrs list.
 */
{
   static enum inst
      valign[] = {VFLD, VDLD, VLD, VFST, VDST, VST},
      vualign[] = {VFLDU, VDLDU, VLDU, VFSTU, VDSTU, VSTU}; 
   enum inst inst;
   const int nvalign = 6;
   int i,j,k;
   short op;
   INSTQ *ip;
   BLIST *bl;
   LOOPQ *lp;
   extern LOOPQ *optloop;

   assert(aptrs);
   lp = optloop;
   for (bl=scope; bl; bl=bl->next)
   {
      for (ip=bl->blk->inst1; ip; ip=ip->next)
      {
         inst = ip->inst[0];
         if ((j = FindInstIndex(nvalign, valign, inst)) != -1) /*vload/vstore*/
         {
            for (i=1; i<4; i++)
            {
               op = ip->inst[i];
               if (op > 0 && IS_DEREF(STflag[op-1]))
               {
                  k = STpts2[op-1];
                  if (!FindInShortList(aptrs[0], aptrs+1, k)
                        && FindInShortList(lp->varrs[0],lp->varrs+1, k))
                  {
                     ip->inst[0] = vualign[j];
                  }
               }
            }
         }
      }
   }
}
void UnalignLoopSpecialization(LOOPQ *lp)
/*
 * this function will add an extra vectorized loop with unaligned loads/stores
 * to manage the alignment in case all arrays are not mutually aligned
 */
{
   int i,k,n;
   int halign;
   short reg0, reg1;
   short lsAlabel,jBlabel; 
   short faptr, ptr;
   short *aptrs;
   short rvar;
   INSTQ *ip;
   BBLOCK *bp0, *bp, *bpN;
   BLIST *bl, *dupblks;
   extern BBLOCK *bbbase;
/*
 * ptr which is already been aligned
 * FIXME: logic for fptr is changed. We marked the most accessed ptr as fptr
 * in case it is not specified through markup!
 * NOTE: we updated optloop->fa2vlen in loop peeling to be that.
 */
#if 0   
   if (lp->faalign)
      faptr = lp->faalign[1];
   else
      faptr = lp->varrs[1];
#else
   faptr = lp->fa2vlen;
   assert(faptr);
#endif
/*
 * If there are ptrs which are mutually aligned with faptr, there are also
 * aligned after aplying loop peeling. make a list of them
 */
#if 0
/* no need to consider this case, it is checked by IsAlignLoopSpecNeeded() */
   if (!lp->maaligned && lp->mbaligned) /* means all ptrs are malign */
      aptrs = lp->varrs; 
   else 
#endif      
   if (lp->maaligned)
   {
      for (n=lp->maaligned[0],i=1; i <= n; i++)
         if (lp->maaligned[i] == faptr)
            break;
      if (i<=n)   /* faptr is also in maaligned ptr */
         aptrs = lp->maaligned;
   }
   if (!lp->maaligned || i > n) /* not maligned or not matched with faptr */
   {
      aptrs = malloc(2*sizeof(short));
      assert(aptrs);
      aptrs[0] = 1;
      aptrs[1] = faptr;
   }
#if 0
   fprintf(stderr, "Aligned PTR=");
   for (n=aptrs[0], i=1; i <= n; i++)
      fprintf(stderr, "%s ", STname[aptrs[i]-1]);
   fprintf(stderr, "\n");
#endif
/*
 * for any loop specialization, we need to consider following issues:
 * 1. where from: need to figure out the condition to switch to this new loop
 * 2. duplicate: need to duplicate loop with necessary changes
 * 3. where to: jump back to appropriate location
 */
/*
 *                step 1
 * ===========================================================================
 * 1. where from: need to figure out the condition to switch to this new loop
 * NOTE: it doesn't work if we place the condition in preheader... register 
 * assignment also assumes preheader and posttail not share with loops..
 * So, place test at the end of each predecessor of the header of loop..
 */
#ifdef AVX
   halign = 0x1F;
#else
   halign = 0x0F;
#endif
   k = STiconstlookup(halign);
   lsAlabel = STlabellookup("_FKO_LOOP_SPEC_ALIGN"); 
   rvar = InsertNewLocal("_RES_ALIGN",T_INT);
   //bp = lp->preheader;
   for (bl=lp->preheader->preds; bl; bl=bl->next)
   {
      bp = bl->blk;
      ip = bp->ainstN;
   /* add extra checking for the alignment */
      for (i=1,n=lp->varrs[0]; i <= n; i++)
      {
         if (FindInShortList(aptrs[0], aptrs+1, lp->varrs[i])) 
            continue;
         ptr = lp->varrs[i]; 
#if 0
         fprintf(stderr, "Check PTR=%s\n", STname[ptr-1]);
#endif
         ip = PrintComment(bp, ip, NULL, "Checking for alignment for" 
                     " loop specialization");
         reg0 = GetReg(T_INT);
         reg1 = GetReg(T_INT);
         ip = InsNewInst(bp, ip, NULL, LD, -reg0, SToff[ptr-1].sa[2], 0);
         /*ip = InsNewInst(bp, ip, NULL, LD, -reg1, SToff[rvar-1].sa[2], 0);*/
         ip = InsNewInst(bp, ip, NULL, AND , -reg0, -reg0, k);
         ip = InsNewInst(bp, ip, NULL, ST, SToff[rvar-1].sa[2], -reg0, 0);
         GetReg(-1);
         break; /* 1 iteration peeling */
      }
      for (i=i+1; i <=n; i++)
      {
         if (FindInShortList(aptrs[0], aptrs+1, lp->varrs[i])) 
            continue;
         ptr = lp->varrs[i]; 
#if 0
         fprintf(stderr, "Check PTR=%s\n", STname[ptr-1]);
#endif
         reg0 = GetReg(T_INT);
         reg1 = GetReg(T_INT);
         ip = InsNewInst(bp, ip, NULL, LD, -reg0, SToff[ptr-1].sa[2], 0);
         ip = InsNewInst(bp, ip, NULL, LD, -reg1, SToff[rvar-1].sa[2], 0);
         ip = InsNewInst(bp, ip, NULL, AND , -reg0, -reg0, k);
         ip = InsNewInst(bp, ip, NULL, OR , -reg1, -reg1, -reg0);
         ip = InsNewInst(bp, ip, NULL, ST, SToff[rvar-1].sa[2], -reg1, 0);
         GetReg(-1); 
      } 
      reg0 = GetReg(T_INT);
      ip = InsNewInst(bp, ip, NULL, LD, -reg0, SToff[rvar-1].sa[2], 0);
      ip = InsNewInst(bp, ip, NULL, CMP, -ICC0, -reg0, STiconstlookup(0));
      ip = InsNewInst(bp, ip, NULL, JNE, -PCREG, -ICC0 ,lsAlabel);
      GetReg(-1);
   }
#if 0
      fprintf(stderr, "alignment checkingi blk:");
      PrintThisBlockInst(stderr, bp);
      //exit(0);
#endif
/*
 *                step 2
 * ===========================================================================
 * 2. duplicate: need to duplicate loop with necessary changes
 * NOTE: need to copy the preheader and post tails to
 */
/*
 * find the last block, add loop specialization after that
 */
   for (bp0=bbbase; bp0->down; bp0=bp0->down);
   bp = NewBasicBlock(bp0, NULL);
   bp0->down = bp;
   bp->up = bp0;
   bp0 = bp;
/*
 * Start new block with a  label
 */
   ip = InsNewInst(bp, NULL, NULL, LABEL, lsAlabel , 0, 0);
/*
 * duplicate the preheader
 */
   bp = DupBlock(lp->preheader);
   if (bp->ainst1->inst[0] == LABEL) /* delete the label */
   {
      DelInst(bp->ainst1);
   }
   bp0->down = bp;
   //bp->up = bp0;
   bp0 = bp;
/*
 * duplicate the loop 
 */
   dupblks = AddLoopDupBlks(lp, bp0, bp0->down);
/*
 * convert the loop for unaligned access 
 */
   ConvertAlign2Unalign(aptrs, dupblks);
/*
 * create new loop and populate it with our new loop
 */
#if 0   
   lpn = NewLoop(lp->flag);
   lpn->I = lp->I;
   lpn->beg = lp->beg;
   lpn->end = lp->end;
   lpn->inc = lp->inc;
   lpn->preheader = bp;
   bp = FindBlockInListByNumber(dupblks, lp->header->bnum);
   lpn->head = bp;
   lpn->body_label = bp->ilab;
   for (bl=lp->tails; bl; bl=bl->next)
   {
      bp = FindBlockInListByNumber(dupblks, bl->blk->bnum);
      lpn->tails = AddBlockToList(lpn->tails, bp);
   }
   lpn->blocks = dupblks;
#endif
/*
 * duplicate the posttails
 * NOTE: to minimize the complexity, we consider only one posttail here
 * we need to extend it for multiple posttails later.
 */
   assert(lp->posttails && !lp->posttails->next);
   bp0 = FindBlockInListByNumber(dupblks, lp->tails->blk->bnum);
   bp = DupBlock(lp->posttails->blk);
#if 0
   fprintf(stderr, "copy of posttail blk\n");
   PrintThisBlockInst(stderr, bp);
#endif
   bp0->down = bp;
   //bp->up = bp0;
   bp0 = bp;
/*
 * jump to the usucc of posttail 
 */
   bp = NewBasicBlock(bp0, NULL);
   bp0->down = bp;

   bpN = lp->posttails->blk->usucc;
   if (bpN->ainst1->inst[0] == LABEL)
      jBlabel = bpN->ainst1->inst[1];
   else
   {
      jBlabel =  STlabellookup("_FKO_LOOP_POSTTAIL"); 
      InsNewInst(bpN, NULL, bpN->ainst1, LABEL, jBlabel, 0,0);
   }
   InsNewInst(bp, NULL, NULL, JMP, -PCREG, jBlabel, 0);

#if 0   
/*
 *                step 3
 * ===========================================================================
 * 3. where to: jump back to appropriate location
 */
   tails = NULL;
   k=0;
   for (bl=lp->tails; bl; bl=bl->next)
   {
      bp0 = FindBlockInListByNumber(dupblks, bl->blk->bnum);
      //tails = AddBlockToList(tails, bp);
      assert(bp0);
      //fprintf(stderr, "tail=%d, tail->down=%d\n", bp0->bnum, bp0->down->bnum);
      bp = NewBasicBlock(bp0, bp0->down);
      bp0->down = bp;
      //fprintf(stderr, "tail=%d, tail->down=%d\n", bp0->bnum, bp0->down->bnum);
/*
 *    NOTE: for multiple tails and posttails, we need to know which posttail is
 *    for which tail. Right now, we skip this out
 */
      assert(lp->posttails && !lp->posttails->next);
      if (lp->posttails->blk->ainst1->inst[0] == LABEL)
         jBlabel = lp->posttails->blk->ainst1->inst[1];
      else
      {
         jBlabel =  STlabellookup("_FKO_LOOP_POSTTAIL"); 
      }
      ip = InsNewInst(bp, NULL, NULL, JMP, -PCREG, jBlabel, 0);
   }
#endif   
#if 0
   for(bl=dupblks; bl; bl=bl->next)
   {
      PrintThisBlockInst(stderr, bl->blk);
   }
   PrintThisBlockInst(stderr, bp);
#endif

   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__,__LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__,__LINE__); 
   /*ShowFlow("lscfg.dot",bbbase);*/
#if 0
   fprintf(stdout, "LIL after loop specialiazation\n");
   PrintInst(stdout, bbbase);
   PrintST(stdout);
   ShowFlow("lscfg.dot",bbbase);
   exit(0);
#endif
}

void UpdateVecLoop(LOOPQ *lp)
/*
 * this function updates data for HIL intrinsic-vectorized loop  
 */
{
   int i, n, N;
   struct ptrinfo *pbase, *p;
   INT_BVI iv;
   BLIST *bl;
   short *sp, *s;
   int vflag;
   extern INT_BVI FKO_BVTMP;
   extern short STderef;
   extern BBLOCK *bbbase;
/*
 * what to update:
 * 1. vflag
 * 2. varrs
 * 3. vvscal
 */
/*
 * finding all variables use and set inside loop
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);

   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
   for (bl=lp->blocks; bl; bl=bl->next)
   {
      iv = BitVecComb(iv, iv, bl->blk->uses, '|');
      iv = BitVecComb(iv, iv, bl->blk->defs, '|');
   }
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);
   
   sp = BitVec2Array(iv, 1-TNREG);
#if 0
   for ( i=1, N=sp[0]; i <= N; i++)
   {
         fprintf(stderr,"%s[%d] ",STname[sp[i]-1],STflag[sp[i]-1]);
   }
   fprintf(stderr, "\n");
#endif
   vflag = 0;
   for (N=sp[0],n=0,i=1; i <= N; i++)
   {
      if (IS_VDOUBLE(STflag[sp[i]-1]))
      {
         vflag = T_VDOUBLE;
         sp[n++] = sp[i];
      }
      else if ( IS_VFLOAT(STflag[sp[i]-1]))
      {
         vflag = T_VDOUBLE;
         sp[n++] = sp[i];
      }
   }
   if (IS_VDOUBLE(vflag) && IS_VFLOAT(vflag))
      fko_error(__LINE__, "Mixed vector type in intrinsic\n");
   lp->vflag = vflag;

   s = malloc(sizeof(short)*(n+1));
   assert(s);
   s[0] = n;
   for (i=1; i <= n; i++)
      s[i] = sp[i-1];
   lp->vvscal = s;
#if 0  
   fprintf(stderr, "vflag=%d\n", lp->vflag);
   if (n)
   {
      fprintf(stderr, "Vars: ");
      for (i=0; i < n ; i++)
      {
         fprintf(stderr,"%s[%d] ",STname[sp[i]-1],sp[i]-1);
      }
      fprintf(stderr, "\n");
   }
   else
      fprintf(stderr, "No Var!\n");
#endif
/*
 * finding moving pointer
 */
   pbase = FindMovingPointers(lp->blocks);
   for (N=0, p=pbase; p; p=p->next) N++;
   s = malloc(sizeof(short)*(N+1));
   assert(s);
   s[0] = N;
   for (i=1, p=pbase; p; p=p->next, i++)
      s[i] = p->ptr;
   lp->varrs = s;
#if 0  
   if (N)
   {
      fprintf(stderr, "Varrays: ");
      for (i=1; i <= N ; i++)
      {
         fprintf(stderr,"%s[%d] ",STname[s[i]-1],s[i]-1);
      }
      fprintf(stderr, "\n");
   }
   else
      fprintf(stderr, "No Var!\n");
#endif

   if (sp) free(sp);
}

