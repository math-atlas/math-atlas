#include "ifko.h"

INSTQ *iqhead=NULL;

INSTQ *NewInst(INSTQ *prev, INSTQ *next, enum inst ins,
               short dest, short src1, short src2)
{
   INSTQ *ip;
   ip = malloc(sizeof(INSTQ));
   assert(ip);
   ip->next = next;
   ip->prev = prev;
   ip->inst[0] = ins;
   ip->inst[1] = dest;
   ip->inst[2] = src1;
   ip->inst[3] = src2;
   return(ip);
}

void KillAllInst(INSTQ *base)
{
   INSTQ *bp, *next;
   if (base)
   {
      bp = base;
      do
      {
         next = bp->next;
	 free(bp);
	 bp = next;
      }
      while(bp != base);
   }
}

INSTQ *InsNewInst(INSTQ *prev, INSTQ *next, enum inst ins,
                  short dest, short src1, short src2)
/*
 * Insert an instruction in queue pointed at by iqhead.
 * If prev is set, add after prev inst.
 * Otherwise, if next is set, add before next in queue.
 * Otherwise (both prev and next are NULL), instruction is added to end.
 */
{
   INSTQ *ip;
   if (!iqhead || !(prev || next))
   {
      if (iqhead)
      {
         ip = NewInst(iqhead->prev, iqhead, ins, dest, src1, src2);
         iqhead->prev = ip;
         ip->prev->next = ip;
      }
      else
      {
         ip = iqhead = NewInst(NULL, NULL, ins, dest, src1, src2);
         iqhead->next = iqhead->prev = iqhead;
      }
   }
   else if (prev)
   {
      ip = prev->next = NewInst(prev, prev->next, ins, dest, src1, src2);
      prev->next->next->prev = prev->next;
   }
   else /* if (next) */
   {
      ip = next->prev = NewInst(next->prev, next, ins, dest, src1, src2);
      next->prev->prev->next = next->prev;
      if (next == iqhead) iqhead = ip;
   }
   return(ip);
}

INSTQ *DelInst(INSTQ *del)
/*
 * Deletes inst del from Q, keeping links cosher
 * RETURNS: next inst in queue
 */
{
   INSTQ *ip=NULL;
   if (iqhead)
   {
      ip = iqhead;
      do
      {
         if (ip == del) break;
         ip = ip->next;
      }
      while(ip != iqhead);
      if (ip == del)
      {
         ip = del->prev->next = del->next;
         del->next->prev = del->prev;
         if (del == iqhead)
         {
            if (iqhead->next = iqhead) iqhead=NULL;
            else iqhead = iqhead->next;
         }
         free(del);
      }
      else ip = NULL;
   }
   return(ip);
}
