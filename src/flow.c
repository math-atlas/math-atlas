#include "ifko.h"

BBLOCK *bbbase=NULL;

BLIST *NewBlockList(BBLOCK *blk, BLIST *next)
{
   BLIST *lp;
   lp = malloc(sizeof(BLIST));
   assert(lp);
   lp->blk = blk;
   lp->next = next;
   return(lp);
}

void KillBlockList(BLIST *lp)
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
   bp->bnum = 0;
   bp->ilab = 0;
   bp->up = up;
   bp->down = down;
   bp->usucc = bp->csucc = NULL;
   bp->ainst1 = bp->ainstN = bp->inst1 = bp->instN = NULL;
   bp->preds = NULL;
   bp->dom = 0;
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
      KillBlockList(bp->preds);
      free(bp);
   }
}

int NumberBasicBlocks(BBLOCK *bp)
/*
 * Numbers the basic block (starting from 1), returning total number of blocks 
 */
{
   int i;
   for (i=1; bp; bp = bp->down, i++) bp->bnum = i;
   return(i-1);
}

void AddBlockComments(BBLOCK *bp)
/*
 * Adds comments indicating start of basic blocks
 * NOTE: perform this right before assembly gen, as it will screw up assumption
 *       about label being first statement of block
 */
{
   int i;
   for (i=0; bp; bp = bp->down, i++)
   {
      bp->inst1 = PrintComment(NULL, bp->inst1, "**************************");
      bp->inst1 = PrintComment(NULL, bp->inst1, "   doms = %s",
                               PrintVecList(bp->dom, 1));
      bp->inst1 = PrintComment(NULL, bp->inst1, "   usucc=%d, csucc=%d",
                               bp->usucc ? bp->usucc->bnum : 0, 
                               bp->csucc ? bp->csucc->bnum : 0);
      bp->inst1 = PrintComment(NULL, bp->inst1, "Begin basic block %d (%d)", 
                               i+1, bp->bnum);
      bp->inst1 = PrintComment(NULL, bp->inst1, "**************************");
   }
}

BBLOCK *FindBasicBlocks(BBLOCK *base0)
/*
 * Computes new basic blocks by examining inst in old basic block queue,
 * including setting ilab if appropriate for each block
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
               in = NewInst(bn, NULL, NULL, ip->inst[0],
                            ip->inst[1], ip->inst[2], ip->inst[3]);
               bn->ilab = ip->inst[1];
               continue;
            }
         }
         else  /* this is 1st inst in block, labels OK */
         {
            in = NewInst(bn, NULL, NULL, ip->inst[0], ip->inst[1],
                         ip->inst[2], ip->inst[3]);
            if (ip->inst[0] == LABEL) bn->ilab = ip->inst[1];
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

BBLOCK *FindBlockWithLabel(BBLOCK *bp, int ilab)
/*
 * Given a label's ST index, return the block that starts with it
 */
{
   if (bp)
   {
      do
      {
         if (bp->ilab == ilab) break;
         bp = bp->down;
      }
      while(bp);
   }
   return(bp);
}

void SetBlockActiveInst(BBLOCK *bp)
/*
 * Given a set of basic blocks, find 1st and last non-comment instruction
 */
{
   INSTQ *ip;
   for (; bp; bp = bp->down)
   {
      for (ip=bp->inst1; ip && ip->inst[0] == COMMENT; ip = ip->next);
      bp->ainst1 = ip;
      for (ip=bp->instN; ip && ip->inst[0] == COMMENT; ip = ip->prev);
      bp->ainstN = ip;
   }
}

void KillPredSuccInfo(BBLOCK *bp)
/*
 * Kills pred/succ info in block list
 */
{
   for (; bp; bp = bp->down)
   {
      bp->usucc = bp->csucc = NULL;
      if (bp->preds)
      {
         KillBlockList(bp->preds);
         bp->preds = NULL;
      }
   }
}

void FindPredSuccBlocks(BBLOCK *bbase)
/*
 * Calculates predecessor and successor to blocks in bp list.  Does this
 * by examining successors only.
 */
{
   BLIST *lp, *lp0;
   BBLOCK *bp;
   short inst;

   #if IFKO_DEBUG_LEVEL >= 1
      for (bp=bbase; bp; bp = bp->down)
         assert(!(bp->usucc || bp->csucc || bp->preds));
   #endif
   for (bp=bbase; bp; bp = bp->down)
   {
      inst = bp->ainstN->inst[0];
      if (IS_BRANCH(inst))
      {
/*
 *       Unconditional jump sets only usucc
 */
         if (inst != JMP)
         {
            bp->usucc = FindBlockWithLabel(bbase, bp->ainstN->inst[1]);
         }
         else
         {
            bp->csucc = FindBlockWithLabel(bbase, bp->ainstN->inst[1]);
            bp->usucc = bp->down;
         }
      }
      else bp->usucc = bp->down;
      if (bp->usucc) bp->usucc->preds = NewBlockList(bp, bp->usucc->preds);
      if (bp->csucc) bp->csucc->preds = NewBlockList(bp, bp->csucc->preds);
   }
}

void CalcDoms(BBLOCK *bbase)
/*
 * Calculates dominator information for all basic blocks
 */
{
   int n, CHANGE, old;
   BBLOCK *bp;
   BLIST *lp;
   extern int FKO_BVTMP;

   if (!bbase) return;
/*
 * Number basic blocks, and set top block as only being dominated by itself
 */
   n = NumberBasicBlocks(bbase);
   bbase->dom = NewBitVec(n);
   SetVecBit(bbase->dom, bbase->bnum-1, 1);
   if (FKO_BVTMP) old = FKO_BVTMP;
   else old = FKO_BVTMP = NewBitVec(n);
/*
 * Init all other dom lists to all blocks
 */
   for (bp=bbase->down; bp; bp = bp->down)
   {
      bp->dom = NewBitVec(n);
      SetVecAll(bp->dom, 1);
   }

   do
   {
      CHANGE = 0;
      for (bp=bbase->down; bp; bp = bp->down)
      {
         BitVecCopy(old, bp->dom);
         if (bp->preds)
         {
            bp->dom = BitVecCopy(bp->dom, bp->preds->blk->dom);
            for (lp=bp->preds->next; lp; lp = lp->next)
               BitVecComb(bp->dom, bp->dom, lp->blk->dom, '&');
         }
         else SetVecAll(bp->dom, 0);
         SetVecBit(bp->dom, bp->bnum-1, 1);
         CHANGE |= BitVecComp(old, bp->dom);
      }
   }
   while(CHANGE);
}

void KillUselessBlocks(BBLOCK *bbase)
/*
 * Remove useless blocks from queue.  Useless blocks are:
 * (1) blocks with no statments
 * (2) Blocks that are only a label are moved to next block
       -- May require renaming label to next block's label
 * (3)
 */
{

}
