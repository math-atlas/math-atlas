#include "ifko.h"

BBLOCK *bbbase=NULL;

BLIST *NewBlockList(BBLOCK *blk, BLIST *next)
{
   BLIST *lp;
   lp = malloc(sizeof(BLIST));
   assert(lp);
   lp->blk = blk;
   lp->next = next;
   lp->ptr = NULL;
   return(lp);
}

BLIST *AddBlockToList(BLIST *list, BBLOCK *blk)
/*
 * Adds a block to a blist (in second position).  
 * RETURNS: ptr to list (created if list is NULL, list otherwise).
 */
{
   list = NewBlockList(blk, list);
   return(list);
}

static BLIST *KillBlockListEntry(BLIST *lp)
{
   BLIST *ln;
   if (lp)
   {
      ln = lp->next;
      free(lp);
   }
   else ln = NULL;
   return(ln);
}

BLIST *RemoveBlockFromList(BLIST *list, BBLOCK *blk)
/*
 * Removes block blk from list list, if it exists
 * RETURNS: (possibly new) list
 */
{
   BLIST *lprev, *lp;
   lp = list;
   if (!list) return(NULL);
   if (list->blk == blk) list = list->next;
   else
   {
      for (lprev=list; lp && lp->blk != blk; lp = lp->next) lprev=lp;
      if (lp) lprev->next = lp->next;
   }
   KillBlockListEntry(lp);
   return(list);
}

int BlockList2BitVec(BLIST *lp)
{
   static int iv=0;

   if (!iv) iv = NewBitVec(32);
   SetVecAll(iv, 0);
   for (; lp; lp = lp->next)
      SetVecBit(iv, lp->blk->bnum-1, 1);
   return(iv);
}

BLIST *BitVec2BlockList(int iv)
/*
 * Given a bitvec of block nums, return a BLIST of the appropriate blocks
 */
{
   BBLOCK *bp;
   BLIST *lp=NULL;

   for (bp=bbbase; bp; bp = bp->down)
   {
      if (BitVecCheck(iv, bp->bnum-1))
         lp = AddBlockToList(lp, bp);
   }
   return(lp);
}

BLIST *FindInList(BLIST *lp, BBLOCK *blk)
{
   for (; lp; lp = lp->next)
      if (lp->blk == blk) return(lp);
   return(NULL);
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

BLIST *MergeBlockLists(BLIST *l1, BLIST *l2)
/*
 * Adds list l2 to list l1, making sure not to duplicate entries, and
 * deallocates l2
 */
{
   BLIST *p;
   int iv1, iv2;
   extern int FKO_BVTMP;

   if (!l1) 
      return(l2);
   if (!l2)
      return(l1);
   iv1 = BlockList2BitVec(l1);
   iv1 = FKO_BVTMP = BitVecCopy(FKO_BVTMP, iv1);
   iv2 = BlockList2BitVec(l2);
   BitVecComb(iv1, iv2, iv1, '-');
   for (p=l2; p; p = p->next)
   {
      if (BitVecCheck(iv1, p->blk->bnum-1))
      {
         l1 = AddBlockToList(l1, p->blk);
         l1->ptr = p->ptr;
      }
   }
   KillBlockList(l2);
   return(l1);
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
   bp->ins = bp->outs = bp->uses = bp->defs = bp->dom = 0;
   bp->loop = NULL;
   bp->conin = bp->conout = 0;
   return(bp);
}

BBLOCK *KillBlock(BBLOCK *base, BBLOCK *killme)
/*
 * Removes block killme from queue starting at base
 * RETURNS: (possibly new) base
 */
{
/* 
 * Remove killme from block queue
 */
   if (killme->up) killme->up->down = killme->down;
   else base = base->down;
   if (killme->down) killme->down->up = killme->up;
/*
 * If this block usucc of killme->up, change it to killme->down
 */
   if (killme->up->usucc == killme)
   {
      assert(GET_INST(killme->ainstN->inst[0]) != JMP);
      killme->up->usucc = killme->down;
   }
/*
 * Remove this block from pred of any successor
 */
   if (killme->usucc)
      RemoveBlockFromList(killme->usucc->preds, killme);
   if (killme->csucc)
      RemoveBlockFromList(killme->csucc->preds, killme);
/*
 * HERE HERE HERE
 */
   return(base);
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

BBLOCK *FindBlockByNumber(BBLOCK *bp, const short bnum)
/*
 * Finds block with number bnum, and returns its pointer
 */
{
   if (!bp) bp = bbbase;
   for(; bp && bp->bnum != bnum; bp = bp->down);
   return(bp);
}

char *PrintBlockList(BLIST *lp)
{
   static char ln[512];
   int j;

   for (j=0; lp; lp=lp->next)
   {
      if (lp->blk)
         j += sprintf(ln+j, "%d,", lp->blk->bnum);
      else
         j += sprintf(ln+j, "0,");
   }
   if (j) ln[j-1] = '\0';
   else strcpy(ln, "NONE");
   return(ln);
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
      PrintComment(bp, NULL, bp->inst1, "   uses = %s",
                   BV2VarNames(bp->uses));
      PrintComment(bp, NULL, bp->inst1, "   defs = %s",
                   BV2VarNames(bp->defs));
      PrintComment(bp, NULL, bp->inst1, "   outs = %s",
                   BV2VarNames(bp->outs));
      PrintComment(bp, NULL, bp->inst1, "   ins  = %s",
                   BV2VarNames(bp->ins));
      PrintComment(bp, NULL, bp->inst1, "   label = %d (%s)", bp->ilab, 
                               bp->ilab ? STname[bp->ilab-1] : "NULL");
      PrintComment(bp, NULL, bp->inst1, "   doms = %s", bp->dom ?
                   PrintVecList(bp->dom, 1) : "NOT SET");
      PrintComment(bp, NULL, bp->inst1, "   preds = %s", 
                   PrintBlockList(bp->preds));
      PrintComment(bp, NULL, bp->inst1, "   usucc=%d, csucc=%d",
                   bp->usucc ? bp->usucc->bnum : 0, 
                   bp->csucc ? bp->csucc->bnum : 0);
      PrintComment(bp, NULL, bp->inst1, "Begin basic block %d (%d)",
                   i+1, bp->bnum);
      PrintComment(bp, NULL, bp->inst1, "**************************");
      PrintComment(bp, NULL, bp->inst1, "");
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
fprintf(stderr, "looking for %d, block=%d, ilab=%d\n", ilab, bp->bnum, bp->ilab);
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
            if (!bp->usucc)
            {
               fprintf(stderr, "could not find label %d (%s)!!\n",
                       bp->ainstN->inst[2], STname[bp->ainstN->inst[2]-1]);
               while(1);
            }
            assert(bp->usucc);
         }
      }
      else bp->usucc = bp->down;
      if (bp->usucc) bp->usucc->preds = NewBlockList(bp, bp->usucc->preds);
      if (bp->csucc) bp->csucc->preds = NewBlockList(bp, bp->csucc->preds);
   }
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
 *    --- Not right now, info only figured for loops anyway . . .
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

BBLOCK *NewBasicBlocks(BBLOCK *base0)
/*
 * Uses the instructions of old set of basic blocks to compute a new set;
 * kills the old set.
 */
{
   BBLOCK *base;
   base = FindBasicBlocks(base0);
   KillAllBasicBlocks(base0);
   SetBlocksActiveInst(base);
   FindPredSuccBlocks(base);
   bbbase = base;
   CheckFlow(base, __FILE__, __LINE__);
   CFU2D = 1;
   return(base);
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
   extern BBLOCK *bbbase;

   if (!bbase) return;
   if (!CFU2D && bbase == bbbase)
      bbase = NewBasicBlocks(bbase);
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
            BitVecCopy(bp->dom, bp->preds->blk->dom);
            for (lp=bp->preds->next; lp; lp = lp->next)
               BitVecComb(bp->dom, bp->dom, lp->blk->dom, '&');
         }
         else SetVecAll(bp->dom, 0);
         SetVecBit(bp->dom, bp->bnum-1, 1);
         CHANGE |= BitVecComp(old, bp->dom);
      }
   }
   while(CHANGE);
   if (bbase == bbbase)
      CFDOMU2D = 1;
}

LOOPQ *NewLoop(int flag)
{
   LOOPQ *lp, *l;
   short lnum=0;

   lp = malloc(sizeof(struct loopq));
   assert(lp);
   lp->flag = flag;
   lp->depth = lp->I = lp->beg = lp->end = lp->inc = lp->body_label = 0;
   lp->loopnum = lp->maxunroll = lp->writedd = 0;
   lp->slivein = lp->sliveout = lp->adeadin = lp->adeadout = lp->nopf =
                 lp->aaligned = NULL;
   lp->abalign = NULL;
   lp->preheader = lp->header = NULL;
   lp->blocks = NULL;
   lp->tails = lp->posttails = NULL;
   lp->blkvec = lp->outs = lp->sets = 0;
   lp->iglist = NULL;
   lp->next = NULL;
   return(lp);
}

LOOPQ *KillLoop(LOOPQ *lp)
{
   LOOPQ *ln=NULL;

   if (lp)
   {
      ln = lp->next;
      if (lp->slivein) free(lp->slivein);
      if (lp->sliveout) free(lp->sliveout);
      if (lp->adeadin) free(lp->adeadin);
      if (lp->adeadout) free(lp->adeadout);
      if (lp->nopf) free(lp->nopf);
      if (lp->aaligned) free(lp->aaligned);
      if (lp->blocks) KillBlockList(lp->blocks);
      free(lp);
   }
   return(ln);
}

void KillAllLoops()
{
   while (loopq) loopq = KillLoop(loopq);
}

LOOPQ *InsNewLoop(LOOPQ *prev, LOOPQ *next, int flag)
{
   LOOPQ *lp, *loop;

   loop = NewLoop(flag);
   if (loopq)
   {
      if (prev)
      {
         for (lp=loopq; lp && lp != prev; lp = lp->next);
         assert(lp);
         loop->next = lp->next;
         lp->next = loopq;
      }
      else
      {
         for (lp=loopq; lp && lp->next != next; lp = lp->next);
         assert(lp);
         loop->next = lp->next;
         lp->next = loop;
      }
   }
   else loopq = loop;
   return(loop);
}

void InsertLoopBlock(short blkvec, BBLOCK *blk)
{
   BLIST *lp;
   if (!BitVecCheck(blkvec, blk->bnum-1))
   {
      SetVecBit(blkvec, blk->bnum-1, 1);
      for (lp=blk->preds; lp; lp = lp->next)
         InsertLoopBlock(blkvec, lp->blk);
   }
}

void FindBlocksInLoop(BBLOCK *head, BBLOCK *tail)
/* 
 * Finds all blocks in the loop with provided head and tail
 */
{
   LOOPQ *loop;
/*
 * If head already head of loop, just add new blocks to existing loop
 */
   if (head->loop) loop = head->loop;
/*
 * Found new loop, allocate new loop struct
 */
   else
   {
      head->loop = loop = InsNewLoop(NULL, NULL, 0);
      loop->header = head;
      loop->blkvec = NewBitVec(32);
      SetVecBit(loop->blkvec, head->bnum-1, 1);
   }
   InsertLoopBlock(loop->blkvec, tail);
}

int CalcLoopDepth()
/*
 * Calculates loop depth, assuming initialized to 1
 * RETURNS: max loop depth
 */
{
   LOOPQ *loop;
   BLIST *lp;
   short maxdep=0, dep;

   for (loop=loopq; loop; loop = loop->next)
   {
      for (lp=loopq->blocks; lp; lp = lp->next)
      {
         if (lp->blk != loop->header && lp->blk->loop)
            dep = lp->blk->loop->depth++;
         else if (lp->blk->loop) dep = lp->blk->loop->depth;
         else dep = 0;
         if (dep > maxdep) maxdep = dep;
      }
   }
   return(maxdep);
}
   
void RemoveLoopFromQ(LOOPQ *loop)
/*
 * Removes loop from loopq
 */
{
   LOOPQ *lp, *prev=NULL;

   for (lp=loopq; lp && lp != loop; lp = lp->next) prev = lp;
   assert(lp);
   if (prev) prev->next = lp->next;
   else loopq = loopq->next;
}
void SortLoops(short maxdepth)
/*
 * Sorts loops so in decreasing depth order, putting optloop first
 */
{
   LOOPQ *lbase=NULL, *lp, *ln;
   int i;

   if (optloop)
   {
      RemoveLoopFromQ(optloop->next);
      optloop->depth = optloop->next->depth;
      optloop->preheader = optloop->next->preheader;
      optloop->header = optloop->next->header;
      optloop->blocks = optloop->next->blocks;
      optloop->blkvec = optloop->next->blkvec;
      optloop->tails = optloop->next->tails;
      optloop->posttails = optloop->next->posttails;
      optloop->outs = optloop->next->outs;
      optloop->next->outs = 0;
      optloop->next->blocks = NULL;
      KillLoop(optloop->next);
      optloop->next = NULL;
   }
   for (i=1; i <= maxdepth; i++)
   {
      for (lp=loopq; lp; lp = ln)
      {
         ln = lp->next;
         if (lp->depth == i)
         {
            RemoveLoopFromQ(lp);
            lp->next = lbase;
            lbase = lp;
         }
      }
   }
   if (optloop)
   {
      optloop->next = lbase;
      lbase = optloop;
   }
   assert(!loopq);
   loopq = lbase;
}
void FinalizeLoops()
{
   LOOPQ *lp;
   int maxdep, i;
   int phbv, iv;
   BLIST *bl;
   extern int FKO_BVTMP;

   for (lp=loopq; lp; lp = lp->next)
   {
      lp->blocks = BitVec2BlockList(lp->blkvec);
      lp->depth = 1;
      lp->body_label = lp->header->ilab;
      if (optloop && lp->body_label == optloop->body_label)
         optloop->next = lp;         
/*
 *    See if we have a natural preheader (has only header as successor,
 *    and all paths to the header from outside the loop reach the header
 *    through it)
 */
      if (FKO_BVTMP) phbv = FKO_BVTMP;
      else phbv = FKO_BVTMP = NewBitVec(32);
/*
 *    Find preds of header that are not in loop.  If there is only one,
 *    it's the preheader if it has no other successors
 */
      BitVecInvert(phbv, lp->blkvec);
      i = BlockList2BitVec(lp->header->preds);
      BitVecComb(phbv, phbv, i, '&');
      if (CountBitsSet(phbv) == 1)
      {
         i = GetSetBitX(phbv, 1);  /* i now block number of preheader */
         lp->preheader = FindBlockByNumber(NULL, i+1);
         if ( (lp->preheader->usucc && lp->preheader->usucc != lp->header) ||
              (lp->preheader->csucc && lp->preheader->csucc != lp->header) )
            lp->preheader = NULL;
      }
/*
 *    Find the tail(s) of the loop.  Loop tails are preds of the header
 *    which are in the loop.
 */
      fprintf(stderr, "\n\nheader preds = %s\n",
              PrintBlockList(lp->header->preds));
      phbv = BlockList2BitVec(lp->header->preds);
      BitVecComb(phbv, phbv, lp->blkvec, '&');
      fprintf(stderr, "blkvec = %s\n", PrintVecList(lp->blkvec, 1));
      lp->tails = BitVec2BlockList(phbv);
      fprintf(stderr, "tails = %s\n",
              PrintBlockList(lp->tails));
/*
 *    Find the post-tail(s) of the loop.  Loop post-tails are succ of
 *    tail that are outside the loop and have no non-tail preds
 */
      lp->posttails = BitVec2BlockList(phbv);
      iv = BlockList2BitVec(lp->tails);
      phbv = BitVecCopy(FKO_BVTMP, iv);
      for (bl=lp->posttails; bl; bl = bl->next)
      {
         if (bl->blk->csucc == lp->header)
         {
            assert(!BitVecCheck(lp->blkvec, bl->blk->usucc->bnum-1));
            iv = BlockList2BitVec(bl->blk->usucc->preds);
            BitVecComb(iv, iv, phbv, '-');
            if (GetSetBitX(iv, 1)) bl->blk = NULL;
            else bl->blk = bl->blk->usucc;
         }
         else
         {
            assert(bl->blk->usucc == lp->header);
            assert(!BitVecCheck(lp->blkvec, bl->blk->csucc->bnum-1));
            iv = BlockList2BitVec(bl->blk->csucc->preds);
            BitVecComb(iv, iv, phbv, '-');
            if (GetSetBitX(iv, 1)) bl->blk = NULL;
            else bl->blk = bl->blk->csucc;
         }
      }
      fprintf(stderr, "posttails = %s\n",
              PrintBlockList(lp->posttails));
   }
/*   prepostloops(); */
   maxdep = CalcLoopDepth();
   if (optloop)
   {
      assert(optloop->next);
      fprintf(stderr, "maxdep=%d, optdep=%d\n", maxdep, optloop->next->depth);
      assert(optloop->next->depth == maxdep);
   }
   SortLoops(maxdep);
   for (i=1, lp=loopq; lp; i++, lp = lp->next) lp->loopnum = i;
}

void FindLoops()
/*
 * Finds loops, killing old list
 */
{
   BBLOCK *bp;
   if (!CFDOMU2D)
   {
      CalcDoms(bbbase);
      CFDOMU2D = 1;
   }
   CheckFlow(bbbase, __FILE__, __LINE__);
   if (loopq) KillAllLoops();
/*
 * Search for edges whose heads dominate their tails, i.e., find blocks
 * that are dominated by their successors
 */
   for (bp=bbbase; bp; bp = bp->down)
   {
      if (bp->usucc && BitVecCheck(bp->dom, bp->usucc->bnum-1))
         FindBlocksInLoop(bp->usucc, bp);
      if (bp->csucc && BitVecCheck(bp->dom, bp->csucc->bnum-1))
         FindBlocksInLoop(bp->csucc, bp);
   }
   FinalizeLoops();
}

void AddLoopComments()
/*
 * Adds comments indicating start of loops
 * NOTE: perform this right before assembly gen, as it will screw up assumption
 *       about label being first statement of block
 */
{
   LOOPQ *lp;
   BBLOCK *bp;
   for (lp=loopq; lp; lp = lp->next)
   {
      bp = lp->header;
      PrintComment(bp, NULL, bp->inst1, "===============================");
      PrintComment(bp, NULL, bp->inst1, "   live outs = %s",
                   BV2VarNames(lp->outs));
      PrintComment(bp, NULL, bp->inst1, "   blocks = %s",
                   PrintVecList(lp->blkvec, 1));
      PrintComment(bp, NULL, bp->inst1, "   posttails=%s", 
                   PrintBlockList(lp->posttails));
      PrintComment(bp, NULL, bp->inst1, "   tails=%s", 
                   PrintBlockList(lp->tails));
      PrintComment(bp, NULL, bp->inst1, "   header=%d, preheader=%d",
                   lp->header ? lp->header->bnum : 0,
                   lp->preheader ? lp->preheader->bnum : 0);
      PrintComment(bp, NULL, bp->inst1, "   flag=%d, depth=%d",
                   lp->flag, lp->depth);
      PrintComment(bp, NULL, bp->inst1, "Begin loop %d", lp->loopnum);
      PrintComment(bp, NULL, bp->inst1, "===============================");
   }
}

