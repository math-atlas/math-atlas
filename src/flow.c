#include "ifko.h"

BBLOCK *bbbase=NULL;

BLIST *NewBlockList(BBLOCK *blk)
{
   BLIST *lp;
   lp = malloc(sizeof(BLIST));
   assert(lp);
   lp->blk = blk;
   lp->next = NULL;
   return(lp);
}

void KillAllBlockList(BLIST *lp)
{
   BLIST *ln;
   for(; lp; lp = ln)
   {
      ln = lp->next;
      free(lp);
   }
}

BBLOCK *NewBasicBlock(BBLOCK *up, BBLOCK *down)
/*
 * Allocates new basic block, returns allocated block
 */
{
   BBLOCK *bp;
   bp = malloc(sizeof(BBLOCK));
   assert(bp);
   bp->up = up;
   bp->down = down;
   bp->usucc = bp->csucc = NULL;
   bp->inst1 = bp->instN = NULL;
   bp->preds = NULL;
   return(bp);
}

void KillAllBasicBlocks(BBLOCK *base)
/*
 * Frees all basic blocks, including all intructions in them
 */
{
   BBLOCK *bp, *bn;
   for (bp=base; bp; bp = bn)
   {
      bn = bp->down;
      KillAllInst(bp->inst1);
      KillAllBlockList(bp->preds);
      free(bp);
   }
}

BBLOCK *FindBasicBlocks(BBLOCK *base0)
/*
 * Computes new basic blocks by examining inst in old basic block queue
 * RETURNS: new base ptr
 */
{
   BBLOCK *bbase=NULL, *bp, *bn;
   INSTQ *ip, *in, *inst1=NULL;

   if (!base0) return(NULL);
   bn = bbase = NewBasicBlock(NULL, NULL);
   for (bp=base0; bp; bp = bp->down)
   {
      for (ip=bp->inst1; ip; ip = ip->next)
      {
         if (bn->inst1)
         {
            if (ip->inst[0] != LABEL)
            {
               in->next = NewInst(bn, in, NULL, ip->inst[0], ip->inst[1],
                                  ip->inst[2], ip->inst[3]);
               in = in->next;
            }
            else  /* labels mark beginning of new basic block */
            {
               bn->down = NewBasicBlock(bn, NULL);
               bn = bn->down;
               in = bn->inst1 = NewInst(bn, NULL, NULL, ip->inst[0],
                                        ip->inst[1], ip->inst[2], ip->inst[3]);
               continue;
            }
         }
         else  /* this is 1st inst in block, labels OK */
         {
            in = bn->inst1 = NewInst(bn, NULL, NULL, ip->inst[0], ip->inst[1],
                                     ip->inst[2], ip->inst[3]);
         }
/*
 *       Branches mark end of this block
 */
         if (IS_BRANCH(in->inst[0]))
         {
            bn->down = NewBasicBlock(bn, NULL);
            bn = bn->down;
            in = NULL;
         }
      }
   }
/* 
 * If last block is empty, delete it
 */
   if (!bn->inst1)
   {
      assert(bn->up);
      bn->up->down = NULL;
      free(bn);
   }
   return(bbase);
}

void AddBlockComments(BBLOCK *bp)
/*
 * Adds comments indicating start of basic blocks
 */
{
   int i;
   for (i=0; bp; bp = bp->down, i++)
   {
      bp->inst1 = PrintComment(NULL, bp->inst1, "**************************");
      bp->inst1 = PrintComment(NULL, bp->inst1, "Begin basic block %d", i+1);
      bp->inst1 = PrintComment(NULL, bp->inst1, "**************************");
   }
}
