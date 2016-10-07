/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#include "fko.h"

/* Majedul: to save the paths for speculative Vectorization. */
static LOOPPATH **PATHS = NULL;
static int NPATH = 0, TNPATH = 0, VPATH = -1;

int SKIP_PELOGUE = 0;  /* needed to skip for loop nest vec */
int SKIP_PEELING = 0;  /* needed to skip for loop nest vec */
int SKIP_CLEANUP = 0;  /* can safely skip the cleanup after vectorixzation*/

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
 * to manage pack in SLP
 */
static PACK **PACKS = NULL;
static int NPACK = 0, TNPACK = 0;

void NewPackTable(int chunk)
{
   int i, n;
   PACK **new;
   n = TNPACK + chunk;
   new = malloc(n*sizeof(PACK*));
   assert(new);
   for (i=0; i != TNPACK; i++)
      new[i] = PACKS[i];
   for ( ; i != n; i++)
      new[i] = NULL;
   if (PACKS)
      free(PACKS);
   PACKS = new;
   TNPACK = n;
}

int AddPack2Table(PACK *pack)
{
   if (NPACK == TNPACK)
      NewPackTable(32);
   PACKS[NPACK] = pack;
   pack->pnum = NPACK;
   return(NPACK++);
}

PACK *NewPack(ILIST *sil, int len)
{
   PACK *new;
   new = malloc(sizeof(PACK));
   assert(new);
   if (sil)
   {
      new->sil = sil;
      new->vlen = len;
   }
   else
   {
      new->sil = NULL;
      new->vlen = 0;
   }
   new->vflag = 0;
   new->isVec = 1; /* vectorizable unless proved otherwise */
   new->pktype = 0; /* pack type */
   new->vil = NULL;
   new->depil = NULL;
   new->uses = NewBitVec(TNREG+32);
   new->defs = NewBitVec(TNREG+32);
   //new->scal = NULL;
   //new->vscal = NULL;
   new->scdef = NULL;
   new->vsc = NULL;
   new->scuse = NULL;
   new->pnum = AddPack2Table(new);
   return new;
}
void KillPack(PACK *pack)
{
   ILIST *il;
   if (pack->sil) 
   {
/*
 *    here, we want to delete instq with the ilist, assuming the instq is created
 *    just for packing; not using the pointer to the instq of original code
 */
      il = pack->sil;
      while(il)
      {
         KillAllInst(il->inst);
         il = il->next;
      }
      KillAllIlist(pack->sil);
   }

   if (pack->vil)
   {
      il = pack->vil;
      while (il)
      {
         KillAllInst(il->inst);
         il = il->next;
      }
      KillAllIlist(pack->vil);
   }
   
   if (pack->depil)
   {
/*
 *    in depil, we point inst of a block. So, don't try to kill it 
 */
      /*il = pack->depil;
      while (il) 
      {
         KillAllInst(il->inst);
         il = il->next;
      }*/
      KillAllIlist(pack->depil);
   }

   if (pack->uses) 
      KillBitVec(pack->uses);
   if (pack->defs) 
      KillBitVec(pack->defs);
   //if (pack->scal)
   //   free(pack->scal);
   //if (pack->vscal)
   //   free(pack->vscal);
   if (pack->scdef)
      free(pack->scdef);
   if (pack->scuse)
      free(pack->scuse);
   if (pack->vsc)
      free(pack->vsc);

   PACKS[pack->pnum] = NULL;
   free(pack);
}

void KillPackFromTable(PACK *apack)
{
   int i, j;
   for (i=0; i < NPACK; i++)
      if (PACKS[i] == apack)
         break;
   if (i != NPACK)
   {
      KillPack(PACKS[i]);
      PACKS[i] = NULL;
      for (j = i+1; j < NPACK; i++, j++)
      {
         PACKS[i] = PACKS[j];
         PACKS[i]->pnum = i;
      }
      NPACK--;
   }
}

void KillPackTableEntries()
{
   int i;
   for (i=0; i < NPACK; i++)
   {
      if (PACKS[i])
      {
         KillPack(PACKS[i]);
         PACKS[i] = NULL;
      }
   }
   NPACK = 0;
}

void KillPackTable()
{
   KillPackTableEntries();
   free(PACKS);
   PACKS = NULL;
   TNPACK = 0;
}

/*Majedul: Changed for AVX*/
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
      KillAllIlist(ib);
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
 * NOTE: 2D array ptrs are considered to find the candiadate ptr
 */
{
   int i, k, n;
   short ptr, id;
   short *s;
   int type;

   assert(lp->varrs);
/*
 * if there exist a ptr from FORCE_ALIGN markup
 * NOTE: We don't check alignment in force_align ptrs except the value 0 which
 * mean not alignable!
 */
   if (lp->faalign)
   {
      for (i=1, n=lp->faalign[0]; i <= n; i++)
      {
         if (lp->fbalign[i]) /* skip with alignment 0 */
         {
/*
 *          if it is 2D array ptr, all column ptr must be mutually aligned!
 */
            id = STarrlookup(lp->faalign[i]);
            if (!id)
            {
               lp->fa2vlen = lp->faalign[i];
               break;
            }
            else /* 2d array pointer */
            {               
               if (!lp->maaligned && lp->mbalign) /*all mutually aligned */ 
               {
                  if (lp->mbalign[1] >= GetVecAlignByte())
                  {
                     lp->fa2vlen = STarr[id-1].colptrs[1];
                     break;
                  }
               }
               else if (lp->maaligned 
                        && (k=FindInShortList(lp->maaligned[0], lp->maaligned+1, 
                           lp->faalign[i])))
               {
                  if (lp->mbalign[k] >= GetVecAlignByte())
                  {
                     lp->fa2vlen = STarr[id-1].colptrs[1];
                     break;
                  }
               }
               else 
               {
                  fko_warn(__LINE__, "force align to 2d array," 
                        " but not mutually aligned! considered 1st col ptr \n");
                  lp->fa2vlen = STarr[id-1].colptrs[1];
               }  
            }
         }
      }
   }
/*
 * force any ptr, still need to check for 2D..skip them if not mutually aligned
 */
   else if (!lp->faalign && lp->fbalign)
   {
      //lp->fa2vlen = lp->varrs[1];
      for (i=1, n=lp->varrs[0]; i <= n; i++)
      {
         id = STarrColPtrlookup(lp->varrs[i]);
         if (!id)
         {
            lp->fa2vlen = lp->varrs[i];
            break;
         }
         else /* col ptr in 2D */
         {
            if (!lp->maaligned && lp->mbalign) /*all mutually aligned */ 
            {
               if (lp->mbalign[1] >= GetVecAlignByte())
               {
                  lp->fa2vlen = lp->varrs[i];
                  break;
               }
            }
            else if (lp->maaligned && 
                  (k=FindInShortList(lp->maaligned[0], lp->maaligned+1, id)))
            {
               if (lp->mbalign[k] >= GetVecAlignByte())
               {
                  lp->fa2vlen = lp->varrs[i];
                  break;
               }
            }
         }
      }
   }
/*
 * if no ptr is specified in markup, find out best candidate
 * here is strategy to do that:
 * 1. ptr which is mutually aligned with other ptr
 * 2. ptr which has max read/write access
 */   
   else
   {
      ptr = 0;
      if (lp->maaligned)
      {
/*
 *       NOTE: consider 2D array... prioritize it, since all of the col ptrs 
 *       would be mutually aligned
 */
         for (i=1, n=lp->maaligned[0]; i <= n; i++)
         {
            if (lp->mbalign[i] >= GetVecAlignByte())
            {
               id = STarrlookup(lp->maaligned[i]);
               if (id)
               {
                  ptr = STarr[id-1].colptrs[1];
                  break;
               }
            }
         }
/*
 *       no 2D array ptr, choice first one 
 */
         if (!ptr)
         {
            for (i=1, n=lp->maaligned[0]; i <= n; i++)
            {
               if (lp->mbalign[i] >= GetVecAlignByte())
               {
                  ptr = lp->maaligned[i];
                  break;
               }
            } 
         }
      }
      else if (!lp->maaligned && lp->mbalign) /* all malign */
      {
         if (lp->mbalign[1] >= GetVecAlignByte())
         {
            //ptr = lp->varrs[1];
            for (i=1, n=lp->varrs[0]; i <= n; i++)
            {
               id = STarrColPtrlookup(lp->varrs[i]);
               if (id)
               {
                  ptr = lp->varrs[i];
                  break;
               }
            }
            if (!ptr) /* no 2D array */
            {
               ptr = lp->varrs[1];
            }
         }
      }
/*
 *    still no forceable ptr! find most frequent one 
 */ 
      if (!ptr) /* no falign and no malign ptr, find the most accessed one */
         ptr = FindMostAccessedPtr(lp->varrs, lp->blocks);
      
      lp->fa2vlen = ptr;
   }
#if 1
   if (lp->fa2vlen)
   {
      fprintf(stderr, "force align ptr = %s\n", STname[lp->fa2vlen-1]);
   }
#endif
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
  assert(fptr);
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
 *       NOTE: later this V[F/D/I]LDS will be changed into two inst in ra:
 *           V[F/D/I]ZERO vr
 *           V[F/D/I]MOVS vr, r, vr
 *           so, upper elements of vector will be zerod. 
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

   if (!iv)
      return;
   sp = BitVec2Array(iv, 1-TNREG);
   fprintf(out, "%s : ", title);
   for (N=sp[0], i=1; i <= N; i++)
   {
      if (sp[i] > 0)
         fprintf(out, "%s(%d) ",STname[sp[i]-1]? STname[sp[i]-1]: "NULL",
               sp[i]-1);
      else
         fprintf(out,"[ST NULL:%d] ",sp[i]-1);
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
#if 0
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
#endif
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
 *       NOTE: later this V[F/D/I]LDS will be changed into two inst in ra:
 *           V[F/D/I]ZERO vr
 *           V[F/D/I]MOVS vr, r, vr
 *           so, upper elements of vector will be zerod. 
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
 *       NOTE: later this V[F/D/I]LDS will be changed into two inst in ra:
 *           V[F/D/I]ZERO vr
 *           V[F/D/I]MOVS vr, r, vr
 *           so, upper elements of vector will be zerod. 
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
 *          FIXME: what happens after adding PREF inst
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
int RcVectorAnalysis(LOOPQ *lp)
{
   int i, j, k, n;
   int *vpindx;
   int errcode, isIscal;
   char ln[512];
   short *sp;
   LOOPPATH *vpath;
   /*LOOPQ *lp;*/
   /*extern LOOPQ *optloop;*/
   extern BBLOCK *bbbase;
   extern int VECT_FLAG;


   /*lp = optloop;*/
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
               if (ip0->inst[0] == ADD)
               {
                  if (IS_CONST(STflag[ip0->inst[3]-1]))
                  {
                     initconst = SToff[ip0->inst[3]-1].i;
                  }
                  else fko_error(__LINE__,"Can only be updated with const");
               }
               else if (ip0->inst[0] == SUB)
               {
                  if (IS_CONST(STflag[ip0->inst[3]-1]))
                  {
                     initconst = -SToff[ip0->inst[3]-1].i; 
                  }
                  else fko_error(__LINE__,"Can only be updated with const");
               }
               ip0 = ip0->prev;
               DelInst(ip0->next);
            }
            ip = DelInst(ip0->next);
/*
 *          insert new instruction for this candidate
 *          imax_1 = imax_1 + vlen
 *          FIXME: need to support imax = i + const. need to keep track that. 
 */
            ip = ip->prev; /* point back and inst after this*/
            reg0 = GetReg(T_INT);
            reg1 = GetReg(T_INT);
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
 *       NOTE: later this V[F/D/I]LDS will be changed into two inst in ra:
 *           V[F/D/I]ZERO vr
 *           V[F/D/I]MOVS vr, r, vr
 *           so, upper elements of vector will be zerod. 
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
   const int nivinst = 9;
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
   if (IsSIMDalignLoopPeelable(lp) && !SKIP_PEELING)
      GenForceAlignedPeeling(lp);
#endif   

#if 0
   fprintf(stdout, "LIL After Peeling\n");
   PrintInst(stdout, bbbase);
#endif
/*
 * Generate scalar cleanup loop before simdifying loop
 * NOTE: NO_CLEANUP markup no longer used for vect
 */
   
   /*if (!lp->LMU_flag & LMU_NO_CLEANUP)*/
   if (!SKIP_CLEANUP)
      GenCleanupLoop(lp);
   else
      Set_OL_NEINC_One();
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
   if (!SKIP_PELOGUE)
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

int RcVectorization(LOOPQ *lp)
{
   int fstat; 
   /*LOOPQ *lp;*/
   extern LOOPQ *optloop;

   /*lp = optloop;*/
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
   short op, id;
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
               if (op > 0 && NonLocalDeref(op) )
               {
                  k = STpts2[op-1];
#if 0
                  if (!FindInShortList(aptrs[0], aptrs+1, k)
                        && FindInShortList(lp->varrs[0],lp->varrs+1, k))
                  {
                     ip->inst[0] = vualign[j];
                  }
#else
/*
 *                check 2D array and skip if it is in aptrs
 */
                  id = STarrColPtrlookup(k);
                  if (id && STarr[id-1].ndim >= 2) /* 2d array */
                  {
                     if (!FindInShortList(aptrs[0], aptrs+1, STarr[id-1].ptr))
                     {
                        if (!FindInShortList(aptrs[0], aptrs+1, k))
                           ip->inst[0] = vualign[j];
                        else /* found in aptr */
                        {
                           if (SToff[op-1].sa[1]) /* lda active? */
                              ip->inst[0] = vualign[j];
                        }
                     }
                  }
                  else /* 1D */
                  {
                     if (!FindInShortList(aptrs[0], aptrs+1, k) 
                           || SToff[op-1].sa[1] ) // index var?  
                        ip->inst[0] = vualign[j];
                  }

#endif
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
   int i, j, k, n, m;
   int halign;
   short reg0, reg1;
   short lsAlabel,jBlabel; 
   short faptr, ptr;
   short *aptrs;
   short *s, id;
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
   faptr = lp->fa2vlen;
   /*assert(faptr);*/
   if (!faptr)
   {
      fko_warn(__LINE__, "No ptr to force align, skip specialization!");
      return;
   }
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
/*
 * FIXME: need to consider 2D array pointers
 * main idea: if faptr has one of the col ptr of 2D array and they are aligned
 * or mutually aligned with vector-length, all the col ptrs can be treated as
 * aligned.
 */
#if 0 
   i = n = 0;
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
#else
   s = malloc(sizeof(short)*(lp->varrs[0]+1)); // max moving ptr
   assert(s);
   aptrs = NULL;
/*
 * aadup faptr. faptr is col ptr incase of 2D array ptr
 * NOTE: add original 2D array pointer if all col ptrs are aligned/mutually 
 * aligned
 */
   k = 1;
   //s[k++] = faptr;
/*
 * addup all aligned ptrs
 * NOTE: all aligned case is already tested in IsAlignLoopSpecNeeded()
 */
   if (lp->aaligned)
   {
      for (i=1, n=lp->aaligned[0]; i <= n; i++)
      {
         if (lp->abalign[i] >= GetVecAlignByte())
         {
            id = STarrlookup(lp->aaligned[i]);
            if (id && STarr[id-1].ndim >= 2)
            {
#if 0
               for (j=1, m=STarr[id-1].colptrs[0]; j <= m; j++)
               {
                  if (FindInShortList(lp->varrs[0], lp->varrs+1, 
                           STarr[id-1].colptrs[j]))
                  {
                     s[k++] = STarr[id-1].colptrs[j];
                  }
               }
#else
/*
 *             add original 2D array... otherwise, it would be tough to manage 
 *             in case -a applied
 */
               s[k++] = lp->aaligned[i]; 
#endif
            }
            else
            {
               s[k++] = lp->aaligned[i];
            }
         }
      }
   }
/*
 * addup mutually aligned ptr with fptr. fptr can be a col ptr 
 * all mutually align case has been already tested  in IsAlignLoopSpecNeeded()
 */
   if (lp->maaligned)
   {
      id = STarrColPtrlookup(faptr);
      if (id)
      {
         if ( (i=FindInShortList(lp->maaligned[0], lp->maaligned+1, 
                     STarr[id-1].ptr)) )
         {
            s[k++] = lp->maaligned[i]; 
         }
      }
      else
      {
         if (FindInShortList(lp->maaligned[0], lp->maaligned+1, faptr))
         {
            for (i=1, n=lp->maaligned[0]; i <= n; i++)
            {
               s[k++] = lp->maaligned[i];
            }
         }
      }
   }
/*
 * if fpatr is not added, add it
 */
   if (!FindInShortList(k-1, s+1, faptr))
      s[k++] = faptr;
/*
 * now copy it aptrs
 */
   aptrs = malloc(sizeof(short)*k);
   assert(aptrs);
   aptrs[0] = n = k-1;
   for (i=1; i <= n; i++)
      aptrs[i] = s[i];
   free(s);
#endif

#if 1
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
 * assignment also assumes preheader and posttail which may not match with 
 * loop-info..
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
#if 0         
         if (FindInShortList(aptrs[0], aptrs+1, lp->varrs[i])) 
            continue;
#else
         if (FindInShortList(aptrs[0], aptrs+1, lp->varrs[i])) 
            continue;
         else if ( (id = STarrColPtrlookup(lp->varrs[i])))
         {
            if (FindInShortList(aptrs[0], aptrs+1, STarr[id-1].ptr))
               continue;
         }
#endif
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
#if 0         
         if (FindInShortList(aptrs[0], aptrs+1, lp->varrs[i])) 
            continue;
#else
         if (FindInShortList(aptrs[0], aptrs+1, lp->varrs[i])) 
            continue;
         else if ( (id = STarrColPtrlookup(lp->varrs[i])))
         {
            if (FindInShortList(aptrs[0], aptrs+1, STarr[id-1].ptr))
               continue;
         }
#endif
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
   if (pbase) KillAllPtrinfo(pbase);
   if (sp) free(sp);
}

/*=============================================================================
 *       SLP Vectorization
 *    ----------------------
 *    Steps:
 *    1. find adjacent memory ref
 *    2. create initial pack with them (seed)
 *    3. extend pack using def-use and use-def chains
 *    4. schedule the packs and test dependency 
 *
 *
 *===========================================================================*/
int FindPtrInfoFromDT(INSTQ *ip, short *ptr, short *lda, int *offset, int *mul)
{
   short reg1, reg2;
   short dt;
   INSTQ *ip0;
/*
 * inst can be both load and store
 */
#if 0   
   if (ip->inst[2] <= 0 )
      return(0);
#endif
   
   if (IS_LOAD(ip->inst[0]))
      dt = ip->inst[2];
   else if (IS_STORE(ip->inst[0]))
      dt = ip->inst[1];
   else 
      fko_error(__LINE__, "Not a DT of ld/st inst!");

   assert(ip->prev); /*ip has DT, must have a pred */
   
   *offset = SToff[dt-1].sa[3]; // off*sizeof 
   *mul = SToff[dt-1].sa[2];
   reg1 = SToff[dt-1].sa[0];
   reg2 = SToff[dt-1].sa[1];
   *lda = 0;
   
   ip0 = ip->prev;

   while (ip0 && IS_LOAD(ip0->inst[0]))
   {
      if (reg2 && reg2 == ip0->inst[1])
         *lda = STpts2[ip0->inst[2]-1];
            
      if (reg1 == ip0->inst[1])
         *ptr = STpts2[ip0->inst[2]-1];
            
      ip0 = ip0->prev;
   }
#if 0   
   fprintf(stderr, "(base,index, mul), offset: (%s,%s,%d), %d \n", 
           *ptr ? STname[*ptr-1] : "NULL", 
           *lda ? STname[*lda-1] : "NULL",
           *mul, *offset);
#endif
   *offset /= type2len(FLAG2TYPE(STflag[*ptr-1]));
   return(1);
}

ILIST **GetIlist(ILIST **ilam, int oldN, int nN)
{
   int i;
   ILIST **ilb;
   ILIST *il;

   ilb = malloc(nN * sizeof(ILIST*));
   assert(ilb);

   for (i=0; i < oldN; i++)
   {
      il = ilam[i];
      ilb[i] = NULL;
      while(il)
      {
         ilb[i] = NewIlistAtEnd(il->inst, ilb[i]);
         il = il->next;
      }
      KillAllIlist(ilam[i]);
   }
   free(ilam);
   return(ilb);
}

ILIST **FindAdjMemAccess(INSTQ *ipscope, int nptr, int *na, int isload)
{
   int i, j, m, cam, offset, ioffset, mul, imul;
   short ptr, lda;
   short iptr, ilda;
   INSTQ * ip;
   ILIST *il, *il0, *il1, *il2;
   ILIST **ilam, **ilb;
/*
 * allocate mem to store ilist
 */
   m = nptr;
   ilam = calloc(sizeof(ILIST*), m);
   assert(ilam);
/*
 * NOTE: here, we consider mem access, either load or store
 */
   for (ip=ipscope; ip; ip=ip->next)
   {
      if (!ACTIVE_INST(ip->inst[0]) || IS_PREF(ip->inst[0])
            || IS_BRANCH(ip->inst[0])) /* skip inactive, pref and branch inst */
         continue;
      
      if ( (isload && IS_LOAD(ip->inst[0]) && NonLocalDeref(ip->inst[2])) 
            || (!isload && IS_STORE(ip->inst[0]) && NonLocalDeref(ip->inst[1])))
      {
/*
 *       findout ptr, lda, mul  and offset
 */
         FindPtrInfoFromDT(ip, &ptr, &lda, &offset, &mul);
/*
 *       sort by mem offset
 */
         for (i=0; i < m; i++)
         {
            if (ilam[i] && ilam[i]->inst)
            {
              iptr = ilda = 0; 
              ioffset = 0;
              il0 = ilam[i]; 
              FindPtrInfoFromDT(il0->inst, &iptr, &ilda, &ioffset, &imul); 
              if (ptr == iptr && lda == ilda && mul == imul)
              {
                  il1 = NULL;
                  il2 = il0;
                  /*while (il2 && offset > 
                        SToff[il2->inst->inst[2]-1].sa[3] / 
                        type2len(FLAG2TYPE(STflag[ptr-1])) ) */
/*
 *                NOTE: ilam[] constains inst with sorted list
 *                FIXME: same offset is skiped, must be handled in initpack
 */
                  while (il2 && offset > ioffset) 
                  {
                     il1 = il2;
                     il2 = il2->next;
                     if (il2)
                     {
                     #if 0
                        FindPtrInfoFromDT(il2->inst, &iptr, &ilda, &ioffset, 
                           &imul);
                     #else
                        if (isload)
                           ioffset = SToff[il2->inst->inst[2]-1].sa[3] /
                                 type2len(FLAG2TYPE(STflag[ptr-1]));
                        else
                           ioffset = SToff[il2->inst->inst[1]-1].sa[3] /
                                 type2len(FLAG2TYPE(STflag[ptr-1]));
                     #endif
                     }
                  }
                  if (!il1)
                  {
                     /*fprintf(stderr, "%d offset added at the beginning\n",
                               offset);*/
                     ilam[i] = NewIlist(ip, ilam[i]);
                  }
                  else
                  {
                     /*fprintf(stderr, "%d offset added in between\n",offset);*/
                     NewIlistInBetween(ip, il1, il2);
                  }
                  break;
              }
            }
            else
            {
               /*fprintf(stderr, "*****inserting (ptr,lda,mul,off)=(%s,%s,%d,%d)"
                       " in index=%d\n", 
                     STname[ptr-1], lda ? STname[lda-1] : "NULL", mul, offset,
                     i);*/
               ilam[i] = NewIlist(ip, ilam[i]);
               break;
            }
         }
      }
   }
#if 0
   {
      short ptr, lda;
      int offset, mul;
      fprintf(stderr, "\nPrinting the ilists: \n");
      fprintf(stderr, "---------------------\n");
      for (i=0; i < m; i++)
      {
         il = ilam[i];
         if (il && il->inst)
         {
            FindPtrInfoFromDT(il->inst, &ptr, &lda, &offset, &mul);
            fprintf(stderr, " %s(%s) : ", STname[ptr-1], 
                  lda ? STname[lda-1]: "NULL");
            while (il)
            {
               if (isload)
                  fprintf(stderr, "%d ", SToff[il->inst->inst[2]-1].sa[3]);
               else
                  fprintf(stderr, "%d ", SToff[il->inst->inst[1]-1].sa[3]);
               il = il->next;
            }
            fprintf(stderr, "\n");
         }
         else break;
      }
   }
#endif
/* start 2m entry... double it, if needed.*/
   cam = 2 * m;
   ilb = calloc(sizeof(ILIST*), cam);
   assert(ilb);
/*
 * check whether memory access are adjacent. Split and keep adj mem access in 
 * one list
 */

   for (i=0, j=0; i < m; i++)
   {
      il = ilam[i];
      if (il && il->inst)
      {
         if ( j == cam)
         {
            ilb = GetIlist(ilb, cam, cam+cam);
            cam = cam + cam;
         }
         ilb[j++] = NewIlist(il->inst, NULL);  

         FindPtrInfoFromDT(il->inst, &ptr, &lda, &offset, &mul);
         il = il->next;
         while (il)
         {
            if (isload)
               ioffset = SToff[il->inst->inst[2]-1].sa[3] 
                      / type2len(FLAG2TYPE(STflag[ptr-1]));
            else
               ioffset = SToff[il->inst->inst[1]-1].sa[3] 
                      / type2len(FLAG2TYPE(STflag[ptr-1]));

            if (ioffset == (offset + 1))
            {
               ilb[j-1] = NewIlistAtEnd(il->inst, ilb[j-1]);
            }
            else /* not adjacent, split it */
            {
               if (j == cam)
               {
                  ilb = GetIlist(ilb, cam, cam+cam);
                  cam = cam + cam;
               }
               ilb[j++] = NewIlist(il->inst, NULL);  
            }
            offset = ioffset;
            il = il->next;
         }
/*
 *       time to delete the row list
 */
         KillAllIlist(ilam[i]);
      }
   }
   free(ilam);
/*
 * keep all row with adj access 
 */
   for (i=0, j=0; i < cam; i++)
   {
      if (ilb[i] && ilb[i]->next) /* potential one*/
         ilb[j++] = ilb[i];
      else /*if (ilb[i] && !ilb[i]->next)*/ /* single entry */
         KillIlist(ilb[i]);

   }
   *na = j;
   if (!j) /* no adj mem */
   {
      free(ilb);
      return(NULL);
   }
   else
      return(ilb);
}

ILIST **FindAdjMem(INSTQ *ipscope, int *npt)
{
/*
 * main idea: 
 *    if base and index(lda) are same, we need to check the offset for adjacency
 *    will use ILIST: each entry of ILIST has adjacent memory load (from lower 
 *    offset to higher offset).
 *       HIL: x = X[2];                         HIL: x = X[3][1] 
 *       LIL:                                   LIL: 
 *          LD ir, X                               LD ir1, X
 *          FLD fr, x                              LD ir2, _ldas_ 
 *          FLD fr, DT (ir, 0, 0, 2*size )         FLD fr, x
 *          FST x,fr                               FLD fr, DT (ir1, ir2, 1, 3) 
 *                                                 ST x, fr
 */
   int i, j, k, m, n, cpt;
   short sta;
   INSTQ *ip;
   ILIST *il;
   ILIST **ilam = NULL, **ilb=NULL;
   struct ptrinfo *pi, *pbase = NULL;
/*
 * find ptr for mem access 
 */
   m = 0;
   for (ip=ipscope; ip; ip=ip->next)
   {
      if (!ACTIVE_INST(ip->inst[0]) || IS_PREF(ip->inst[0])
            || IS_BRANCH(ip->inst[0])) /* skip inactive, pref and branch inst */
         continue;
      for (i=1; i < 4; i++)
      {
         k = ip->inst[i];
         if (k > 0 && IS_DEREF(STflag[k-1]) && !NonLocalDeref(k))
         {
            k = FindLocalFromDT(k);
            if (k && IS_PTR(STflag[k-1]))
            {
               pi = FindPtrinfo(pbase, k);
               if (!pi)
               {
                  pbase = NewPtrinfo(k, 0, pbase);
                  sta = STarrColPtrlookup(k);
                  if(sta && STarr[sta-1].ndim > 1)
                     m += SToff[STarr[sta-1].urlist[0]-1].i;
                  else m++;
               }
            }
         }
      }
   }
   KillAllPtrinfo(pbase);
/*
 * no ptr access, return NULL
 */
   if (!m)
   {
      *npt=0;
      return(NULL);
   }
/*
 * find out adjacent mem loads
 */
   ilam = FindAdjMemAccess(ipscope, m, &cpt, 1);
   
/*
 * if there is no adj mem loads, try out mem stores
 */
   if (!cpt)
   {
      ilam = FindAdjMemAccess(ipscope, m, &cpt, 0);
   }
   *npt = cpt;
   
   if (!cpt)
      return(NULL);

#if 0
   {
      short ptr, lda;
      int offset, mul;
      fprintf(stderr, "\nPrinting the ilists: \n");
      fprintf(stderr, "---------------------\n");
      for (i=0; i < m; i++)
      {
         il = ilam[i];
         if (il && il->inst)
         {
            FindPtrInfoFromDT(il->inst, &ptr, &lda, &offset, &mul);
            fprintf(stderr, " %s(%s) : ", STname[ptr-1], 
                  lda ? STname[lda-1]: "NULL");
            while (il)
            {
               fprintf(stderr, "%d ", SToff[il->inst->inst[2]-1].sa[3]);
               il = il->next;
            }
            fprintf(stderr, "\n");
         }
         else break;
      }
   }
#endif
/*
 * copy the exact cpt ilists
 * FIXME: sort the ilists based on number adj access
 */
   ilb = malloc(cpt*sizeof(ILIST*));
   assert(ilb);
   for (i=0; i < cpt; i++)
   {
      il = ilam[i];
      ilb[i] = NULL;
      while(il)
      {
         ilb[i] = NewIlistAtEnd(il->inst, ilb[i]);
         il = il->next;
      }
      KillAllIlist(ilam[i]);
   }
   free(ilam);
/*
 * Now sort the list based on number of access 
 * used simple selection 
 */
   for (i=0; i < cpt; i++)
   {
      for (m=0, il=ilb[i]; il; il=il->next, m++)
         ;
      k = i;
      for (j=i+1; j < cpt; j++) /* iamax */
      {
         for (n=0, il=ilb[j]; il; il=il->next, n++)
            ;
         if (n > m) 
         {
            k = j;
            m = n;
         } 
      }
      if ( k != i) /* found the largest, swap it */
      {
         il = ilb[i];
         ilb[i] = ilb[k];
         ilb[k] = il;
      }
   }
#if 0
   {
      short ptr, lda;
      int offset, mul;
      fprintf(stderr, "\nPrinting the ilists: (after sorting) \n");
      fprintf(stderr, "---------------------\n");
      for (i=0; i < cpt; i++)
      {
         il = ilb[i];
         if (il && il->inst)
         {
            FindPtrInfoFromDT(il->inst, &ptr, &lda, &offset, &mul);
            fprintf(stderr, " %s(%s,%d) : ", STname[ptr-1], 
                  lda ? STname[lda-1]: "NULL", mul);
            while (il)
            {
               fprintf(stderr, "%d ", SToff[il->inst->inst[2]-1].sa[3]);
               il = il->next;
            }
            fprintf(stderr, "\n");
         }
         else break;
      }
   }
#endif 

   return(ilb);
}

INT_BVI FilterOutRegs(INT_BVI iv)
{
   int i;
   extern short STderef;
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);
   /*SetVecBit(iv, TNREG-1, 0);*/ // skip STderef... which is used for mem
   return(iv);
}

int IsSameInst(INSTQ *ip0, INSTQ *ip1)
{
   int i;
   int dt0, dt1;
   if (!ip0 || !ip1)
      return(0);

   for (i=0; i < 4; i++)
   {
/*
 *    is nonlocal deref? we create separate entry for each such entry
 *    need to check element of dt to test equality
 */
      if (ip0->inst[i] != ip1->inst[i])
      {
         if (i > 0 && ip0->inst[i] > 0 && ip1->inst[i] > 0)
         {
            dt0 = ip0->inst[i];
            dt1 = ip1->inst[i];
            if (NonLocalDeref(dt0) && NonLocalDeref(dt1))
            {
#if 0               
               fprintf(stderr, "DT0=%d, DT1=%d\n", dt0, dt1);
#endif
               if ( SToff[dt0-1].sa[0] != SToff[dt1-1].sa[0]
                     || SToff[dt0-1].sa[1] != SToff[dt1-1].sa[1]
                     || SToff[dt0-1].sa[2] != SToff[dt1-1].sa[2]
                     || SToff[dt0-1].sa[3] != SToff[dt1-1].sa[3]) 
                  break;
            }
            else
               break;
         }
         else
            break;
      }
   }
   if (i != 4)
   {
      /*fprintf(stderr, "ip0 and ip1 are NOT same\n");*/
      return(0);
   }
   else
   {
#if 0      
      fprintf(stderr, "ip0 and ip1 are same\n");
      PrintThisInst(stderr, 1, ip0);
      PrintThisInst(stderr, 2, ip1);
#endif
      return(1);
   }
}


INSTQ *DelMatchInstQ(INSTQ *instq, INSTQ* delq)
/*
 * search sunstring problem!
 * When bp is not NULL, treat instq as inst1 of the block
 */
{
   int k;
   int del = 0;
   /*int dt0, dt1;*/
   INSTQ *ip0, *ip1, *ipd, *ip;
#if 0
   fprintf(stderr, "deleted inst:\n");
   PrintThisInstQ(stderr, delq);
   fprintf(stderr, "----------\n");
   PrintThisInstQ(stderr, instq);
   fprintf(stderr, "----------\n");
#endif
/*
 * brute force algo: complexity : m x n
 */
   assert(instq);
   
   ip0 = ip = instq;
   while(ip)
   {
      ip1 = ip;
      ipd = delq;
      k = 1;
      for ( ; ipd && ip1; ipd = ipd->next, ip1 = ip1->next)
      {
#if 0         
         for (i=0; i < 4; i++)
         {
/*
 *          FIXME: simplify it 
 */
            if (ipd->inst[i] != ip1->inst[i])
            {
               if (i > 0 && ipd->inst[i] > 0 && ip1->inst[i] > 0)
               {
                  dt0 = ipd->inst[i];
                  dt1 = ip1->inst[i];
                  if (NonLocalDeref(dt0) && NonLocalDeref(dt1))
                  {
                     if ( SToff[dt0-1].sa[0] != SToff[dt1-1].sa[0]
                        || SToff[dt0-1].sa[1] != SToff[dt1-1].sa[1]
                        || SToff[dt0-1].sa[2] != SToff[dt1-1].sa[2]
                        || SToff[dt0-1].sa[3] != SToff[dt1-1].sa[3]) 
                        k = 0;
                     break;
                  }
                  else
                  {
                     k = 0;
                     break;
                  }
               }
               else
                  k = 0;
               break;
            }
         }
#else
         k = IsSameInst(ipd, ip1);
#endif
         if (!k)
            break;
      }
/*
 *    delete inst if all inst in queue are same
 */
      if (!k)
         ip = ip->next;
      else
      {
         del = 1;       /*start deleting  */
         ipd = delq;
         while (ipd)
         {
            if (!ip->myblk) /* instq without block*/
            {
               if (ip->prev) 
                  ip->prev->next = ip->next;
               if (ip->next) 
                  ip->next->prev = ip->prev;
               //PrintThisInst(stderr, 777, ip);  
               if (ip == ip0) /* matching at top, update instq ptr*/
               {
                  ip = KillThisInst(ip);
                  instq = ip0 = ip;
               }
               else
                  ip = KillThisInst(ip);
            }
            else  /* instq in a block */
            {
               ip = DelInst(ip);
               //instq = ip; 
               instq = ip; 
            }
            ipd = ipd->next; 
         }
         break; /* single occurance */
      }
   }
   if (!del)
      fko_error(__LINE__, "Inst not found! something is wrong!!!");
   
   return(instq); 
}

void PrintPack(FILE *fp, PACK *pk)
{
   ILIST *il;
   if (!pk)
      return;

   fprintf(fp, "***PACK %d: \n", pk->pnum);
   fprintf(fp, "len=%d\n", pk->vlen);
   il = pk->sil;
   while (il)
   {
      fprintf(stderr, "print instq :\n");
      PrintThisInstQ(stderr, il->inst);
      il = il->next;
   }
#if 0   
   if (pk->depil)
   {
      fprintf(stderr, "depends upon: \n");
      fprintf(stderr, "-------------\n");
      il = pk->depil;
      while (il)
      {
         fprintf(stderr, "dep instq :\n");
         PrintThisInst(stderr, 0, il->inst);
         il = il->next;
      }
   }
#endif
}

INSTQ *FinalizePack(PACK *pk, INSTQ *uinst, int *change)
/*
 * check dependency within the pack and finalize it if valid, delete inst from
 * upack queue; otherwise delete the pack
 */
{
   int count;
   ILIST *il;
   INT_BVI uses, sets, iv, puses;
   INSTQ *ip;
   short ptr, lda, iptr, ilda;
   int mul, offset, imul, ioffset;

   extern INT_BVI FKO_BVTMP;

   if (!pk) return(uinst);

#if 0
   PrintPack(stderr, pk);
#endif

 /* don't allow that. all dependencies should be removed by scalar expansion 
 * before applying SLP.
 * Remove pack if 
 *    1) dependence in a pack
 *    2) vlen is less than accual vlen ( will extend that to relax later )
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv,0);
   
   uses = NewBitVec(32);
   sets = NewBitVec(32);
   puses = NewBitVec(32);

   count = 0;
   il = pk->sil;
   while(il)
   {
      ip = il->inst;
      SetVecAll(uses, 0);
      SetVecAll(sets, 0);
      while(ip)
      {
         uses = BitVecComb(uses, uses, ip->use, '|');
         sets = BitVecComb(sets, sets, ip->set, '|');
         ip = ip->next;
      }
/*
 *    check dependence in pack:
 *    Raw, War, waw  ==> variable set in a inst can't be set or use in other 
 *    inst of same pack
 */
      iv = BitVecCopy(iv, sets); 
      FilterOutRegs(iv);

      if (BitVecCheckComb(iv, pk->defs, '&')) /* waw */
      {
         #if IFKO_DEBUG_LEVEL > 1
                  fprintf(stderr, "Common variable!! : with sets\n");
                  PrintVars(stderr, "pk->sets", pk->defs);
                  PrintVars(stderr, "sets", iv);
         #endif
         pk->isVec = 0;
         fko_warn(__LINE__, "pack-%d has dependency : WAW", pk->pnum);
      }
      
      pk->uses = BitVecComb(pk->uses, pk->uses, uses, '|');
      pk->defs = BitVecComb(pk->defs, pk->defs, sets, '|');
/*
 *    find out only those uses which not defs in same instruction
 *    example: rC00 += rA0 * rB0; we want to scope out rA0, rB0 (rC00 skipped)
 */
      uses = BitVecComb(uses, uses, sets, '-');
      puses = BitVecComb(puses, puses, uses, '|');
      count++;
      il = il->next;
   }
/*
 * check RAW and WAR (inter instruciton ) 
 * 
 */
   iv = BitVecCopy(iv, puses); 
   FilterOutRegs(iv);
   if (BitVecCheckComb(iv, pk->defs, '&'))
   {
      #if IFKO_DEBUG_LEVEL > 1
         fprintf(stderr, "Common variable!! : \n");
         PrintVars(stderr, "pk->sets", pk->defs);
         PrintVars(stderr, "pk->uses", iv);
      #endif
      fko_warn(__LINE__, "pack-%d has dependency : RAW / WAR", pk->pnum);
      pk->isVec = 0;
   }
/*
 * FIXED: check whether they are memory write/read. They must be 
 * consecutive to form a pack!!
 * For Now: we don't allow multiple memory load in same LIL block... will 
 * extend later
 */
   ptr = 0; 
   for (il=pk->sil; il; il=il->next)
   {
      ip=il->inst;
      while (ip)
      {
         if (IS_LOAD(ip->inst[0]) && NonLocalDeref(ip->inst[2]))
         {
            if (!ptr)
            {
               FindPtrInfoFromDT(ip, &ptr, &lda, &offset, &mul);
            }
            else
            {
               FindPtrInfoFromDT(ip, &iptr, &ilda, &ioffset, &imul);
               
               if (ptr != iptr || lda != ilda || mul != imul)
               {
                  fprintf(stderr, "<%s,%s,%d> ... <%s,%s,%d>", 
                          ptr ? STname[ptr-1] : "NULL", 
                          lda ? STname[lda-1] : "NULL", mul,
                          iptr ? STname[iptr-1] : "NULL", 
                          ilda ? STname[ilda-1] : "NULL", imul
                          );
                  fko_error(__LINE__, "diff ptrs!!!");
               }
               
               if (ioffset != offset + 1)
               {
/*
 *                we support broadcast.. so, all access may point to same loc
 */
                  if (ioffset != offset)
                     fko_error(__LINE__, "mem access not consecutive: %d, %d!", 
                           offset, ioffset);
               }
               offset = ioffset;
            }
         }
/*
 *       assuming these two cases are disjoint
 */
         else if (IS_STORE(ip->inst[0]) && NonLocalDeref(ip->inst[1]))
         {
            if (!ptr)
            {
               FindPtrInfoFromDT(ip, &ptr, &lda, &offset, &mul);
            }
            else
            {
               FindPtrInfoFromDT(ip, &iptr, &ilda, &ioffset, &imul);
               if (ptr != iptr || lda != ilda || mul != imul)
               {
                  fprintf(stderr, "<%s,%s,%d> ... <%s,%s,%d>", 
                          ptr ? STname[ptr-1] : "NULL", 
                          lda ? STname[lda-1] : "NULL", mul,
                          iptr ? STname[iptr-1] : "NULL", 
                          ilda ? STname[ilda-1] : "NULL", imul
                          );
                  fko_error(__LINE__, "diff ptrs!!!");
               }
               if (ioffset != offset + 1)
                  fko_error(__LINE__, "mem access not consecutive: %d, %d!",
                        offset, ioffset);
               offset = ioffset;
            }
         }
         ip = ip->next;
      }
   }

#if 0   
   pk->vlen = count;
   len = Type2Vlen(pk->vflag);
   if (pk->vlen != len)
      pk->isVec = 0;
#endif
/*
 * delete the pack if not vectorizable
 */
   if (!pk->isVec)
   {
      *change = 0;
      KillPack(pk);
   }
/*
 *       pack is valid, delete the inst from original queue
 *       Assuming no redundant inst exists 
 */
   else
   {
      il = pk->sil;
      while(il)
      {
         uinst = DelMatchInstQ(uinst, il->inst);
         il = il->next;
      }
      *change = 1;
   }
   KillBitVec(uses);
   KillBitVec(sets);
   KillBitVec(puses);
#if 0
   {
      int i;
      fprintf(stderr, "Printing Packq:\n");
      for (i=0; i < NPACK; i++)
      {
         if (PACKS[i]) 
         {
            fprintf(stderr, "***PACK %d: \n", i);
            fprintf(stderr, "len=%d\n", PACKS[i]->vlen);
            il = PACKS[i]->sil;
            while (il)
            {
               fprintf(stderr, "print instq :\n");
               PrintThisInstQ(stderr, il->inst);
               il = il->next;
            }
         }
      }
      fprintf(stderr, "\nPrinting uPackq:\n");
      PrintThisInstQ(stderr, uinst); 
   }
#endif
   return(uinst);
}

INSTQ *InitPack(ILIST **ilam, int nptr, INSTQ *uinst, short **pts)
{
   int i, j, ci;
   short ptr, lda;
   short *sp;
   int offset, len, mul;
   ILIST *il, *sil = NULL;
   INSTQ *ip0, *ip, *ipdup;
   PACK *pk;
   INT_BVI iv, uses, sets;
   INT_BVI puses;
   extern INT_BVI FKO_BVTMP;
#if 0
   fprintf(stderr, "\nPrinting the ilists: \n");
   fprintf(stderr, "---------------------\n");
   for (i=0; i < nptr; i++)
   {
      il = ilam[i];
      if (il && il->inst)
      {
         FindPtrInfoFromDT(il->inst, &ptr, &lda, &offset, &mul);
         fprintf(stderr, " %s(%s,%d) : ", STname[ptr-1], 
               lda ? STname[lda-1]: "NULL", mul);
         while (il)
         {
            fprintf(stderr, "%d ", SToff[il->inst->inst[2]-1].sa[3]);
            il = il->next;
         }
         fprintf(stderr, "\n");
      }
      else break;
   }
   exit(0);
#endif
   
/*
 * create packs based on the memory access
 */
   sp = malloc((nptr+1)*sizeof(short));
   assert(sp);
   sp[0] = nptr;
   for (i=0; i < nptr; i++)
   {
      il = ilam[i];
      FindPtrInfoFromDT(il->inst, &ptr, &lda, &offset, &mul);
      len = Type2Vlen(FLAG2TYPE(STflag[ptr-1]));
      sp[i+1] = ptr;
      while (il)
      {
         sil = NULL;
         ci = 0;
         for (j=0; j < len; j++)
         {
            /*ip = il->inst;*/
            ip = il->inst->prev; /* may be a mem store */
            while (IS_LOAD(ip->inst[0]))
            {
               ip0 = ip;
               ip = ip->prev;
            } 
            ip = ip0; 
            ipdup = ip0 = NULL;

            do
            {
               ipdup = NewInst(NULL, ipdup, NULL, ip->inst[0], ip->inst[1], 
                                  ip->inst[2], ip->inst[3]);
               CalcThisUseSet(ipdup);
               if (ip0)
                  ip0->next = ipdup;
               ip0 = ipdup;
               /*for (k=1; k < 4; k++)
               {
                  dt = ipdup->inst[k];
                  if ( dt > 0 && NonLocalDeref(dt))
                     ipdup->inst[k] = AddDerefEntry(SToff[dt-1].sa[0], 
                                                 SToff[dt-1].sa[1],
                                                 SToff[dt-1].sa[2],
                                                 SToff[dt-1].sa[3],
                                                 STpts2[dt-1]);
               }*/
               ip=ip->next;
            } while (!IS_STORE(ipdup->inst[0])); /* pref not added in pack*/ 
/*
 *          go to starting of the instq 
 */
            while(ipdup->prev) ipdup = ipdup->prev;
            ci++; /* count instq */
            sil = NewIlistAtEnd(ipdup, sil);
            il = il->next;
            if(!il) break;
         }
         pk = NewPack(sil, ci);
         pk->vflag = FLAG2TYPE(STflag[ptr-1]);
      }
   }
/*
 * Finalize the pack:
 * check for dependecy inside a pack. We have only mem loads here and we can 
 * assume that memory access are all consecutive (checked before). So, only 
 * dependency occurs if we load memory in same variable. At that stage, we
 * don't allow that. all dependencies should be removed by scalar expansion 
 * before applying SLP.
 * Remove pack if 
 *    1) load memory in same variable in a pack
 *    2) vlen is less than accual vlen ( will extend that to relax later )
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv,0);
   uses = NewBitVec(32);
   sets = NewBitVec(32);
   puses = NewBitVec(32);
   SetVecAll(puses, 0);
   for (i=0; i < NPACK; i++)
   {
      pk = PACKS[i];
      if (pk)
      {
         il = pk->sil;
         while(il)
         {
            ip = il->inst;
            SetVecAll(uses, 0);
            SetVecAll(sets, 0);
            while(ip)
            {
               uses = BitVecComb(uses, uses, ip->use, '|');
               sets = BitVecComb(sets, sets, ip->set, '|');
               ip = ip->next;
            }
            iv = BitVecCopy(iv, sets); 
            FilterOutRegs(iv);
/*
 *          check write dependence among load insts
 *          pk->defs : created with 0 when pack is created
 */
            if (BitVecCheckComb(iv, pk->defs, '&'))
            {
               fko_warn(__LINE__, "Pack-%d not vectorizable due to WAW", 
                     pk->pnum);
               #if IFKO_DEBUG_LEVEL > 1
                  fprintf(stderr, "Common variable!! : \n");
                  PrintVars(stderr, "pk->sets", pk->defs);
                  PrintVars(stderr, "sets", iv);
               #endif
               pk->isVec = 0;
            }
            pk->uses = BitVecComb(pk->uses, pk->uses, uses, '|');
            pk->defs = BitVecComb(pk->defs, pk->defs, sets, '|');
            il = il->next;
         }
/*
 *       for simplicity, we won't handle pack with less than vlen inst
 */
         len = Type2Vlen(pk->vflag);
         /*fprintf(stderr, "pack=%d len=%d vlen=%d\n", i, len, pk->vlen);*/
         if (pk->vlen != len)
            pk->isVec = 0;
/*
 *       delete the pack if not vectorizable
 */
         if (!pk->isVec)
         {
            fko_warn(__LINE__, 
                  "Pack-%d not vectorizable due not to have vlen insts", 
                  pk->pnum);
            KillPack(pk);
         }
#if 1 
/*
 *       pack is valid, delete the inst from original queue
 *       Assuming no redundant inst exists 
 */
         else
         {
            pk->pktype |= PK_INIT_MEM;
            il = pk->sil;
            puses = BitVecComb(puses, puses, pk->uses, '|');
            while(il)
            {
               uinst = DelMatchInstQ(uinst, il->inst);
               il = il->next;
            }
         }
#endif
      }
   }
   KillBitVec(uses);
   KillBitVec(sets);
/*
 * Findout active pointers which is used inside the initial packs 
 */

   iv = Array2BitVec(nptr, sp+1, TNREG-1);
   free(sp);
   puses = BitVecComb(puses, puses, iv, '&');
   sp = BitVec2Array(puses, 1-TNREG);
   *pts = sp;
   KillBitVec(puses);
#if 0
   fprintf(stderr, "Printing Packq after InitPack:\n");
   for (i=0; i < NPACK; i++)
   {
      if (PACKS[i]) 
      {
         fprintf(stderr, "***PACK %d: \n", i);
         fprintf(stderr, "len=%d\n", PACKS[i]->vlen);
         il = PACKS[i]->sil;
         while (il)
         {
            fprintf(stderr, "print instq :\n");
            PrintThisInstQ(stderr, il->inst);
            il = il->next;
         }
      }
   }
   //fprintf(stderr, "\nPrinting uPackq:\n");
   //PrintThisInstQ(stderr, uinst); 
#endif
   return(uinst);
}

int CheckDupPackError(int ipack, int npack)
/*
 * NOTE: following useful checkings have alreadybeen done while creating the 
 * packs, like: 
 *    1. consecutive mem access for mem pack
 *    2. isomorphic inst set in pack
 *    3. data dependencies among the element of packs
 * 
 * Here, we only check whether two packs are duplicated. Our current version of
 * SLP doesn't support that. It complicates the search of pack from a single 
 * instruction of nsbp blk. one of the pack may be selected randomly but the 
 * dependency info are different for each pack. Currently, after applying our
 * MinPtrUpdate optmization, this restriction works for the generated AMM 
 * kernels. We need to remove this restriction eventually.
 *
 */
{
   int i, j;
   ILIST *ili, *ilj;
   INSTQ *ipi, *ipj;
   int same;
   PACK *pki, *pkj;
/*
 * count the packs. if we have only one pack, there is nothing wrong with it
 */
   for (i=ipack, j=0; i < npack; i++)
      if (PACKS[i])
         j++;
   if (j==1)
      return(0);
/*
 * if we have morethan one pack, no two pack can be same
 */
   for (i=ipack; i < npack-1; i++)
   {
      pki = PACKS[i];
      if (!pki) continue;
      /*fprintf(stderr, "****** Checking packs %d with:\n", i);*/
      for (j=i+1; j < npack; j++)
      {
         pkj = PACKS[j]; 
         if (!pkj) continue;
         /*fprintf(stderr, " %d ", j);*/
         for (ili=pki->sil, ilj=pkj->sil; ili && ilj; 
               ili=ili->next, ilj=ilj->next)
         {
            same = 1;
            for (ipi=ili->inst, ipj=ilj->inst; ipi && ipj; 
                  ipi=ipi->next, ipj=ipj->next)
            {
               if (!IsSameInst(ipi, ipj))
               {
#if 0
                  PrintThisInst(stderr, 1, ipi);
                  PrintThisInst(stderr, 1, ipj);
#endif
                  same = 0;;
                  break;
               }
            }
            if (same)
               return(1);
         }
      }
      /*fprintf(stderr, "\n");*/
   }
   return(0);
}

int IsIsomorphicInst(INSTQ *ip1, INSTQ *ip2)
{
   int i;
/*
 * two insts are isomorphic, if they has
 *    1. same operation
 *    2. same type of operands : var, const, register
 */
   if (ip1->inst[0] != ip2->inst[0])
   {
      /*fprintf(stderr, "Operation not match!\n");*/
      fko_warn(__LINE__, "Operation not match!\n");
      return(0);
   }
   for (i=1; i < 4; i++)
   {
      if (ip1->inst[i] > 0)
      {
         if (ip2->inst[i] <= 0 )
         {
            /*fprintf(stderr, "Operand=%d (%d, %d) not match!\n", i, ip1->inst[i],
                     ip2->inst[i]);*/
            fko_warn(__LINE__, "Operand=%d (%d, %d) not match!\n", i, ip1->inst[i],
                     ip2->inst[i]);
            return(0);
         }
         else if (STflag[ip1->inst[i]-1] != STflag[ip2->inst[i]-1])
         {
            /*fprintf(stderr, "Flag of Operand=%d not match!\n", i);*/
            fko_warn(__LINE__, "Flag of Operand=%d not match!\n", i);
            return(0);
         }
         
      }
      else if (!ip1->inst[i])
      {
         if (ip2->inst[i] != 0)
         {
            /*fprintf(stderr, "Operand=%d not zero!\n", i);*/
            fko_warn(__LINE__, "Operand=%d not zero!\n", i);
            return(0);
         }
      }
      else /* < 0 */
      {
         if (ip2->inst[i] >= 0)
         {
            /*fprintf(stderr, "Operand=%d not reg!\n", i);*/
            fko_warn(__LINE__, "Operand=%d not reg!\n", i);
            return(0);
         }
      }
   }
   return(1);
}

INSTQ *FindFirstLILforHIL(INSTQ *ipX)
/* Assumptions: 
 * 1. applied before repeatable inst, so, the initial LIL structure prevails
 * 2. ip is an active instq (not consists of comment or cmpflag)
 * returns the instq pointer of starting of LIL inst of a block 
 *    (converted from HIL)
 */
{
   INSTQ *ip, *ip0;
/*
 * return same ip if jmp/ret. They consist one LIL inst 
 */
   if (IS_BRANCH(ipX->inst[0]) && !IS_COND_BRANCH(ipX->inst[0])) /* JMP, RET */
      return(ipX);
   if (ipX->inst[0] == LABEL)
      return(ipX);
/*
 * last inst of a block would always be the store or branch 
 * so, check for the active inst which is successor of a store/branch/LABEL/NULL
 */
   if (IS_STORE(ipX->inst[0]) || IS_COND_BRANCH(ipX->inst[0]) 
         || IS_PREF(ipX->inst[0]))
      ip = ipX->prev;
   else 
      ip = ipX;
   ip0 = ip;
   while (ip && !IS_BRANCH(ip->inst[0]) && ip->inst[0] != LABEL 
         && !IS_STORE(ip->inst[0]) && !IS_PREF(ip->inst[0]))
   {
      if (ACTIVE_INST(ip->inst[0]))
         ip0 = ip;
      ip = ip->prev;
   }
/*
 * FIXME: for some specific compiler generated instructions, 1st inst may not be
 * the load of vars, like: 
 *    FZEROD reg0;
 *    FSTD v0, reg0;
 */
   //assert(IS_LOAD(ip0->inst[0]));
   return(ip0);
}
 
INSTQ *InstdefsVar(INT_BVI bvvar, INSTQ *upackq)
{
   /*int i, n;*/
   /*short *sp*/;
   /*INSTQ *ip, *ip0;*/
   INSTQ *ip;

   ip = upackq;
   while (ip)
   {
      //PrintThisInst(stderr, 777, ip);
      if (BitVecCheckComb(bvvar, ip->set, '&'))
      {
         //PrintThisInst(stderr, 1, ip);
         return(FindFirstLILforHIL(ip));
      }
      ip = ip->next;
   }
   return(NULL);
}

INSTQ *InstFirstUseVar(INT_BVI bvvar, INSTQ *upackq)
{
   INSTQ *ip;
   ip = upackq;

   while(ip)
   {
      if (BitVecCheckComb(bvvar, ip->use, '&'))
         return(FindFirstLILforHIL(ip));
      ip = ip->next;
   }
   return(NULL);
}

INSTQ *FollowUseDefs(PACK *pk, INSTQ *upackq, int *change)
{
   int i, n;
   int ch, suc;
   short *sp;
   ILIST *il, *iln;
   INSTQ *ip;
   INSTQ *ipu, *ipdup, *ip0;
   PACK *new;
   INT_BVI iv;
   extern INT_BVI FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(128);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0); 
/*
 * need to find out all inst where those variables are used which are defined 
 * in this pack. need to consider the order to create pack, so can't use pk->def
 * here.
 */
#if 0
   iv = BitVecCopy(iv, pk->uses);
   FilterOutRegs(iv);
   sp = BitVec2Array(iv, 1-TNREG);
   fprintf(stderr, "****** uses var count = %d\n", sp[0]);
   for (i=1, n=sp[0]; i <= n; i++)
      fprintf(stderr, "%s\n", STname[sp[i]-1]);
   fprintf(stderr, "======================================\n");

#endif
#if 0
   fprintf(stderr, "Print seed Pack before extending\n");
   fprintf(stderr, "======================================\n");
   PrintPack(stderr, pk);
   fprintf(stderr, "======================================\n");
#endif
/*
 * find out number of uses 
 */
   for (ip = pk->sil->inst; ip; ip=ip->next)
      iv = BitVecComb(iv, iv, ip->use, '|');
   FilterOutRegs(iv);
   sp = BitVec2Array(iv, 1-TNREG);
   n = sp[0];
   free(sp);

   *change = 0;
   for (i=1; i <= n; i++) /* apply uses by order */ 
   {
      suc = 1;
      iln = NULL;
      il = pk->sil;
      while(il)
      {
/*
 *       calc def for this instq
 */
         ip = il->inst;
         SetVecAll(iv, 0); 
         while(ip)
         {
            iv = BitVecComb(iv, iv, ip->use, '|');
            ip = ip->next;
         }
         FilterOutRegs(iv);
#if 0
         PrintVars(stderr, "all uses: ", iv);
         //fprintf(stderr, "upackq :\n");
         //fprintf(stderr, "==========================\n");
         //PrintThisInstQ(stderr, upackq);
         //fprintf(stderr, "==========================\n");
#endif
/*
 *       FIXME: ip->use can be multiple variables. We need to exlore all of them
 *       consequitively, like: all 1st operand then all 2nd operand, etc.
 */
         sp = BitVec2Array(iv, 1-TNREG);
         assert(i <= sp[0]);
         SetVecAll(iv, 0); 
         SetVecBit(iv, sp[i] + TNREG-1, 1); /* STderef = 0 otherwise add this */
         free(sp);
         /*PrintVars(stderr, "working var: ", iv);*/
         ipu = InstdefsVar(iv, upackq);
         if (ipu)
         {
            assert(IS_LOAD(ipu->inst[0]));
            ipdup = ip0 = NULL;
            do
            {
               ipdup = NewInst(NULL, ipdup, NULL, ipu->inst[0], ipu->inst[1], 
                     ipu->inst[2], ipu->inst[3]);
               CalcThisUseSet(ipdup);
               if (ip0)
                  ip0->next = ipdup;
               ip0 = ipdup;
               ipu = ipu->next;
            } while (!IS_STORE(ipdup->inst[0])); /* NOTE: no pref in pack */ 

            while(ipdup->prev) ipdup = ipdup->prev;
            iln = NewIlistAtEnd(ipdup, iln);
         }
         else 
         {
            /*fprintf(stderr, "NO match : uses_defs\n");*/
            suc = 0;
            /*return(upackq);*/
            break;
         }
         il = il->next;
      }
/*
 *    check whether inst are isomorphic, die otherwise
 *    FIXME: need to check again if there are not isomorphic...
 */
      if (suc)
      {
         for (il=iln; il; il=il->next)
         {
            ip0 = iln->inst;
            ip = il->inst;
            while (ip)
            {
               if (!IsIsomorphicInst(ip, ip0))
               {
               #if IFKO_DEBUG_LEVEL > 1
                  PrintThisInst(stderr, 1, ip);
                  PrintThisInst(stderr, 2, ip0);
                  fko_error(__LINE__, "not isomorphic inst\n");
               #endif
                  suc = 0;
                  break;
               }
               ip = ip->next;
               ip0 = ip0->next;
            }
         }
         if (suc)
         {
            new = NewPack(iln, pk->vlen);
            new->vflag = pk->vflag;
            upackq = FinalizePack(new, upackq, &ch); 
            if (ch)
            {
               *change = 1;
#if 0
               fprintf(stderr, "Extending pack=%d from pack=%d by use-def\n", 
                  new->pnum, pk->pnum);
#endif
            }
         }
      }
   } 
   return(upackq);
}

INSTQ *InstUsesVar(INT_BVI bvvar, INSTQ *upackq)
{
   INSTQ *ip;

   ip = upackq;
   while (ip)
   {
      if (BitVecCheckComb(bvvar, ip->use, '&'))
         return(FindFirstLILforHIL(ip));
      ip = ip->next;
   }
   return(NULL);
}

INSTQ *FollowDefUses(PACK *pk, INSTQ *upackq, int *change)
{
   int suc;
   ILIST *il, *iln;
   INSTQ *ip;
   INSTQ *ipu, *ipdup, *ip0;
   PACK *new;
   INT_BVI iv;
   extern INT_BVI FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0); 
/*
 * need to find out all inst where those variables are used which are defined 
 * in this pack. need to consider the order to create pack, so can't use pk->def
 * here.
 */
   iln = NULL;
   il = pk->sil;
   while(il)
   {
/*
 *    calc def for this instq
 */
      ip = il->inst;
      SetVecAll(iv, 0); 
      while(ip)
      {
         iv = BitVecComb(iv, iv, ip->set, '|');
         ip = ip->next;
      }
      FilterOutRegs(iv);
#if 0
      PrintVars(stderr, "all sets: ", iv);
      fprintf(stderr, "upackq :\n");
      fprintf(stderr, "==========================\n");
      PrintThisInstQ(stderr, upackq);
      fprintf(stderr, "==========================\n");
#endif
/*
 *    NOTE: there should be only one set var... so, we may extend one pack
 */
      ipu = InstUsesVar(iv, upackq);
      if (ipu)
      {
         ipdup = ip0 = NULL;
         do
         {
            ipdup = NewInst(NULL, ipdup, NULL, ipu->inst[0], ipu->inst[1], 
                               ipu->inst[2], ipu->inst[3]);
            CalcThisUseSet(ipdup);
            if (ip0)
               ip0->next = ipdup;
            ip0 = ipdup;
            ipu = ipu->next;
         } while (!IS_STORE(ipdup->inst[0])); /* note: no pref in pack */ 
         
         while(ipdup->prev) ipdup = ipdup->prev;
         iln = NewIlistAtEnd(ipdup, iln);
      }
      else 
      {
         /*fprintf(stderr, "NO match : def_uses\n");*/
         *change = 0;
         return(upackq);
      }
      il = il->next;
   }
/*
 *       check whether inst are isomorphic, die otherwise
 *       FIXME: need to check again if there are not isomorphic...
 */
   suc = 1;
   for (il=iln; il; il=il->next)
   {
      ip0 = iln->inst;
      ip = il->inst;
      while (ip)
      {
         if (!IsIsomorphicInst(ip, ip0))
         {
         #if IFKO_DEBUG_LEVEL > 1
            PrintThisInst(stderr, 1, ip);
            PrintThisInst(stderr, 2, ip0);
            fko_error(__LINE__, "not isomorphic inst\n");
         #endif
            suc = 0;
            *change = 0;
            break;
         }
         ip = ip->next;
         ip0 = ip0->next;
      }
   }
/*
 * time to create a new pack 
 */
   if (suc)
   {
      new = NewPack(iln, pk->vlen);
      new->vflag = pk->vflag;
/*
 * during finalizing the pack, we check for dependencies within a pack.
 * if vlen is less than actual vlen in the system, we don't allow SLP for now.
 * check for consecutive memory access too 
 */
      upackq = FinalizePack(new, upackq, change); 
#if 0
      if (*change)
         fprintf(stderr, "Extending pack=%d from pack=%d by def-use\n", 
                 new->pnum, pk->pnum);
#endif
   }
   return(upackq);
}

int IsInitPackInUse(PACK *ipk)
{
   int i;
   PACK *pk;
   INT_BVI iv;
   extern INT_BVI FKO_BVTMP;
  
   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(128);
   iv = FKO_BVTMP;
   
   iv = BitVecCopy(iv, ipk->defs);
   FilterOutRegs(iv); 

   for (i=0; i < NPACK; i++)
   {
      pk = PACKS[i];
      if (pk) //&& !(pk->pktype & PK_INIT))
      {
         
         if (!BitVecCheckComb(iv, pk->uses, '-') )
               // || !BitVecCheckComb(iv, pk->defs, '-')) 
         {

            return(1);
         }
      }
   }
   return(0);
}
int IsPackInConflict(PACK *pk0)
{
   int i, N;
   PACK *pk;
   short *sp;
   short ptr, lda;
   int offset, mul;
   ILIST *il;
   INSTQ *ip;
   INT_BVI iv;
   extern INT_BVI FKO_BVTMP;
  
   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(128);
   iv = FKO_BVTMP;
/*
 * can be mem load or store
 */
   iv = BitVecCopy(iv, pk0->defs);
   iv = FilterOutRegs(iv);
   sp = BitVec2Array(iv, 1-TNREG);
   N = sp[0];
   if (N <= 1) /* it's a mem store */
   {
      iv = BitVecCopy(iv, pk0->uses);
/*
 *    PTR and lda also are in-use here, no need to consider them since they are
 *    not vectorized
 */
      for (il=pk0->sil; il; il=il->next)
      {
         for (ip=il->inst; ip; ip=ip->next)
         {
            if (IS_STORE(ip->inst[0]))
            {
               ptr = lda = offset = mul = 0;
               FindPtrInfoFromDT(ip, &ptr, &lda, &offset, &mul);
               if (ptr)
                  SetVecBit(iv, ptr+TNREG-1, 0);
               if (lda)
                  SetVecBit(iv, lda+TNREG-1, 0);
            }
         }
      }
      iv = FilterOutRegs(iv);
      /*PrintVars(stderr,"after st" ,iv);*/
   }
   free(sp);

   for (i=0; i < NPACK; i++)
   {
      pk = PACKS[i];
      if (pk) 
      {
         if ( BitVecCheckComb(iv, pk->uses, '&')) /* has common vars*/
         {
/*
 *          If we have common variables in both packs, all of them have to be
 *          used
 */
            if (BitVecCheckComb(iv, pk->uses, '-'))
            {
               fko_warn(__LINE__, "*****pack-%d is in conflict with %d!!!", 
                     pk0->pnum, pk->pnum);
#if 0
               iv = BitVecComb(iv, iv, pk->uses, '-');
               PrintVars(stderr, "common: ", iv);
               PrintPack(stderr, pk);
               PrintPack(stderr, pk0);
#endif
               return(1);
            }
         }
      }
   }
   return(0);
}


INSTQ *ExtendPack(INSTQ *upackq, int ipk)
/*
 * ipk is initial pack number to start with... need to added that to extend
 * from pre-head
 */
{
   int i, changes, change;
   PACK *pk;

   for (i=ipk; i < NPACK; i++)
   {
      pk = PACKS[i];
      if (pk && !(pk->pktype & PK_MEM_BROADCAST) )
      {
#if 0         
         if ( (pk->pktype & PK_INIT) && (pk->pktype & PK_INIT_SCATTER))
            continue;
         else
#endif
         {
            do 
            {
               changes = 0;
               upackq = FollowDefUses(pk, upackq, &change);
               changes += change; 
               upackq = FollowUseDefs(pk, upackq, &change);
               changes += change; 
               if (changes && (pk->pktype & PK_INIT_MEM) )
                  pk->pktype |= PK_INIT_MEM_ACTIVE;
            } while (changes);
         }
      }
   }
/*
 * delete those initial packs which can't be extended
 * FIXED: they can be in use in some packs... need to check that too
 * NOTE: we don't want to delete those init packs which are from vlist; only
 * consider those which are from MEM!!!
 * FIXME: what if there exist only mem inst!!! need to fix that from scheduling
 * case study: mvec4x4 amm kernel
 */
#if 0   
   for (i=ipk; i < NPACK; i++)
   {
      pk = PACKS[i];
      if (pk)
      {
#if 0         
         if ( (pk->pktype & PK_INIT) && !(pk->pktype & PK_INIT_ACTIVE))
               KillPack(pk);
#else
         if (pk->pktype & PK_INIT_MEM)
         {
            if (!(pk->pktype & PK_INIT_MEM_ACTIVE))
            {
               if (!IsInitPackInUse(pk)) 
                  KillPack(pk);
               else
                  pk->pktype |= PK_INIT_MEM_ACTIVE;
            }
         }
         else /* init packs are always at the first */
            break;
#endif
      }
   }
#endif  
/*
 * Main idea: 
 * If we have init mem pack which won't extend any pack and has inconsistancy 
 * with other packs, we need to delete them. Only delete those which are not 
 * consistant. Blks may have only mem access without any code; in that case,
 * we need to keep init pack with no extended packs
 * FIXME: May have some packs which extend other packs but have inconsistancy 
 * (in terms of scalar uses) with other packs!!! Scheduling step should catch
 * them.
 */
   for (i=ipk; i < NPACK; i++)
   {
      pk = PACKS[i];
      if (pk)
      {
         if ( (pk->pktype & PK_INIT_MEM) && !(pk->pktype & PK_INIT_MEM_ACTIVE))
         {
            if (IsPackInConflict(pk))
               KillPack(pk);
            else
               pk->pktype |= PK_INIT_MEM_ACTIVE;
         }
      }
   }

#if 0   
   fprintf(stderr, "Packq After Extending :\n");
   for (i=0; i < NPACK; i++)
   {
      ILIST *il;
      if (PACKS[i]) 
      {
         fprintf(stderr, "***PACK %d: \n", i);
         fprintf(stderr, "len=%d\n", PACKS[i]->vlen);
         il = PACKS[i]->sil;
         while (il)
         {
            fprintf(stderr, "print instq :\n");
            PrintThisInstQ(stderr, il->inst);
            il = il->next;
         }
      }
   }
#endif
   return (upackq);
}

INSTQ *DupInstQ(INSTQ *ip)
{
   INSTQ *ip0, *ip1, *ipdup;
   ip0 = ip1 = NULL;
   for ( ; ip; ip = ip->next)
   {
      ip1 = NewInst(NULL, ip1, NULL, ip->inst[0], ip->inst[1], ip->inst[2], 
                    ip->inst[3]);
      CalcThisUseSet(ip1);
      if (ip0)
         ip0->next = ip1;
      ip0 = ip1;
   }
   while(ip1->prev) ip1 = ip1->prev;
   ipdup = ip1;
   return(ipdup);
}

BBLOCK *DupInstQInBlock(BBLOCK *bp, INSTQ *bip)
{
   INSTQ *ip, *ip1;

   for ( ip=bip; ip; ip=ip->next)
   {
      ip1 = InsNewInst(bp, ip1, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                       ip->inst[3]);
      CalcThisUseSet(ip1);
   }
   return(bp);
}

PACK *FindPackFromInst(INSTQ *ipuse, int ipk)
{
   int i, nosame;
   INSTQ *ip0, *ip;
   INSTQ *ipu;
   ILIST *il;

   assert(ACTIVE_INST(ipuse->inst[0]));
   
   for (i=ipk; i < NPACK; i++)
   {
      if (PACKS[i])
      {
         for (il=PACKS[i]->sil; il; il=il->next)
         {
            nosame = 0;
            ipu = ipuse; /* keep track of original ipuse */

            ip0 = il->inst;
            ip = ip0;
/*
 *          NOTE: this func is called both from update_dep and schedule_pack
 *          Need to check the whole il list whether there is any match with 
 *          ipuse to any of the inst of il
 */
            while (ip)
            {
               if (IsSameInst(ip, ipuse))
                  break;
               ip = ip->next;
            }
/*
 *          if we have a match, but it's from the middle of the il
 *          track back to the head
 */
            if (ip) /* match */
            {
               while (ip != ip0) /* go back to starting inst */
               {
                  ip = ip->prev;
                  ipu = ipu->prev;
/*
 *                NOTE: ipu may contain non active inst, like: comments
 *                skip them too.
 */
                  while (ipu && !ACTIVE_INST(ipu->inst[0]))
                     ipu = ipu->prev;
               }
#if 0
               fprintf(stderr, "===================================\n");
               if(ip)
               {
                  PrintThisInst(stderr, 1, ip);
                  if (ip->next)
                  {
                     PrintThisInst(stderr, 2, ip->next);
                     if (ip->next->next)
                        PrintThisInst(stderr, 3, ip->next->next);
                  }
               }
               fprintf(stderr, "---------------------------------\n");
               if (ipu)
               {
                  PrintThisInst(stderr, 1, ipu);
                  if (ipu->next)
                  {
                     PrintThisInst(stderr, 2, ipu->next);
                     if (ipu->next->next)
                        PrintThisInst(stderr, 2, ipu->next->next);
                  }
               }
               fprintf(stderr, "===================================\n");
#endif
               while (ip) /* match all inst from queue */
               {
                  if (!IsSameInst(ip, ipu)) 
                  {
                     //PrintThisInst(stderr, 1, ip);
                     //if(ipu) PrintThisInst(stderr, 2, ipu);
                     nosame = 1;
                     break;
                  }
                  ip = ip->next;
                  ipu = ipu->next;
/*
 *                skip non active inst, like: comments
 */
                  while (ipu && !ACTIVE_INST(ipu->inst[0]))
                     ipu = ipu->next;
               }
               if (!nosame)
               {
                  //fprintf(stderr, "*****Found\n");
                  return(PACKS[i]);
               }
            }
         }
      }
   }
   //fprintf(stderr, "**************Not Found\n");
   return(NULL);
}

void UpdateDep(INSTQ *bip, int ipk)
{  
   int i, k, n;
   INSTQ *ip;
   INT_BVI iv, bvvars;
   short *sp, *vars, *uvar=NULL;
   ILIST **ils, *il;
   PACK *pk;
   extern INT_BVI FKO_BVTMP;

   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);
   
   bvvars = NewBitVec(128);

   for (ip=bip; ip; ip=ip->next)
   {
      bvvars = BitVecComb(bvvars, bvvars, ip->set, '|'); 
      bvvars = BitVecComb(bvvars, bvvars, ip->use, '|'); 
   }
   FilterOutRegs(bvvars);
#if 0
   PrintVars(stderr, "all vars", iv);
   exit(0);
#endif
   sp = BitVec2Array(bvvars, 1-TNREG);
#if 0
   for (n=sp[0], i=1 ; i <= n; i++ )
      fprintf(stderr, "%d ", sp[i]);
   fprintf(stderr, "\n");
#endif
   n = sp[0];
   vars = sp + 1; /* point from the vars */
/*
 * NOTE: upto this points, insts of pack won't point to the instq. they are just
 * independent packs. But now we need to point the dependency to the copied 
 * instq
 */
   ils = malloc(n*sizeof(ILIST*));
   for (i=0; i < n; i++)
      ils[i] = NULL;
   assert(ils);
   
   for (ip=bip; ip; ip=ip->next)
   {
      if (BitVecCheckComb(bvvars, ip->use, '&')) 
      {
         iv = BitVecComb(iv, bvvars, ip->use, '&');
         uvar = BitVec2Array(iv, 1-TNREG);
         for (n=uvar[0], i=1; i <= n; i++ )
         {
            k = FindInShortList(sp[0], vars, uvar[i]);
            if (k)
            {
               if (ils[k-1])
               {
#if 0
                  //PrintThisInst(stderr, 77, il[k-1]->inst);
                  fprintf(stderr, "------------------\n");
                  PrintThisInst(stderr, 0, ip);
                  if (ip->next) 
                     PrintThisInst(stderr, 0, ip->next);
                  if (ip->next->next) 
                     PrintThisInst(stderr, 0, ip->next->next);
                  if (ip->next->next->next) 
                     PrintThisInst(stderr, 0, ip->next->next->next);
                  if (ip->next->next->next->next) 
                     PrintThisInst(stderr, 0, ip->next->next->next->next);
#endif
/*
 *                find pack which has this ip inst
 */
                  pk = FindPackFromInst(ip, ipk);
                  if (pk)
                  {
                  /*fprintf(stderr, "******Found the pack = %d\n", pk->pnum);*/
                     il = pk->depil;
                     while (il)
                     {
                        if (il->inst == ils[k-1]->inst)
                           break;
                        il = il->next;
                     }
                     if (!il)
                        pk->depil = NewIlist(ils[k-1]->inst, pk->depil);
/*
 *                   now, point the dependencies 
 */
                     /*while(ip && !IS_STORE(ip->inst[0])) // assuming no jump
                        ip = ip->next;*/
                  }
                  else
                  {
                     /*fprintf(stderr, "****** Pack NOT Found for inst: \n");*/
                     fko_warn(__LINE__, "Pack Not found for inst");
                  }

               }
               else
               {
                  /*fprintf(stderr, "Not defined: ");*/
                  /*PrintThisInst(stderr, 999, ip);*/
               }
            }
         }
         if (uvar) free(uvar);
      }
/*
 *    if var is set in this inst, update the list
 */
      if (BitVecCheckComb(bvvars, ip->set, '&')) 
      {
         k = STpts2[ip->inst[1]-1];
         i = FindInShortList(sp[0], vars, k);
         if (i)
         {
            if (!ils[i-1])
            {
               ils[i-1] = NewIlist(ip, ils[i-1]); 
            }
            else
               ils[i-1]->inst = ip;
         }
      }
   }

#if 0
   fprintf(stderr, "\nn=%d\n", n);
   for (i=0; i < sp[0]; i++)
      if (ils[i])
         PrintThisInst(stderr, 1, ils[i]->inst);
#endif
#if 0   
   fprintf(stderr, "Pack info:\n");
   for (i=ipk; i < NPACK; i++)
   {
      if (PACKS[i])
      {
         il = PACKS[i]->depil;
         if (il)
         {
            fprintf(stderr, "pack%d depends on following insts:\n", i);
            while (il)
            {
               PrintThisInst(stderr, 0, il->inst);
               il = il->next;
            }
         }
      }
   }
#endif
/*
 * delete all temp data/storage 
 */
   for (i=0, n = sp[0]; i < n; i++)
      if (ils[i]) 
         KillAllIlist(ils[i]);
   free(ils);
   free(sp);
   KillBitVec(bvvars);
}

int IsInstInQueue(INSTQ *ipq, INSTQ *inst)
{
   INSTQ *ip;
   for (ip=ipq; ip; ip=ip->next)
   {
      if (ip == inst) /* depil keep track of biq inst */
         return(1);
   }
   return(0);
}


INSTQ *SchInstQ(PACK *pk, BBLOCK *nsbp, BBLOCK *sbp, INSTQ *anchor)
/*
 * NOTE: this is just for debug the scheduling; it doesn't actually vectorize
 * the code
 */
{
   int i, k;
   short op[4];
   INSTQ *ip;
   ILIST *il;

   for (il=pk->sil; il; il=il->next)
   {
      for (ip=il->inst; ip; ip=ip->next)
      {
/*
 *       add inst at the end in sbp
 *       FIXED: create new DT entry for each mem access
 */
         /*InsNewInst(sbp, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2], 
                 ip->inst[3]);*/
         op[0] = ip->inst[0];
         for (i=1; i < 4; i++)
         {
            op[i] = ip->inst[i];
            if (op[i] > 0 && NonLocalDeref(op[i]))
            {
               k = op[i]-1;
               op[i] = AddDerefEntry(SToff[k].sa[0], SToff[k].sa[1], 
                     SToff[k].sa[2], SToff[k].sa[3], STpts2[k]);
            }
         }
         InsNewInst(sbp, NULL, NULL, op[0], op[1], op[2], op[3]);
      }
      anchor = DelMatchInstQ(nsbp->inst1, il->inst);
   }
   return(anchor);
}

void UpdateMemUseSet(PACK *pk, int isload )
{
   int i, N;
   INT_BVI iv;
   short *sp;
   ILIST *il;
   INSTQ *ip;
   extern short STderef;
/*
 * find out all use and set
 */   
   iv = NewBitVec(32);
   iv = BitVecCopy(iv, pk->uses);
   iv = FilterOutRegs(iv);
   //PrintVars(stderr, "------use: ",iv);
   sp = BitVec2Array(iv, 1-TNREG);
   N = sp[0];
   pk->scuse = malloc((N+1)*sizeof(short));
   assert(pk->scuse);
   pk->scuse[0] = N;
/*
 * now considering 2D array, there can be two scuse vars
 */
#if 0   
   if (isload && N == 1)
      pk->scuse[1] = sp[1];
#else
   if (isload)
   {
      if (N == 1)
         pk->scuse[1] = sp[1];
      else if (N == 2)
      {
         pk->scuse[1] = sp[1];
         pk->scuse[2] = sp[2];
      }
      else fko_error(__LINE__, "N must be 1 or 2");
   }
#endif
   free(sp);
   
   iv = BitVecCopy(iv, pk->defs);
   iv = FilterOutRegs(iv);
   //PrintVars(stderr, "def: ",iv);
   sp = BitVec2Array(iv, 1-TNREG);
   N = sp[0];
   pk->scdef = malloc((N+1)*sizeof(short));
   assert(pk->scdef);
   pk->scdef[0] = N;
/*
 * NOTE: we opt out _NONLOCDEREF by using filter out regs 
 */
   if (!isload)
   {
      if (N==1)
         pk->scdef[1] = sp[1];
   }
   free(sp);

/*
 * if is load, last inst stores it in var; otherwise last inst stores in mem
 */
   i = 1;
   if (isload)
   {
      for (il=pk->sil; il; il=il->next)
      {
         for (ip = il->inst; ip && ip->next; ip = ip->next)
            ;
         assert(IS_STORE(ip->inst[0]));
         pk->scdef[i++] = STpts2[ip->inst[1]-1];
      }
      pk->scdef[0] = i - 1;
/*
 *    DT can use 2 vars: array and index
 */
      assert(pk->scuse[0] <= 2); 
   }
   else /* MEM_STORE */
   {
      for (il=pk->sil; il; il=il->next)
      {
         ip = il->inst;
/*
 *       NOTE: assuming 1st inst is always the load of fp variable, not a load
 *       of index/ldas 
 */
         assert(IS_LOAD(ip->inst[0]) && IS_FP(STflag[STpts2[ip->inst[2]-1]-1]));
         pk->scuse[i++] = STpts2[ip->inst[2]-1];
      }
      pk->scuse[0] = i - 1;
      assert(pk->scdef[0] <= 2);
   }
   KillBitVec(iv);
}

short InsertSlpVector(int stype)
{
   static int vid = 0;
   int j, k, type;
   char ln[128];
   
   if (FLAG2TYPE(stype) == T_FLOAT || FLAG2TYPE(stype) == T_VFLOAT)
      type = T_VFLOAT;
   else if (FLAG2TYPE(stype) == T_DOUBLE || FLAG2TYPE(stype) == T_VDOUBLE)
      type = T_VDOUBLE;
   else
      fko_error(__LINE__, "type not supported for slp vector yet!");
      

   sprintf(ln, "_VEC_%d", ++vid);
   k = STdef(ln, type | LOCAL_BIT, 0);
   j = AddDerefEntry(-REG_SP, k, -k, 0, k);
   SToff[k-1].sa[2] = j;
   return(k);
}

SLP_VECTOR *CreateVector(SLP_VECTOR *vlist, int flag, int vlen, short *svars, 
      int islivein, short vec)
/*
 * added at the endof the list ;
 */
{
   int i, n;
   SLP_VECTOR *vl;

   if (vlist)
   {
      for (vl=vlist; vl->next; vl = vl->next)
         ;
      vl->next = malloc(sizeof(SLP_VECTOR));
      assert(vl->next);
      vl = vl->next;
      vl->next = NULL;
   }
   else
   {
      vl = vlist = malloc(sizeof(SLP_VECTOR));
      assert(vl);
      vl->next = NULL;
   }
/*
 *    create new vector 
 *    slp vector: flag, vec, vlen, *svars, *next
 *    islivein 
 */
   vl->islive = 1;  /* always live at creation */
   vl->islivein = islivein; /* livein at the entry of blk */
   vl->isused = 0; /* by default 0, updated from schedule of slp */
   vl->flag = flag;
   vl->vlen = vlen;
   vl->redvar = 0; 
   n = svars[0];
   vl->svars = malloc((n+1)*sizeof(short));
   assert(vl->svars);
   for (i=0; i <= n; i++)
      vl->svars[i] = svars[i];
/*
 * create new vector variable if not specified
 */
   if (!vec)
      vl->vec = InsertSlpVector(vl->flag);
   else
      vl->vec = vec;

   return(vlist);
}

SLP_VECTOR *AddVectorInList(SLP_VECTOR *vlist, SLP_VECTOR *vec, int islivein, 
      int islive)
/*
 * this function adds a vector in list after coping the vector
 */
{
   int i, n;
   SLP_VECTOR *vl;

   if (vlist)
   {
      for (vl=vlist; vl->next; vl = vl->next)
         ;
      vl->next = malloc(sizeof(SLP_VECTOR));
      assert(vl->next);
      vl = vl->next;
      vl->next = NULL;
   }
   else
   {
      vl = vlist = malloc(sizeof(SLP_VECTOR));
      assert(vl);
      vl->next = NULL;
   }

/*
 * copy the vector
 */
   /*vl->islive = 1; */ // need to test ???  
   vl->islive = islive; // need to test ???  
   vl->islivein = islivein; /* livein at the entry of blk */
   vl->isused = 0; /* only set from scheduling in slp  */
   
   vl->flag = vec->flag;
   vl->vlen = vec->vlen;
   vl->redvar = vec->redvar;
   n = vec->svars[0];
   vl->svars = malloc((n+1)*sizeof(short));
   assert(vl->svars);
   for (i=0; i <= n; i++)
      vl->svars[i] = vec->svars[i];
   vl->vec = vec->vec;

   return(vlist);
}

SLP_VECTOR *FindVectorFromSingleScalar(short var, SLP_VECTOR *vlist)
{
   int i, n;
   int found;
   SLP_VECTOR *vl, *vec;

   found = 0;
   for (vl=vlist; vl; vl=vl->next)
   {
      n = vl->svars[0];
      for (i=1; i <= n; i++)
      {
         if (var == vl->svars[i])
         {
            found = 1;
            vec = vl;
            break;
         }
      }
      if (found)
         break;
   }
   if (found)
      return(vec);
   return(NULL);
}

SLP_VECTOR *FindVectorFromScalars(short *scalars, SLP_VECTOR *vlist)
{
   int i, n;
   int found;
   SLP_VECTOR *vl, *vec;

   found = 0;
   for (vl=vlist; vl; vl=vl->next)
   {
      n = scalars[0];
      if (n == vl->svars[0])
      {
         for (i=1; i <= n; i++)
         {
            if (scalars[i] != vl->svars[i])
               break;
         }
         if (i == n+1 ) //&& vl->islive) /* matched all and live */
         {
            found = 1;
            vec = vl;
            break;
         }
      }
   }
   if (found)
      return(vec);
   
   return(NULL);
}

SLP_VECTOR *FindVectorByVec(short vid, SLP_VECTOR *vlist)
{
   int found;
   SLP_VECTOR *vl, *vec;
   found = 0;
   for (vl=vlist; vl; vl=vl->next)
   {
      if (vl->vec == vid)
      {
         found = 1;
         vec = vl;
         break;
      }
   }
   if (found)
      return(vec);
   return(NULL);
}

SLP_VECTOR *FindVectorFromRedvar(short rvar, SLP_VECTOR *vlist)
{
   SLP_VECTOR *vl;
   for (vl=vlist; vl; vl=vl->next)
      if (vl->redvar == rvar)
         return(vl);
      
   return(NULL);
}

SLP_VECTOR *AppendVoVlist(SLP_VECTOR *vd, SLP_VECTOR *vs)
/*
 * append vlist vs to vd without copying. After appending vs may be changed, 
 * since we may delete the duplicate blocks. So, don't use vs pointer afterward
 * NOTE: if duplicate exists, update isused
 */
{
   SLP_VECTOR *vl, *vl1, *vln, *vld; 

   if (!vd)
   {
      vd = vs;
      return(vd);
   }
/*
 * traverse upto the last element
 */
   for (vl=vd; vl->next; vl=vl->next)
      ;
   vln = vl;
/*
 * append unique vs at the end of vd
 */
   vl = vs;
   while (vl)
   {
      vl1 = FindVectorByVec(vl->vec, vd);
      if (!vl1)
      {
         vln->next = vl;
         vln = vl;
         vl = vl->next;
         vln->next = NULL;
      }
      else
      {
         vld = vl;
         vl = vl->next;
/*
 *       set isused if is set in either list
 */
         if (vld->isused)
            vl1->isused = 1;
/*
 *       kill duplicate vector
 */
         if (vld->svars)
            free(vld->svars);
         free(vld); 
      }
   }
   vln->next = NULL;
   return(vd);
}

void SetVectorDead(short var, SLP_VECTOR *vlist)
{
   int i, n;
   int found = 0;
   short *sp;
   SLP_VECTOR *vl;

   for (vl=vlist; vl; vl=vl->next)
   {
      sp = vl->svars;
      for (i=1, n=sp[0]; i <= n; i++)
      {
         if (sp[i] == var)
         {
            found = 1;
            break;
         }
      }
      if (found)
      {
         vl->islive = 0;
         break;
      }
   }
}

void PrintThisVector(FILE *fout, SLP_VECTOR *vp)
{
   int i, n;
      
   if (vp->vec)
      fprintf(fout, "%s : ", STname[vp->vec-1]);
   fprintf(fout, "<");
   for (i=1, n=vp->svars[0]; i <= n; i++)
      if(vp->svars[i]) 
         fprintf(fout, "%s, ", STname[vp->svars[i]-1]);
   fprintf(fout, "> ");
   fprintf(fout, " livein = %d\n", vp->islivein);
}

void PrintVectors(FILE *fout, SLP_VECTOR *vlist)
{
   int i, n;
   SLP_VECTOR *vp;

   fprintf(fout, "Printing vectors : \n");
   for (vp=vlist; vp; vp=vp->next)
   {
      if (vp->vec)
         fprintf(fout, "%s : ", STname[vp->vec-1]);
      fprintf(fout, "<");
      for (i=1, n=vp->svars[0]; i <= n; i++)
         if(vp->svars[i]) 
            fprintf(fout, "%s, ", STname[vp->svars[i]-1]);
      fprintf(fout, "> ");
      fprintf(fout, " livein = %d\n", vp->islivein);
   }

}

SLP_VECTOR *KillVecFromList(SLP_VECTOR *vlist, SLP_VECTOR *vec)
{
   SLP_VECTOR *vl, *vl0;

/*
 * manage NULL case
 */
   if (!vlist)
      return(NULL);
   if (!vec)
      return(vlist);
/*
 * search for the vector
 */
   vl0 = vlist;
   for (vl=vlist; vl; vl=vl->next)
   {
      if (vl == vec)
      {
         /*fprintf(stderr, "Found = %s\n", STname[vl->vec-1]);*/
         break;
      }
      vl0 = vl;
   }
/*
 * kill the vec now
 */
   if(!vl)
   {
      /*fprintf(stderr, "Not found!\n");*/
      fko_error(__LINE__, "Vector not found to kill!\n");
   }
   else
   {
      if (vl == vlist)
         vlist = vlist->next;
      else
         vl0->next = vl->next;
/*
 *    delete it
 */
      if (vl->svars)
         free(vl->svars);
      free(vl);
   }
   return(vlist);
}

void KillVlist(SLP_VECTOR *vlist)
{
   SLP_VECTOR *vl;

   while(vlist)
   {
      vl = vlist->next;
      if (vlist->svars)
         free(vlist->svars);
      free(vlist);
      vlist = vl;
   }
}

int NeedBroadcast(short *svars)
{
   int i, n, nb;
   short var;
   n = svars[0];
   assert(n > 1);
   var = svars[1];

   nb = 1;
   for (i=2; i <= n ; i++)
   {
      if (var != svars[i])
      {
         nb = 0;
         break;
      }
   }
   return(nb);
}

int HasSVar(BBLOCK *vbp, short sv)
{
   int hasvar = 0;
   INSTQ *ip;

   for (ip=vbp->ainstN; ip; ip=ip->prev)
   {
      if (IS_STORE(ip->inst[0]) && ip->inst[1] == SToff[sv-1].sa[2])
      {
         hasvar = 1;
         break;
      }
   }
   return(hasvar);
}

void AddGatherInst(BBLOCK *vbp, short vec, short sv, int vlen, SLP_VECTOR *vlist)
{
   short vreg0;
   enum inst vlds, vshuf, vst;
   short styp, vtyp;
   SLP_VECTOR *vl;

   assert(!IS_CONST(STflag[sv-1])); /* not consider const yet*/ 
   
   if (!HasSVar(vbp, sv))
   {
      vl = FindVectorFromSingleScalar(sv, vlist); 
      if (vl)
      {
         fprintf(stderr, "No scalar %s, converted into vector =%s\n", 
               STname[sv-1], STname[vl->vec-1]);
      }
      /*AddVarScatterInstFromVec(vbp, vl, sv);*/
      assert(HasSVar(vbp, sv));
   }

   
   styp = FLAG2TYPE(STflag[sv-1]);
   vtyp = FLAG2TYPE(STflag[vec-1]);

   switch(styp)
   {
      case T_FLOAT:
         vlds = VFLDS;
         vst = VFST;
         vshuf = VFSHUF;
         break;
      case T_DOUBLE:
         vlds = VDLDS;
         vst = VDST;
         vshuf = VDSHUF;
         break;
      default: 
         fko_error(__LINE__, "Unsupported type!");
   }
   
   vreg0 = GetReg(vtyp);
   PrintComment(vbp, NULL, NULL, "Broadcast %s from %s", STname[sv-1], 
         STname[vec-1]);
   InsNewInst(vbp, NULL, NULL, vlds, -vreg0, SToff[sv-1].sa[2], 0);
   InsNewInst(vbp, NULL, NULL, vshuf, -vreg0, -vreg0, STiconstlookup(0));
   InsNewInst(vbp, NULL, NULL, vst, SToff[vec-1].sa[2], -vreg0, 0);
   GetReg(-1);
}

void AddVectorInst(BBLOCK *vbp, PACK *pk, SLP_VECTOR *vlist)
{
   int i, j, k, n;
   static enum inst
      sfinsts[] = { FLD, FST, FMUL, FMAC, FADD, FSUB, FABS, FZERO, FNEG, 
                    FMAX, FMIN},
      vfinsts[] = { VFLD, VFST, VFMUL, VFMAC, VFADD, VFSUB, VFABS, VFZERO, VFNEG, 
                    VFMAX, VFMIN},
      sdinsts[] = { FLDD, FSTD, FMULD, FMACD, FADDD, FSUBD, FABSD, FZEROD, FNEGD, 
                    FMAXD,FMIND}, 
      vdinsts[] = { VDLD, VDST, VDMUL, VDMAC, VDADD, VDSUB, VDABS, VDZERO, VDNEG,
                    VDMAX, VDMIN};
   const int nvinst = 11;
   static enum inst
      valign[]    = { VFLD, VDLD, VFST, VDST},
      vualign[]   = { VFLDU, VDLDU, VFSTU, VDSTU},
      vbroadcast[] = {VFLDSB, VDLDSB };
      
   const int nmem = 4;
   enum inst *sinst, *vinst;
   enum inst inst;
   short sregs[TNFR], vregs[TNFR];
   short *scal, *vscal;
   SLP_VECTOR *vl;
   INSTQ *ip;
   short op, ipop[3];
   int vtype;
   int nfr = 0;

   if (IS_FLOAT(pk->vflag) || IS_VFLOAT(pk->vflag))
   {
      sinst = sfinsts;
      vinst = vfinsts;
      vtype = T_VFLOAT;
   }
   else if (IS_DOUBLE(pk->vflag) || IS_VDOUBLE(pk->vflag))
   {
      sinst = sdinsts;
      vinst = vdinsts;
      vtype = T_VDOUBLE;
   }
   else
      fko_error(__LINE__, "unsupported vtype in SLP\n");

   n = pk->vsc[0];
   
   vscal = malloc((n+1)*sizeof(short));
   scal = malloc((n+1)*sizeof(short));
   assert(vscal && scal);
   vscal[0] = scal[0] = n;

   for (i=1; i <= n; i++)
   {
      vscal[i] = pk->vsc[i];
      vl = FindVectorByVec(pk->vsc[i], vlist);
      scal[i] = vl->svars[1]; /* always the 1st scalar*/
   }
#if 0
   fprintf(stderr, "+++++++++ vars: \n");
   for (i=1; i <= n; i++)
   {
      fprintf(stderr, "%s -> %s\n", STname[scal[i]-1], STname[vscal[i]-1]);
   }
#endif
   ip = pk->sil->inst;  /* analysis only the 1st ilist, all other are isomorphic*/
   
   PrintComment(vbp, NULL, NULL, "Vetcorizing pack = %d", pk->pnum);
   for (; ip; ip=ip->next) 
   {
      inst = GET_INST(ip->inst[0]); 
      if (ACTIVE_INST(inst))
      {
         for (i=0; i < nvinst; i++)
         {
            if (sinst[i] == inst)
            {
               inst = vinst[i];
               for (j=1; j < 4; j++)
               {
                  op = ip->inst[j];
                  if (!op) 
                     ipop[j-1] = 0;
                  else if (op < 0)
                  {
                     op = -op;
                     k = FindInShortList(nfr, sregs, op);
                     if (!k)
                     {
                        nfr = AddToShortList(nfr, sregs, op);
                        k = FindInShortList(nfr, sregs, op);
                        vregs[k-1] = GetReg(vtype);
                     }
                     ipop[j-1] = -vregs[k-1];  
                  }
                  else
                  {
                     if (IS_DEREF(STflag[op-1]))
                     {
                        k = STpts2[op-1];
                        k = FindInShortList(scal[0], scal+1, k);
                        if (k)
                           ipop[j-1] = SToff[vscal[k]-1].sa[2];
                        else
                        {
/*
 *                         NOTE:
 *                         in case of nonlocal deref, we will create new deref
 *                         with all info as same, unlike any other vectorization
 *                         method
 */
                           if (NonLocalDeref(op))
                           {
                              k = op-1;
                              #if 0 
                                 fprintf(stderr, "NonLocalDeref(%s) = (%d,%d,%d,%d)\n",
                                        STname[STpts2[k]-1],
                                        SToff[k].sa[0], 
                                        SToff[k].sa[1], 
                                        SToff[k].sa[2], 
                                        SToff[k].sa[3] 
                                        );
                              #endif
                              ipop[j-1] = AddDerefEntry(SToff[k].sa[0], 
                                                        SToff[k].sa[1], 
                                                        SToff[k].sa[2], 
                                                        SToff[k].sa[3], 
                                                        STpts2[k]);
                           #if 1
/*
 *                         for now, we use unaligned ld/st
 *                         FIXME: need a markup which works for the routine to
 *                         scope this out!
 *                         NOTE: we are using markup to make it align inside the
 *                         optloop after SLP is done.
 */
                              for (k=0; k < nmem; k++)
                              {
                                 if (valign[k] == inst)
                                    break;
                              }
                              if (pk->pktype & PK_MEM_BROADCAST)
                              {
                                 #if defined(ArchHasMemBroadcast)
                                    assert(k <= 2);
                                    inst = vbroadcast[k];
                                    /*fprintf(stderr, 
                                          "****vector broadcast applied!!\n");*/
                                 #else
                                    fko_error(stderr, 
                                          "Need to implemenet vbroadcast!!!");
                                 #endif
                              }
                              else
                              {
                                 assert(k != nmem);
                                 inst = vualign[k];
                              }
                           #endif
                           }
/*
 *                         NOTE: it can be lda/index or other integer vars.. 
 *                         so, keep it intact
 */
                           else
                              /*fprintf(stderr, "Not Found = %s\n", 
                                    STname[STpts2[op-1]]);*/
                              ipop[j-1] = ip->inst[j];
                        }
                     }
                     else
                        ipop[j-1] = ip->inst[j];
                  }
               }
               InsNewInst(vbp, NULL, NULL, inst, ipop[0], ipop[1], ipop[2]);
               break;
            }
         }
         if (i == nvinst)
            InsNewInst(vbp, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2], 
                  ip->inst[3]);
      }
   }

   free(vscal);
   free(scal);
   GetReg(-1);
}

int ConvertLoad2MemBroadcast(INSTQ *ip0, short var, short vec)
{
   int i, j, k;
   int type, vtype;
   short op;
   INSTQ *ip;
   enum inst fld, vld, fst, vst, vbld;
   int nfr = 0;
   short sregs[TNFR], vregs[TNFR];

   type = FLAG2TYPE(STflag[var-1]);

   if (type == T_FLOAT)
   {
      fld = FLD;
      vld = VFLD;
      vbld = VFLDSB;
      fst = FST;
      vst = VFST;
      vtype = T_VFLOAT;
   }
   else if (type == T_DOUBLE)
   {
      fld = FLDD;
      vld = VDLD;
      vbld = VDLDSB;
      fst = FSTD;
      vst = VDST;
      vtype = T_VDOUBLE;
   }
   else
      fko_error(__LINE__, "type not supported in slp");
/*
 * Checking for correct format first
 */
   k = 0;
   ip = ip0;
   while (ip)
   {
      if (ACTIVE_INST(ip->inst[0]))
      {
         if (IS_LOAD(ip->inst[0]))
         {
            if (NonLocalDeref(ip->inst[2])) /* has mem load*/
               k = 1; 
         }
         else if (IS_STORE(ip->inst[0]))
         {
            if (ip->inst[1] != SToff[var-1].sa[2])
            {
               fprintf(stderr, "diff var updated!!\n");
               k = 0;
            }
            break; /* need no further checking*/
         }
         else
         {
            fprintf(stderr, 
                  "inst other than ld/st: %s!!\n", instmnem[ip->inst[0]]);
            k = 0;
            break;
         }
      }
      ip = ip->next;
   }
   if (!k) /* not in format */
   {
      fprintf(stderr, "failed conditions!!\n");
      return(0);
   }
/*
 * assuming ip0 is the first LIL inst of the instq which translated from mem 
 * load of HIL     
 */
   for (ip=ip0; ip && IS_LOAD(ip->inst[0]); ip=ip->next )
   {
      if (ip->inst[0] == fld)
      {
         if (NonLocalDeref(ip->inst[2]))
            ip->inst[0] = vbld;
         else 
            ip->inst[0] = vld;
            
         for (i=1; i < 4; i++)
         {
            op = ip->inst[i];
            if (!op) continue;
            else if (op < 0)
            {
               op = -op;
               j = FindInShortList(nfr, sregs, op);
               if (!j)
               {
                  nfr = AddToShortList(nfr, sregs, op);
                  j = FindInShortList(nfr, sregs, op);
                  vregs[j-1] = GetReg(vtype);
               }
                  ip->inst[i] = -vregs[j-1];
            }
            else
            {
               if (IS_DEREF(STflag[op-1]) && !NonLocalDeref(op))
               {
                  assert(op == SToff[var-1].sa[2]);
                  ip->inst[i] = SToff[vec-1].sa[2];
               }
            }
         }
      }
   }
   GetReg(-1);

   assert(IS_STORE(ip->inst[0]));
   if (ip->inst[0] == fst)
      ip->inst[0] = vst;
   
   for (i=1; i < 4; i++)
   {
      op = ip->inst[i];
      if (!op) continue;
      else if (op < 0)
      {
         op = -op;
         j = FindInShortList(nfr, sregs, op);
         assert(j);
         ip->inst[i] = -vregs[j-1];
      }
      else if (IS_DEREF(STflag[op-1]))
         ip->inst[i] = SToff[vec-1].sa[2];
   }
   return(1);
}

int MemBroadcast(BBLOCK *blk, short var, short vec)
{
   int i, j;
   int type, vtype;
   INSTQ *ip, *ip0;
   short ptr, lda, op;
   int offset, mul;
   int ismemb = 0;
   enum inst fld, vld, fst, vst, vbld;
   int nfr = 0;
   short sregs[TNFR], vregs[TNFR];

   type = FLAG2TYPE(STflag[var-1]);

   if (type == T_FLOAT)
   {
      fld = FLD;
      vld = VFLD;
      vbld = VFLDSB;
      fst = FST;
      vst = VFST;
      vtype = T_VFLOAT;
   }
   else if (type == T_DOUBLE)
   {
      fld = FLDD;
      vld = VDLD;
      vbld = VDLDSB;
      fst = FSTD;
      vst = VDST;
      vtype = T_VDOUBLE;
   }
   else
      fko_error(__LINE__, "type not supported in slp");
/*
 * search from last inst to first 
 */
   ptr = lda = offset = mul = 0;
   assert(blk->instN);
   /*for (ip=blk->ainstN; ip; ip=ip->prev)*/
   for (ip=blk->instN; ip; ip=ip->prev)
   {
      if (IS_STORE(ip->inst[0]) && ip->inst[1] == SToff[var-1].sa[2])
      {
         ip0 = ip->prev;
         while (ip0 && IS_LOAD(ip0->inst[0]))
         {
            if (NonLocalDeref(ip0->inst[2]))
            {
               FindPtrInfoFromDT(ip0, &ptr, &lda, &offset, &mul);
            }
            else if (ip0->inst[2] == SToff[var-1].sa[2] 
                     || (ptr && ip0->inst[2] == SToff[ptr-1].sa[2]) 
                     || (lda && ip0->inst[2] == SToff[lda-1].sa[2]) )
            {
               ismemb = 1;
            }
            else 
            {
               ismemb = 0;
               break;
            }
            ip0 = ip0->prev;
         }
         break;
      }
   }

   if (ismemb && ptr )
   {
      if (ip0)
         ip0 = ip0->next;
      else
         ip0 = FindFirstLILforHIL(ip);
/*
 *    now, time to covert the mem load into vmem load
 *    FIXME: why don't we convert all such mem load into vmem load
 */
      assert(ConvertLoad2MemBroadcast(ip0, var, vec));
      return(1);
   }
   else
   {
/*
 *    FIXME: not sure whether it's possible. return error for now!
 */
      fprintf(stderr, "Load inst not found for %s = %s(%d, %d)\n", 
              STname[vec-1], 
              STname[var-1], 
              ptr, ismemb);
      PrintThisInstQ(stderr, blk->inst1);
      fko_error(__LINE__, "should have one load to broadcast!!! double check!");
   }
   
   return(0);
}

SLP_VECTOR *AddVecInst(PACK *pk, BBLOCK *vbp, SLP_VECTOR *vlist, INT_BVI livein)
{
   int i, j, k, n;
   INSTQ *ip, *ip0;
   ILIST *il;
   INT_BVI iv;
   short *sp;
   SLP_VECTOR *vd, *vs1, *vs2;
   enum PackType {NONE, MEM, MEM_LOAD, MEM_STORE, VOP};
   enum PackType pt; 
   extern INT_BVI FKO_BVTMP;
   const int sclil = 4;
   short **scals;
   short op;

   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);
   vd = vs1 = vs2 = NULL;
/*
 * all inst in pck are isomorphic; so, checking one ilist node is sufficient
 * FIXME: need to handle assignment: specially with zero
 */
   pt = NONE;
   ip = pk->sil->inst;
#if 0   
   while (ip && ip->next)
   {
      if ( !IS_LOAD(ip->inst[0]) && !IS_STORE(ip->inst[0]) 
            && !IS_BRANCH(ip->inst[0]))
      {
         pt = VOP;
         break;
      }
      for (i=1; i < 4; i++)
         if (ip->inst[i] > 0 && NonLocalDeref(ip->inst[i]))
            pt = MEM;
      ip = ip->next;
   }
#else
/*
 * FIXED: mem store would be the last inst
 */
   ip0 = NULL;
   while(ip)
   {
      ip0 = ip;   
      if ( !IS_LOAD(ip->inst[0]) && !IS_STORE(ip->inst[0]) 
            && !IS_BRANCH(ip->inst[0]))
      {
         pt = VOP;
         break;
      }
      for (i=1; i < 4; i++)
         if (ip->inst[i] > 0 && NonLocalDeref(ip->inst[i]))
            pt = MEM;
      ip = ip->next;
   }
   assert(ip0);
   ip = ip0;
#endif

   if ( pt != VOP)
   {
      assert (IS_STORE(ip->inst[0])) /* ip is the last inst in queue */
      assert(pt==MEM); /* only support load, store and arithmatic op */
      if (NonLocalDeref(ip->inst[1]))
         pt = MEM_STORE;
      else
         pt = MEM_LOAD;
   }
#if 0
   FilterOutRegs(pk->uses);
   PrintVars(stderr, "uses: ", pk->uses);
   FilterOutRegs(pk->defs);
   PrintVars(stderr, "defs: ", pk->defs);
#endif

   if (pt == MEM_LOAD)
   {
      UpdateMemUseSet(pk, 1);
#if 0
      n = pk->scdef[0];
      fprintf(stderr, "++++++ Mem load(%d): ", n);
      for (i=1; i <= n; i++)
      {
         fprintf(stderr, "%s, ", STname[pk->scdef[i]-1]);
      }
      fprintf(stderr, "\n");
#endif
      if (pk->pktype & PK_MEM_BROADCAST)
         vd = FindVectorFromSingleScalar(pk->scdef[1], vlist);
      else
         vd = FindVectorFromScalars(pk->scdef, vlist);
      if (!vd)
      {
         vlist = CreateVector(vlist, pk->vflag, pk->vlen, pk->scdef, 0, 0);
         vd = FindVectorFromScalars(pk->scdef, vlist); /*avoid extra search!*/
         assert(vd);
      } 
      vd->isused = 1; /* marked as used */
#if 0      
      if (pk->pktype & PK_MEM_BROADCAST)
      {
         fprintf(stderr, "***** broadcast vector = %s (%s)\n", 
               STname[vd->vec-1], STname[pk->scdef[1]-1]);
         PrintVectors(stderr, vlist);
      }
#endif
#if 0      
      ip = pk->sil->inst;
      for ( ; ip; ip = ip->next)
         if (IS_LOAD(ip->inst[0]) && NonLocalDeref(ip->inst[2]))
            break;
      assert(ip); 
      FindPtrInfoFromDT(ip, &ptr, &lda, &offset, &mul);
/*
 *    consider simple case first: single ptr, no lda and const offset
 */
      assert(!lda);
      assert(pk->scuse[0] == 1 && pk->scuse[1] == ptr);
#endif
      pk->vsc = malloc(2*sizeof(short));
      assert(pk->vsc);
      pk->vsc[0] = 1;
      pk->vsc[1] = vd->vec;
      pk->pktype |= PK_MEM_LOAD;
   }
   else if (pt == MEM_STORE)
   {
      UpdateMemUseSet(pk, 0);
      vs1 = FindVectorFromScalars(pk->scuse, vlist);
      if (!vs1)
      {
/*
 *       check whether all of them are live in
 */
         iv = BitVecCopy(iv, pk->uses);
         FilterOutRegs(iv);
#if 0
         PrintVars(stderr, "uses: ",iv);
#endif
         assert(!BitVecCheckComb(iv, livein, '-'));
#if 0
         n = pk->scuse[0];
         fprintf(stderr, "++++++ Mem store(%d): ", n);
         for (i=1; i <= n; i++)
         {
            fprintf(stderr, "%s, ", STname[pk->scuse[i]-1]);
         }
         fprintf(stderr, "\n");
#endif
         vlist = CreateVector(vlist, pk->vflag,pk->vlen, pk->scuse, 1, 0);
         vs1 = FindVectorFromScalars(pk->scuse, vlist);
         assert(vs1);
      } 
      vs1->isused = 1; /* marked as used */
      pk->vsc = malloc(2*sizeof(short));
      assert(pk->vsc);
      pk->vsc[0] = 1;
      pk->vsc[1] = vs1->vec;
      pk->pktype |= PK_MEM_STORE;
   }
   else if (pt == VOP)
   {
/*
 * Assuming LIL statement can't have more than 3 vars in single statement 
 * if not, update sclil with max
 */
      n = pk->vlen;
      scals = malloc(n*sizeof(short*));
      assert(scals);
      for (i=0; i < n; i++)
      {
         scals[i] = calloc((sclil+1), sizeof(short));
         assert(scals[i]);
      }
      
      for (il=pk->sil, k=0; il; il=il->next, k++)
      {
/*
 *       NOTE: we don't consider any memory operation here. so, all scalars 
 *       does count
 */
         for (ip=il->inst, j=0; ip; ip=ip->next)
         {
            //PrintThisInst(stderr, 111, ip);
            for (i=1; i < 4; i++)
            {
               if (ip->inst[i] > 0 && IS_DEREF(STflag[ip->inst[i]-1]))
               {
                  assert(!NonLocalDeref(ip->inst[i]));
                  op = STpts2[ip->inst[i]-1];
                  if (!FindInShortList(j, scals[k], op))
                     j = AddToShortList(j, scals[k], op);
               }
            }
         }
      }
      sp = malloc((n+1)*sizeof(short));
      assert(sp);
      sp[0] = n; 
      pk->vsc = malloc((j+1)*sizeof(short));
      assert(pk->vsc);
      pk->vsc[0] = j;
      pk->pktype |= PK_ARITH_OP;
      for (i = 0; i < j; i++)
      {
         for (k=1; k <= sp[0]; k++)
            sp[k] = scals[k-1][i]; 

         vd = FindVectorFromScalars(sp, vlist);
         if (!vd)
         {
            iv = BitVecCopy(iv, Array2BitVec(sp[0], sp+1, TNREG-1));
            FilterOutRegs(iv);
            if (BitVecCheckComb(iv, livein, '-')) /* not livein: all ? any? */
            {
               vlist = CreateVector(vlist, pk->vflag, pk->vlen, sp, 0, 0);
               vd = FindVectorFromScalars(sp, vlist);
               assert(vd);
               if ( NeedBroadcast(sp))
               {
                  #if defined(ArchHasMemBroadcast) //&& 0
                  if (!MemBroadcast(vbp, sp[1], vd->vec) )
                  #endif
                     AddGatherInst(vbp, vd->vec, sp[1], pk->vlen, vlist); 
               }
               else if (BitVecCheckComb(iv, pk->uses, '&') )
               {
                  PrintVars(stderr, "vars in use: ", iv);
                  FilterOutRegs(pk->uses);
                  PrintVectors(stderr, vlist);
                  PrintVars(stderr, "pk->uses: ", pk->uses);
                  fko_error(__LINE__, "used vector must be livein " 
                            "Or, must be formed by broadcast");
               }
            }
            else
            {
               vlist = CreateVector(vlist, pk->vflag, pk->vlen, sp, 1, 0);
               vd = FindVectorFromScalars(sp, vlist);
               assert(vd);
            }
         }
         else  /* already created, but need to see */
         {
/*
 *          we have already add checking in Scheduling to skip such case
 */
#if 0
            if (!vd->islive)
            {
               if ( NeedBroadcast(sp))
               {
                  fko_warn(__LINE__, "*****Vetcor =%s is dead, need broadcast\n", 
                        STname[vd->vec-1]);
                  #if defined(ArchHasMemBroadcast) //&& 0
                  if (!MemBroadcast(vbp, sp[1], vd->vec) )
                  #endif
                     AddGatherInst(vbp, vd->vec, sp[1], pk->vlen, vlist); 
                  vd->islive = 1;
               }
               else
               {
                  fko_error(__LINE__, "not supported if can't be broadcast!");
               }
            }
#endif            
         }
         vd->isused = 1; /* marked as used */
         pk->vsc[i+1] = vd->vec;
      }

#if 0
      PrintVectors(stderr, vlist);
      for (i=0; i < n; i++)
      {
         fprintf(stderr, " op %d: ", i);
         for (k=0; k < sclil; k++)
         {
            if (scals[i][k])
            {
               fprintf(stderr, "%s ", STname[scals[i][k]-1]);
            }
         }
         fprintf(stderr, "\n");
      }
      exit(0);
#endif
/*
 *    free memory
 */
      for (i=0; i < n; i++)
         if (scals[i])
            free(scals[i]);
      free(scals);
      if(sp) free(sp);

   }

#if 0
      PrintVectors(stderr, vlist);
#endif
/*
 * convert and insert vector insts  
 */
   AddVectorInst(vbp, pk, vlist);

   return(vlist);
}

/*
 * NOTE: Algorithm for general cases:
 * [ Not implemented here ]
 * 1. pack Vi = <ai, ai+1, ...> when <ai, ai+1,...> are live-in at the entry of
 * block. Place it at the predecessor of the block. add Vi in live vector-list
 *
 * 2. for i=1 to |B|
 *       si: statement at i, a = f(bk, bk+1, ..) where a is set and bi are used
 *       if si is element of pack pk <si, si+1, ... >
 *          if all the variable used in pk isn't in any live vector
 *             create vectors v = <bki, bki+1, ...> ... 
 *             pack those vectors and add them in live vector list
 *       if si is not in any pack
 *          if a is in any vector vi
 *             unpack the vector vi into scalars variables
 *             delete the vi entry from live-vector list
 *          if bk, bk+1, ... in vector vk
 *             unpack vk before si
 *
 * 3. unpack all the vectors whose scalars are liveout at the exit of the block
 * place it at the successor of the block.
 *    
 * NOTE: we avoid all unpacking of vectors since upacking/repacking is costly 
 * for current generation of hardware. It limits the successful vectorization 
 * in our ofcourse. Blk only be vectorized when we don't have to unpack.
 * 
 */

SLP_VECTOR *SchVectorInst(BBLOCK *nsbp, BBLOCK *sbp, BBLOCK *vbp, INT_BVI livein, 
                  SLP_VECTOR *vlist, int ipk, int *err)
{
   int i,k;
   int nosch, isstore;
   INSTQ *ip, *ip1;
   ILIST *il;
   /*ILIST *iln;*/
   PACK *pk;
   SLP_VECTOR *vl, *vls;
   /*SLP_VECTOR *vlist = NULL;*/
/*
 * NOTE:
 * we want to keep all inactive inst, like: CMPFLAG, intact. So, used inst1 
 * instead of ainst1.
 */
   
   /*for (ip=nsbp->ainst1; ip; )*/
   for (ip=nsbp->inst1; ip; )
   {
      if (!ACTIVE_INST(ip->inst[0]))
      {
/*
 *       FIXED: we need to add all inactive flags like CMPFLAG for later use.
 */
         InsNewInst(sbp, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                    ip->inst[3]);
         InsNewInst(vbp, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                    ip->inst[3]);
         ip = DelInst(ip);
         continue;
      }
/*
 *    got label, add it to sbp and delete from nsbp
 */
      if (ip->inst[0] == LABEL)
      {
         InsNewInst(sbp, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                    ip->inst[3]);
         InsNewInst(vbp, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                    ip->inst[3]);
         ip = DelInst(ip);
         continue;
      } 
      nosch = 0; /* not schedulable */
/*
 *    find pack from inst
 */
#if 0
      fprintf(stderr, "Handling inst: \n");
      fprintf(stderr, "----------------\n");
      PrintThisInst(stderr, 111, ip);
      fprintf(stderr, "----------------\n");
#endif
      pk = FindPackFromInst(ip, ipk);
      if (pk)  
      {
#if 0
         fprintf(stderr, "Pack-%d found while scheduling\n", pk->pnum);
         il = pk->sil;
         while (il)
         {
            PrintThisInstQ(stderr, il->inst);
            il = il->next;
         }
#endif
         il = pk->depil;
         while (il)
         {
/*
 *          check whether the inst which this pack depends on already scheduled
 *          If those remain in nsbp, that means they are not scheduled yet. in
 *          that case, we can't schedule this pack
 */
            /*if (IsInstInQueue(nsbp->ainst1, il->inst))*/
            if (IsInstInQueue(nsbp->inst1, il->inst))
            {
               nosch = 1;
#if 0
               fprintf(stderr, "Unscheduled dependent inst\n");
               PrintThisInst(stderr, 123, il->inst);
               fprintf(stderr, "inst to schedule\n");
               PrintThisInstQ(stderr, pk->sil->inst);
               fprintf(stderr, "printing nsbp\n");
               PrintThisInstQ(stderr, nsbp->ainst1);
               exit(0);
#endif
               break;
            }
            il = il->next;
         }
/*
 *       all scheduled the inst which this pack depends on
 */
         if (!nosch) /* already scheduled those which this pack depends on */
         {
/*
 *          mov instq of the pack from biq to sbiq
 *          FIXED: what if biq becomes invalid due to deleting the top inst
 *          to solve this, we always start from the beginning.
 */
            /*fprintf(stderr, "pack=%d, start scheduling ... ...\n", pk->pnum);*/
#if 0
            iln = pk->sil;
            while (iln)
            {
               PrintThisInstQ(stderr, iln->inst);
               iln = iln->next;
            }
#endif
            SchInstQ(pk, nsbp, sbp, ip);
            /*ip = nsbp->ainst1;*/ 
            ip = nsbp->inst1; /* start from top again*/
#if 0
            fprintf(stderr, "biq: \n");
            PrintThisInstQ(stderr,nsbp->ainst1);
            fprintf(stderr, "sbiq: \n");
            PrintThisInstQ(stderr,sbp->ainst1);
#endif
/*
 *          now, insert vector inst
 */
            vlist = AddVecInst(pk, vbp, vlist, livein);
         }
         else /* FIXME: need to figure out the action for this scenario*/
         {
/*
 *          instead of stopping, set error and return
 *          vlist may be changed, be careful to use it later
 */
            /*fko_error(__LINE__, "Dependent inst not scheduled yet!!!");*/
            fko_warn(__LINE__, "Dependent inst not scheduled yet!!!");
#if 0
            PrintThisInst(stderr, 0, ip);
            PrintThisInst(stderr, 1, pk->depil->inst);
            for (i=0; i < NPACK; i++)
               if(PACKS[i]) 
                  PrintPack(stderr, PACKS[i]);
#endif
            *err = 1;
            return(vlist);
         }
      }
      else /* not in the pack, schedule as standalone */
      {
/*
 *       FIXME: If we consider scatter/gather, we have to scatter the vector 
 *       into scalar (which is used/defined in this standalone insts) and 
 *       execute this standalone inst. Since we avoid scatter/gather all 
 *       together, vectorization fails. Making the vector dead won't work!!!
 */
         //ip0 = ip;
         isstore = 0;
         PrintComment(vbp, NULL, NULL, "Schedule as standalone inst");
#if 0         
         PrintThisInst(stderr, 100, ip);
         if(ip->next) PrintThisInst(stderr, 101, ip->next);
         if(ip->next && ip->next->next) 
         {
            PrintThisInst(stderr, 102, ip->next->next);
            if (IS_STORE(ip->next->next->inst[0]) 
                  && STpts2[ip->next->next->inst[1]-1] == FindVarFromName("rA0"))
            {
               fprintf(stderr, "biq: \n");
               PrintThisInstQ(stderr, nsbp->ainst1);
            }
         }
#endif
#if 0
         fprintf(stderr, "*** standalone inst:\n");
#endif
/*
 *       NOTE: copied the inst and then check whether it conflicts with 
 *       existing vlist. resolved one such conflict with mem broadcast!
 */
         isstore = 0;
         vls = NULL;
         while(1)
         {
            if (!ip)
               break;
            for (i=1; i < 4; i++)
            {
               k = ip->inst[i];
               if (k > 0 && !NonLocalDeref(k))
               {
                  k = STpts2[k-1];
                  vl = FindVectorFromSingleScalar(k, vlist);
/*
 *                NOTE: redvars would be scalarized in vvrsum process. so, we 
 *                can skip them
 */
                  if (vl && (k != vl->redvar) ) 
                  {
                     /*fprintf(stderr, "scalar %s is vectorized in %s\n", 
                           STname[k-1], STname[vl->vec-1]);*/
                     vls = AddVectorInList(vls, vl, vl->islivein, vl->islive);
                  }
               }
            }
            if (IS_STORE(ip->inst[0]) //|| ip->inst[0] == LABEL 
                  || IS_BRANCH(ip->inst[0]) || IS_PREF(ip->inst[0]))
            {
               isstore = 1;
            #if 0               
               if (IS_STORE(ip->inst[0]) && !NonLocalDeref(ip->inst[1]))
               {
                  SetVectorDead(STpts2[ip->inst[1]-1], vlist);
               }
            #endif
            }
            InsNewInst(sbp, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                  ip->inst[3]);
            InsNewInst(vbp, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                  ip->inst[3]);
            /*PrintThisInst(stderr, 1, ip);*/
            ip = DelInst(ip);
            if (isstore)
               break;
         }
/*
 *       conflict with vectors
 *       NOTE: will replace these fko_errors with errr returns
 */
         if (vls)
         {
            if (vls->next)
            {
            #if IFKO_DEBUG_LEVEL > 1  
               fko_error(__LINE__, 
                     "Conflict with more than one vec while scheduling");
            #endif
               fko_warn(__LINE__, 
                     "Conflict with more than one vec while scheduling:%s->%s",
                     STname[vls->vec-1], STname[vls->next->vec-1]);
               PrintThisVector(stderr, vls);
               KillVlist(vls);
               *err = 1; /* don't use anything other than 1 to report err */
               return(vlist);
            }
            if (NeedBroadcast(vls->svars))
            {
               ip1 = FindFirstLILforHIL(vbp->instN);
               if (!ConvertLoad2MemBroadcast(ip1, vls->svars[1], vls->vec))
               {
                  /*fko_error(__LINE__, 
                     "Conflict can't be resloved with mem broadcast");*/
                  fko_warn(__LINE__, 
               "can't use scalars which is a part of vec as standalone inst");
                  KillVlist(vls);
                  *err = 1; /* always use 1 to report erro */
                  return(vlist);
               }
            }
            KillVlist(vls);
         }

#if 0
         fprintf(stderr, "************\n");
#endif
#if 0         
         fprintf(stderr, "sbiq: \n");
         PrintThisInstQ(stderr, sbp->ainst1);
         fprintf(stderr, "biq: \n");
         PrintThisInstQ(stderr, nsbp->ainst1);
#endif
      }
   }
/*
 * slp successful, set error as 0
 */
   *err = 0;
   return(vlist);
}


void AddSlpPrologue(BBLOCK *blk, SLP_VECTOR *vlist, int endpos)
/*
 * FIXME: rewrite the function to remove the duplication
 */
{
   int i, n;
   short vr0, vr1, vr2;
   short s0, s1, s2, s3, s4, s5, s6, s7, vec;
   SLP_VECTOR *vl;
   INT_BVI iv1; 
   INSTQ *iptp, *iptn, *iph;
   
   if (!vlist)  /* no vector to init */ 
      return;  
/*
 * set iptp, iptn, iph
 */
   iptp = blk->ainst1;
   if (iptp->inst[0] == LABEL)
      iptn = NULL;
   else
   {
      iptp = NULL;
      iptn = blk->inst1;
   }
   iph = blk->ainstN;
   if (iph && IS_BRANCH(iph->inst[0]))
      iph = iph->prev;
   else
      iph = NULL;
/*
 * init all vectors in list 
 */
   for (vl=vlist; vl; vl=vl->next)
   {
      vec = SToff[vl->vec-1].sa[2];
      if (IS_DOUBLE(STflag[vl->vec-1]) || IS_VDOUBLE(STflag[vl->vec-1]))
      {
         #ifdef AVX
            assert(vl->svars[0] == 4);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            s2 = SToff[vl->svars[3]-1].sa[2];
            s3 = SToff[vl->svars[4]-1].sa[2];
            
            vr0 = GetReg(T_VDOUBLE);
            vr1 = GetReg(T_VDOUBLE);
            vr2 = GetReg(T_VDOUBLE);
/*
 *          not handled this case: s0 = s1 = s2 = s3
 *          but can be implemented quick easily! will add that later 
 */
            if ( (vl->svars[1] == vl->svars[2])
               && (vl->svars[1] == vl->svars[3])
               && (vl->svars[1] == vl->svars[4]) )
            {
               if (vl->flag & NSLP_ACC)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "ACC Vector init: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VDLDS, -vr0, s0, 0);
                     InsNewInst(blk, NULL, iph, VDST, vec, -vr0, 0);
                  }
                  else
                  {
                     iptp = PrintComment(blk, iptp, iptn, "ACC Vector init: %s", 
                                         STname[vl->vec-1]);
                     iptp = InsNewInst(blk, iptp, NULL, VDLDS, -vr0, s0, 0);
                     iptp = InsNewInst(blk, iptp, NULL, VDST, vec, -vr0, 0); 
                  }
               }
               else if (vl->flag & NSLP_SCAL)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "Vector init: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VDLDS, -vr0, s0, 0);
                     InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                            STiconstlookup(0));
                     InsNewInst(blk, NULL, iph, VDST, vec, -vr0, 0);
                  }
                  else
                  {
                     iptp = PrintComment(blk, iptp, iptn, "Vector init: %s", 
                                         STname[vl->vec-1]);
                     iptp = InsNewInst(blk, iptp, NULL, VDLDS, -vr0, s0, 0);
                     iptp = InsNewInst(blk, iptp, NULL, VDST, vec, -vr0, 0); 
                  }

               }
               else
                  fko_error(__LINE__, "not handled the case in SLP");
               GetReg(-1);
               continue;
            }
/*
 *          Regular SLP vector init
 */
            if (endpos)
            {
               PrintComment(blk, NULL, iph, "Vector init: %s", 
                            STname[vl->vec-1]);
               InsNewInst(blk, NULL, iph, VDLDS, -vr0, s0, 0);
               InsNewInst(blk, NULL, iph, VDLDS, -vr1, s1, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr1, 
                          STiconstlookup(0x3240));
               InsNewInst(blk, NULL, iph, VDLDS, -vr1, s2, 0);
               InsNewInst(blk, NULL, iph, VDLDS, -vr2, s3, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr1, -vr2, 
                          STiconstlookup(0x3240));
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr1, 
                          STiconstlookup(0x5410));
               InsNewInst(blk, NULL, iph, VDST, vec, -vr0, 0);
            }
            else /* at the begining of a blk*/
            {
               iptp = PrintComment(blk, iptp, iptn, "Vector init: %s", 
                                   STname[vl->vec-1]);
               iptp = InsNewInst(blk, iptp, NULL, VDLDS, -vr0, s0, 0);
               iptp = InsNewInst(blk, iptp, NULL, VDLDS, -vr1, s1, 0);
               iptp = InsNewInst(blk, iptp, NULL, VDSHUF, -vr0, -vr1, 
                                 STiconstlookup(0x3240));
               iptp = InsNewInst(blk, iptp, NULL, VDLDS, -vr1, s2, 0);
               iptp = InsNewInst(blk, iptp, NULL, VDLDS, -vr2, s3, 0);
               iptp = InsNewInst(blk, iptp, NULL, VDSHUF, -vr1, -vr2, 
                                 STiconstlookup(0x3240));
               iptp = InsNewInst(blk, iptp, NULL, VDSHUF, -vr0, -vr1, 
                                 STiconstlookup(0x5410));
               iptp = InsNewInst(blk, iptp, NULL, VDST, vec, -vr0, 0); 
            }
            GetReg(-1);
         #else
            assert(vl->svars[0] == 2);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            vr0 = GetReg(T_VDOUBLE);
            vr1 = GetReg(T_VDOUBLE);
/*
 *          NSLP Vect
 */
            if (vl->svars[1] == vl->svars[2])
            {
               if (vl->flag & NSLP_ACC)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "ACC Vector init: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VDLDS, -vr0, s0, 0);
                     InsNewInst(blk, NULL, iph, VDST, vec, -vr0, 0);
                  }
                  else
                  {
                     iptp = PrintComment(blk, iptp, iptn, "ACC Vector init: %s", 
                                         STname[vl->vec-1]);
                     iptp = InsNewInst(blk, iptp, NULL, VDLDS, -vr0, s0, 0);
                     iptp = InsNewInst(blk, iptp, NULL, VDST, vec, -vr0, 0); 
                  }
               }
               else if (vl->flag & NSLP_SCAL)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "Vector init: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VDLDS, -vr0, s0, 0);
                     InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                            STiconstlookup(0));
                     InsNewInst(blk, NULL, iph, VDST, vec, -vr0, 0);
                  }
                  else
                  fko_error(__LINE__, "not handled the simplest case in SLP");
               }
               GetReg(-1);
               continue;
            }
/*
 *          SLP vector init
 */
            if (endpos)
            {
               PrintComment(blk, NULL, iph, "Vector init: %s", 
                           STname[vl->vec-1]);
               InsNewInst(blk, NULL, iph, VDLDS, -vr0, s0, 0);
               InsNewInst(blk, NULL, iph, VDLDS, -vr1, s1, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr1, 
                          STiconstlookup(0x20));
               InsNewInst(blk, NULL, iph, VDST, vec, -vr0, 0);
            }
            else
            {
               iptp = PrintComment(blk, iptp, iptn, "Vector init: %s", 
                                   STname[vl->vec-1]);
               iptp = InsNewInst(blk, iptp, NULL, VDLDS, -vr0, s0, 0);
               iptp = InsNewInst(blk, iptp, NULL, VDLDS, -vr1, s1, 0);
               iptp = InsNewInst(blk, iptp, NULL, VDSHUF, -vr0, -vr1, 
                                 STiconstlookup(0x20));
               iptp = InsNewInst(blk, iptp, NULL, VDST, vec, -vr0, 0);
            }
            GetReg(-1);
         #endif
      }
      else if (IS_FLOAT(STflag[vl->vec-1]) || IS_VFLOAT(STflag[vl->vec-1]))
      {
         #ifdef AVX
            assert(vl->svars[0] == 8);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            s2 = SToff[vl->svars[3]-1].sa[2];
            s3 = SToff[vl->svars[4]-1].sa[2];
            s4 = SToff[vl->svars[5]-1].sa[2];
            s5 = SToff[vl->svars[6]-1].sa[2];
            s6 = SToff[vl->svars[7]-1].sa[2];
            s7 = SToff[vl->svars[8]-1].sa[2];
            vr0 = GetReg(T_VFLOAT);
            vr1 = GetReg(T_VFLOAT);
            vr2 = GetReg(T_VFLOAT);
            
            if ( (vl->svars[1] == vl->svars[2]) 
                  && (vl->svars[1] == vl->svars[3]) 
                  && (vl->svars[1] == vl->svars[4])
                  && (vl->svars[1] == vl->svars[5])
                  && (vl->svars[1] == vl->svars[6])
                  && (vl->svars[1] == vl->svars[7])
                  && (vl->svars[1] == vl->svars[8]) )
            {
               if (vl->flag & NSLP_ACC)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "ACC Vector init: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VFLDS, -vr0, s0, 0);
                     InsNewInst(blk, NULL, iph, VFST, vec, -vr0, 0);
                  }
                  else
                  {
                     iptp = PrintComment(blk, iptp, iptn, "ACC Vector init: %s", 
                                         STname[vl->vec-1]);
                     iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr0, s0, 0);
                     iptp = InsNewInst(blk, iptp, NULL, VFST, vec, -vr0, 0); 
                  }
               }
               else if (vl->flag & NSLP_SCAL)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "Vector init: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VFLDS, -vr0, s0, 0);
                     InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                            STiconstlookup(0));
                     InsNewInst(blk, NULL, iph, VFST, vec, -vr0, 0);
                  }
                  else
                  fko_error(__LINE__, "not handled the simplest case in SLP");
               }
               GetReg(-1);
               continue;
            }
/*
 *          General SLP vector init
 */
            if (endpos)
            {
               PrintComment(blk, NULL, iph, "Vector init: %s", 
                            STname[vl->vec-1]);
               InsNewInst(blk, NULL, iph, VFLDS, -vr0, s6, 0);
               InsNewInst(blk, NULL, iph, VFLDS, -vr1, s7, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr1, 
                          STiconstlookup(0x76549180));
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                          STiconstlookup(0x76549810));
               InsNewInst(blk, NULL, iph, VFLDS, -vr1, s4, 0);
               InsNewInst(blk, NULL, iph, VFLDS, -vr2, s5, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr1, -vr2, 
                          STiconstlookup(0x76549180));
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr1, 
                          STiconstlookup(0x76549810));
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                          STiconstlookup(0xBA983210));
               
               InsNewInst(blk, NULL, iph, VFLDS, -vr1, s2, 0);
               InsNewInst(blk, NULL, iph, VFLDS, -vr2, s3, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr1, -vr2, 
                          STiconstlookup(0x76549180));
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr1, 
                         STiconstlookup(0x76549810));
               InsNewInst(blk, NULL, iph, VFLDS, -vr1, s0, 0);
               InsNewInst(blk, NULL, iph, VFLDS, -vr2, s1, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr1, -vr2, 
                          STiconstlookup(0x76549180));
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr1, 
                          STiconstlookup(0x76549810));
               InsNewInst(blk, NULL, iph, VDST, vec, -vr0, 0);
            }
            else
            {
               iptp = PrintComment(blk, iptp, iptn, "Vector init: %s", 
                                   STname[vl->vec-1]);
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr0, s6, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr1, s7, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr0, -vr1, 
                                 STiconstlookup(0x76549180));
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr0, -vr0, 
                                 STiconstlookup(0x76549810));
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr1, s4, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr2, s5, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr1, -vr2, 
                                 STiconstlookup(0x76549180));
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr0, -vr1, 
                                 STiconstlookup(0x76549810));
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr0, -vr0, 
                                 STiconstlookup(0xBA983210));
               
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr1, s2, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr2, s3, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr1, -vr2, 
                                 STiconstlookup(0x76549180));
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr0, -vr1, 
                                 STiconstlookup(0x76549810));
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr1, s0, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr2, s1, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr1, -vr2, 
                                 STiconstlookup(0x76549180));
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr0, -vr1, 
                                 STiconstlookup(0x76549810));
               iptp = InsNewInst(blk, iptp, NULL, VDST, vec, -vr0, 0);
            }
            GetReg(-1);
         #else
            assert(vl->svars[0] == 4);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            s2 = SToff[vl->svars[3]-1].sa[2];
            s3 = SToff[vl->svars[4]-1].sa[2];
            vr0 = GetReg(T_VFLOAT);
            vr1 = GetReg(T_VFLOAT);
            vr2 = GetReg(T_VFLOAT);
            
            if ( (vl->svars[1] == vl->svars[2])
                  && (vl->svars[1] == vl->svars[3])
                  && (vl->svars[1] == vl->svars[4]) )
            {
               //fko_error(__LINE__, "not handled the simplest case in SLP");
               if (vl->flag & NSLP_ACC)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "ACC Vector init: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VFLDS, -vr0, s0, 0);
                     InsNewInst(blk, NULL, iph, VFST, vec, -vr0, 0);
                  }
                  else
                  {
                     iptp = PrintComment(blk, iptp, iptn, "ACC Vector init: %s", 
                                         STname[vl->vec-1]);
                     iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr0, s0, 0);
                     iptp = InsNewInst(blk, iptp, NULL, VFST, vec, -vr0, 0); 
                  }
               }
               else if (vl->flag & NSLP_SCAL)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "Vector init: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VFLDS, -vr0, s0, 0);
                     InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                            STiconstlookup(0));
                     InsNewInst(blk, NULL, iph, VFST, vec, -vr0, 0);
                  }
                  else
                  fko_error(__LINE__, "not handled the simplest case in SLP");
               }
               GetReg(-1);
               continue;
            }
            if (endpos)
            {
               PrintComment(blk, NULL, iph, "Vector init: %s", 
                            STname[vl->vec-1]);
               InsNewInst(blk, NULL, iph, VFLDS, -vr0, s2, 0);
               InsNewInst(blk, NULL, iph, VFLDS, -vr1, s3, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr1, 
                          STiconstlookup(0x3240));
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                          STiconstlookup(0x3254));
               InsNewInst(blk, NULL, iph, VFLDS, -vr1, s0, 0);
               InsNewInst(blk, NULL, iph, VFLDS, -vr2, s1, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr1, -vr2, 
                          STiconstlookup(0x3240));
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr1, 
                          STiconstlookup(0x3254));
               InsNewInst(blk, NULL, iph, VDST, vec, -vr0, 0);
            }
            else
            {
               iptp = PrintComment(blk, iptp, iptn, "Vector init: %s", 
                                   STname[vl->vec-1]);
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr0, s2, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr1, s3, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr0, -vr1, 
                                 STiconstlookup(0x3240));
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr0, -vr0, 
                                 STiconstlookup(0x3254));
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr1, s0, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFLDS, -vr2, s1, 0);
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr1, -vr2, 
                                 STiconstlookup(0x3240));
               iptp = InsNewInst(blk, iptp, NULL, VFSHUF, -vr0, -vr1, 
                                 STiconstlookup(0x3254));
               iptp = InsNewInst(blk, iptp, NULL, VDST, vec, -vr0, 0);
            }
            GetReg(-1);
         #endif
      }
      else
         fko_error(__LINE__, "Unsupported vector type in prologue!");
   }
}

void AddSlpEpilogue(BBLOCK *blk, SLP_VECTOR *vlist, int endpos)
{
   int i, n;
   short vr0;
   short s0, s1, s2, s3, s4, s5, s6, s7, vec;
   INSTQ *iptp, *iptn, *iph;
   SLP_VECTOR *vl;
   INT_BVI iv1; 
   
   if (!vlist)  /* no vector to reduce */ 
      return;  
/*
 * set iptp, iptn, iph
 */
   iptp = blk->ainst1;
   if (iptp->inst[0] == LABEL)
      iptn = NULL;
   else
   {
      iptp = NULL;
      iptn = blk->inst1;
   }
   iph = blk->ainstN;
   if (iph && IS_BRANCH(iph->inst[0]))
      iph = iph->prev;
   else
      iph = NULL;
   
   for (vl=vlist; vl; vl=vl->next)
   {
      vec = SToff[vl->vec-1].sa[2];
      if (IS_DOUBLE(STflag[vl->vec-1]) || IS_VDOUBLE(STflag[vl->vec-1]))
      {
         #ifdef AVX
            assert(vl->svars[0] == 4);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            s2 = SToff[vl->svars[3]-1].sa[2];
            s3 = SToff[vl->svars[4]-1].sa[2];
            vr0 = GetReg(T_VDOUBLE);
/*
 *          Special case 
 */
            if ( (vl->svars[1] == vl->svars[2])
                  && (vl->svars[1] == vl->svars[3])
                  && (vl->svars[1] == vl->svars[4]))
            {
               //fko_error(__LINE__, "not handle in SLP epilogue!");
               if (vl->flag & NSLP_ACC)
               {
                  fko_error(__LINE__, "ACC should be handled using vvrsums!");
               }
               else if (vl->flag & NSLP_SCAL)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "Vector reduction: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
                     InsNewInst(blk, NULL, iph, VDSTS, s0, -vr0, 0);
                  }
                  else
                  {
                     iptp = PrintComment(blk, iptp, iptn, "Scatter of vector %s", 
                                         STname[vl->vec-1]);
                     iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
                     iptp = InsNewInst(blk, iptp, iptn, VDSTS, s0, -vr0, 0);
                  }
               }
               GetReg(-1);
               continue;
            }

            if (endpos)
            {
               PrintComment(blk, NULL, iph, "Scatter of vector %s", 
                           STname[vl->vec-1]);
#if 1
               InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VDSTS, s0, -vr0, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                          STiconstlookup(0x3211));
               InsNewInst(blk, NULL, iph, VDSTS, s1, -vr0, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                          STiconstlookup(0x3232));
               InsNewInst(blk, NULL, iph, VDSTS, s2, -vr0, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                          STiconstlookup(0x3211));
               InsNewInst(blk, NULL,iph, VDSTS, s3, -vr0, 0);
#else
               InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VDSTS, s0, -vr0, 0);
               
               InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3211));
               InsNewInst(blk, NULL, iph, VDSTS, s1, -vr0, 0);
               
               InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3232));
               InsNewInst(blk, NULL, iph, VDSTS, s2, -vr0, 0);
               
               InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3232));
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3211));
               InsNewInst(blk, NULL, iph, VDSTS, s3, -vr0, 0);
#endif
            }
            else
            {
               iptp = PrintComment(blk, iptp, iptn, "Scatter of vector %s", 
                        STname[vl->vec-1]);
#if 1
               iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s0, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3211));
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s1, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3232));
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s2, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3211));
               iptp = InsNewInst(blk, iptp,iptn, VDSTS, s3, -vr0, 0);
#else
               iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s0, -vr0, 0);
               
               iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3211));
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s1, -vr0, 0);
               
               iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3232));
               iptp = InsNewInst(blk, iptp, iptp, VDSTS, s2, -vr0, 0);
               
               iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3232));
               iptp = InsNewInst(blk, iptp, iptn, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3211));
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s3, -vr0, 0);
#endif
            }
            GetReg(-1);               
         #else
            assert(vl->svars[0] == 2);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            vr0 = GetReg(T_VDOUBLE);
            
            if (vl->svars[1] == vl->svars[2])
            {
               //fko_error(__LINE__, "not handle in SLP epilogue!");
               if (vl->flag & NSLP_ACC)
               {
                  fko_error(__LINE__, "ACC should be handled using vvrsums!");
               }
               else if (vl->flag & NSLP_SCAL)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "Vector reduction: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
                     InsNewInst(blk, NULL, iph, VDSTS, s0, -vr0, 0);
                  }
                  else
                  {
                     iptp = PrintComment(blk, iptp, iptn, "Scatter of vector %s", 
                                         STname[vl->vec-1]);
                     iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
                     iptp = InsNewInst(blk, iptp, iptn, VDSTS, s0, -vr0, 0);
                  }
               }
               GetReg(-1);
               continue;
            }
            
            if (endpos)
            {
               PrintComment(blk, NULL, iph, "Scatter of vector %s", 
                        STname[vl->vec-1]);
#if 1
               InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VDSTS, s0, -vr0, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                     STiconstlookup(0x11));
               InsNewInst(blk, NULL, iph, VDSTS, s1, -vr0, 0);
#else
               InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VDSTS, s0, -vr0, 0);
               
               InsNewInst(blk, NULL, iph, VDLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x11));
               InsNewInst(blk, NULL, iph, VDSTS, s1, -vr0, 0);
#endif
            }
            else
            {
               iptp = PrintComment(blk, iptp, iptn, "Scatter of vector %s", 
                        STname[vl->vec-1]);
#if 1
               iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s0, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSHUF, -vr0, -vr0, 
                     STiconstlookup(0x11));
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s1, -vr0, 0);
#else
               iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s0, -vr0, 0);
               
               iptp = InsNewInst(blk, iptp, iptn, VDLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VDSHUF, -vr0, -vr0, 
                        STiconstlookup(0x11));
               iptp = InsNewInst(blk, iptp, iptn, VDSTS, s1, -vr0, 0);
#endif
            }
            GetReg(-1);               
         #endif
      }
      else if (IS_FLOAT(STflag[vl->vec-1]) || IS_VFLOAT(STflag[vl->vec-1]))
      {
         #ifdef AVX
            assert(vl->svars[0] == 8);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            s2 = SToff[vl->svars[3]-1].sa[2];
            s3 = SToff[vl->svars[4]-1].sa[2];
            s4 = SToff[vl->svars[5]-1].sa[2];
            s5 = SToff[vl->svars[6]-1].sa[2];
            s6 = SToff[vl->svars[7]-1].sa[2];
            s7 = SToff[vl->svars[8]-1].sa[2];
            vr0 = GetReg(T_VFLOAT);

            if ( (vl->svars[1] == vl->svars[2])
                  && (vl->svars[1] == vl->svars[3])
                  && (vl->svars[1] == vl->svars[4])
                  && (vl->svars[1] == vl->svars[5])
                  && (vl->svars[1] == vl->svars[6])
                  && (vl->svars[1] == vl->svars[7])
                  && (vl->svars[1] == vl->svars[8]) )
            {
               //fko_error(__LINE__, "not handle in SLP epilogue!");
               if (vl->flag & NSLP_ACC)
               {
                  fko_error(__LINE__, "ACC should be handled using vvrsums!");
               }
               else if (vl->flag & NSLP_SCAL)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "Vector reduction: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VFLD, -vr0, vec, 0);
                     InsNewInst(blk, NULL, iph, VFSTS, s0, -vr0, 0);
                  }
                  else
                  {
                     iptp = PrintComment(blk, iptp, iptn, "Scatter of vector %s", 
                                         STname[vl->vec-1]);
                     iptp = InsNewInst(blk, iptp, iptn, VFLD, -vr0, vec, 0);
                     iptp = InsNewInst(blk, iptp, iptn, VFSTS, s0, -vr0, 0);
                  }
               }
               GetReg(-1);
               continue;
            }
            
            if (endpos)
            {
               PrintComment(blk, NULL, iph, "Scatter of vector %s", 
                        STname[vl->vec-1]);
               InsNewInst(blk, NULL, iph, VFLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VFSTS, s0, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543211));
               InsNewInst(blk, NULL, iph, VFSTS, s1, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543322));
               InsNewInst(blk, NULL, iph, VFSTS, s2, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543232));
               InsNewInst(blk, NULL, iph, VFSTS, s3, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76547654));
               InsNewInst(blk, NULL, iph, VFSTS, s4, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543211));
               InsNewInst(blk, NULL, iph, VFSTS, s5, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543322));
               InsNewInst(blk, NULL, iph, VFSTS, s6, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543232));
               InsNewInst(blk, NULL, iph, VFSTS, s7, -vr0, 0);
            }
            else
            {
               iptp = PrintComment(blk, iptp, iptn, "Scatter of vector %s", 
                        STname[vl->vec-1]);
               iptp = InsNewInst(blk, iptp, iptn, VFLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s0, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543211));
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s1, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543322));
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s2, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543232));
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s3, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76547654));
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s4, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543211));
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s5, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543322));
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s6, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x76543232));
               iptp = InsNewInst(blk, iptp, iptp, VFSTS, s7, -vr0, 0);
            }
            GetReg(-1);               
         #else
            assert(vl->svars[0] == 4);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            s2 = SToff[vl->svars[3]-1].sa[2];
            s3 = SToff[vl->svars[4]-1].sa[2];
            vr0 = GetReg(T_VFLOAT);

            if ( (vl->svars[1] == vl->svars[2])
                  && (vl->svars[1] == vl->svars[3])
                  && (vl->svars[1] == vl->svars[4]) )
            {
               //fko_error(__LINE__, "not handle in SLP epilogue!");
               if (vl->flag & NSLP_ACC)
               {
                  fko_error(__LINE__, "ACC should be handled using vvrsums!");
               }
               else if (vl->flag & NSLP_SCAL)
               {
                  if (endpos)
                  {
                     PrintComment(blk, NULL, iph, "Vector reduction: %s", 
                            STname[vl->vec-1]);
                     InsNewInst(blk, NULL, iph, VFLD, -vr0, vec, 0);
                     InsNewInst(blk, NULL, iph, VFSTS, s0, -vr0, 0);
                  }
                  else
                  {
                     iptp = PrintComment(blk, iptp, iptn, "Scatter of vector %s", 
                                         STname[vl->vec-1]);
                     iptp = InsNewInst(blk, iptp, iptn, VFLD, -vr0, vec, 0);
                     iptp = InsNewInst(blk, iptp, iptn, VFSTS, s0, -vr0, 0);
                  }
               }
               GetReg(-1);
               continue;
            }
            
            if (endpos)
            {
               PrintComment(blk, NULL, iph, "Scatter of vector %s", 
                        STname[vl->vec-1]);
               InsNewInst(blk, NULL, iph, VFLD, -vr0, vec, 0);
               InsNewInst(blk, NULL, iph, VFSTS, s0, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3211));
               InsNewInst(blk, NULL, iph, VFSTS, s1, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3322));
               InsNewInst(blk, NULL, iph, VFSTS, s2, -vr0, 0);
               InsNewInst(blk, NULL, iph, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3232));
               InsNewInst(blk, NULL, iph, VFSTS, s3, -vr0, 0);
            }
            else
            {
               iptp = PrintComment(blk, iptp, iptn, "Scatter of vector %s", 
                        STname[vl->vec-1]);
               iptp = InsNewInst(blk, iptp, iptn, VFLD, -vr0, vec, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s0, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3211));
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s1, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3322));
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s2, -vr0, 0);
               iptp = InsNewInst(blk, iptp, iptn, VFSHUF, -vr0, -vr0, 
                        STiconstlookup(0x3232));
               iptp = InsNewInst(blk, iptp, iptn, VFSTS, s3, -vr0, 0);
            }
            GetReg(-1);               
         #endif
      }
      else
         fko_error(__LINE__, "unsupported type for SLP " );
   }
}

void FinalizeVecBlock(BBLOCK *lbp, BBLOCK *vbp)
{
   INSTQ *ip;
   extern BBLOCK *bbbase;
/*
 * replace with vector code
 */
   KillAllInst(lbp->inst1);
   lbp->inst1 = lbp->instN = lbp->ainst1 = lbp->ainstN = NULL;
   for (ip=vbp->inst1; ip; ip=ip->next)
      InsNewInst(lbp, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2], 
            ip->inst[3]);
#if 0
/*
 * update CFG
 * FIXME: can't update now, optloop structure would be changed then
 */
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
#endif
}

#if 0
short *FindPtrFromPack(ILIST **ilam, int nptr)
{
   int i;
   int npt = 0;
   short *pt, *sp; 
   short ptr, lda;
   int offset, mul;
   ILIST *il;
   INT_BVI bvp, bvi; 
   PACK *pk;

#if 0
   fprintf(stderr, "\nPrinting the ilists: (FIND PTR) \n");
   fprintf(stderr, "---------------------\n");
   for (i=0; i < nptr; i++)
   {
      il = ilam[i];
      if (il && il->inst)
      {
         FindPtrInfoFromDT(il->inst, &ptr, &lda, &offset, &mul);
         fprintf(stderr, " %s(%s) : ", STname[ptr-1], 
               lda ? STname[lda-1]: "NULL");
         while (il)
         {
            fprintf(stderr, "%d ", SToff[il->inst->inst[2]-1].sa[3]);
            il = il->next;
         }
         fprintf(stderr, "\n");
      }
      else break;
   }
   //exit(0);
#endif
/*
 * add all pointers in sp 
 */
   sp = malloc((nptr+1)*sizeof(short));
   assert(sp);

   for (i=0; i < nptr; i++)
   {
      il = ilam[i];
      if (il && il->inst)
      {
         FindPtrInfoFromDT(il->inst, &ptr, &lda, &offset, &mul);
#if 0        
         if(!FindInShortList(npt, sp+1, ptr))
         {
            npt = AddInShortList(npt, sp+1, ptr);
         }
#else
         fprintf(stderr, " %s\n", STname[ptr-1]);
         npt = AddToShortList(npt, sp+1, ptr);
#endif
      }
   }
   sp[0] = npt;
#if 0
   fprintf(stderr, "SP(%d) : ", sp[0]);
   for (i=1; i <= sp[0]; i++)
      fprintf(stderr, " %s,", STname[sp[i]-1]);
   fprintf(stderr, "\n");
#endif
/*
 * filter out those which don't belong to init packs
 */
   bvi = Array2BitVec(npt, sp+1, TNREG-1); 
   bvp = NewBitVec(128); 
   SetVecAll(bvp, 0);
   for (i=0; i < NPACK; i++)
   {
      pk = PACKS[i];
      if (pk)
      {
         bvp = BitVecComb(bvp, bvp, pk->uses, '|' );
      }
   }
   bvp = BitVecComb(bvp, bvp, bvi, '&');
   pt = BitVec2Array(bvp, 1-TNREG);
#if 0
   fprintf(stderr, "PTR(%d) : ", pt[0]);
   for (i=1; i <= pt[0]; i++)
      fprintf(stderr, " %s,", STname[pt[i]-1]);
   fprintf(stderr, "\n");
#endif

}
#endif

void SwapPack(int pi, int pj)
{
   int i, j;
   PACK *pk;

   i = PACKS[pi]->pnum;
   j = PACKS[pj]->pnum; 
   
   pk = PACKS[pi];
   PACKS[pi] = PACKS[pj];
   PACKS[pi]->pnum = i;
   PACKS[pj] = pk;
   PACKS[pj]->pnum = j;
}

void SortPackBasedOnPtr(short *pts, int inpack)
{
   int i, j, k, n;
   short ptr;
   PACK *pk;

#if 0
   fprintf(stderr, "intit pack = ");
   for (i=0; i < NPACK; i++)
   {
      if (PACKS[i])
         fprintf(stderr, "%d(%d) ", i, PACKS[i]->pnum);
   }
   fprintf(stderr, "\n");
#endif

   for (i=1, j=inpack, n=pts[0]; i <= n; i++)
   {
      ptr = pts[i];
/*
 *    skip if j starts with pack with ptr
 */
      for ( ; j < NPACK; j++)
      {
         pk = PACKS[j];
         if (pk && !BitVecCheck(pk->uses, ptr + TNREG-1))
            break;
      }
/*
 *    j points a pack which doesn't contain ptr
 */
      for (k = j + 1; k < NPACK; k++)
      {
         pk = PACKS[k];
         if (pk && BitVecCheck(pk->uses, ptr + TNREG-1))
         {
            SwapPack(j, k);
            j++;
         }
      }
   }
}

INSTQ *FindInitPackFromVlist(INSTQ *upackq, SLP_VECTOR *vl, int *change)
{
   int i, k, n;
   int isdef = 1;
   ILIST *il, *iln = NULL;
   PACK *new;
   INSTQ *ip, *ipu, *ip0, *ipdup = NULL;
   INT_BVI iv;
   extern INT_BVI FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(128);
   iv = FKO_BVTMP;

#if 1
/*
 * create pack using mem broadcast for the vector
 */
   n = vl->svars[0];
   if (n > 1)
   {
      k = vl->svars[1];
      for (i=2; i <= n; i++)
      {
         if (k != vl->svars[i])
            break;
      }
      if (i > n)
      {
         SetVecAll(iv, 0); 
         SetVecBit(iv, vl->svars[1]+TNREG-1, 1);
         ipu = InstdefsVar(iv, upackq);     
         if(ipu) 
         {
            k = 0;
            ip = ipu; 
            while (IS_LOAD(ip->inst[0]))
            {
               if (NonLocalDeref(ip->inst[2]))
               {
                  k = 1;
                  break;
               }
               ip = ip->next;
            }
            if (k)
            {
               /*fprintf(stderr, "*** %s is a candidate of boardcast\n", 
                     STname[vl->svars[1]-1]);*/
               assert(IS_LOAD(ipu->inst[0]));
               ipdup = ip0 = NULL;
               do
               {
                  ipdup = NewInst(NULL, ipdup, NULL, ipu->inst[0], ipu->inst[1], 
                                  ipu->inst[2], ipu->inst[3]);
                  CalcThisUseSet(ipdup);
                  if (ip0)
                     ip0->next = ipdup;
                  ip0 = ipdup;
                  ipu = ipu->next;
               } while (!IS_STORE(ipdup->inst[0])); 
         
               while(ipdup->prev) ipdup = ipdup->prev;
               iln = NewIlistAtEnd(ipdup, iln); 
               new = NewPack(iln, vl->vlen);
               new->vflag = vl->flag; /* vl->flag == pk->vflag*/
               new->pktype = PK_INIT_VLIST;
               new->pktype |= PK_MEM_BROADCAST;
               upackq = FinalizePack(new, upackq, change); 
               return(upackq);
            }
         }

      }
   }
#endif

   SetVecAll(iv, 0); 
   for (i=1, n=vl->svars[0], k=0; i <= n; i++)
   {
      /*SetVecAll(iv, 0); */
      SetVecBit(iv, vl->svars[i]+TNREG-1, 1);
#if 0
      ipu = InstdefsVar(iv, upackq);
#else
      if (isdef)
      {
         ipu = InstdefsVar(iv, upackq);         
         if (!ipu)
         {
            isdef = 0;
            ipu = InstFirstUseVar(iv, upackq);
         }
      }
      else
         ipu = InstFirstUseVar(iv, upackq);
#endif
      if (ipu)
      {
/*
 *       FIXME: after supporting accumulator expansion, we may have inst group
 *       with no loads, like:
 *          FZEROD reg;
 *          FSTD var, reg
 */
         //assert(IS_LOAD(ipu->inst[0]));
         ipdup = ip0 = NULL;
         do
         {
            ipdup = NewInst(NULL, ipdup, NULL, ipu->inst[0], ipu->inst[1], 
                               ipu->inst[2], ipu->inst[3]);
            CalcThisUseSet(ipdup);
            if (ip0)
               ip0->next = ipdup;
            ip0 = ipdup;
            ipu = ipu->next;
         } while (!IS_STORE(ipdup->inst[0])); 
         
         while(ipdup->prev) ipdup = ipdup->prev;
         iln = NewIlistAtEnd(ipdup, iln);
         k++;
#if 0
         PrintThisInstQ(stderr, ipdup);
#endif
      }
      else /* no pack using this vector, don't report error, just return*/
      {
         fko_warn(__LINE__, "no def/use found for this var=%s", 
                  STname[vl->svars[i]-1]);
         *change = 0;
         return(upackq);
      }
      
      SetVecBit(iv, vl->svars[i]+TNREG-1, 0);
   }
/*
 * finalize pack
 */
   assert(k==vl->vlen);
   for (il=iln; il; il=il->next)
   {
      ip0 = iln->inst;
      ip = il->inst;
      while (ip)
      {
         if (!IsIsomorphicInst(ip, ip0))
         {
#if 0
            PrintThisInst(stderr, 1, ip);
            PrintThisInst(stderr, 2, ip0);
            fko_error(__LINE__, "not isomorphic inst\n");
#else
            KillAllIlist(iln);
            if (ipdup)
               KillAllInst(ipdup);
            *change = 0;
            return(upackq);
#endif
         }
         ip = ip->next;
         ip0 = ip0->next;
      }
   }
   new = NewPack(iln, k);
   new->vflag = vl->flag; /* vl->flag == pk->vflag*/
   new->pktype = PK_INIT_VLIST;
#if 0
   fprintf(stderr, "vl->flag = %d\n", vl->flag);
#endif
   upackq = FinalizePack(new, upackq, change); 
   return(upackq);
}



int FindRedVarN(BBLOCK *blk, short *svars)
/*
 * Format supported:
 *    1. rC00 = rC00a + rC00b;
 *       rC00 += rC00c + rC00d;
 *       --- no longer happens since we apply local variable renaming!
 *    2. rC00a += rC00b;
 *       rC00a += rC00c;
 *       rC00a += rC00d;
 *       ---- won't support any more. need compliated analysis
 *    need to support: 
 *    3. rC00_1 = rC00a + rC00b
 *       rC00_2 = rC001 + rC00c;
 *       rC00_3 = rC003 + rC00c;
 *       ---- case-1 would be converted into this case after renaming private 
 *       variable
 *    Assumption: all the operands (svars) are never used later without 
 *    accumulation
 */
{
   int i, j, k, n;
   int dest = 0;
   int *scount; 
   INSTQ *ip, *ip0;

#if 0
   fprintf(stderr, "svars: ");
   for (i=1, n=svars[0]; i <= n; i++)
      fprintf(stderr, "%s ", STname[svars[i]-1]);
   fprintf(stderr, "\n");
   exit(0);
#endif
   
   n = svars[0];
   scount = calloc((n+1), sizeof(int));
   assert(scount);
   
   for (ip=blk->ainst1; ip; ip=ip->next)
   {
      if (IS_LOAD(ip->inst[0]) && !NonLocalDeref(ip->inst[2]))
      {
         k = 0;
         while (ip && IS_LOAD(ip->inst[0]))
         {
            j = FindInShortList(svars[0], svars+1, STpts2[ip->inst[2]-1]); 
            if (j)
            {
               scount[j]++;
               k = 1;
            }
            ip = ip->next;
         }
         if (k)
         {
            if (ip->inst[0] == FADD || ip->inst[0] == FADDD)
            {
/*
 *             both operands must be either last dest or one of svars
 */
               ip0 = ip->prev;
               while (ip0 && IS_LOAD(ip0->inst[0]))
               {
                  if (!FindInShortList(svars[0], svars+1, 
                                       STpts2[ip0->inst[2]-1])
                        && STpts2[ip0->inst[2]-1] != dest)
                  {
                     free(scount);
                     return(0);
                  }
                  ip0 = ip0->prev;
               }
/*
 *             assign the new dest
 */
               ip = ip->next;
               assert(IS_STORE(ip->inst[0]));
               dest = STpts2[ip->inst[1]-1];
            }
            else /* op must be ADD*/
            {
               free(scount);
               return(0);
            }
         }
      }
   }
/*
 * check whether all value od scount is exactly 1
 */
   for (i=1, n=svars[0]; i <= n; i++)
   {
      if (scount[i] != 1)
      {
         fprintf(stderr, "not 1!!!\n");
         free(scount);
         return(0);
      }
   }
   free(scount);
#if 0 
   if (dest)
   {
      fprintf(stderr, "final accumulator = %s\n", STname[dest-1]);
   }
#endif
   return(dest);
}

int FindRedVarLgN(BBLOCK *blk, short *svars)
/*
 * supporting format: 
 *    <0> += <1>
 *    <2> += <3>
 *    <4> += <5>
 *    <6> += <7>
 *    <0> += <2>
 *    <4> += <6>
 *    <0> += <4>
 * NOTE: Generated by PreSlpAccumExpans(). also works if user follows this 
 * sequence
 */
{
   int i, j, k, ne;
   int i1, i2, type, dest;
   enum inst inst;
   INSTQ *ip;
#if 0
   for (i=1, n=svars[0]; i <= n; i++)
      fprintf(stderr, "%d:  %s\n", i-1, STname[svars[i]-1]);
   exit(0);
#endif
   assert(svars[0] > 1);
   type = FLAG2TYPE(STflag[svars[1]-1]);
   switch(type)
   {
      case T_FLOAT:
         inst = FADD;
         break;
      case T_DOUBLE:
         inst = FADDD;
         break;
      default:
         fko_error(__LINE__, "Unknown type file %s", __FILE__);
   }
   ip = blk->ainst1;
   dest = svars[1]; 
   ne = svars[0];
   for (i=1, j=1; i < ne; i <<= 1, j--)
   {
      j = j << 1;
      for (k=0; k < ne/j; k++)
      {
         i1 = k*(i+i);
         i2 = i1 + i;
         if (i1 < ne && i2 < ne)
         {
            while (ip)
            {
               if (IS_LOAD(ip->inst[0]) && svars[i1+1] == STpts2[ip->inst[2]-1])
               {
                  ip = ip->next;
                  if (IS_LOAD(ip->inst[0]) 
                        && svars[i2+1] == STpts2[ip->inst[2]-1])
                  {
                     ip = ip->next;
                     if (ip->inst[0] = inst)
                     {
                        ip = ip->next;
                        break;
                     }
                     else
                        return(0);
                  }
                  else
                     return(0);
               }
               ip = ip->next;
            }
         }
      }
   }
   return(dest);
}

int FindRedVar(BBLOCK *blk, short *svars)
{
   int dest = 0; 

   dest = FindRedVarLgN(blk, svars);
#if 0
   if (dest)
      fprintf(stderr, "dest = %s\n", STname[dest-1]);
   else
      fprintf(stderr, "red var not found!!!\n");
#endif
   if (!dest)
      dest = FindRedVarN(blk, svars);
   return(dest);
}

int CheckRedVarforVectors(SLP_VECTOR *vlist, BBLOCK *blk)
{
   int rvar;
   int suc = 1;
   SLP_VECTOR *vl;

   for (vl=vlist; vl; vl=vl->next)
   {
      rvar = FindRedVar(blk, vl->svars);
      if (!rvar)
      {
         suc = 0;
      }
      else
      {
         vl->redvar = rvar;
         /*fprintf(stderr, "vec=%s, redvar=%s\n", STname[vl->vec-1], 
               STname[rvar-1]);*/
         fprintf(stderr, "vec=%s, redvar=%s\n", STname[vl->vec-1], 
               STname[rvar-1]);
      }
   }
   return(suc);
}

int CountRvarVect(BBLOCK *blk, SLP_VECTOR *vlist)
{
   int rvar;
   int nr = 0;
   SLP_VECTOR *vl;
   
   for (vl=vlist; vl; vl=vl->next)
   {
/*
 *    FIXME: can we depend on vectors islivein? or, should we match it with 
 *    blk->ins
 */
      if (vl->islivein) /* vector with redvar must be livein*/
      {
         if (vl->redvar) /* already known the redvar, possibly by LNZV method*/
            nr++;
         else
         {
            rvar = FindRedVar(blk, vl->svars);
            if (rvar)
            {
               vl->redvar = rvar;
               nr++;
            }
         }
      }
   }
   return(nr); 
}


INSTQ *FVVRSUM8(SLP_VECTOR *rvl, SLP_VECTOR *vd)
{
   int i;
   SLP_VECTOR *vl;
   INSTQ *ip, *ip0;
   static int vid = 0;
   char vname[64];
   short ns0, ns1, ns2, ns4, ns6; 
   short vs;
   short s[8];
   short vreg0, vreg1, vreg2;
   const int nv = 8;

   vs = vd->vec;
/*
 * create tmp vectors, not needed to save them in list
 * NOTE: var names are copied inside symbol table
 */
   sprintf(vname, "_NVRS8_%d", vid++);
   ns0 = InsertNewLocal(vname, T_VFLOAT);

   sprintf(vname, "_NVRS8_%d", vid++);
   ns1 = InsertNewLocal(vname, T_VFLOAT);

   sprintf(vname, "_NVRS8_%d", vid++);
   ns2 = InsertNewLocal(vname, T_VFLOAT);

   sprintf(vname, "_NVRS8_%d", vid++);
   ns4 = InsertNewLocal(vname, T_VFLOAT);
   
   sprintf(vname, "_NVRS8_%d", vid++);
   ns6 = InsertNewLocal(vname, T_VFLOAT);
/*
 * init s var
 * Assumption: must have 8 vec now... we will relax that later 
 */
   vl = rvl;
   for (i=0; i < nv; i++)
   {
#if 0
      if (vl)
         s[i] = vl->vec;
      else
         s[i] = 0;
#else
      assert(vl && vl->vec);
      s[i] = vl->vec;
#endif
      vl = vl->next;
   }
/*
 *    calc:
 *       input: s0, s1, s2, s3, s4, s5, s6, s7
 *       out: vs
 *       tmp: ns0, ns1, ns2, ns4, ns6, 
 *
 *       ns0 = HADD(s0, s1);
 *       ns2 = HADD(s2, s3);
 *       ns4 = HADD(s4, s5);
 *       ns6 = HADD(s6, s7);
 *       
 *       ns0 = HADD (ns0, ns2);
 *       ns4 = HADD (ns4, ns6);
 *
 *       ns1 = PERMUTE2f128(ns0, ns4, 0x31);
 *       ns0 = PERMUTE2f128(ns0, ns4, 0x20);
 *       
 *       vs = VADD(ns0, ns1);
 *    
 *    Alternative sequence:
 *    ---------------------- 
 *       ns0 = HADD(s0,s1);
 *       ns1 = HADD(s2,s3);
 *       ns0 = HADD(ns0, ns1);
 *
 *       ns1 = HADD(s4,s5);
 *       ns2 = HADD(s6,s7);
 *       ns1 = HADD(ns1, ns2);
 *     
 *       ns2 = ns1;
 *       ns1 = PERMUTEF128(ns0, ns1,0x31); 
 *       ns0 = PERMUTEF128(ns0,ns2, 0x20); 
 *
 *       vs = VADD(ns0, ns2);
 */
#ifndef AVX
   fko_error(__LINE__, "fvvrsum8 can only be called in AVX machine!");
#endif

   vreg0 = GetReg(T_VFLOAT);
   vreg1 = GetReg(T_VFLOAT);
   vreg2 = GetReg(T_VFLOAT);

   //ns0 = HADD(s0, s1);  rd, rs1, rs2 
   ip = ip0 = NewInst(NULL, NULL, NULL, VFLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[1]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //ns2 = HADD(s2, s3);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[s[2]-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[3]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns2-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //ns4 = HADD(s4, s5);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[s[4]-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[5]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns4-1].sa[2], -vreg2, 0); 
   ip = ip->next;          
   
   //ns6 = HADD(s6, s7);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[s[6]-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[7]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns6-1].sa[2], -vreg2, 0); 
   ip = ip->next;          
  
   //ns0 = HADD (ns0, ns2);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns2-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

   //ns4 = HADD (ns4, ns6);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns4-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns6-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns4-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

   //ns1 = PERMUTE2f128(ns0, ns4, 0x31);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns4-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHIHALF, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns1-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //ns0 = PERMUTE2f128(ns0, ns4, 0x20);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns4-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFLOHALF, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

   //vs = VADD(ns0, ns1);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns1-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFADD, -vreg0, -vreg0, -vreg1 );
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

   GetReg(-1);

   return(ip0);
}

INSTQ *DVVRSUM4(SLP_VECTOR *rvl, SLP_VECTOR *vd)
{
   int i;
   SLP_VECTOR *vl;
   INSTQ *ip, *ip0;
   static int vid = 0;
   char vname[64];
   short ns0, ns1, ns2;
   short vs;
   short s[4];
   short vreg0, vreg1, vreg2;
   const int nv = 4;

   vs = vd->vec;
/*
 * create tmp vectors, not needed to save them in list
 * NOTE: var names are copied inside symbol table
 */
   sprintf(vname, "_NDVRS4_%d", vid++);
   ns0 = InsertNewLocal(vname, T_VDOUBLE);

   sprintf(vname, "_NDVRS4_%d", vid++);
   ns1 = InsertNewLocal(vname, T_VDOUBLE);
   
   sprintf(vname, "_NDVRS4_%d", vid++);
   ns2 = InsertNewLocal(vname, T_VDOUBLE);
/*
 * init s var
 * Assumption: must have 4 vec now... we will relax that later 
 */
   vl = rvl;
   for (i=0; i < nv; i++)
   {
      assert(vl && vl->vec);
      s[i] = vl->vec;
      vl = vl->next;
   }

/*
 *    calc:
 *       input: s0, s1, s2, s3
 *       out: vs
 *       tmp: ns0, ns1, ns2 
 *
 *       ns0 = HADD(s0, s1);
 *       ns2 = HADD(s2, s3);
 *
 *       ns1 = PERMUTE2F128(ns0, ns2, 0x31);
 *       ns0 = PERMUTE2F128(ns0, ns2, 0x20);
 *
 *       vs = VADD(ns0, ns1);
 */
#ifndef AVX
   fko_error(__LINE__, "dvvrsum4 can only be called in AVX machine!");
#endif

   vreg0 = GetReg(T_VDOUBLE);
   vreg1 = GetReg(T_VDOUBLE);
   vreg2 = GetReg(T_VDOUBLE);

   //ns0 = HADD(s0, s1);  rd, rs1, rs2 
   ip = ip0 = NewInst(NULL, NULL, NULL, VDLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[s[1]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[ns0-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //ns2 = HADD(s2, s3);
   ip->next= NewInst(NULL, ip, NULL, VDLD, -vreg0, SToff[s[2]-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[s[3]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[ns2-1].sa[2], -vreg2, 0); 
   ip = ip->next;          
   
   //ns1 = PERMUTE2F128(ns0, ns2, 0x31);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[ns2-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDHIHALF, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[ns1-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //ns0 = PERMUTE2F128(ns0, ns2, 0x20);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[ns2-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDLOHALF, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          
   
   //vs = VADD(ns0, ns1);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[ns1-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDADD, -vreg0, -vreg0, -vreg1 );
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

   GetReg(-1);

   return(ip0);
}

INSTQ *FVVRSUM4(SLP_VECTOR *rvl, SLP_VECTOR *vd)
{
   int i;
   SLP_VECTOR *vl;
   INSTQ *ip, *ip0;
   static int vid = 0;
   char vname[64];
   short ns0, ns1, ns2;
   short vs;
   short s[4];
   short vreg0, vreg1, vreg2;
   const int nv = 4;

   vs = vd->vec;
/*
 * create tmp vectors, not needed to save them in list
 * NOTE: var names are copied inside symbol table
 */
   sprintf(vname, "_NVRS4_%d", vid++);
   ns0 = InsertNewLocal(vname, T_VFLOAT);

#ifdef AVX
   sprintf(vname, "_NVRS4_%d", vid++);
   ns1 = InsertNewLocal(vname, T_VFLOAT);
#endif

   sprintf(vname, "_NVRS4_%d", vid++);
   ns2 = InsertNewLocal(vname, T_VFLOAT);
/*
 * init s var
 * Assumption: must have 8 vec now... we will relax that later 
 */
   vl = rvl;
   for (i=0; i < nv; i++)
   {
      assert(vl && vl->vec);
      s[i] = vl->vec;
      vl = vl->next;
   }

/*
 *    calc:
 *    input: s0, s1, s2, s3
 *    out: vs
 *    FOR AVX
 *       tmp: ns0, ns1, ns2
 *
 *       ns0 = HADD(s0, s1);
 *       ns2 = HADD(s2, s3);
 *
 *       ns0 = HADD(ns0, ns2);
 *
 *       ns1 = PERMUTE2f128(ns0, ns0, 0x31);
 *       ns0 = PERMUTE2f128(ns0, ns0, 0x20);
 *       
 *       vs = VADD(ns0, ns1);
 *    
 *    FOR SSE3:
 *       tmp: ns0, ns1
 *       
 *       ns0 = HADD(s0, s1);
 *       ns2 = HADD(s2, s3);
 *       vs = HADD(ns0, ns2);
 */
   vreg0 = GetReg(T_VFLOAT);
   vreg1 = GetReg(T_VFLOAT);
   vreg2 = GetReg(T_VFLOAT);

#if defined(AVX)

   //ns0 = HADD(s0, s1); rd, rs1, rs2
   ip = ip0 = NewInst(NULL, NULL, NULL, VFLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[1]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //ns2 = HADD(s2, s3);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[s[2]-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[3]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns2-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //ns0 = HADD(ns0, ns2);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns2-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          
   
/* ns1 = PERMUTE2f128(ns0, ns0, 0x31); */
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFHIHALF, -vreg1, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns1-1].sa[2], -vreg1, 0); 
   ip = ip->next;  

/* ns0 = PERMUTE2f128(ns0, ns0, 0x20); */
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLOHALF, -vreg0, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

   //vs = VADD(ns0, ns1);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns1-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFADD, -vreg0, -vreg0, -vreg1 );
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

#elif defined(SSE3)
   
   //ns0 = HADD(s0, s1);
   ip = ip0 = NewInst(NULL, NULL, NULL, VFLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[1]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

   //ns2 = HADD(s2, s3);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[s[2]-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[3]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns2-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

   //vs = HADD(ns0, ns2);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns2-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

#else
   fko_error(__LINE__, "fvvrsum4 is not implemented in this architecture");
#endif

   GetReg(-1);

   return(ip0);
}

INSTQ *DVVRSUM2(SLP_VECTOR *rvl, SLP_VECTOR *vd)
{
   int i;
   SLP_VECTOR *vl;
   INSTQ *ip, *ip0;
   static int vid = 0;
   char vname[64];
#ifdef AVX
   short ns0, ns1;
#endif
   short vs;
   short s[2];
   short vreg0, vreg1, vreg2;
   const int nv = 2;

   vs = vd->vec;
#if 0
   PrintVectors(stderr, vd);
   fprintf(stderr, " vvrsum(%d) = %s\n", vs, STname[vs-1]);
#endif
/*
 * create tmp vectors, not needed to save them in list
 * NOTE: var names are copied inside symbol table
 */
#ifdef AVX
   sprintf(vname, "_NDVRS2_%d", vid++);
   ns0 = InsertNewLocal(vname, T_VDOUBLE);

   sprintf(vname, "_NDVRS2_%d", vid++);
   ns1 = InsertNewLocal(vname, T_VDOUBLE);
#endif
/*
 * init s var
 * Assumption: must have 4 vec now... we will relax that later 
 */
   vl = rvl;
   for (i=0; i < nv; i++)
   {
      assert(vl && vl->vec);
      s[i] = vl->vec;
      vl = vl->next;
   }
/*
 *    calc:
 *    input: s0, s1
 *    out: vs
 *    FOR AVX
 *       tmp: ns0, ns1
 *
 *       ns0 = HADD(s0, s1);
 *       ns1 = PERMUTE2f128(ns0, s1, 0x31);
 *       vs = VADD(ns0, ns1);
 *    
 *    SSE3:
 *       vs = HADD(s0,s1);
 */

   vreg0 = GetReg(T_VDOUBLE);
   vreg1 = GetReg(T_VDOUBLE);
   vreg2 = GetReg(T_VDOUBLE);

#if defined(AVX)
   
   //ns0 = HADD(s0, s1);
   ip = ip0 = NewInst(NULL, NULL, NULL, VDLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[s[1]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[ns0-1].sa[2], -vreg2, 0); 
   ip = ip->next;         

   //ns1 = PERMUTE2f128(ns0, s1, 0x31);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[s[1]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDHIHALF, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[ns1-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //vs = VADD(ns0, ns1);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[ns1-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDADD, -vreg2, -vreg0, -vreg1 );
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[vs-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

#elif defined(SSE3)
   
   //vs = HADD(s0,s1);
   ip = ip0 = NewInst(NULL, NULL, NULL, VDLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[s[1]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDHADD, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;         

#else
   fko_error(__LINE__, "dvvrsum2 is not implemented in this architecture");
#endif

   GetReg(-1);

   return(ip0);
}

INSTQ *FVVRSUM2(SLP_VECTOR *rvl, SLP_VECTOR *vd)
{
   int i;
   SLP_VECTOR *vl;
   INSTQ *ip, *ip0;
   static int vid = 0;
   char vname[64];
   short ns0;
#ifdef AVX
   short ns1;
#endif
   short vs;
   short s[2];
   short vreg0, vreg1, vreg2;
   const int nv = 2;

   vs = vd->vec;
/*
 * create tmp vectors, not needed to save them in list
 * NOTE: var names are copied inside symbol table
 */
   sprintf(vname, "_NVRS2_%d", vid++);
   ns0 = InsertNewLocal(vname, T_VFLOAT);

#ifdef AVX
   sprintf(vname, "_NVRS2_%d", vid++);
   ns1 = InsertNewLocal(vname, T_VFLOAT);
#endif
/*
 * init s var
 * Assumption: must have 4 vec now... we will relax that later 
 */
   vl = rvl;
   for (i=0; i < nv; i++)
   {
      assert(vl && vl->vec);
      s[i] = vl->vec;
      vl = vl->next;
   }
/*
 *    calc:
 *    input: s0, s1
 *    out: vs
 *    FOR AVX
 *       tmp: ns0, ns1
 *
 *       ns0 = HADD(s0, s1);
 *       ns0 = HADD(ns0, ns0);
 *
 *       ns1 = PERMUTE2f128(ns0, ns0, 0x31);
 *       ns0 = PERMUTE2f128(ns0, ns0, 0x20);
 *
 *       vs = VADD(ns0, ns1);
 *    
 *    SSE3:
 *       ns0 = HADD(s0, s1);
 *       vs = HADD(ns0, ns0);
 */
   vreg0 = GetReg(T_VFLOAT);
   vreg1 = GetReg(T_VFLOAT);
   vreg2 = GetReg(T_VFLOAT);

#if defined(AVX)
   //ns0 = HADD(s0, s1);
   ip = ip0 = NewInst(NULL, NULL, NULL, VFLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[1]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg2, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //ns0 = HADD(ns0, ns0);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          
   
   //ns1 = PERMUTE2f128(ns0, ns0, 0x31);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFHIHALF, -vreg1, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns1-1].sa[2], -vreg1, 0); 
   ip = ip->next;  

   //ns0 = PERMUTE2f128(ns0, ns0, 0x20);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLOHALF, -vreg0, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;         

   //vs = VADD(ns0, ns1);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns1-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFADD, -vreg0, -vreg0, -vreg1 );
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

#elif defined(SSE3)
   //ns0 = HADD(s0, s1);
   ip = ip0 = NewInst(NULL, NULL, NULL, VFLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[s[1]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg1);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          
   
   //vs = HADD(ns0, ns0);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

#else
   fko_error(__LINE__, "fvvrsum2 is not implemented in this architecture");
#endif

   GetReg(-1);

   return(ip0);
}

INSTQ *DVVRSUM1(SLP_VECTOR *rvl, SLP_VECTOR *vd)
{
   SLP_VECTOR *vl;
   INSTQ *ip, *ip0;
   static int vid = 0;
   char vname[64];
#ifdef AVX
   short ns0, ns1;
#endif
   short vs;
   short s[1];
   short vreg0, vreg1, vreg2;

   vs = vd->vec;

/*
 * create tmp vectors, not needed to save them in list
 * NOTE: var names are copied inside symbol table
 */
#ifdef AVX
   sprintf(vname, "_NDVRS1_%d", vid++);
   ns0 = InsertNewLocal(vname, T_VDOUBLE);

   sprintf(vname, "_NDVRS1_%d", vid++);
   ns1 = InsertNewLocal(vname, T_VDOUBLE);
#endif
/*
 * init s var
 * Assumption: must have 4 vec now... we will relax that later 
 */
   vl = rvl;
   s[0] = vl->vec;
/*
 *    calc:
 *    input: s0
 *    out: vs
 *    FOR AVX
 *       tmp: ns0, ns1
 *
 *       ns0 = HADD(s0, s0);
 *       ns1 = PERMUTE2f128(ns0, s0, 0x31); ???
 *       vs = VADD(ns0, ns1);
 *    
 *    SSE3:
 *       vs = HADD(s0,s0);
 */

   vreg0 = GetReg(T_VDOUBLE);
   vreg1 = GetReg(T_VDOUBLE);
   vreg2 = GetReg(T_VDOUBLE);

#if defined(AVX)
   
   //ns0 = HADD(s0, s0);
   ip = ip0 = NewInst(NULL, NULL, NULL, VDLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VDHADD, -vreg1, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[ns0-1].sa[2], -vreg1, 0); 
   ip = ip->next;         

   //ns1 = PERMUTE2f128(ns0, s0, 0x31);
   //ns1 = PERMUTE2f128(ns0, ns0, 0x31);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[s[0]-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDHIHALF, -vreg2, -vreg0, -vreg1);
   //ip->next = NewInst(NULL, ip, NULL, VDHIHALF, -vreg2, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[ns1-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

   //vs = VADD(ns0, ns1);
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VDLD, -vreg1, SToff[ns1-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDADD, -vreg2, -vreg0, -vreg1 );
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[vs-1].sa[2], -vreg2, 0); 
   ip = ip->next;          

#elif defined(SSE3)
   
   //vs = HADD(s0,s0);
   ip = ip0 = NewInst(NULL, NULL, NULL, VDLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VDHADD, -vreg0, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VDST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;         

#else
   fko_error(__LINE__, "dvvrsum2 is not implemented in this architecture");
#endif

   GetReg(-1);

   return(ip0);
   
}

INSTQ *FVVRSUM1(SLP_VECTOR *rvl, SLP_VECTOR *vd)
{
   SLP_VECTOR *vl;
   INSTQ *ip, *ip0;
   static int vid = 0;
   char vname[64];
   short ns0;
#ifdef AVX
   short ns1;
#endif
   short vs;
   short s[1];
   short vreg0, vreg1;

   vs = vd->vec;
/*
 * create tmp vectors, not needed to save them in list
 * NOTE: var names are copied inside symbol table
 */
   sprintf(vname, "_NVRS1_%d", vid++);
   ns0 = InsertNewLocal(vname, T_VFLOAT);

#ifdef AVX
   sprintf(vname, "_NVRS1_%d", vid++);
   ns1 = InsertNewLocal(vname, T_VFLOAT);
#endif
/*
 * init s var
 * Assumption: must have 4 vec now... we will relax that later 
 */
   vl = rvl;
   s[0] = vl->vec;
/*
 *    calc:
 *    input: s0, s1
 *    out: vs
 *    FOR AVX
 *       tmp: ns0, ns1
 *
 *       ns0 = HADD(s0, s0);
 *       ns0 = HADD(ns0, ns0);
 *
 *       ns1 = PERMUTE2f128(ns0, ns0, 0x31);
 *       ns0 = PERMUTE2f128(ns0, ns0, 0x20);
 *
 *       vs = VADD(ns0, ns1);
 *    
 *    SSE3:
 *       ns0 = HADD(s0, s0);
 *       vs = HADD(ns0, ns0);
 */
   vreg0 = GetReg(T_VFLOAT);
   vreg1 = GetReg(T_VFLOAT);

#if defined(AVX)
   //ns0 = HADD(s0, s0);
   ip = ip0 = NewInst(NULL, NULL, NULL, VFLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg1, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg1, 0); 
   ip = ip->next;          

   //ns0 = HADD(ns0, ns0);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          
   
   //ns1 = PERMUTE2f128(ns0, ns0, 0x31);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFHIHALF, -vreg1, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns1-1].sa[2], -vreg1, 0); 
   ip = ip->next;  

   //ns0 = PERMUTE2f128(ns0, ns0, 0x20);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLOHALF, -vreg0, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;         

   //vs = VADD(ns0, ns1);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg1, SToff[ns1-1].sa[2], 0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFADD, -vreg0, -vreg0, -vreg1 );
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

#elif defined(SSE3)
   //ns0 = HADD(s0, s0);
   ip = ip0 = NewInst(NULL, NULL, NULL, VFLD, -vreg0, SToff[s[0]-1].sa[2], 0);
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[ns0-1].sa[2], -vreg0, 0); 
   ip = ip->next;          
   
   //vs = HADD(ns0, ns0);
   ip->next = NewInst(NULL, ip, NULL, VFLD, -vreg0, SToff[ns0-1].sa[2], 0);
   ip = ip->next;
   ip->next = NewInst(NULL, ip, NULL, VFHADD, -vreg0, -vreg0, -vreg0);
   ip = ip->next;          
   ip->next = NewInst(NULL, ip, NULL, VFST, SToff[vs-1].sa[2], -vreg0, 0); 
   ip = ip->next;          

#else
   fko_error(__LINE__, "fvvrsum2 is not implemented in this architecture");
#endif

   GetReg(-1);

   return(ip0);

}

INSTQ *InstVVRSUM(SLP_VECTOR *rvl, int len, SLP_VECTOR *vd)
{
   short type;
   INSTQ *ip = NULL;

#if 0
   fprintf(stderr, "Applying VVRSUM_%d\n", len);
#endif
   type = FLAG2TYPE(rvl->flag);
   if (type == T_FLOAT  || type == T_VFLOAT)
   {
      switch(len)
      {
         case 1: 
            ip = FVVRSUM1(rvl, vd);
            break;
         case 2:
            ip = FVVRSUM2(rvl, vd);
            break;
         case 4:
            ip = FVVRSUM4(rvl, vd);
            break;
         case 8:
            ip = FVVRSUM8(rvl, vd);
            break;
         default:
            fko_error(__LINE__, "not supported fvvrsum for %d\n", len);
      }
   }
   else if (type == T_DOUBLE  || type == T_VDOUBLE)
   {
      switch(len)
      {
         case 1: 
            ip = DVVRSUM1(rvl, vd);
            break;
         case 2:
            ip = DVVRSUM2(rvl, vd);
            break;
         case 4:
            ip = DVVRSUM4(rvl, vd);
            break;
         default:
            fko_error(__LINE__, "not supported dvvrsum for %d\n", len);
      }
   }
   else
      fko_error(__LINE__, "unknown type!");
#if 1
   {
      char ln[2048];
      sprintf(ln, "VVRSUM code for length=%d", len);
      ip = NewInst(NULL, NULL, ip, COMMENT, STstrconstlookup(ln), 0, 0);
   }
#endif

   return(ip);
}

void DelRedCodeFromRvars(BBLOCK *blk, short *svars, short rvar)
{
   int i, ok, n;
   int isend;
   INSTQ *ip, *ip0;
   
   ip = blk->ainst1;
   while(ip)
   {
      if (IS_LOAD(ip->inst[0]) && !NonLocalDeref(ip->inst[2]-1))
      {
#if 0         
         ok = 1;
         while(IS_LOAD(ip->inst[0]))
         {
            if (STpts2[ip->inst[2]-1] != rvar
                  && !FindInShortList(svars[0], svars+1, STpts2[ip->inst[2]-1]))
               ok = 0;
            ip = ip->next;
         }
         if (ok)
         {
            if (ip->inst[0] == FADD || ip->inst[0] == FADDD)
            {
               if (ip->next && IS_STORE(ip->next->inst[0]))
               {
                  if (STpts2[ip->next->inst[1]-1] == rvar 
                        || FindInShortList(svars[0], svars+1, 
                                           STpts2[ip->next->inst[1]-1]))
                  {
                     ip0 = FindFirstLILforHIL(ip);
                     isend = 0;
                     while(ip0)
                     {
                        if(IS_STORE(ip0->inst[0]) )
                           isend = 1;
                        PrintThisInst(stderr, 1, ip0);
                        ip0 = DelInst(ip0);
                        if (isend)
                           break;
                     }
                     ip = blk->ainst1;
                  }
               }
            }
         }
#else
/*
 *       assumption: LIL with svar which is not rvar creates reduction code. 
 *       we ensure it in analysis
 */
         ok = 0;
         while(IS_LOAD(ip->inst[0]))
         {
            if (FindInShortList(svars[0], svars+1, STpts2[ip->inst[2]-1])
                  && STpts2[ip->inst[2]-1] != rvar)
               ok = 1;
            ip = ip->next;
         }
         if (ok)
         {
            assert(ip->inst[0] == FADD || ip->inst[0] == FADDD);
            ip0 = FindFirstLILforHIL(ip);
            isend = 0;
            while(ip0)
            {
               if(IS_STORE(ip0->inst[0]) )
                  isend = 1;
               ip0 = DelInst(ip0);
               if (isend)
                  break;
            }
            ip = blk->ainst1;
         }
#endif
      }
      else
         ip = ip->next;
   }
/*
 * no longer should have the reduction code
 */
   /*assert(!FindRedVar(blk, svars));*/
   for (i=1, n = svars[0]; i <= n; i++)
      if (svars[i] != rvar) /* if not the rvar, must be deleted */
         for (ip=blk->ainst1; ip; ip=ip->next)
            if (IS_LOAD(ip->inst[0]) && STpts2[ip->inst[2]-1] == svars[i])
               fko_error(__LINE__, "Reduction code not deleted properly!");
}

INSTQ *AddVectorScatterInst(INSTQ *ip0, SLP_VECTOR *vld, int nelem)
/*
 * added at the end of ipn 
 */
{
   short vr0;
   INSTQ *ip;
   SLP_VECTOR *vl;
   short s0, s1, s2, s3, s4, s5, s6, s7, vec;
/*
 * find the last inst
 */
   if (ip0)
   {
      for (ip=ip0; ip->next; ip=ip->next)
         ;
   }
   else
      ip0 = ip = NewInst(NULL, NULL, NULL, COMMENT, 0, 0, 0);

   for (vl = vld; vl; vl = vl->next)
   {
      vec = SToff[vl->vec-1].sa[2];
      if (IS_DOUBLE(vl->flag) || IS_VDOUBLE(vl->flag) )
      {
         #ifdef AVX
            assert(vl->svars[0] == 4);  
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            s2 = SToff[vl->svars[3]-1].sa[2];
            s3 = SToff[vl->svars[4]-1].sa[2];
               
            vr0 = GetReg(T_VDOUBLE);
            //PrintComment(blk, NULL, NULL, "Scatter of vector %s", 
            //            STname[vl->vec-1]);
            if (nelem > 0)
            {
               ip->next = NewInst(NULL, ip, NULL, VDLD, -vr0, vec, 0);
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VDSTS, s0, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 1)
            {
               ip->next = NewInst(NULL, ip, NULL, VDSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x3211));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VDSTS, s1, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 2)
            {
               ip->next = NewInst(NULL, ip, NULL, VDSHUF, -vr0, -vr0, 
                            STiconstlookup(0x3232));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VDSTS, s2, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 3)
            {
               ip->next = NewInst(NULL, ip, NULL, VDSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x3211));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VDSTS, s3, -vr0, 0);
               ip = ip->next;
            }
            
         #else
            assert(vl->svars[0] == 2);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            vr0 = GetReg(T_VDOUBLE);
            
            //PrintComment(NULL, NULL, NULL, "Scatter of vector %s", 
            //            STname[vl->vec-1]);
            if (nelem > 0)
            {
               ip->next = NewInst(NULL, ip, NULL, VDLD, -vr0, vec, 0);
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VDSTS, s0, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 1)
            {
               ip->next = NewInst(NULL, ip, NULL, VDSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x11));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VDSTS, s1, -vr0, 0);
               ip = ip->next;
            }
         #endif
         GetReg(-1);               
      }
      else if (IS_FLOAT(vl->flag) || IS_VFLOAT(vl->flag))
      {
         #ifdef AVX
            assert(vl->svars[0] == 8);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            s2 = SToff[vl->svars[3]-1].sa[2];
            s3 = SToff[vl->svars[4]-1].sa[2];
            s4 = SToff[vl->svars[5]-1].sa[2];
            s5 = SToff[vl->svars[6]-1].sa[2];
            s6 = SToff[vl->svars[7]-1].sa[2];
            s7 = SToff[vl->svars[8]-1].sa[2];
            vr0 = GetReg(T_VFLOAT);
               
            //PrintComment(blk, NULL, NULL, "Scatter of vector %s", 
            //            STname[vl->vec-1]);
            if (nelem > 0)
            {
               ip->next = NewInst(NULL, ip, NULL, VFLD, -vr0, vec, 0);
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s0, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 1)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x76543211));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s1, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 2)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x76543322));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s2, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 3)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x76543232));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s3, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 4)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x76547654));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s4, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 5)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x76543211));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s5, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 6)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x76543322));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s6, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 7)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x76543232));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s7, -vr0, 0);
               ip = ip->next;
            }
            
         #else
            assert(vl->svars[0] == 4);
            s0 = SToff[vl->svars[1]-1].sa[2];
            s1 = SToff[vl->svars[2]-1].sa[2];
            s2 = SToff[vl->svars[3]-1].sa[2];
            s3 = SToff[vl->svars[4]-1].sa[2];
            vr0 = GetReg(T_VFLOAT);
            //PrintComment(blk, NULL, NULL, "Scatter of vector %s", 
            //            STname[vl->vec-1]);
            if (nelem > 0)
            {
               ip->next = NewInst(NULL, ip, NULL, VFLD, -vr0, vec, 0);
               ip = ip->next;
               ip->next = NewInst(NULL, NULL, NULL, VFSTS, s0, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 1)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x3211));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s1, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 2)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x3322));
               ip = ip->next;
               ip->next = NewInst(NULL, ip, NULL, VFSTS, s2, -vr0, 0);
               ip = ip->next;
            }
            if (nelem > 3)
            {
               ip->next = NewInst(NULL, ip, NULL, VFSHUF, -vr0, -vr0, 
                                  STiconstlookup(0x3232));
               ip = ip->next;
               ip->next = NewInst(NULL, NULL, NULL, VFSTS, s3, -vr0, 0);
               ip = ip->next;
            }
         #endif
         GetReg(-1);               
         }
         else
            fko_error(__LINE__, "unsupported type for SLP " );

   }
   return(ip0);
}

INSTQ *AddInstAtEnd(INSTQ *ip0, INSTQ *ipn)
{
   INSTQ *ip, *ip1;

   if (!ipn)
      return(ip0);

   ip = ipn;
   
   if (ip0)
   {
      for (ip1=ip0; ip1->next; ip1=ip1->next)
         ;
      ip1->next = NewInst(NULL, ip1, NULL, ip->inst[0], ip->inst[1], 
                                   ip->inst[2], ip->inst[3]);
      ip1 = ip1->next;
      ip = ip->next;
   }
   else
   {
      ip0 = ip1 = NewInst(NULL, NULL, NULL, ip->inst[0], ip->inst[1], 
                          ip->inst[2], ip->inst[3]);
      ip = ip->next;
   }

   for ( ; ip; ip=ip->next)
   {
      ip1->next = NewInst(NULL, ip1, NULL, ip->inst[0], ip->inst[1], 
                          ip->inst[2], ip->inst[3]);
      ip1=ip1->next;
   }
   return(ip0);
}

INSTQ *InstUnordVVRSUMscatter(SLP_VECTOR *rv)
{
   int i, vlen, nelem, nvrs;
   SLP_VECTOR *vl, *vl0, *vli=NULL;
   short *rvar;
   INSTQ *ip, *ipv=NULL;
   SLP_VECTOR *rvec=NULL;

   vl = rv;
   vlen = vl->vlen;
   rvar = malloc( sizeof(short)*(vlen+1) );
   assert(rvar);
   while(vl)
   {    
      vl0 = vl;
      for (i=0; i < vlen && vl; i++)
      {
         rvec = AddVectorInList(rvec, vl, vl->islivein, vl->islive);
         rvar[i+1] = vl->redvar;
         assert(rvar[i+1]);
         vl = vl->next;
      }
      nelem = i;
      nvrs = vlen;
      if (i != vlen)
      {
         nvrs = 1;
         while (nvrs < i) 
            nvrs <<= 1;
         
         for (; i < vlen; i++)
         {
            rvec = AddVectorInList(rvec, vl0, vl0->islivein, vl0->islive); 
            rvar[i+1] = vl0->redvar;
            assert(rvar[i+1]);
         }
      }
      rvar[0] = vlen; // i
      vli = CreateVector(vli, vl0->flag, vlen, rvar, 0, 0); 
      /*ipv = InstVVRSUM(rvec, vlen, vli); */
      ip = InstVVRSUM(rvec, nvrs, vli); 
      ip = AddVectorScatterInst(ip, vli, nelem);
      ipv = AddInstAtEnd(ipv, ip);
/*
 *    delete temporary data
 */
      KillAllInst(ip);
      KillVlist(rvec);
      rvec = NULL;
      KillVlist(vli);
      vli = NULL;
   }
   
   free(rvar);
   return(ipv);
}

int PreSLPchecking(LOOPQ *lp)
/*
 * NOTE: this checking is not required for SLP, but may be needed for other
 * optimizations. Most of them are assumed to vectorization by other 
 * optimzations. 
 */
{
   int i, k, N;
   short op;
   INSTQ *ip, *ip0, *ipl, *ipN;
   struct ptrinfo *pbase, *p;
   short *s;
   BLIST *bl;
/*
 * for now, we consider single posttails
 */
   if (!lp->posttails || lp->posttails->next)
   {
      fprintf(stderr, "only one posttails is considered in SLP\n");
      return(1);
   }
/*
 * SLP is applied on single blk
 */
   if (!lp->blocks || lp->blocks->next)
   {
      fprintf(stderr, "only one block is considered in SLP\n");
      return(1);
   }
#if 0   
/*
 * NOTE: all the ptr checking is skipped to apply SLP on generated mvec codes
 * do moving pointer analysis. 
 * NOTE: It is not essential for SLP itseft, but normally, it is assumed 
 * whenever vec is applied. needed later for prefecting 
 * FIXME: need to add CMPFLAG for CF_LOOP_PTRUPDATE 
 */
   pbase = FindMovingPointers(lp->blocks);
   for (N=0, p=pbase; p; p=p->next) N++;
   s = malloc(sizeof(short)*(N+1));
   assert(s);
   s[0] = N;
   for (i=1, p=pbase; p; p=p->next, i++)
   {
      s[i] = p->ptr;
#if 0
      fprintf(stderr, "%s : (%d, %d), ",STname[p->ptr-1], SToff[p->upst-1].i, 
               p->nupdate );
      if (p->flag & PTRF_INC) fprintf(stderr, " INC ");
      if (p->flag & PTRF_CONTIG) fprintf(stderr, " CONTIG ");
      if (p->flag & PTRF_CONSTINC) fprintf(stderr, " CONSTINC ");
      if (p->flag & PTRF_MIXED) fprintf(stderr, " MIXED ");
      fprintf(stderr, "\n");
#endif
      if (p->nupdate > 1)
      {
         fprintf(stderr, "multiple pointer update prohibit vect!\n");
         return(1);
      }
/*
 *    check whether ptr is accessed after ptr update. we won't vectorize if 
 *    it is so.
 */

      for (ip = p->ilist->inst->next; ip; ip=ip->next)
      {
         for (k=1; k < 4; k++)
         {
            op = ip->inst[k];
            if (op > 0 && IS_DEREF(STflag[op-1]) && STpts2[op-1] == p->ptr)
            {
               fprintf(stderr, "Use of ptr %s after being updated\n", 
                     STname[p->ptr-1]);
               return(1);
            }
         }
      }
   }
   optloop->varrs = s;

/*
 * delete pointer updates and add at the bottom before loop control 
 * NOTE: we can't delete ptr updates if they are being accessed after the 
 * updates. SO, don't skip the above checking if you want to use following codes
 */
   ip0 = KillPointerUpdates(pbase, 1);
   
   for (bl=lp->tails; bl; bl = bl->next)
   {
      ipl = FindCompilerFlag(bl->blk, CF_LOOP_UPDATE);
      assert(ipl);
      ipN = InsNewInst(NULL, NULL, ipl, CMPFLAG, CF_LOOP_PTRUPDATE, 0, 0);
      CalcThisUseSet(ipN);

      for (ip = ip0; ip; ip = ip->next)
      {
         ipN = InsNewInst(NULL, NULL, ipl, ip->inst[0], ip->inst[1],
                          ip->inst[2], ip->inst[3]);
         CalcThisUseSet(ipN);
      }
   } 
   if (pbase) KillAllPtrinfo(pbase);
   if (ip0) KillAllInst(ip0);
   INUSETU2D = 0;
#endif
   return(0);
}

int MemUnalign2Align(LOOPQ *lp)
{
   int type;
   int i, j, k, n, m;
   short id, ptr, op;
   short *pts, *s;
   INSTQ *ip;
   BLIST *bl;
   struct ptrinfo *pi, *pbase;
   int ninst = 4;
   enum inst inst;
   static enum inst
      valign[]    = { VFLD, VDLD, VFST, VDST},
      vualign[]   = {VFLDU, VDLDU, VFSTU, VDSTU};
 
#if 0
/*
 * mixed fp kernel can also be candidate for SLP. So, check type for each 
 * ptr individually later
 */
   if ( IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag) )
      type = T_VFLOAT;
   else if ( IS_DOUBLE(lp->vflag) || IS_VDOUBLE(lp->vflag) )
      type = T_VDOUBLE;
   else 
      fko_error(__LINE__, "unknown vector type! ");
#endif

   pbase = FindMovingPointers(lp->blocks);  
   for (n=0, pi=pbase; pi; pi=pi->next) 
      n++;
   
   s = malloc(sizeof(short)*(n+1));
   assert(s);
   s[0] = n;
   for (i=1, n=s[0], pi=pbase; i <= n; i++, pi=pi->next)
      if (IS_FP(STflag[pi->ptr-1]))
         s[i] = pi->ptr;
#if 0
   fprintf(stderr, "ptrs: \n");
   for (i=1, n=s[0], pi=pbase; i <= n; i++, pi=pi->next)
      fprintf(stderr, "%s ", STname[s[i]-1]);
   fprintf(stderr, "\n");
#endif
/*
 * NOTE: 2D array aligned means all col ptrs are aligned.
 */
   pts = malloc(sizeof(short)*(n+1));
   assert(pts);
   j = 1;
   if (lp->aaligned)
   {
#if 0      
      for (i=1, j=1, n=lp->aaligned[0]; i <= n; i++)
      {
         type = FLAG2TYPE(STflag[lp->aaligned[i]-1]) == T_FLOAT? 
            T_VFLOAT : T_VDOUBLE;
         if ( FindInShortList(s[0], s+1, lp->aaligned[i]) )
         {
            if (lp->abalign[i] >= type2len(type)) 
               pts[j++] = lp->aaligned[i];
         }
         else if (lp->abalign[i] >= type2len(type)) 
         {
            id = STarrlookup(lp->aaligned[i]);
            if (id && STarr[id-1].ndim > 1)
            {
               for (k=1, m=STarr[id-1].colptrs[0]; k <= m; k++)
               {
                  pts[j++] = STarr[id-1].colptrs[k];
               }
            }
         }
      }
      pts[0] = j-1;
#else
      for (i=1, n=lp->aaligned[0]; i <= n; i++)
      {
         if (lp->abalign[i] >= GetVecAlignByte())
         {
            id = STarrlookup(lp->aaligned[i]);
            if (!id)
            {
               if (FindInShortList(s[0], s+1, lp->aaligned[i]))
                  pts[j++] = lp->aaligned[i];
            }
            else if (STarr[id-1].ndim > 1)/* multi dim array */
            {
               pts[j++] = lp->aaligned[i];
            }
         }
      }

#endif
   }
   else if (!lp->aaligned && lp->abalign)
   {
#if 0      
      for (i=1, j=1, n=s[0]; i <=n; i++)
      {
         type = FLAG2TYPE(STflag[s[i]-1]) == T_FLOAT? 
            T_VFLOAT : T_VDOUBLE;
         if (lp->abalign[1] >= type2len(type))
            pts[j++] = s[i];
      }
#else
      if (lp->abalign[1] >= GetVecAlignByte())
      {
         for (i=1, n=s[0]; i <=n; i++)
         {
            id = STarrColPtrlookup(s[i]);
            if (!id)
            {
               pts[j++] = s[i];
            }
            else if (STarr[id-1].ndim > 1)
            {
               if (!FindInShortList(j-1, pts+1, STarr[id-1].ptr))
               {
                  pts[j++] = STarr[id-1].ptr;
               }
            }
         }
      }
#endif
   }
   pts[0] = j-1;

   if(!pts[0])  /* doesn't meet the alignment requirement*/
   {
      free(s);
      free(pts);
      KillAllPtrinfo(pbase);
      fko_warn(__LINE__, "no ptr meet alignment requirement!!!\n");
      return(1);
   }

#if 0
   fprintf(stderr, "ALIGNED ptrs: \n");
   for (i=1, n=pts[0]; i <= n; i++)
      fprintf(stderr, "%s ", STname[pts[i]-1]);
   fprintf(stderr, "\n");
#endif
   free(s); 
   KillAllPtrinfo(pbase);

#if 0
   fprintf(stderr, "aligned ptr = ");
   for (i=1, n=pts[0]; i <= n; i++)
   {
      fprintf(stderr, "%s ", STname[pts[i]-1]);
   }
   fprintf(stderr, "\n");
#endif
/*
 * convert all related mem access to aligned/unaligned access based on markup
 * some vec method may assume aligned access all the time.. so, make it 
 * unaligned, since we don't want to add loop peeling! 
 */
   for (bl=lp->blocks; bl; bl=bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip=ip->next)
      {
         inst = ip->inst[0];
#if 0         
         if ( IS_UNALIGNED_VLOAD(inst))
         {
            if (FindInShortList(pts[0], pts+1, STpts2[ip->inst[2]-1]))
            {
               for (j=0; j < ninst; j++)
                  if (vualign[j] == inst)
                     break;
               assert(j != ninst);
               ip->inst[0] = valign[j];
            }
         }
         else if (IS_UNALIGNED_VSTORE (inst))
         {
            if (FindInShortList(pts[0], pts+1, STpts2[ip->inst[1]-1]))
            {
               for (j=0; j < ninst; j++)
                  if (vualign[j] == inst)
                     break;
               assert(j != ninst);
               ip->inst[0] = valign[j];
            }
         }
#else
         if (IS_VLOAD(inst) || IS_VSTORE(inst) )
         {
            if (IS_VLOAD(inst))
            {
               if (NonLocalDeref(ip->inst[2]))
               {
                  ptr = STpts2[ip->inst[2]-1];
                  op = ip->inst[2];
               }
               else continue;
            }
            else
            {
               if (NonLocalDeref(ip->inst[1]))
               {
                  ptr = STpts2[ip->inst[1]-1];
                  op = ip->inst[1];
               }
               else continue;
            }
/*
 *          findou the index of ld/st
 */
            for (j=0; j < ninst; j++)
            {
               if ( (inst == valign[j]) || (inst == vualign[j]))
                  break;
            }
            assert(j!=ninst);
/*
 *          handle 2d array as well
 */
            id = STarrColPtrlookup(ptr); 
            if (!id)
            {
               if (FindInShortList(pts[0], pts+1, ptr))
                  ip->inst[0] = valign[j];
               else
                  ip->inst[0] = vualign[j];
            }
            else
            {
               if (FindInShortList(pts[0], pts+1, STarr[id-1].ptr))
               {
                  ip->inst[0] = valign[j];
               }
               else if (FindInShortList(pts[0], pts+1, ptr))
               {
                  if (SToff[op-1].sa[1])   /* active lda? */
                     ip->inst[0] = vualign[j];
                  else
                  ip->inst[0] = valign[j];
               }
               else
                  ip->inst[0] = vualign[j];
            }
         }
#endif
      }
   }
   free(pts);
   return(0);
}

int BlkPrivateVariableRename(BBLOCK *blk)
/*
 * Main idea: 
 * 1. Find Local varibale of the block
 *    - remove all live-in and live-out variable from the variable list used
 *      inside the block
 * 2. Scan top to bottom, rename the variable in-between two sets (upto the set)
 *
 */
{
   int i, k, n;
   short *sp, *pvars;
   short vp, var;
   int *nv;
   char vnam[128];
   INSTQ *ip;
   INT_BVI iv;
   extern INT_BVI FKO_BVTMP;

   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(128);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);
/*
 * Assuming all data flow analysis already done and updated
 */
  
   iv = BitVecComb(iv, blk->uses, blk->defs, '|');
   iv = BitVecComb(iv, iv, blk->ins, '-');
   iv = BitVecComb(iv, iv, blk->outs, '-');
   
   FilterOutRegs(iv);
#if 0
   fprintf(stderr, "blk-%d :\n", blk->bnum);
   PrintVars(stderr, "Local vars", iv);
#endif
   sp = BitVec2Array(iv, 1-TNREG);
   n = sp[0];
   
   if (!n)
   {
      free(sp);
      return(0);
   }
   nv = calloc(n+1, sizeof(int)); /* all 0 */
   assert(nv);
#if 0
   fprintf(stderr, "Local vars: ");
   for(i=1; i <= n; i++)
      fprintf(stderr, "%s ", STname[sp[i]-1]);
   fprintf(stderr, "\n");
#endif
   for (ip=blk->ainst1; ip; ip=ip->next)
   {
      for (i=1; i <= n; i++)
      {
         if (BitVecCheck(ip->use, sp[i]+TNREG-1))
         {
            assert(nv[i]);
            sprintf(vnam,"_%s_%d", STname[sp[i]-1], nv[i]);
            vp = STstrlookup(vnam);
            for (k=2; k < 4; k++)
            {
               if (ip->inst[k] > 0 && STpts2[ip->inst[k]-1] == sp[i])
               {
                  //ip->inst[k] = SToff[sp[i]-1].sa[2];
#if 0
                  fprintf(stderr, "***updating USE of %s with %s (%d)\n", 
                        STname[STpts2[ip->inst[k]-1]-1],
                        STname[vp-1],
                        SToff[vp-1].sa[2]
                        );
#endif
                  ip->inst[k] = SToff[vp-1].sa[2];
               }
            }
            CFUSETU2D = INUSETU2D = INDEADU2D = 0;
/*
 *          dest-in-use type inst, dest var shouldn't be private!
 */
            assert(!IS_DEST_INUSE_IMPLICITLY(ip->inst[0]));
         }
         if (BitVecCheck(ip->set, sp[i]+TNREG-1))
         {
            nv[i]++;
/*
 *          start replacing with renaming var
 */
            sprintf(vnam,"_%s_%d", STname[sp[i]-1], nv[i]);
            vp = STstrlookup(vnam);
            if (!vp)
               vp = InsertNewLocal(vnam, FLAG2TYPE(STflag[sp[i]-1]));
            //PrintVars(stderr, "ip->set: ", ip->set);
            //PrintThisInst(stderr, 0, ip);
            //fprintf(stderr, "var = %s(%d)\n", STname[sp[i]-1], sp[i]);
            assert(ip->inst[1] > 0);
            assert(STpts2[ip->inst[1]-1] == sp[i]);
            /*if (STpts2[ip->inst[1]-1] != sp[i])
            {
               fprintf(stderr, "%d ----> %d\n", STpts2[ip->inst[1]-1], sp[i]);
               assert(STpts2[ip->inst[1]-1] == sp[i]);
            }*/
#if 0
            fprintf(stderr, "***updating DEF of %s with %s(%d)\n", 
                  STname[STpts2[ip->inst[1]-1]-1],
                  STname[vp-1],
                  SToff[vp-1].sa[2]
                  );
#endif
            ip->inst[1] = SToff[vp-1].sa[2];
            CFUSETU2D = INDEADU2D = 0;
         }
      }
   }
   if (sp) free(sp);
   if (nv) free(nv);
   return(0);
}


int FindNumAccVar(BLIST *scope, short var)
{
   int nc;
   BLIST *bl;
   INSTQ *ip;
   enum inst ld, st, add, mac;
   int i;

   i = FLAG2TYPE(STflag[var-1]);
   switch(i)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      add = FADD;
      mac = FMAC;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      add = FADDD;
      mac = FMACD;
      break;
   case T_INT:
      ld = LD;
      st = ST;
      add = ADD;
      mac = UNIMP; /* INT MAC is not implemented yet */
      break;
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__, "Already vectorized!! type=%d, file=%s", i, __FILE__);
   default:
      fko_error(__LINE__, "Unknown type=%d, file=%s", i, __FILE__);
   }
   nc = 0;
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (ip->inst[0] == ld && STpts2[ip->inst[2]-1] == var)
         {
            if (ip->next->inst[0] == ld)
               ip = ip->next;
            if (ip->next->inst[0] == add || ip->next->inst[0] == mac)
            {
               if (mac != UNIMP)
                  nc ++;
               else 
                  return(0);
            }
            else
               return(0);
            ip = ip->next->next;
            if (!ip)
               return(0);
            if (ip->inst[0] != st || STpts2[ip->inst[1]-1] != var)
               return(0);
         }
         else if (ip->inst[0] == st && STpts2[ip->inst[1]-1] == var)
            return(0);
      }
   }
   return(nc);
}

int PreSlpAccumExpans(LOOPQ *lp)
{
   int i, j, n;
   int nchanges = 0;
   short id;
   short *sp, *ses;
   int *nac;
   char sn[128];
   INSTQ *ipb;
   INT_BVI iv;
   extern INT_BVI FKO_BVTMP;

   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(128);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);
   
   iv = BitVecCopy(iv, lp->header->ins);
   assert(lp->tails && !lp->tails->next); /* assuming single posttail */
   iv = BitVecComb(iv, iv, lp->tails->blk->outs, '|');

   FilterOutRegs(iv);
   sp = BitVec2Array(iv, 1-TNREG);
   n = sp[0];
   if (!n)
   {
      free(n);
      return(0);
   }
#if 0
   fprintf(stderr, "Local vars: ");
   for(i=1; i <= n; i++)
      fprintf(stderr, "%s ", STname[sp[i]-1]);
   fprintf(stderr, "\n");
#endif
   nac = calloc(n+1, sizeof(int)); /* all 0 */
   assert(nac);
   for (i=1; i <= n; i++ )
   {
      nac[i] = FindNumAccVar(lp->blocks, sp[i]);
   }
#if 0
   fprintf(stderr, "Acc vars: ");
   for(i=1; i <= n; i++)
   {
      if (nac[i])
         fprintf(stderr, "%s(%d) ", STname[sp[i]-1], nac[i]);
   }
   fprintf(stderr, "\n");
   exit(0);
#endif

   for (i=1; i <= n; i++)
   {
      if (nac[i] > 1)
      {
/*
 *       create shadow variables for accumulator
 */
         ses = malloc(sizeof(short)*(nac[i]+1));
         assert(ses);
         ses[0] = nac[i]-1;
         for (j=1; j < nac[i]; j++) /* we need 1 less shadow*/
         {
            sprintf(sn, "_%s_%d", STname[sp[i]-1], j);
            id = STstrlookup(sn);
            if (!id)
               id = InsertNewLocal(sn, FLAG2TYPE(STflag[sp[i]-1]));
            ses[j] = id;
         }
/*
 *       Call all steps for scalar expansion
 */
         ipb = GetSEHeadTail(lp, sp[i], nac[i], ses, 0, SC_ACC);
         AddInstToPrehead(lp, ipb->prev, ipb->inst[0], ipb->inst[1], 0);
         KillAllInst(ipb->prev);
         AddInstToPosttail(lp, ipb->next, ipb->inst[0],ipb->inst[1], 
               ipb->inst[2]);
         KillAllInst(ipb->next);
         nchanges += DoScalExpansOnLoop(lp, ipb->inst[0], sp[i], ses);
         ipb->prev = ipb->next = NULL;
         KillThisInst(ipb);
         free(ses);
      }
   }
   if (nchanges)
   {
      CFUSETU2D = INDEADU2D = INUSETU2D = 0;  
#if 0
      extern BBLOCK *bbbase;
      fprintf(stdout, "After ACCUM EXP: ");
      PrintInst(stdout, bbbase);
      exit(0);
#endif
   }
   return(nchanges);
}


/*=============================================================================
 *    Loop Nest vectorization
 *
 *============================================================================*/

/*
 * to manage the loopq list, will move to misc.h later   
 * Add a loop at the beginning of the list
 */
LPLIST *NewLPlist(LPLIST *next, LOOPQ *loop)
{
   LPLIST *lp;
   lp = malloc(sizeof(LPLIST));
   assert(lp);
   lp->loop = loop;
   lp->next = next;
   return(lp);
}

LPLIST *KillLPlist(LPLIST *del)
{
   LPLIST *lp;
   if (del)
   {
      lp = del->next;
      free(del);
   }
   else
      lp = NULL;
   
   return(lp);
}

void KillAllLPlist(LPLIST *lp)
{
   while(lp)
      lp = KillLPlist(lp);
}

INSTQ *InitPackFromAdjMem(INSTQ *upackq)
{  
   int i, nptr;
   ILIST **ilam;
   short *pta=NULL;
   ilam = FindAdjMem(upackq, &nptr);
   if (ilam)
   {
      upackq = InitPack(ilam, nptr, upackq, &pta);
      for (i=0; i < nptr; i++)
         if (ilam[i])
            KillAllIlist(ilam[i]);
      free(ilam);
/*
 *    NOTE: packs are sorted by ptr with large mem access while initializing..
 *    so, we skip explicit sorting here!!!
 */
      /*SortPackBasedOnPtr(pta, inpack);*/
      if (pta) free(pta);
   }
   return(upackq);
}

int AnyPack(int inpack, int nnpack)
{
   int i;
   PACK *pk = NULL;

   for (i=inpack; i < nnpack; i++)
   {
      if (PACKS[i])
      {
         pk = PACKS[i];
         break;
      }
   }
   if (pk)
      return(1);
   
   return(0);
}

int IsPreheadConsistent(BBLOCK *blk, SLP_VECTOR *vi, SLP_VECTOR *vo)
{
   int suc = 1;
   SLP_VECTOR *vl, *vl1;
   INSTQ *ip;
   INT_BVI iv, iv1, ivset;
   extern INT_BVI FKO_BVTMP;

   ivset = NewBitVec(128);
   for (ip=blk->inst1; ip; ip=ip->next)
   {
      if (ACTIVE_INST(ip->inst[0]))
         ivset = BitVecComb(ivset, ivset, ip->set, '|');
   }

   iv = FKO_BVTMP;
   if (!iv)
      NewBitVec(128);

   for (vl=vi; vl; vl=vl->next)
   {
      iv = BitVecCopy(iv, Array2BitVec(vl->svars[0], vl->svars+1, TNREG-1));
      if (!FindVectorByVec(vl->vec, vo))
      {
         for (vl1=vo; vl1; vl1=vl1->next)
         {
            if (vl1->isused)
            {
               iv1 = Array2BitVec(vl1->svars[0], vl1->svars+1, TNREG-1);
               if (BitVecCheckComb(iv, iv1, '&') ) /* a common scalar */
               {
                  if ( BitVecCheckComb(iv, iv1, '-') 
                           || BitVecCheckComb(iv1, iv, '-') ) /* iv != iv1*/
                  {
                     suc = 0;
                     break;
                  }
               }
            }
         }
/*
 *       unmatched vector can't be set in blk
 */
         if (suc)
         {
            if (BitVecCheckComb(ivset, iv, '&')) 
            {
               suc = 0;
               break;
            }
         }
         else
            break;
      }
   }
   KillBitVec(ivset);
   return(suc);
}

SLP_VECTOR *DoSingleBlkSLP(BBLOCK *blk, INT_BVI ivin, SLP_VECTOR *vi, int *err, 
      int isPosttail)
/*
 * single block SLP with vvrsums 
 */
{
   int i, n, nelem, inpack, suc;
   int hasredcode=0, mixed=0;
   SLP_VECTOR *vl, *vl1, *vr, *vs, *vo, *vli, *visr;
   PACK *pk;
   INSTQ *upackq, *ip, *ipv, *ipn, *ipvrsums;
   BBLOCK *nsbp, *sbp, *vbp, *rbp, *bp;
   INT_BVI livein, iv, iv1;
   extern INT_BVI FKO_BVTMP;
  
   inpack = NPACK;
   bp = blk;
/*
 * no init vec
 */
   if (!vi) 
   {
      upackq = DupInstQ(bp->ainst1);
      upackq = InitPackFromAdjMem(upackq);
   }
/*
 * init packs from vlist.... but before that we need to delete reduction code
 * if needed
 */
   else /* has vlist to init pack*/
   {
/*
 *    check for reduction code, delete it if found
 */
      if (isPosttail && CountRvarVect(bp, vi))     
      {
         hasredcode=1;
         vr = NULL;
/*
 *       copy the blk in case SLP fails
 */
         rbp = NewBasicBlock(NULL, NULL);
         rbp = DupInstQInBlock(rbp, bp->inst1);
         bp = rbp;
         for (vl=vi; vl; vl=vl->next)
         {
            if (vl->redvar) 
            {
               vr = AddVectorInList(vr, vl, vl->islivein, vl->islive);
               if (!(vl->flag & NSLP_ACC))
                  DelRedCodeFromRvars(bp, vl->svars, vl->redvar);
            }
         }
#if 0
         fprintf(stderr, "postails: \n");
         PrintThisInstQ(stderr, bp->inst1);
#endif
      }
/*
 *    create init pack from vector list
 */
      upackq = DupInstQ(bp->ainst1);
      for (vl=vi; vl; vl=vl->next)
      {
/*
 *       HERE HERE 
 *       NOTE: we can't extend pack from Non SLP vectors, because they are 
 *       created by implicite vectorization 
 */
         if (! (vl->flag & (NSLP_ACC | NSLP_SCAL)))
         {
/*
 *          FIXME: we may found multiple packs from same same vector
 */
            /*PrintThisVector(stderr, vl);*/
            while (upackq)
            {
               ip = upackq;
               upackq = FindInitPackFromVlist(upackq, vl, &suc);
               #if 0
                  if (suc) 
                     fprintf(stderr, "pack-%d created for %s\n", NPACK, 
                        STname[vl->vec-1]);
               #endif
               if (!suc)
                  break;
            }
         }
      }
/*
 *    NOTE: If we don't have active reduction code (or, livein NSLP vectors), 
 *    We need to utilize all the live in vectors to create packs.... otherwise,
 *    slp fails and need to scatter/gather vectors!!!
 */
      if (!AnyPack(inpack, NPACK))
      {
         KillAllInst(upackq);
         upackq = DupInstQ(bp->ainst1);
         upackq = InitPackFromAdjMem(upackq);
#if 0
         for (i=0; i < NPACK; i++)
            if(PACKS[i]) 
               PrintPack(stderr, PACKS[i]);
#endif
      }
   }
/*
 *    we need at least one init pack to begin with 
 */
   if (!AnyPack(inpack, NPACK))
   {
      fko_warn(__LINE__, "no effective seed/init pack found!");
/*
 *    free all memory
 */
      if (hasredcode)
      {
         KillAllInst(rbp->inst1);
         free(rbp);   
         KillVlist(vr);
      }
      KillAllInst(upackq);
      *err = 1;
      return(NULL);
   }
#if 0
   fprintf(stderr, "*************************Before Extending: %d\n", NPACK);
   for (i=inpack; i < NPACK; i++)
      if(PACKS[i]) 
         PrintPack(stderr, PACKS[i]);
#endif
/*
 * step: 2
 * ========
 * extend pack using def-use chain
 */
   upackq = ExtendPack(upackq, inpack);
   if (CheckDupPackError(inpack, NPACK))
   {
      fko_error(__LINE__, 
            "*******Rename scalars, duplicated packs are not supported yet");
      if (hasredcode)
      {
         KillAllInst(rbp->inst1);
         free(rbp);   
         KillVlist(vr);
      }
      KillAllInst(upackq);
      *err = 1;
      return(NULL);
   }
/*
 * step: 3
 * =======
 * Calculate dependencies
 */
   nsbp = NewBasicBlock(NULL, NULL);
   nsbp = DupInstQInBlock(nsbp, bp->inst1);
   UpdateDep(nsbp->inst1, inpack);

#if 0
   fprintf(stderr, "************************* After Extending: %d\n", NPACK);
   for (i=inpack; i < NPACK; i++)
      if(PACKS[i]) 
         PrintPack(stderr, PACKS[i]);
#endif
/*
 * step: 4
 * ========
 * schedule packs and emit vector inst accordingly
 */
   livein = NewBitVec(128);
   livein = BitVecCopy(livein, ivin);
   FilterOutRegs(livein);
   
   sbp = NewBasicBlock(NULL, NULL);
   vbp = NewBasicBlock(NULL, NULL);
/*
 * NOTE: we need to provide this vector list so far we don't create duplicate 
 * one
 */
   vo = NULL;
   for (vl=vi; vl; vl=vl->next)
         if (! (vl->flag & (NSLP_ACC | NSLP_SCAL))) /* */
            vo = AddVectorInList(vo, vl, 1, 1); 

   /* consider redvar as livein for now */
   if (hasredcode)
   {
      for (vl=vr; vl; vl=vl->next)
         SetVecBit(livein, vl->redvar+TNREG-1, 1);
   }
#if 0
   fprintf(stderr, "BLK before scheduling:\n");
   fprintf(stderr, "==================================\n");
   PrintThisInstQ(stderr, nsbp->inst1);
   fprintf(stderr, "==================================\n");
#endif
   vo = SchVectorInst(nsbp, sbp, vbp, livein, vo, inpack, err);
/*
 * delete temporaries    
 */
   KillAllInst(upackq); 
   KillAllInst(nsbp->inst1);
   free(nsbp);
   KillAllInst(sbp->inst1);
   free(sbp); 
   KillBitVec(livein); 
   
   for (i=inpack; i < NPACK; i++)
   {
      pk = PACKS[i];
      if (pk)
         KillPack(pk);
   }
   NPACK = inpack;
/*
 * check whether the scheduling is successful
 */
   if (*err)
   {
      fko_warn(__LINE__, 
            "Scheduling of SLP isn't successful, error code = %d\n", *err);
/*
 *    rollback all chnages
 */
      KillAllInst(vbp->inst1);
      free(vbp);
     
      if (hasredcode) 
      {
         KillAllInst(rbp->inst1);
         free(rbp); 
         KillVlist(vr);
      }
      KillVlist(vo);
      return(NULL);
   }
/*
 * special validity checking for preheader vectorization before updating the
 * blk
 */
/*
 * Preheader: 
 * All vectors in vi may not be used as init vector in SLP
 * Not apply SLP, if :
 * 1. There is a conflict in between Vo and Vi; 
 * 2. scalar element of Vi is set in blk, meaning the live range is changed!!
 */
   else if (vi && !isPosttail)
   {
/*
 *    in conflict, not vectorizable
 */
      if (!IsPreheadConsistent(bp, vi, vo))
      {
         fko_warn(__LINE__, "--------------prehead fails\n");
         *err = 1;
         KillVlist(vo);
         KillAllInst(vbp->inst1);
         free(vbp);
         return(NULL);
      }
      else
      {
         FinalizeVecBlock(bp, vbp);
      }
   }
/*
 * safe to vectorize, so do it
 */
   else
      FinalizeVecBlock(bp, vbp);
#if 0
   fprintf(stderr, "After SLP: \n");
   PrintThisInstQ(stderr, bp->inst1);
#endif
   KillAllInst(vbp->inst1);
   free(vbp);
/*
 * posttail and has reduction code
 */
   if(isPosttail && hasredcode)
   {
/*
 *    vi = input livein vectors
 *    vo = output vectors of SLP includes vi
 *    vr = subset of vi which is reducible, input of vvrsum
 *    vs = vvrsum output
 *    Vli = sub-grouph of vr which is the input of vvrsum
 *    visr = vi - vr
 *    Algorithm: 
 *    ---------
      Visr = Vi - Vr
      ip = NULL
      foreach vector vs in V0 which is livein but not in Vi 
            and consists of reduction variables
         vli = NULL
         foreach scalar sc in vs
            vli += FindVectorFromReduceScalar(sc, Vr)
         Vs += GetVecVVRSUM(vli)
         ip += GetInstVVRSUM(vli) 
         Vr -= vli
   //Element vectors which are not captured by SLP
      if (Vr has any remaining vector) 
         foreach vlen group of vectors vri in Vr
            vl = GetVecVVRSUM(vri)
            ip += GetInstRandomOrderVVRSUM(vri)
            ip += AddVectorScatter(vl)
            Vr -= vri
   //Cleanup
      if ( less than vlen vectors in remaining Vr)
         n = Count(Vr)
         vl = GetVecVVRSUM(Vr, n)
         ip += GetInstRandomOrderVVRSUM(Vr, n)
         ip += AddVectorElementScatter(vl, n)
         Vr is set to empty

      Visr = Visr - V0
      foreach vector vl in visr
         ip += AddVectorScatter(vl)
      B = AddInstAtBegin(B, ip)
 *
 */
      visr = NULL;
      for (vl=vi; vl; vl=vl->next)
         if (!FindVectorByVec(vl->vec, vr))
            visr = AddVectorInList(visr, vl, vl->islivein, vl->islive);
/*
 *    All reducible vr must be handled here... they must be dead inside this 
 *    block. So, we can mark them as used vector... not true if NSLP_ACC
 */
      for (vl=vr; vl; vl=vl->next)
      {
         vl1 = FindVectorByVec(vl->vec, vo);
         if (vl1 && !(vl1->flag & NSLP_ACC) )
            vl1->isused = 1;
      }
      vli = NULL;
      vs = NULL;
      ipvrsums = ipv = NULL;
#if 0
      //fprintf(stderr, "output vectors:\n");
      //PrintVectors(stderr, vo);
      PrintVectors(stderr, vi);
#endif
      for (vl=vo; vl; vl=vl->next)
      {
         if (vl->islivein && !(vl->flag & NSLP_ACC)
               && !FindVectorByVec(vl->vec, vi) )
         {
            nelem = 0;
            for (i=1, n=vl->svars[0]; i <= n; i++ )
            {
               vl1 = FindVectorFromRedvar(vl->svars[i], vr); 
               if (vl1)
               {
                  vli = AddVectorInList(vli, vl1, vl1->islivein, vl1->islive);
                  vr = KillVecFromList(vr, vl1);
                  nelem++;
               }
               /*else
               {
                  fprintf(stderr, "Not found = %s\n", STname[vl->svars[i]-1]);
               }*/
            }
            if (!nelem)
               continue;
/*
 *          matched all elements
 */
            if (nelem == vl->vlen)
            {
               ipv = InstVVRSUM(vli, nelem, vl);
               vs = AddVectorInList(vs, vl, vl->islivein, vl->islive);
               vl->islivein = 0; /* no more live in */
               KillVlist(vli);
               vli = NULL;
            }          
/*
 *          not matched  
 */
            else if (nelem) 
            {
               PrintVectors(stderr, vl);
               fprintf(stderr, "mixed up=%s(%d,%d) !!", STname[vl->vec-1], 
                       nelem, vl->vlen);
               mixed = 1;
/*
 *             rollback
 */
               for (vl=vli; vl; vl=vl->next)
                  vr = AddVectorInList(vr, vl, vl->islivein, vl->islive);
               for (vl=vs; vl; vl=vl->next)
                  vr = AddVectorInList(vr, vl, vl->islivein, vl->islive);
               KillVlist(vli);
               vli = NULL;
               KillVlist(vs);
               vs = NULL;
               break; /* exit from the loop*/
            }
         }
         else /* not a candidate for vvrsum at all*/ 
            continue;
/*
 *       copy vvrsum inst into ipvrsums
 */
         if (!mixed)
         {
            assert(ipv);
            ipvrsums = AddInstAtEnd(ipvrsums, ipv);
            KillAllInst(ipv);
            ipv = NULL;
         }
         else /* delete ipvvrsum*/
         {
            KillAllInst(ipvrsums);
            ipvrsums = NULL;
         }
      }
/*
 *    mixed elements!!!
 *    FIXME: die now! but later, we want to undo SLP and send fail. want to 
 *    apply vvrsums??? depends on caller function. 
 */
      if (mixed)
         fko_error(__LINE__, "mixed case is not handled yet");

/*
 *    vr = remaining elements of vr which is not handled by vvrsum yet
 */
      if (vr)
      {
         ipv = InstUnordVVRSUMscatter(vr);
         ipvrsums = AddInstAtEnd(ipvrsums, ipv);
         KillAllInst(ipv);
         ipv = NULL;
      }
/*
 *    need to unpack those vi who are not used in v0 at all
 */
      ip = NULL;
      for (vl=visr; vl; vl=vl->next)
      {
         if (!FindVectorFromScalars(vl->svars, vo))
            ip = AddVectorScatterInst(ip, vl, vl->vlen); 
      }
      if (ip)
      {
         ipvrsums = AddInstAtEnd(ipvrsums, ip);
         KillAllInst(ip);
      }
/*
 *    Add ipvvrsums at the beginning of the bp block
 */
      assert(ipvrsums);
      ip = ipvrsums;
      ipn = InsNewInstAfterLabel(bp, ip->inst[0], ip->inst[1], ip->inst[2], 
            ip->inst[3]);
      ip = ip->next;
      while (ip)
      {
         ipn = InsNewInst(bp, ipn, NULL, ip->inst[0], ip->inst[1], ip->inst[2], 
                          ip->inst[3]);
         ip=ip->next;
      }
      KillAllInst(ipvrsums);
/*
 *    update the original posttail
 */
      FinalizeVecBlock(blk, bp); 
      KillAllInst(rbp->inst1);
      free(rbp);
/*
 *    delete all temp vector list, if exists
 */
      if (visr) KillVlist(visr);
      if (vli) KillVlist(vli);
      if (vs) KillVlist(vs);
      if (vr) KillVlist(vr);
   } // end of hasredcode if

#if 0 
   fprintf(stderr, "Vectorized code for block: %d\n", blk->bnum);
   fprintf(stderr, "***********************************\n");
   PrintThisInstQ(stderr, blk->inst1);
   fprintf(stderr, "***********************************\n");
#endif
   return(vo);
}

int IsSlpFlowConsistent(BBLOCK *sbp1, BBLOCK *sbp2, SLP_VECTOR *v1, 
      SLP_VECTOR *v2, INT_BVI ins, INT_BVI outs)
/*
 *    Algorithm:
 *       At any junction of the two blocks, live-out variables as vector can't 
 *       be used as scalar without scatter. 
 *       Note: it can be live-in as scalar if the successor block doesn't use 
 *       it at all. May be, it is used on any later block. we need to scatter 
 *       it on that block
 *          -------
 *          |     |
 *          |     |
 *          -------  Live-out as vectors
 *             |
 *          -------  
 *          |     |  can't be used as scalar without scatter
 *          |     |
 *          -------
 *    
 */   
{
   int res=1;
   INT_BVI iv, slivein, vliveout, uses;
   SLP_VECTOR *vl;
  
   vliveout = NewBitVec(128);
   slivein = NewBitVec(128);
   uses = NewBitVec(128);
   
   if (v1) /* 1st blk is vectorized */
   {
      for (vl=v1; vl; vl=vl->next)
      {
/*
 *       FIXED: acc vector for nslp would always be dead using vvrsum or scatter
 *       first. so, don't consider them as liveout here
 */
         if (vl->isused && !(vl->flag & NSLP_ACC))
         {
            iv = Array2BitVec(vl->svars[0], vl->svars+1, TNREG-1);
            vliveout = BitVecComb(vliveout, vliveout, iv, '|');
         }
      }
      iv = BitVecCopy(iv, outs);
      FilterOutRegs(iv); 
      vliveout = BitVecComb(vliveout, vliveout, iv, '&');
/*
 *    NOTE: if v0 has vectors, meaning we don't used scatter/gather for used 
 *    vectors at all.... that means they are not used as scalar!
 *    Assumption: the scalar which is used as vector can't be used as scaler in 
 *    same block!!!may not always be true! specially not for boardcast!
 */
      slivein = BitVecCopy(slivein, ins);
      FilterOutRegs(slivein); 
      for (vl=v2; vl; vl=vl->next)
      {
         if (vl->isused)
         {
            iv = Array2BitVec(vl->svars[0], vl->svars+1, TNREG-1);
            slivein = BitVecComb(slivein, slivein, iv, '-');
         }
      }
/*
 *    remove those which are not used inside successor. They are live-in for 
 *    later use
 */
#if 0
      for (ip=bp2->ainst1; ip; ip=ip->next)
         uses = BitVecComb(uses, uses, ip->uses, '|');
      slivein = BitVecComb(slivein, slivein, uses, '&');
#else
      slivein = BitVecComb(slivein, slivein, sbp2->uses, '&');
#endif
/*
 *    to be consistent, slivein can't have any variable which also belongs to 
 *    vlivein
 */
      if (BitVecCheckComb(slivein, vliveout, '&'))
      {
         res = 0;
         PrintVars(stderr, "common var: ", BitVecComb(slivein, slivein, 
                  vliveout, '&'));
      }
   }
   KillBitVec(vliveout);
   KillBitVec(slivein);
   KillBitVec(uses);
   return(res);
}

int IsSlpConsistent(SLP_VECTOR *v0, SLP_VECTOR *v1, SLP_VECTOR *v2, 
      LOOPQ *loop, BBLOCK *vbp0, BBLOCK *vbp1, INT_BVI ins, INT_BVI outs)
{
   int res, safe;
   SLP_VECTOR *vl;
/*
 * NOTE: in our single loop nest definition, loop only has one prehead and 
 * one posttail. So, liveout of prehead is directly livein in at the beginning 
 * of loop. So is the posttail! but we have two liveout path for posttail and 
 * two livein path for prehead
 * NOTE: we can't recompute the ins and outs of block, since we need to 
 * recompute CFG and it will destroy the loop structure... we need to figure
 * out live-out vectors without recomputing data-flow.
 */
   //fprintf(stderr, "++++++checking consistency at prehead -> loop\n");
   res = IsSlpFlowConsistent(loop->preheader, loop->header, v1, v0, 
            loop->header->ins, loop->preheader->outs);
#if 0
   fprintf(stderr, "------vector used on loop:\n");
   for (vl=v0; vl; vl=vl->next)
      if (vl->isused)
         PrintThisVector(stderr,vl);
   fprintf(stderr, "------vector used on posttail:\n");
   for (vl=v2; vl; vl=vl->next)
      if (vl->isused)
         PrintThisVector(stderr,vl);
#endif
   //fprintf(stderr, "+++++checking consistency at loop -> posttail\n");
   res += IsSlpFlowConsistent(loop->tails->blk, loop->posttails->blk, v0, v2, 
            loop->posttails->blk->ins, loop->tails->blk->outs);
   //fprintf(stderr, "+++++checking consistency at posttail -> prehead\n");
   res += IsSlpFlowConsistent(loop->posttails->blk, loop->preheader, v2, v1, 
            loop->preheader->ins, loop->posttails->blk->outs);

   if (res < 3)
   {
      fprintf(stderr, "############# SLP fails !!!!\n");
      safe = 0;
   }
   else
      safe = 1;
#if 0
   fprintf(stderr, "------vector used on loop:\n");
   for (vl=v0; vl; vl=vl->next)
      if (vl->isused)
         PrintThisVector(stderr,vl);
   fprintf(stderr, "------vector used on prehead:\n");
   for (vl=v1; vl; vl=vl->next)
      if (vl->isused)
         PrintThisVector(stderr,vl);
   fprintf(stderr, "------vector used on posttail:\n");
   for (vl=v2; vl; vl=vl->next)
      if (vl->isused)
         PrintThisVector(stderr,vl);
   exit(0);
#endif
   return(safe);
}

int IsSlpConsistent0(SLP_VECTOR *v0, SLP_VECTOR *v1, SLP_VECTOR *v2, 
      LOOPQ *loop, BBLOCK *bp0, BBLOCK *bp1)
{
   int safe;
   int i, n, j, m;
   short *sp;
   BLIST *bl, *blscope;
   INT_BVI ivset, ivuse;
   INT_BVI ivvset, ivvuse;
   INSTQ *ip;
   SLP_VECTOR *vl, *vlist = NULL;
/*
 * findout all use and sets in the scope
 */
   blscope = NULL;
   blscope = AddBlockToList(blscope, bp1);
   blscope = AddBlockToList(blscope, bp0);
/*
 * update use/set for inst
 */
   CalcUseSet(bp0);
   CalcUseSet(bp1);
   for (bl=loop->blocks; bl; bl=bl->next)
   {
      CalcUseSet(bl->blk);
      blscope = AddBlockToList(blscope, bl->blk);
   }
/*
 * vlist = v0 + v1 + v2
 */
   for (vl=v0; vl; vl=vl->next)
      vlist = AddVectorInList(vlist, vl, vl->islivein, vl->islive); 
   for (vl=v1; vl; vl=vl->next)
      vlist = AddVectorInList(vlist, vl, vl->islivein, vl->islive); 
   for (vl=v2; vl; vl=vl->next)
      vlist = AddVectorInList(vlist, vl, vl->islivein, vl->islive); 
/*
 * init all bitvec
 */
   ivset = NewBitVec(128);
   ivuse = NewBitVec(128);
   ivvset = NewBitVec(128);
   ivvuse = NewBitVec(128);

   for (bl=blscope; bl; bl=bl->next)
   {
      for (ip=bl->blk->inst1; ip; ip=ip->next)
      {
         ivset = BitVecComb(ivset, ivset, ip->set, '|');
         ivuse = BitVecComb(ivuse, ivuse, ip->use, '|');
      }
   }
   FilterOutRegs(ivset); 
   FilterOutRegs(ivuse); 
   KillBlockList(blscope);

   sp = BitVec2Array(ivset, 1-TNREG);
   for (i=1, n=sp[0]; i <= n; i++)
   {
      if ( IS_VEC(FLAG2TYPE(STflag[sp[i]-1]) ) )
      {
         vl = FindVectorByVec(sp[i], vlist);
         if (vl)
         {
            for (j=1, m=vl->svars[0]; j <=m ; j++ )
            {
               SetVecBit(ivvset, vl->svars[j]+TNREG-1, 1);
            }
         }
      }
   }
   free(sp);
   sp = BitVec2Array(ivuse, 1-TNREG);
   for (i=1, n=sp[0]; i <= n; i++)
   {
      if ( IS_VEC(FLAG2TYPE(STflag[sp[i]-1]) ) )
      {
         vl = FindVectorByVec(sp[i], vlist);
         if (vl)
         {
            for (j=1, m=vl->svars[0]; j <=m ; j++ )
            {
               SetVecBit(ivvuse, vl->svars[j]+TNREG-1, 1);
            }
         }
      }
   }
   free(sp);
   KillVlist(vlist);
   
#if 0
   PrintVars(stderr, "***set :", ivset);
   fprintf(stderr, "\n");
   //PrintVars(stderr, "vuse", ivvuse);
   PrintVars(stderr, "set&vuse", BitVecComb(ivset, ivset, ivvuse, '&'));      
   PrintVars(stderr, "use&vset", BitVecComb(ivuse, ivuse, ivvset, '&'));      
   //fprintf(stderr, "---%d ", BitVecCheckComb(ivset, ivvuse, '&'));
   //fprintf(stderr, "---%d \n", BitVecCheckComb(ivuse, ivvset, '&'));
   
   exit(0);
#endif

   if ( BitVecCheckComb(ivset, ivvuse, '&') 
         || BitVecCheckComb(ivuse, ivvset, '&') )
      safe = 0;
   else
      safe = 1;
/*
 * delete all temporaries
 */
   KillBitVec(ivset);
   KillBitVec(ivuse);
   KillBitVec(ivvset);
   KillBitVec(ivvuse);

   return(safe);
}

SLP_VECTOR *CreateVlistFromLoop(LOOPQ *lp)
{
   int i, j, n, ne, type;
   short *sp;
   SLP_VECTOR *vl, *vlist=NULL;

   for (i=1, n=lp->vscal[0]; i <= n; i++)
   {
      type = FLAG2TYPE(STflag[lp->vvscal[i]-1]);
      ne = vtype2elem(type); 
      sp = malloc((ne+1)*sizeof(short)); 
      assert(sp);
      sp[0] = ne;

      for(j=1; j <=ne; j++)
         sp[j] = lp->vscal[i];

      if (lp->vsflag[i] & (VS_LIVEIN | VS_LIVEOUT) )
         vlist = CreateVector(vlist, type, ne, sp, 1, lp->vvscal[i]);
      else
         vlist = CreateVector(vlist, type, ne, sp, 0, lp->vvscal[i]);
/*
 *    added the vector at the end. so, traverse the list to access it
 */
      for(vl=vlist; vl->next; vl=vl->next)
         ;
      if (lp->vsoflag[i] & VS_ACC) /* */
      {
         vl->redvar = lp->vscal[i];
         vl->flag |= NSLP_ACC;
         /*for (j=2; j <= ne; j++)
            vl->svars[j] = 0;*/

      }
      else
         vl->flag |= NSLP_SCAL;
      vl->isused = 1; /* marked as used */
   }
   return(vlist);
}

SLP_VECTOR *DoLoopNestVec(LPLIST *lpl, int *err)
{
   int tr, err0, err1;
   INT_BVI iv, ins, outs;
   SLP_VECTOR *vl, *vo, *v1, *v2, *vo1, *vo2;
   BBLOCK *bp, *blk, *preblk, *postblk;
   LOOPQ *lp;
   INSTQ *ip, *ipn, *ipvrsums;
   extern LOOPQ *optloop;
   extern int VECT_FLAG;
   extern INT_BVI FKO_BVTMP;
   
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(128);
   iv = FKO_BVTMP;
/*
 * exit condition of the recursion: when it reaches optloop
 */
   if (!lpl->next ) //optloop
   {
      if (VECT_FLAG & VECT_SLP)
      {
         assert(lpl->loop->blocks && !lpl->loop->blocks->next); //single blk
         if (PreSLPchecking(lpl->loop)) /* needed for other optimizations */
         {
            fprintf(stderr, "Prechecking failed for SLP\n");
            *err = 1;
            return(NULL);
         }
         blk = lpl->loop->blocks->blk; 
         ins = NewBitVec(128);
         ins = BitVecCopy(ins, blk->ins);
         FilterOutRegs(ins);
         vo = NULL;
#if 0
         fprintf(stderr, "*****************Applying SLP on optloop\n");
         fprintf(stderr, "Scalar codes:\n");
         PrintThisInstQ(stderr, lpl->loop->blocks->blk->inst1);
         fprintf(stderr, "==================================\n");
#endif
         vo = DoSingleBlkSLP(lpl->loop->blocks->blk, ins, vo, err, 0);
         KillBitVec(ins);
         if (*err)
         {
            fprintf(stderr, "SLP fails in optloop!\n");
            KillVlist(vo);
            return NULL;
         }
#if 0
         {
            //extern BBLOCK *bbbase;
            fprintf(stderr, "vectorized loop block:\n");
            //PrintInst(stderr, bbbase);
            PrintThisInstQ(stderr, lpl->loop->blocks->blk->inst1); 
            //exit(0);
         }
#endif
/*
 *       NOTE: no need, since we ensure there is no scalar and vector inst  
 *       for a varibale at the same time
 */
#if 0
         if (!IsSlpFlowConsistent(vo, vo, lpl->loop->blocks->blk->ins, 
               lpl->loop->blocks->blk->outs))
            fko_error(__LINE__, "SLP vec mismatch at optloop!!!");
#endif
         return(vo);
      }
      else if (VECT_FLAG & VECT_NCONTRL)
      {
         SKIP_PELOGUE = 1;
         SKIP_PEELING = 1;
         SKIP_CLEANUP = 1;
         if (RcVectorAnalysis(lpl->loop))
         {
            *err = 1;
            return(NULL);
         }
         if (RcVectorization(lpl->loop))
         {
            *err = 2;
            return(NULL);
         }
         *err = 0;
         /*FinalizeVectorCleanup(lpl->loop, 1);*/
         SKIP_PELOGUE = 0;
         /*SKIP_PEELING = 0;*/  
#if 0
         {
            extern BBLOCK *bbbase;
            fprintf(stderr, "vectorized loop block:\n");
            PrintInst(stderr, bbbase);
            exit(0);
         }
#endif
/*
 *       NOTE: generating these vectors from optloop won't help slp for next 
 *       steps except the posttail's vvrsum codes. They are main controlled by
 *       NSLP_ACC and NSLP_SCAL flag
 */
         vo = CreateVlistFromLoop(lpl->loop);
#if 0
         fprintf(stderr, "generated vectors");
         PrintVectors(stderr, vo);
         exit(0);
#endif
         return(vo);
      }
      else 
         fko_error(__LINE__, "loopnest vectorization is not supported for this"
               " vectorization technique!!!");
   }
/*
 * recursion with one step down 
 */  
   lp = lpl->next->loop;
   vo = DoLoopNestVec(lpl->next, err); 
/*
 * vectorize prehead and posttail 
 */
   ins = NewBitVec(128);
   ins = BitVecCopy(ins, lp->preheader->outs);
   
   outs = NewBitVec(128);
   outs = BitVecCopy(outs, lp->posttails->blk->ins);

   preblk = NewBasicBlock(NULL, NULL);
   preblk = DupInstQInBlock(preblk, lp->preheader->inst1);
   
   postblk = NewBasicBlock(NULL, NULL);
   postblk = DupInstQInBlock(postblk, lp->posttails->blk->inst1);

   FilterOutRegs(ins);
   FilterOutRegs(outs);
   
   v1 = v2 = NULL;
   for (vl=vo; vl; vl=vl->next)
   {
      iv = BitVecCopy(iv, Array2BitVec(vl->svars[0], vl->svars+1, TNREG-1) );
      if ( BitVecCheckComb(iv, ins, '&') )
         v1 = AddVectorInList(v1, vl, 0, 0); /* not ins for prehead */
      if ( BitVecCheckComb(iv, outs, '&') )
         v2 = AddVectorInList(v2, vl, 1, 1); 
   }
#if 0
   fprintf(stderr, "Livein vectors:\n");
   PrintVectors(stderr, v1);
   fprintf(stderr, "Liveout vectors:\n");
   PrintVectors(stderr, v2);
#endif
   /*fprintf(stderr, "Applying SLP on prehead\n");
   fprintf(stderr, "=========================\n");*/

   ins = BitVecCopy(ins, lp->preheader->ins);
   FilterOutRegs(ins);
   vo1 = DoSingleBlkSLP(preblk, ins, v1, &err0, 0);
   
   /*fprintf(stderr, "Applying SLP on posttail\n");
   fprintf(stderr, "=========================\n");*/
   ins = BitVecCopy(ins, lp->posttails->blk->ins);
   FilterOutRegs(ins);
   vo2 = DoSingleBlkSLP(postblk, ins, v2, &err1, 1);
   /*vo2 = DoSingleBlkSLP(postblk, outs, v2, &err1);*/
   
/*
 * check consistency of SLP throughout the loop, i.e., loopi-1 + prehead 
 *    + posttail 
 */
   /*fprintf(stderr, "%d %d\n", err0, err1);*/
/*
 * NOTE: can be vectorized even if prehead is not vectorizable!
 * check consistency if either one can be vectorized  
 */
   tr = err0 + err1;
   ins = BitVecCopy(ins, lp->preheader->ins);
   FilterOutRegs(ins);
   outs = BitVecCopy(outs, lp->posttails->blk->outs);
   FilterOutRegs(outs);

   if ((tr < 2) && 
         IsSlpConsistent(vo, vo1, vo2, lp, preblk, postblk, ins, outs) )
   {
/*
 *    handle prehead
 */
      if(!err0) /* vec successful */
      {
         FinalizeVecBlock(lp->preheader, preblk); 
         vo = AppendVoVlist(vo, vo1);
         vo1 = NULL;
      }
      else
         AddSlpPrologue(lp->preheader, v1, 1); 
/*
 *    handle posttail 
 */
      if (!err1) /* vec successful */
      {
         FinalizeVecBlock(lp->posttails->blk, postblk);
         vo = AppendVoVlist(vo, vo2);
         vo2 = NULL;
      }
      else
      {
/*
 *       we can still apply vvrsum, if we have accumulators
 */
         KillVlist(vo2);
         vo2 = NULL;
         for (vl=v2; vl; vl=vl->next)
            if (vl->flag & NSLP_ACC)
               vo2 = AddVectorInList(vo2, vl, vl->islivein, vl->islive);
         if (vo2)
         {
            for (vl=vo2; vl; vl=vl->next)
               v2 = KillVecFromList(v2, vl);
/*
 *          add vvrsums code at the beginning
 */
            ipvrsums = InstUnordVVRSUMscatter(vo2);
            assert(ipvrsums);
            ip = ipvrsums;
            bp = lp->posttails->blk;
            ipn = InsNewInstAfterLabel(bp, ip->inst[0], ip->inst[1], 
                     ip->inst[2], ip->inst[3]);
            ip = ip->next;
            while (ip)
            {
               ipn = InsNewInst(bp, ipn, NULL, ip->inst[0], ip->inst[1], 
                        ip->inst[2], ip->inst[3]);
               ip=ip->next;
            }
            KillAllInst(ipvrsums);
         }
         AddSlpEpilogue(lp->posttails->blk, v2, 0); 
      }
      
      KillAllInst(preblk->inst1);
      free(preblk);
      KillAllInst(postblk->inst1);
      free(postblk);
      
      if (vo2) KillVlist(vo2);
      if (vo1) KillVlist(vo1);
      KillVlist(v1);
      KillVlist(v2);
   }
   else /* failed */
   {
      KillAllInst(preblk->inst1);
      free(preblk);
      KillAllInst(postblk->inst1);
      free(postblk);
      KillVlist(vo1);
      KillVlist(vo2);
/*
 *    vector prologue and epilogue if needed
 */   
#if 0 
      v1 = NULL;
      v2 = NULL;
      for (vl=vo; vl; vl=vl->next)
      {
         iv = BitVecCopy(iv, Array2BitVec(vl->svars[0], vl->svars+1, TNREG-1) );
         if ( BitVecCheckComb(iv, ins, '&') )
            v1 = AddVectorInList(v1, vl, 1, 1);
         if ( BitVecCheckComb(iv, outs, '&') )
            v2 = AddVectorInList(v2, vl, 1, 1); 
      }
#endif

#if 0
      if (v1 || v2) 
         fprintf(stderr, "live in/out vectors!!!");
#endif
      AddSlpPrologue(lp->preheader, v1, 1); 
      AddSlpEpilogue(lp->posttails->blk, v2, 0); 
      KillVlist(v1);
      KillVlist(v2);
      KillVlist(vo);
      vo = NULL;
   }

   KillBitVec(ins); 
   KillBitVec(outs); 
   return(vo);
}

int LoopNestVec()
{
   int err=1;
   LOOPQ *lp, *lp0;
   LPLIST *ll, *l;
   SLP_VECTOR *vo;
   BLIST *bl;
   extern LOOPQ *loopq;
   extern BBLOCK *bbbase;
   
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#if 0
   PrintLoop(stderr, loopq);
#endif
/*
 * Find suitable loopnest for vectorization, list from upper to inner loopq
 */
   ll = NULL;
   lp = optloop;
   while(lp)
   {
      ll = NewLPlist(ll, lp);
      lp0 = lp;
      if (lp->preheader)
         lp = lp->preheader->loop;
      else lp = NULL;
      if (lp) // preheader is the header of upper loop
      {
         if (!lp0->posttails || lp0->posttails->next   
               || lp0->posttails->blk != lp->tails->blk ) 
            lp = NULL;
      }
   }
#if 0
   for (l = ll; l; l=l->next)
      fprintf(stderr, "%s\n", STname[l->loop->body_label-1]);
   exit(0);
#endif
#if 1  
/*
 * renaming local variables
 */
   for (bl=ll->loop->blocks; bl; bl=bl->next)
      BlkPrivateVariableRename(bl->blk);
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#endif
#if 0
   fprintf(stdout, "After applying var renaming: \n");
   PrintInst(stdout, bbbase);
   PrintST(stderr);
   exit(0);
#endif
/*
 * Apply accumulator expansion (which is not local) if needed
 */
   if (PreSlpAccumExpans(optloop))
   {
      /*fprintf(stderr, "pre slp accum expansion applied\n");*/
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#if 0
   fprintf(stdout, "After applying AccumExpan: \n");
   PrintInst(stdout, bbbase);
   PrintST(stderr);
   exit(0);
#endif
/*
 * NOTE: err==0 when optloop is successfully vectorized; even nothing else 
 */
   if (ll)
      vo = DoLoopNestVec(ll, &err);
#if 0
   fprintf(stderr, "loop nest vect error = %d (%p)\n", err, &err);
#endif
   if (vo) KillVlist(vo);
   KillAllLPlist(ll); 
   KillPackTable();
/*
 * if user markup says that some ptr are aligned, replace all unaligned ld/st
 * with aligned ld/st for them
 */
   if (optloop->aaligned || (!optloop->aaligned && optloop->abalign) )
         MemUnalign2Align(optloop);
/*
 * recompute the CFG
 */
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
#if 0
   fprintf(stdout, "vectorized code: \n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif
   return(err);
}
