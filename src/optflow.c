#include "fko.h"

BLIST *FindGlobalBlockList(void)
/*
 * RETURNS: list of all blocks in func
 */
{
   BLIST *base, *bl;
   BBLOCK *bp;
   if (!bbbase)
      return(NULL);
   bl = base = NewBlockList(bbbase, NULL);
   for (bp=bbbase->down; bp; bp = bp->down)
   {
      bl->next = NewBlockList(bp, NULL);
      bl = bl->next;
   }
   return(base);
}

ILIST *FindAllLabels(BLIST *scope)
/*
 * RETURNS: ILIST of all branches in scope, NULL if no branches
 * NOTE: Assumes blocks setup correctly, so it looks only at blocks with ilab
 *       set, and assumes label is 1st active instruction.
 */
{
   INSTQ *ip;
   BLIST *bl, *freeme=NULL;
   ILIST *ilbase=NULL;

   if (!scope)
      scope = freeme = FindGlobalBlockList();
   for (bl=scope; bl; bl = bl->next)
   {
      if (bl->blk->ilab)
         ilbase = NewIlist(bl->blk->ainst1, ilbase);
   }
   if (freeme)
       KillBlockList(freeme);
   return(ilbase);
}

ILIST *FindAllLabels_dirty(BLIST *scope)
/*
 * RETURNS: ILIST of all branches in scope, NULL if no branches
 * NOTE: does not assume blocks setup correctly, so it looks through all
 *       instructions (rather than only last active inst)
 */
{
   INSTQ *ip;
   BLIST *bl, *freeme=NULL;
   ILIST *ilbase=NULL;
   enum inst inst;
   if (!scope)
      scope = freeme = FindGlobalBlockList();
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         inst = GET_INST(ip->inst[0]);
         if (inst == LABEL)
            ilbase = NewIlist(ip, ilbase);
      }
   }
   if (freeme)
       KillBlockList(freeme);
   return(ilbase);
}

ILIST *FindAllJumps(BLIST *scope, int COND)
/*
 * RETURNS: ILIST of all branches in scope, NULL if no branches
 *          if (COND), include conditional branches, otherwise only
 *          look for unconditional
 * NOTE: Assumes blocks setup correctly, so it looks for jumps only as
 *       last active inst.
 */
{
   INSTQ *ip;
   BLIST *bl, *freeme=NULL;
   ILIST *ilbase=NULL;
   enum inst inst;

   if (!scope)
      scope = freeme = FindGlobalBlockList();
   for (bl=scope; bl; bl = bl->next)
   {
      ip = bl->blk->ainstN;
      inst = GET_INST(ip->inst[0]);
      if (inst == JMP || (COND && IS_BRANCH(inst)))
         ilbase = NewIlist(ip, ilbase);
   }
   if (freeme)
       KillBlockList(freeme);
   return(ilbase);
}

ILIST *FindAllJumps_dirty(BLIST *scope, int COND)
/*
 * RETURNS: ILIST of all branches in scope, NULL if no branches
 *          if (COND), include conditional branches, otherwise only
 *          look for unconditional
 * NOTE: does not assume blocks setup correctly, so it looks through all
 *       instructions (rather than only last active inst)
 */
{
   INSTQ *ip;
   BLIST *bl, *freeme=NULL;
   ILIST *ilbase=NULL;
   enum inst inst;

   if (!scope)
      scope = freeme = FindGlobalBlockList();
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         inst = GET_INST(ip->inst[0]);
         if (inst == JMP || (COND && IS_BRANCH(inst)))
            ilbase = NewIlist(ip, ilbase);
      }
   }
   if (freeme)
       KillBlockList(freeme);
   return(ilbase);
}
/*
 * NOTE: make all funcs return int of # of changes, so we can see if
 *       the phase has done anything
 */
int UselessJumpElim(void)
/*
 * Given list of all jumps in code (jumps), removes those that unconditionally
 * jump to following block
 * RETURNS: number of jumps removed
 */
{
   ILIST *jl, *jn, *jumps;
   INSTQ *ip;
   BBLOCK *bp;
   int n=0;

   if (CFU2D)
      jumps = FindAllJumps(NULL, 0);
   else
      jumps = FindAllJumps_dirty(NULL, 0);
   for (jl=jumps; jl; jl = jn)
   {
      jn = jl->next;
      ip = jl->inst;
/*
 *    Remove jump only if it is unconditional and last active inst in block
 */
      if (ip->myblk->ainstN == ip)
      {
         bp = FindBlockWithLabel(bbbase, ip->inst[2]);
         if (bp == ip->myblk->down)
         {
            DelInst(ip);
            n++;
         }
      }
   }
   fprintf(stderr, "Eliminated %d useless jumps!\n", n);
   KillAllIlist(jumps);
   return(n);
}

int BranchChaining(void)
/*
 * Replaces jumps to unconditional jump(s) with jumps to final target
 */
{
   int n=0;
   ILIST *jl, *jumps;
   if (CFU2D)
      jumps = FindAllJumps(NULL, 1);
   else
      jumps = FindAllJumps_dirty(NULL, 1);
   for (jl=jumps; jl; jl = jl->next)
   {
   }
   KillAllIlist(jumps);
   return(n);
}

int DeadCodeElim(BBLOCK *base)
/*
 * Finds blocks other than base that have no predecessors, and delete them.
 */
{
   BBLOCK *bp;
   if (!base) return;
   for (bp=base->down; bp; bp = bp->down)
   {
      if (!AnyBitsSet(bp->preds))
      {
      }
   }
}

int LabelFusion(BBLOCK *base)
/*
 * Finds blocks consisting only of a label, and either moves that label down
 * to the successor block, or if the successor block already has a label,
 * replaces all jumps to the useless label with jumps to the successor label.
 */
{
}
