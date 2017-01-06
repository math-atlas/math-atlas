/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#include "fko.h"

int DoStoreNT(BLIST *scope)
/*
 * Changes all stores of selected arrays to non-temporal (cache-through) form
 */
{
   BLIST *bl;
   int i;
   int nchanges=0;
   short st;
   INSTQ *ip;
   enum inst store;
   extern int NWNT;
   extern char **ARRWNT;
   BLIST *killme;
   /*short k;*/

   if (!scope)
      killme = scope = GetGlobalScope();
   for (i=0; i < NWNT; i++)
   {
      st = FindVarFromName(ARRWNT[i]);
      /*k = st + TNREG - 1;*/
      assert(st);
      for (bl=scope; bl; bl = bl->next)
      {
         for (ip=bl->blk->ainst1; ip; ip = ip->next)
         {
            store = 0;
            if (ip->inst[0] == VFST)
               store = VFSTNT;
            else if (ip->inst[0] == VDST)
               store = VDSTNT;
            if (store && STpts2[ip->inst[1]-1] == st)
            {
               ip->inst[0] = store;
               nchanges++;
            }
         }
      }
   }
   if (killme)
      KillBlockList(killme);
   return(nchanges);
}

int DeadDefElimination()
/*
 * it will delete all the useless definitions which are dead...
 * will be moved to vars.c later 
 */
{
   int i, k, n;
   int nchanges=0;
   INSTQ *ip, *ip0, *ipn;
   BBLOCK *bp;
   INT_BVI iv, ivds, ivvars;
   ILIST **ils;
   short *vars, *deadvars;
   extern INT_BVI FKO_BVTMP;
   extern BBLOCK *bbbase;
   
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
   
#if 0
   fprintf(stdout, "LIL before dead code\n");
   PrintInst(stdout, bbbase);
   fflush(stdout);
#endif
   
   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;

   ivds = NewBitVec(128);
   assert(ivds);
   
   ivvars = NewBitVec(128);
   assert(ivds);

   for (bp = bbbase; bp; bp=bp->down)
   {
      /*fprintf(stderr, "----blk = %d\n", bp->bnum);*/
/*
 *    findout all the variables we need to consider for this block first
 */
      SetVecAll(ivvars, 0);
      for (ip=bp->ainst1; ip; ip=ip->next)
      {
         ivvars = BitVecComb(ivvars, ivvars, ip->set, '|');
         ivvars = BitVecComb(ivvars, ivvars, ip->use, '|');
      }
      ivvars = FilterOutRegs(ivvars);
      vars = BitVec2Array(ivvars, 1-TNREG);
#if 0      
      fprintf(stderr, "Print vars: ");
      for (i=1; i <= vars[0]; i++)
         fprintf(stderr, "%s ", STname[vars[i]-1]);
      fprintf(stderr, "\n");
#endif
/*
 *    setup an ILIST to keep track of ipset
 */
      n = vars[0];
      if (!n)
      {
         free(vars);
         continue;
      }
      ils = malloc(sizeof(ILIST*)*n);
      assert(ils);
      for (i=0; i < n; i++)
         ils[i] = NULL;
/*
 *    check all inst for dead variable inside this single block
 */
      SetVecAll(ivds, 0);
      for (ip=bp->ainst1; ip; ip=ip->next)
      {
/*
 *       if one of the variale is set using this inst
 */
         if (ip->set && BitVecCheckComb(ivvars, ip->set, '&') )
         {
/*
 *          check whether the var is already set and not dead, it will be a 
 *          candidate for dead var then
 */
            iv = BitVecComb(iv, ivds, ip->set, '&');
            iv = FilterOutRegs(iv); 
            if (AnyBitsSet(iv))
            {
               k = FindInShortList(vars[0], vars+1, GetSetBitX(iv, 1)-TNREG);
               assert(k);
               /*fprintf(stderr, "***dead var = %s\n", STname[vars[k]-1]);*/
               ip0 = FindFirstLILforHIL(ils[k-1]->inst);
               ils[k-1]->inst = NULL;
/*
 *             delete the inst seqment which previously set the var
 */
               if (ip0)
               {
                  nchanges++;
                  while (ip0)
                  {
                     /*PrintThisInst(stderr, 0, ip0);*/
                     if (IS_STORE(ip0->inst[0]))
                     {
                        DelInst(ip0);
                        break;
                     }
                     ip0 = DelInst(ip0);
                  }
               }
            }
/*
 *          mark the var as being set in ivds and update the ilist to point 
 *          the inst
 */
            ivds = BitVecComb(ivds, ivds, ip->set, '|');
#if 0
            if (ip->inst[1] > 0 && !NonLocalDeref(ip->inst[1]))
            {
               i = FindInShortList(vars[0], vars+1, STpts2[ip->inst[1]-1] );
               assert(i);
               assert(i<=n);
               if (ils[i-1])
                  ils[i-1]->inst = ip;
               else
                  ils[i-1] = NewIlist(ip, NULL);
            }
#else
            iv = BitVecComb(iv, ivvars, ip->set, '&');
            k = FindInShortList(vars[0], vars+1, GetSetBitX(iv, 1)-TNREG);
            assert(k);
            if (ils[k-1])
               ils[k-1]->inst = ip;
            else
               ils[k-1] = NewIlist(ip, NULL);
            
#endif
         }
/*
 *       check for varibale being dead by its last-use in this inst
 *       remove it form ivds since it only keep track the live variable with
 *       active value (being set by an inst before)
 */
         if (ip->deads && BitVecCheckComb(ivvars, ip->deads, '&'))
         {
            ivds = BitVecComb(ivds, ivds, ip->deads, '-');
         }
      }
      
/*
 *    There may be an active value (has a set) but it's dead at the end of the
 *    block and it doesn't have any last-use. That means, it doesn't have any 
 *    usage at all, there would be a last-use case otherwise. So, it's safe to
 *    delete the set of the variable.
 */
      iv = BitVecComb(iv, ivds, bp->outs, '-');
      iv = FilterOutRegs(iv);
      if (AnyBitsSet(iv))
      {
         k = FindInShortList(vars[0], vars+1, GetSetBitX(iv, 1)-TNREG);
         assert(k);
         /*fprintf(stderr, "***dead var = %s\n", STname[vars[k]-1]);*/
         ip0 = FindFirstLILforHIL(ils[k-1]->inst);
         if (ip0)
         {
            nchanges++;
            while (ip0)
            {
               /*PrintThisInst(stderr, 0, ip0);*/
               if (IS_STORE(ip0->inst[0]))
               {
                  DelInst(ip0);
                  break;
               }
               ip0 = DelInst(ip0);
            }
         }
      }
/*
 *    delete all temporaries for this block
 */
      if (vars)
         free(vars);
      if (ils)
      {
         for (i=0; i < n; i++)
         {
            if (ils[i])
               free(ils[i]);
         }
         free(ils);
      }
   
   }
/*
 * delete all tempories for this function
 */
   KillBitVec(ivds);
   KillBitVec(ivvars);
   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
   return(nchanges);
}

void DoDeadDefElim()
{
   int nchanges;
   short nlab=0, labs[4];
   extern BBLOCK *bbbase;
/*
 * delete all useless label to simplify the cfg
 */
   nlab = 2;
   labs[0] = STlabellookup(rout_name);
   labs[1] = STlabellookup("_IFKO_EPILOGUE");
   DoUselessLabelElim(nlab, labs);
   
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__,__LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
/*
 * apply dead definition elimination repeatatively until no more def can be 
 * deleted/eliminated
 */
   do
   {
      nchanges = DeadDefElimination(); 
   } 
   while(nchanges);
   
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
}

int MinBlkPtrUpdates(BBLOCK *blk, struct ptrinfo *pi0)
{
   int k, inc;
   INSTQ *ip, *ip0, *ipn;
   short *sp;
   short reg, dt;
   struct ptrinfo *pi;
   ILIST *il;
   /*fprintf(stderr, "----------blk=%d\n", blk->bnum);*/
   for (pi=pi0; pi; pi=pi->next)
   {
      /*fprintf(stderr, "%s : %d\n", STname[pi->ptr-1], pi->flag);*/
/*
 *    assumption: ilist in ptrinfo is correctly populated and point sequential 
 *    from up to down
 */
#if 0      
      assert(pi->ilist);
      ip0 = pi->ilist->inst;
      assert(ip0->inst[2] == ip0->prev->inst[2]); 
      assert(IS_CONST(STflag[ip0->prev->inst[3]-1]));
      pi->upst = SToff[ip0->prev->inst[3]-1].i;
      
      fprintf(stderr, "init inc=%d\n", pi->upst);
      
      inc = pi->upst;
#endif
      assert(pi->ilist);
      reg = -(pi->ilist->inst->inst[2]);
      inc = 0;
      for (il=pi->ilist; il; il=il->next)
      {
         ip0 = il->inst;
         assert(ip0->inst[2] == ip0->prev->inst[2]); 
         assert(IS_CONST(STflag[ip0->prev->inst[3]-1]));
         inc += SToff[ip0->prev->inst[3]-1].i;
         
         if (il->next)
            ipn = il->next->inst;
         ipn = NULL;
/*
 *       we assume only const inc of ptr
 */
         for (ip = ip0; ip != ipn; ip=ip->next)
         {
            dt = 0;
            if (IS_LOAD(ip->inst[0]) && NonLocalDeref(ip->inst[2]))
               dt = ip->inst[2];
            else if (IS_STORE(ip->inst[0]) && NonLocalDeref(ip->inst[1]))
               dt = ip->inst[1];
            
            if (dt && STpts2[dt-1] == pi->ptr)
            {
/*
 *             NOTE: since we are considering only const inc now, we won't have 
 *             any load of index. this assertion is to protect this assumption 
 */
               assert( (ip->prev->inst[0] == LD)
                     && (STpts2[ip->prev->inst[2]-1])==pi->ptr );
               k = -ip->prev->inst[1];
               sp = UpdateDeref(ip, k, inc);
               if (sp)
               {
                  for (k=0; k < 4; k++)
                     ip->inst[k] = sp[k];
               }
               else
               {
#if 1
                  fprintf(stderr, "DT: <%d, %d, %d, %d>\n", 
                        SToff[dt-1].sa[0],
                        SToff[dt-1].sa[1],
                        SToff[dt-1].sa[2],
                        SToff[dt-1].sa[3] );
                  assert(0);
#else
                  InsNewInst(blk, NULL, ip, ADD, -k, -k, STiconstlookup(inc));
#endif
               }
            }
#if 0                   
            if (IS_LOAD(ip->inst[0]) && NonLocalDeref(ip->inst[2])) 
            {
               k = STpts2[ip->inst[2]-1];
               if (k != pi->ptr)
                  continue;
/*
 *             since it's const inc, there is no load of index.. so, ip->prev 
 *             should be load of ptr
 */
               assert( (ip->prev->inst[0] == LD)
                     && (k==STpts2[ip->prev->inst[2]-1]) );
               k = -ip->prev->inst[1];
               sp = UpdateDeref(ip, k, inc);
               if (sp)
               {
                  for (k=0; k < 4; k++)
                     ip->inst[k] = sp[k];
               }
               else
               {
#if 1
                  fprintf(stderr, "DT: <%d, %d, %d, %d>\n", 
                        SToff[ip->inst[2]-1].sa[0],
                        SToff[ip->inst[2]-1].sa[1],
                        SToff[ip->inst[2]-1].sa[2],
                        SToff[ip->inst[2]-1].sa[3] );
                  assert(0);
#else
                  InsNewInst(blk, NULL, ip, ADD, -k, -k, STiconstlookup(inc));
#endif
               }
            }
            else if (IS_STORE(ip->inst[0]) && NonLocalDeref(ip->inst[1]))
            {
               k = STpts2[ip->inst[1]-1];
               if (k != pi->ptr)
                  continue;
               assert( (ip->prev->inst[0] == LD)
                     && (k==STpts2[ip->prev->inst[2]-1]) );
               k = -ip->prev->inst[1];
               sp = UpdateDeref(ip, k, inc);
               if (sp)
               {
                  for (k=0; k < 4; k++)
                     ip->inst[k] = sp[k];
               }
               else
               {
#if 1
                  fprintf(stderr, "DT: <%d, %d, %d, %d>\n", 
                        SToff[ip->inst[1]-1].sa[0],
                        SToff[ip->inst[1]-1].sa[1],
                        SToff[ip->inst[1]-1].sa[2],
                        SToff[ip->inst[1]-1].sa[3] );
                  assert(0);
#else
                  InsNewInst(blk, NULL, ip, ADD, -k, -k, STiconstlookup(inc));
#endif
               }
            }
#endif
         }
/*
 *       delete pointer update... 
 */
         ip = ip0->prev->prev;
         ip = DelInst(ip);
         ip = DelInst(ip);
         ip = DelInst(ip);
         il->inst = NULL;
      }
/*
 *    add pointer update at the end of the block before branch unless we have 
 *    loop cmpflags 
 */
      ip = FindCompilerFlag(blk, CF_LOOP_PTRUPDATE );
      if (!ip)
      {
         ip = FindCompilerFlag(blk, CF_LOOP_UPDATE);
         if (!ip)
         {
            ip = FindCompilerFlag(blk, CF_LOOP_INIT);
            if (!ip)
            {
               ip = blk->instN;
               if (IS_BRANCH(ip->inst[0]))
                  ip = ip->prev;
               while (ip && !IS_STORE(ip->inst[0]) && !IS_BRANCH(ip->inst[0])
                      && ip->inst[0] != LABEL)
               {
                  ip = ip->prev;
               }
               ip = ip->next;
            }
         }
         else
         {
            ip = InsNewInst(blk, NULL, ip, CMPFLAG, CF_LOOP_PTRUPDATE, 0, 0 );
            ip = ip->next;
         }
      }
      else
      {
         ip = ip->next;
      }
/*
 *    inst shouldn't have any access of ptr after this point
 *    FIXME: do we need to place a checking in case CMPFLAG got displaced???
 */
      InsNewInst(blk, NULL, ip, LD, -reg, SToff[pi->ptr-1].sa[2], 0 );
      if (pi->flag & PTRF_INC)
         InsNewInst(blk, NULL, ip, ADD, -reg, -reg, STiconstlookup(inc));
      else assert(0);
      InsNewInst(blk, NULL, ip, ST, SToff[pi->ptr-1].sa[2], -reg,  0 );
      INUSETU2D = CFUSETU2D = INDEADU2D = 0;
   }
}

int LocalMinPtrUpdate(BLIST *scope)
{
   BLIST *bl, *bscope;
   struct ptrinfo *pi;

   bscope = NewReverseBlockList(scope); /* to reverse the order, not needed! */
   for (bl=bscope; bl; bl=bl->next)
   {
      pi = FindConstMovingPtr(bl->blk);
      if (pi)
      {
         MinBlkPtrUpdates(bl->blk, pi);
         KillAllPtrinfo(pi);
      }
   }
   KillBlockList(bscope);
}

