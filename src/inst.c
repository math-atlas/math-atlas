#include "ifko.h"

INSTQ *NewInst(BBLOCK *myblk, INSTQ *prev, INSTQ *next, enum inst ins,
               short dest, short src1, short src2)
{
   INSTQ *ip;
   ip = malloc(sizeof(INSTQ));
   assert(ip);
   ip->myblk = myblk;
   ip->next = next;
   ip->prev = prev;
   ip->inst[0] = ins;
   ip->inst[1] = dest;
   ip->inst[2] = src1;
   ip->inst[3] = src2;
   ip->use = ip->set = ip->deads = 0;
   return(ip);
}

void KillAllInst(INSTQ *base)
{
   INSTQ *next;
   for(; base; base = next)
   {
      next = base->next;
      free(base);
   }
}

INSTQ *InsNewInst(BBLOCK *blk, INSTQ *prev, INSTQ *next, enum inst ins,
                  short dest, short src1, short src2)
/*
 * Insert an instruction in queue pointed at by iqhead.
 * If prev is set, add after prev inst.
 * Otherwise, if next is set, add before next in queue.
 * Otherwise (both prev and next are NULL), instruction is added to end.
 */
{
   INSTQ *ip;
   extern BBLOCK *bbbase;
/*
 * Adding to end of queue (which may not yet exist)
 */
   if (!blk)
   {
      if (next && next->myblk) blk = next->myblk;
      else if (prev && prev->myblk) blk = prev->myblk;
      else blk = bbbase;
   }
   assert(blk);
   if (!blk->inst1 || !(prev || next))
   {
      if (blk->inst1)  /* add to end of already existing queue */
      {
         ip = NewInst(blk->inst1->myblk, blk->instN, NULL, ins, 
                      dest, src1, src2);
         blk->instN->next = ip;
         blk->instN = ip;
      }
      else
         ip = blk->inst1 = blk->instN = NewInst(blk, NULL, NULL, 
                                                ins, dest, src1, src2);
   }
/*
 * Adding after prev
 */
   else if (prev)
   {
      ip = prev->next = NewInst(blk->inst1->myblk, prev, prev->next, ins,
                                dest, src1, src2);
      if (ip->next) ip->next->prev = ip;
      else blk->instN = ip;
   }
   else /* if (next) */
   {
      ip = next->prev = NewInst(blk->inst1->myblk, next->prev, next,
                                ins, dest, src1, src2);
      if (ip->prev) ip->prev->next = ip;
      else blk->inst1 = ip;
   }
   return(ip);
}

void InsInstInBlockList(BLIST *blist, int FIRST, enum inst ins,
                        short dest, short src1, short src2)
/*
 * Inserts the given instruction in all blocks pointed to by blist.
 * if FIRST is non-zero, insert as first instruction in block, else
 * insert as last
 */
{
   INSTQ *next=NULL;

   for(; blist; blist = blist->next)
   {
      if (FIRST)
         next = blist->blk->inst1;
      InsNewInst(blist->blk, NULL, next, ins, dest, src1, src2);
   }
}

INSTQ *DelInst(INSTQ *del)
/*
 * Deletes inst del from Q, keeping links cosher
 * RETURNS: next inst in queue
 * NOTE: instruction must be in a basic block
 */
{
   INSTQ *ip=NULL;
   if (!del) return(NULL);
   assert(del->myblk);
   for (ip=del->myblk->inst1; ip && ip != del; ip = ip->next);
   if (ip)
   {
      if (ip->prev) ip->prev->next = ip->next;
      else
      {
         del->myblk->inst1 = del->myblk->inst1->next;
         if (del->myblk->inst1) del->myblk->inst1->prev = NULL;
      }
      if (ip->next) ip->next->prev = ip->prev;
      else
      {
         del->myblk->instN = del->myblk->instN->prev;
         if (del->myblk->instN) del->myblk->instN->next = NULL;
      }
      ip = del->next;
      free(del);
   }
   return(ip);
}
