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

BBLOCK *FindBlockInList(BLIST *lp, BBLOCK *blk)
{
   for (; lp; lp = lp->next)
      if (lp->blk == blk) return(blk);
   return(NULL);
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
      PrintComment(bp, NULL, bp->inst1, "**************************");
      PrintComment(bp, NULL, bp->inst1, "   label = %d (%s)", bp->ilab, 
                               bp->ilab ? STname[bp->ilab-1] : "NULL");
      PrintComment(bp, NULL, bp->inst1, "   doms = %s",
                   PrintVecList(bp->dom, 1));
      PrintComment(bp, NULL, bp->inst1, "   usucc=%d, csucc=%d",
                   bp->usucc ? bp->usucc->bnum : 0, 
                   bp->csucc ? bp->csucc->bnum : 0);
      PrintComment(bp, NULL, bp->inst1, "Begin basic block %d (%d)",
                   i+1, bp->bnum);
      PrintComment(bp, NULL, bp->inst1, "**************************");
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
   INSTQ *ip, *inst1=NULL;
   int j=2;

   if (!base0) return(NULL);
   bn = bbase = NewBasicBlock(NULL, NULL);
   bn->bnum = 1;
   for (bp=base0; bp; bp = bp->down)
   {
      for (ip=bp->inst1; ip; ip = ip->next)
      {
         if (bn->inst1)
         {
            if (ip->inst[0] != LABEL)
            {
               InsNewInst(bn, NULL, NULL, ip->inst[0], ip->inst[1],
                          ip->inst[2], ip->inst[3]);
            }
            else  /* labels mark beginning of new basic block */
            {
               bn->down = NewBasicBlock(bn, NULL);
               bn = bn->down;
               bn->bnum = j++;
               InsNewInst(bn, NULL, NULL, ip->inst[0],
                          ip->inst[1], ip->inst[2], ip->inst[3]);
               bn->ilab = ip->inst[1];
               continue;
            }
         }
         else  /* this is 1st inst in block, labels OK */
         {
            InsNewInst(bn, NULL, NULL, ip->inst[0], ip->inst[1],
                       ip->inst[2], ip->inst[3]);
            if (ip->inst[0] == LABEL) bn->ilab = ip->inst[1];
            fprintf(stderr, "in=%d, LABEL=%d, ilab=%d op1=%d\n", ip->inst[0], LABEL, bn->ilab, ip->inst[1]);
         }
/*
 *       Branches mark end of this block
 */
         if (IS_BRANCH(ip->inst[0]))
         {
            bn->down = NewBasicBlock(bn, NULL);
            bn = bn->down;
            bn->bnum = j++;
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
fprintf(stderr, "looking for %d, found %d\n", ilab, bp->ilab);
         if (bp->ilab == ilab) break;
         bp = bp->down;
      }
      while(bp);
   }
   return(bp);
}

void SetBlocksActiveInst(BBLOCK *bp)
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
         if (inst != JMP && inst != RET)
         {
            bp->csucc = FindBlockWithLabel(bbase, bp->ainstN->inst[3]);
            bp->usucc = bp->down;
         }
         else if (inst != RET)
         {
            bp->usucc = FindBlockWithLabel(bbase, bp->ainstN->inst[2]);
            assert(bp->usucc);
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

#if IFKO_DEBUG_LEVEL > 0
void CheckFlow(BBLOCK *bbase, char *file, int line)
{
   BBLOCK *bp;
   INSTQ *ip;
   int error=0, i;
   short in;
   if (bbase->preds)
   {
      fprintf(stderr, "Predecessor for root block (%d) : %d!!\n", 
              bbase->bnum, bbase->preds->blk->bnum);
      error = 1;
   }
   for (i=0, bp=bbase; bp; i++, bp = bp->down)
   {
      if (bp->bnum != i+1)
      {
         fprintf(stderr, "Block %d, should be %d\n", bp->bnum, i+1);
         error = i+1;
      }
      if (!(bp->inst1 || bp->instN))
      {
         fprintf(stderr, "Block %d, inst1=%d, instN=%d\n", bp->bnum,
                 bp->inst1, bp->instN);
         error = i+1;
      }
/*
 *    Make sure this node a pred of any seccessor
 */
      if (bp->usucc && !FindBlockInList(bp->usucc->preds, bp))
      {
         fprintf(stderr, 
                 "Unconditional successor %d does not have %d in preds!\n",
                 bp->usucc->bnum, bp->bnum);
         error = i+1;
      }
      if (bp->csucc && !FindBlockInList(bp->csucc->preds, bp))
      {
         fprintf(stderr, 
                 "Conditional successor %d does not have %d in preds!\n",
                 bp->csucc->bnum, bp->bnum);
         error = i+1;
      }
      if (!bp->inst1 && !bp->instN)
         fprintf(stderr, "WARNING, block %d has no instructions!\n", bp->bnum);
      if (bp->inst1)
      {
         for (ip=bp->inst1; ip && ip->inst[0] == COMMENT; ip = ip->next);
         if (ip != bp->ainst1)
         {
            fprintf(stderr, "Wrong ainst (%d) for block %d, expected %d\n",
                    bp->ainst1, bp->bnum, ip);
            error = i+1;
         }
         for (ip=bp->inst1; ip->next; ip = ip->next);
         if (ip != bp->instN)
         {
            fprintf(stderr, "Wrong instN (%d) for block %d, expected %d\n",
                    bp->instN, bp->bnum, ip);
            error = i+1;
         }
         for (; ip && ip->inst[0] == COMMENT; ip = ip->prev);
         if (ip != bp->ainstN)
         {
            fprintf(stderr, "Wrong ainstN (%d) for block %d, expected %d\n",
                    bp->ainstN, bp->bnum, ip);
            error = i+1;
         }
      }
      if (!bp->ainst1)
      {
         if (bp->ainstN)
         {
            fprintf(stderr, "BLOCK %d wrong, ainstN is set, but ainst1 not!\n",
                    bp->bnum);
            error = i+1;
         }
         else fprintf(stderr, "WARNING, block %d ainst1 not set!\n", bp->bnum);
      }
      if (!bp->ainstN)
      {
         if (bp->ainst1)
         {
            fprintf(stderr, "BLOCK %d wrong, ainst1 is set, but ainstN not!\n",
                    bp->bnum);
            error = i+1;
         }
         else fprintf(stderr, "WARNING, block %d ainstN not set!\n", bp->bnum);
      }
      in = bp->ainstN ? bp->ainstN->inst[0] : -1;
      if (!bp->usucc)
      {
         if (bp->down && in != RET)
         {
            fprintf(stderr, 
                    "No uncond succ for block %d, next blk=%d, last inst=%d\n",
                    bp->bnum, bp->down ? bp->down->bnum : -1, in);
            error = i+1;
         }
      }
      else if (!bp->down && in != JMP)
      {
         fprintf(stderr, "Block %d: uncond succ %d but last inst = %d\n", 
                 bp->bnum, bp->usucc->bnum, in);
         error = i+1;
      }
      if (bp->csucc)
      {
         if (!IS_BRANCH(in) || in == RET || in == JMP)
         {
            fprintf(stderr, "Block %d: cond succ %d but last inst = %d\n", 
                    bp->bnum, bp->csucc->bnum, in);
            error = i+1;
         }
      }
      else if (IS_BRANCH(in) && in != RET && in != JMP)
      {
         fprintf(stderr, "Block %d: no cond succ but last inst = %d\n", 
                 bp->bnum, in);
         error = i+1;
      }
/*
 *    May want to put in dominator sanity test?
 */
   }
   if (error)
   {
      fprintf(stderr, "Error in CheckFlow called from line %d of %s\n",
              line, file);
/*      while(1); */
   }
}
#endif

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
