/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#include "fko.h"

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
 * Adds a block to a blist (in first position).  
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


#if 0
int BlockList2BitVec(BLIST *lp)
{
   static int iv=0;
   if (!iv) iv = NewBitVec(32);
   SetVecAll(iv, 0);
   for (; lp; lp = lp->next)
      SetVecBit(iv, lp->blk->bnum-1, 1);
   return(iv);
}
#else
INT_BVI BlockList2BitVecFlagged(BLIST *lp, int init)
/*
 * Added the facility to initialize the static variable
 */
{
   static INT_BVI iv=0;
   if (!init)
   {
      if (!iv) iv = NewBitVec(32);
      SetVecAll(iv, 0);
      for (; lp; lp = lp->next)
         SetVecBit(iv, lp->blk->bnum-1, 1);
   }
   else
   {
      if (iv) KillBitVec(iv);
      iv = 0;
   }
   return(iv);
}
INT_BVI BlockList2BitVec(BLIST *lp)
/*
 * Majedul: it's a wrapper function so that existing code can be kept unchanged 
 */
{
   return(BlockList2BitVecFlagged(lp, 0));
}
#endif

BLIST *BitVec2BlockList(INT_BVI iv)
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

BLIST *GetGlobalScope()
{
   BBLOCK *bp;
   extern BBLOCK *bbbase;
   BLIST *blb=NULL;
   if (bbbase)
   {
      for (bp=bbbase; bp->down; bp = bp->down);
      for (; bp; bp = bp->up)
         blb = NewBlockList(bp, blb);
   }
   return(blb);
}

BBLOCK *FindBlockInListByNumber(BLIST *lp, ushort bnum)
{
   for (; lp; lp = lp->next)
      if (lp->blk->bnum == bnum)
         return(lp->blk);
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
      if(lp) free(lp);
   }
}
BLIST *NewReverseBlockList(BLIST *list)
/*
 * Given a list creates a new list with reverse the block points
 * doesn't delete the old blist.
 */
{
   BLIST *ln=NULL, *bl;
   for (bl=list; bl; bl = bl->next)
   {
      ln = AddBlockToList(ln, bl->blk);
      ln->ptr = bl->ptr;
   }
   return(ln);
}

BLIST *ReverseBlockList(BLIST *list)
{
   BLIST *ln=NULL, *bl;
   for (bl=list; bl; bl = bl->next)
   {
      ln = AddBlockToList(ln, bl->blk);
      ln->ptr = bl->ptr;
   }
   KillBlockList(list);
   return(ln);
}

BLIST *CopyBlockList(BLIST *list)
/*
 * Given a blocklist, this function duplicates the list by keeping the order same
 * and return the new list without killing the original one.
 * NOTE: unlike DupBlockList in optloop.c, this function copies only the list, 
 * not the blocks.
 */
{
   BLIST *ln = NULL; 
   ln = NewReverseBlockList(list);
   ln = ReverseBlockList(ln);
   return(ln);
}

BLIST *MergeBlockLists(BLIST *l1, BLIST *l2)
/*
 * Adds list l2 to list l1, making sure not to duplicate entries, and
 * deallocates l2
 */
{
   BLIST *p;
   int iv1, iv2;
   extern INT_BVI FKO_BVTMP;

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
BLIST *RemoveBlksFromList(BLIST *l1, BLIST *l2)
/*
 * remove blocks of l2 from blist l1 and return the updated l1
 * assumption:  no duplicated copies of blocks
 */
{
   BLIST *bl;
   for (bl=l2; bl; bl=bl->next)
   {
      l1 = RemoveBlockFromList(l1, bl->blk);
   }
   return (l1);
}


BBLOCK *NewBasicBlock(BBLOCK *up, BBLOCK *down)
/*
 * Allocates new basic block, returns allocated block
 */
{
   BBLOCK *bp;
   bp = malloc(sizeof(BBLOCK));
   assert(bp);
/*
 * Majedul: FIXED: creating new block with 0 would complicate the bvec checking
 * as bvec is used for bnum-1!!!
 */
#if 0   
   bp->bnum = 0;
#else
/*
 * Majedul: need to check if bnum = 0 has any special usage!!!
 */
   bp->bnum = 1;
#endif
   bp->ilab = 0;
   bp->up = up;
   bp->down = down;
   bp->usucc = bp->csucc = NULL;
   bp->ainst1 = bp->ainstN = bp->inst1 = bp->instN = NULL;
   bp->preds = NULL;
   bp->ins = bp->outs = bp->uses = bp->defs = bp->dom = 0;
   bp->loop = NULL;
   bp->ignodes = bp->conin = bp->conout = 0;
   return(bp);
}

BBLOCK *KillBlock(BBLOCK *base, BBLOCK *killme)
/*
 * Removes block killme from queue starting at base
 * RETURNS: (possibly new) base
 * NOTE: this function doesn't free the blk!!!
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
 * NOTE: BUG: Majedul: RemoveBlockFromList may change the ptr of preds list
 * So, must re-assign the pointer with return value
 */
   if (killme->usucc)
      killme->usucc->preds = RemoveBlockFromList(killme->usucc->preds, killme);
   if (killme->csucc)
      killme->csucc->preds = RemoveBlockFromList(killme->csucc->preds, killme);
/*
 * HERE HERE HERE
 */
   return(base);
}

BBLOCK *DelBlock(BBLOCK *delblk)
/*
 * This function will not only remove a blk from CFG but also free its storage 
 * will return the down blk. 
 * NOTE: not updated some ref like: optloop->tails, optloop->blocks
 */
{
   BBLOCK *bdown;
   bdown = delblk->down; 

/*
 * remove the blk from CFG
 */
   if (delblk->up) 
      delblk->up->down = delblk->down;
   if (delblk->down)
      delblk->down->up = delblk->up;
/*
 * If this block usucc of up blk, change it to down
 */
   if (delblk->up && delblk->up->usucc == delblk)
   {
      assert(GET_INST(delblk->up->ainstN->inst[0] != JMP));
      delblk->up->usucc = delblk->down;
   }
/*
 * Update preds list of other blks
 */
   if (delblk->usucc)
      delblk->usucc->preds = RemoveBlockFromList(delblk->usucc->preds, 
                                                  delblk);
   if (delblk->csucc)
      delblk->csucc->preds = RemoveBlockFromList(delblk->csucc->preds, 
                                                  delblk);
   /*
    *    now, we can delete the blk
    */
   KillAllInst(delblk->inst1);
   KillBlockList(delblk->preds);
   free(delblk);
   
   return bdown; 
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
   static char ln[2048];
   int j;

   for (j=0; lp; lp=lp->next)
   {
      if (lp->blk)
         j += sprintf(ln+j, "%d,", lp->blk->bnum);
      else
         j += sprintf(ln+j, "0,");
      assert(j < 2048);
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
      PrintComment(bp, NULL, bp->inst1, "   conout = %s", 
                   bp->conout ? PrintVecList(bp->conout, 0): "NULL");
      PrintComment(bp, NULL, bp->inst1, "   conin  = %s", 
                   bp->conin ? PrintVecList(bp->conin, 0): "NULL");
#if 0
      PrintComment(bp, NULL, bp->inst1, "   ignodes = %s", 
                   bp->ignodes ? PrintVecList(bp->ignodes, 0): "NULL");
#endif
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
   INSTQ *ip;
   int j=2;

   if (!base0) return(NULL);
   bn = bbase = NewBasicBlock(NULL, NULL);
   bn->bnum = 1;
   for (bp=base0; bp; bp = bp->down)
   {
      for (ip=bp->inst1; ip; ip = ip->next)
      {
         if (bn->ainst1)
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
         else  /* this is 1st active inst in block, labels OK */
         {
            InsNewInst(bn, NULL, NULL, ip->inst[0], ip->inst[1],
                       ip->inst[2], ip->inst[3]);
/*
 *          Majedul: checking for possible memory leak!!!
 */
#if 0
            fprintf(stderr, "%s %d %d %d\n\n", instmnem[ip->inst[0]], 
                    ip->inst[1], ip->inst[2], ip->inst[3]);
#endif            
            if (ip->inst[0] == LABEL) bn->ilab = ip->inst[1];
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

BLIST *FindBlockListWithLabel(BLIST *list, int ilab)
/*
 * Given a label's ST index, return BLIST entry containing it
 */
{
   BLIST *bl;
   for (bl=list; bl; bl = bl->next)
      if (bl->blk->ilab == ilab) return(bl);
      
   return(NULL);
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

void SetBlocksActiveInst(BBLOCK *bp)
/*
 * Given a set of basic blocks, find 1st and last non-comment instruction
 */
{
   INSTQ *ip;
   for (; bp; bp = bp->down)
   {
      for (ip=bp->inst1; ip && !ACTIVE_INST(ip->inst[0]); ip = ip->next);
      bp->ainst1 = ip;
      for (ip=bp->instN; ip && !ACTIVE_INST(ip->inst[0]); ip = ip->prev);
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
   BBLOCK *bp;
   short inst;

   #if IFKO_DEBUG_LEVEL >= 1
      for (bp=bbase; bp; bp = bp->down)
         assert(!(bp->usucc || bp->csucc || bp->preds));
   #endif
   for (bp=bbase; bp; bp = bp->down)
   {
      if (bp->ainstN)
      {
         inst = bp->ainstN->inst[0];
         if (IS_BRANCH(inst))
         {
/*
 *          Unconditional jump sets only usucc
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
      if (!bp->inst1 && !bp->instN)
         fprintf(stderr, "WARNING, block %d has no instructions!\n", bp->bnum);
      else if (!(bp->inst1 || bp->instN))
      {
         fprintf(stderr, "Block %d, inst1=%d, instN=%d\n", bp->bnum,
                 bp->inst1->inst[0], bp->instN->inst[0]);
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
      if (bp->inst1)
      {
         for (ip=bp->inst1; ip && !ACTIVE_INST(ip->inst[0]); ip = ip->next);
         if (ip != bp->ainst1)
         {
            fprintf(stderr, "Wrong ainst1 (%p) for block %d, expected %p\n",
                    bp->ainst1, bp->bnum, ip);
            fprintf(stderr, "ainst1 = ");
            PrintThisInst(stderr, 0, bp->ainst1);
            fprintf(stderr, "Expected = ");
            if (ip) PrintThisInst(stderr, 0, ip);
            else fprintf(stderr, "NULL\n");
            error = i+1;
         }
         for (ip=bp->inst1; ip->next; ip = ip->next);
         if (ip != bp->instN)
         {
            fprintf(stderr, "Wrong instN (%p) for block %d, expected %p\n",
                    bp->instN, bp->bnum, ip);
            error = i+1;
         }
         for (; ip && !ACTIVE_INST(ip->inst[0]); ip = ip->prev);
         if (ip != bp->ainstN)
         {
            fprintf(stderr, "Wrong ainstN (%p) for block %d, expected %p\n",
                    bp->ainstN, bp->bnum, ip);
            fprintf(stderr, "ainstN = ");
            PrintThisInst(stderr, 0, bp->ainstN);
            fprintf(stderr, "Expected = ");
            if (ip) PrintThisInst(stderr, 0, ip);
            else fprintf(stderr, "NULL\n");
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
         fprintf(stderr, "Block %d: no cond succ but last inst = %d(%s)\n", 
                 bp->bnum, in, (in==-1)?"null": instmnem[in]);
         error = i+1;
      }
/*
 *    May want to put in dominator sanity test?
 *    --- Not right now, info only figured for loops anyway . . .
 */
   }
   if (error)
   {
      fprintf(stderr, "Error %d in CheckFlow called from line %d of %s\n",
              error, line, file);
/*      while(1); */
      exit(error);
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
   CFUSETU2D = INUSETU2D = INDEADU2D = CFDOMU2D = CFLOOP = 0;
   CFU2D = 1;
   return(base);
}

void CalcDoms(BBLOCK *bbase)
/*
 * Calculates dominator information for all basic blocks
 */
{
   int n, CHANGE;
   INT_BVI old;
   BBLOCK *bp;
   BLIST *lp;
   extern INT_BVI FKO_BVTMP;
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
      if (!bp->dom)
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
#if 0
LOOPQ *NewLoop0(int flag, int reset)
{
   LOOPQ *lp;
/*
 * loopnum needs to be updated when we consider multiple loop in grammar, 
 * as loop_body label is generated based on this in h2l
 * loopnum will be updated at the FindLoops() function.
 */
   static short lnum=0;

   if(!reset)
   {
      lp = calloc(1, sizeof(struct loopq));
      assert(lp);
      lp->flag = flag;
      lp->loopnum = lnum++; 
/*
 *    it should not be needed as we use calloc here. still will check.
 */
      lp->maxvars = NULL;
      lp->minvars = NULL;
      lp->se = NULL;
      lp->ae = NULL;
   }
/*
 * To keep a reset mechanism for lnum
 */
   else
   {
      lp = NULL;
      lnum = 0;   
   }

   return(lp);
}

LOOPQ *NewLoop(int flag)
{
   return(NewLoop0(flag, 0));
}
#else
LOOPQ *NewLoop(int flag)
{
   LOOPQ *lp;
   short lnum=0;

   lp = calloc(1, sizeof(struct loopq));
   assert(lp);
   lp->flag = flag;
   lp->loopnum = lnum; 
/*
 * it should not be needed as we use calloc here. still will check.
 */
   lp->maxvars = NULL;
   lp->minvars = NULL;
   lp->se = NULL;
   lp->ae = NULL;

   return(lp);
}
#endif
LOOPQ *KillFullLoop(LOOPQ *lp)
/*
 * As KillLoop function doesn't kill all element, rather preserve some element
 * which will be used in InvalidateLoop, we need a function which will kill 
 * the Loop Completely
 */
{
   int i, N;
   LOOPQ *ln=NULL;
   if (lp)
   {
      ln = lp->next;
      if (lp->vsflag)
         free(lp->vsflag);
      if (lp->vsoflag)
         free(lp->vsoflag);
      if (lp->vscal)
         free(lp->vscal);
      if (lp->varrs)
         free(lp->varrs);
      if (lp->nopf)
         free(lp->nopf);
      if (lp->aaligned)
         free(lp->aaligned);
      if (lp->abalign)
         free(lp->abalign);
      if (lp->faalign)
         free(lp->faalign);
      if (lp->fbalign)
         free(lp->fbalign);
      if (lp->maaligned)
         free(lp->maaligned);
      if (lp->mbalign)
         free(lp->mbalign);
      if (lp->outs)
         KillBitVec(lp->outs);
      if (lp->sets)
         KillBitVec(lp->sets);
      if (lp->blocks)
         KillBlockList(lp->blocks);
      if (lp->tails)
         KillBlockList(lp->tails);
      if (lp->blkvec)
         KillBitVec(lp->blkvec);
      if (lp->posttails)
         KillBlockList(lp->posttails);
      if (lp->vvscal)
         free(lp->vvscal);
      if (lp->bvvscal)
         free(lp->bvvscal);
      if (lp->vvinit)
         free(lp->vvinit);
      if (lp->pfarrs)
         free(lp->pfarrs);
      if (lp->pfdist)
         free(lp->pfdist);
      if (lp->pfflag)
         free(lp->pfflag);
      if (lp->ae)
      {
         for (N=lp->ae[0],i=0; i < N; i++)
            free(lp->aes[i]);
         free(lp->aes);
      }
      if (lp->ae)
         free(lp->ae);
      if (lp->ne)
         free(lp->ne);
      if (lp->se)
      {
         for (N=lp->se[0],i=0; i < N; i++)
            free(lp->ses[i]);
         free(lp->ses);
      }
      if (lp->se)
         free(lp->se);
      if (lp->nse)
         free(lp->nse);
      if (lp->seflag)
         free(lp->seflag);
      
      if (lp->maxvars)
         free(lp->maxvars);
      if (lp->minvars)
         free(lp->minvars);

      free(lp);
   }
   return(ln);
}

LOOPQ *KillLoop(LOOPQ *lp)
/*
 * this loop killing is for invalidating the loop, not complete killing  
 */
{
   LOOPQ *ln=NULL;

   if (lp)
   {
      ln = lp->next;
      if (lp->vsflag)
         free(lp->vsflag);
      if (lp->vsoflag)
         free(lp->vsoflag);
      if (lp->vscal)
         free(lp->vscal);
      if (lp->varrs)
         free(lp->varrs);
      if (lp->nopf)
         free(lp->nopf);
      if (lp->aaligned)
         free(lp->aaligned);
      if (lp->abalign)
         free(lp->abalign);
      if (lp->outs)
         KillBitVec(lp->outs);
      if (lp->sets)
         KillBitVec(lp->sets);
      if (lp->blocks)
         KillBlockList(lp->blocks);
      if (lp->tails)
         KillBlockList(lp->tails);
      if (lp->blkvec)
         KillBitVec(lp->blkvec);
      if (lp->posttails)
         KillBlockList(lp->posttails);
/*
 *    Majedul: this free list is not updated, we need to kill other elements
 *    NOTE: can't free them, need to check in details ... ...
 */
/*      
      if (lp->vvscal)
         free(lp->vvscal);
      if (lp->pfarrs)
         free(lp->pfarrs);
      if (lp->pfdist)
         free(lp->pfdist);
      if (lp->pfflag)
         free(lp->pfflag);
      if (lp->ae)
         free(lp->ae);
      if (lp->ne)
         free(lp->ne);
      if (lp->maxvars)
         free(lp->maxvars);
      if (lp->minvars)
         free(lp->minvars);
*/
/*
 *    FIXME: need to free space for shadow acc : short **aes
 */
      free(lp);
   }
   return(ln);
}

#if 0
LOOPQ *KillOptLoop(LOOPQ *lp)
/*
 * This loop killing function only kill optloop but doesn't free the elements
 * which is from markup generated while parsing... like: falign ptrs
 * FIXME: need to synchronous and keep commented on this loop killers:
 *       KillLoop
 *       KillFullLoop
 *       KillOptLoop
 *       InvalidateLoop
 *       KillAllLoopComplete
 */
{
   int i, N;
   LOOPQ *ln=NULL;

   if (lp)
   {
      ln = lp->next;
/*
 * not deleted memory : falign
 */
      /*lp->falign)*/
/*
 * deeletd elements 
 */ 
      if (lp->vsflag)
         free(lp->vsflag);
      if (lp->vsoflag)
         free(lp->vsoflag);
      if (lp->vscal)
         free(lp->vscal);
      if (lp->varrs)
         free(lp->varrs);
      if (lp->nopf)
         free(lp->nopf);
      if (lp->aaligned)
         free(lp->aaligned);
      if (lp->abalign)
         free(lp->abalign);
      if (lp->outs)
         KillBitVec(lp->outs);
      if (lp->sets)
         KillBitVec(lp->sets);
      if (lp->blocks)
         KillBlockList(lp->blocks);
      if (lp->tails)
         KillBlockList(lp->tails);
      if (lp->blkvec)
         KillBitVec(lp->blkvec);
      if (lp->posttails)
         KillBlockList(lp->posttails);
      if (lp->vvscal)
         free(lp->vvscal);
      if (lp->bvvscal)
         free(lp->bvvscal);
      if (lp->vvinit)
         free(lp->vvinit);
      if (lp->pfarrs)
         free(lp->pfarrs);
      if (lp->pfdist)
         free(lp->pfdist);
      if (lp->pfflag)
         free(lp->pfflag);
      if (lp->ae)
      {
         for (N=lp->ae[0],i=0; i < N; i++)
            free(lp->aes[i]);
         free(lp->aes);
      }
      if (lp->ae)
         free(lp->ae);
      if (lp->ne)
         free(lp->ne);
      if (lp->se)
      {
         for (N=lp->se[0],i=0; i < N; i++)
            free(lp->ses[i]);
         free(lp->ses);
      }
      if (lp->se)
         free(lp->se);
      if (lp->nse)
         free(lp->nse);
      if (lp->seflag)
         free(lp->seflag);
      
      if (lp->maxvars)
         free(lp->maxvars);
      if (lp->minvars)
         free(lp->minvars);

      free(lp);
   }
   return(ln);
}
#endif

void KillAllLoopsComplete(void)
/*
 * it will kill loops with all of its' elements using KillLoopWhole
 */
{
   while (loopq) loopq = KillFullLoop(loopq);
}

void KillAllLoops(void)
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

void InvalidateLoopInfo(void)
/*
 * Retains optloop info that comes from front-end, throws rest away
 * NOTE: this function is used in different stages of execution, not just 
 * before restoring from saved stage. So, we may need to save computed 
 * information from steps to steps.... 
 */
{
   LOOPQ *lp;

   CFLOOP=0;
   if (optloop)
   {
/*
 *    Remove optloop from queue
 */
      if (loopq)
      {
         if (loopq == optloop)
            loopq = loopq->next;
         else
         {
            for (lp=loopq; lp->next && lp->next != optloop; lp = lp->next);
            if (lp->next == optloop)
               lp->next = optloop->next;
         }
         optloop->next = NULL;
      }
/*
 *    Keep optloop info that doesn't vary, throw rest away
 */
      lp = NewLoop(optloop->flag);
      lp->ndup = optloop->ndup;
      lp->I = optloop->I;
      lp->beg = optloop->beg;
      lp->end = optloop->end;
      lp->inc = optloop->inc;
      lp->body_label = optloop->body_label;
      lp->end_label = optloop->end_label;
      lp->maxunroll = optloop->maxunroll;
      lp->itermul = optloop->itermul;
      lp->LMU_flag = optloop->LMU_flag;   /* save the flag for loop markup */
      
      lp->fa2vlen = optloop->fa2vlen;     /* fptr calculated in vect*/
      lp->aaligned = optloop->aaligned;
      lp->abalign = optloop->abalign;
      lp->maaligned = optloop->maaligned;    /* mutually align for ... */
      lp->mbalign = optloop->mbalign;       /* mutually align for ... */
      lp->faalign = optloop->faalign;       /* need to be force aligned */
      lp->fbalign = optloop->fbalign;       /* need to be force aligned */
      
      lp->varrs = optloop->varrs;
      lp->vscal = optloop->vscal;
      lp->vsflag = optloop->vsflag;
      lp->vsoflag = optloop->vsoflag;
      lp->vvscal = optloop->vvscal;
      lp->bvvscal = optloop->bvvscal;
      lp->vvinit = optloop->vvinit;
      lp->vflag = optloop->vflag;
      lp->pfarrs = optloop->pfarrs;
      lp->pfdist = optloop->pfdist;
      lp->pfflag = optloop->pfflag;
      lp->ae = optloop->ae;
      lp->ne = optloop->ne;
      lp->aes = optloop->aes;
      lp->maxvars = optloop->maxvars;      /* keep track of max var */
      lp->minvars = optloop->minvars;      /* keep track of min var */ 
      lp->se = optloop->se;
      lp->nse = optloop->nse;
      lp->ses = optloop->ses;
      lp->seflag = optloop->seflag;
      lp->nopf = optloop->nopf;
      lp->CU_label = optloop->CU_label;
      lp->NE_label = optloop->NE_label;
      lp->PTCU_label = optloop->PTCU_label;
/*
 *    make the list of original optloop NULL so that killloop function
 *    won't delete/free the allolcated memory
 */
      optloop->vsflag = optloop->vsoflag = optloop->vscal = optloop->varrs = 
         optloop->ae = optloop->ne = optloop->nopf = NULL;
      optloop->bvvscal = NULL;
      optloop->vvinit = NULL;
      optloop->aes = NULL;
      optloop->aaligned = optloop->abalign = optloop->maaligned = 
         optloop->mbalign = optloop->faalign = optloop->fbalign = NULL;
      optloop->maxvars = NULL;     /* sothat killloop not freed prev space */
      optloop->minvars = NULL;     /* sothat killloop not freed prev space */
      optloop->se = optloop->nse = NULL;
      optloop->ses = NULL;
      optloop->seflag = NULL;
      KillLoop(optloop);
      optloop = lp;
   }
   KillAllLoops();
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
/*
 * Majedul: How it works:
 * Only the header block of a loop holds the loop information (blk->loop). For 
 * all other blocks, it is NULL. So, when traversing the blocks of a loop, if 
 * we find a block which is not the header of this loop but header of other 
 * loop, then there must be a loop inside this loop. So, in this case, the 
 * depth of this loop is increased. 
 */
   for (loop=loopq; loop; loop = loop->next)
   {
      /*for (lp=loopq->blocks; lp; lp = lp->next)*/ /* Fixed below */
      for (lp=loop->blocks; lp; lp = lp->next) /* lp is a blist here */
      {
         if (lp->blk != loop->header && lp->blk->loop)
         {
            dep = ++(lp->blk->loop->depth);
         }
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
      optloop->sets = optloop->next->sets;

      optloop->next->blkvec = optloop->next->outs = optloop->next->sets = 0;
      optloop->next->blocks = NULL;
      optloop->next->tails = optloop->next->posttails = NULL;
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
   INT_BVI phbv, iv;
   BLIST *bl;
   extern INT_BVI FKO_BVTMP;

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
#if 0
      BitVecInvert(phbv, lp->blkvec);
      i = BlockList2BitVec(lp->header->preds);
      BitVecComb(phbv, phbv, i, '&');
#else
      i = BlockList2BitVec(lp->header->preds);
      BitVecComb(phbv, i, lp->blkvec, '-');
#endif
      if (CountBitsSet(phbv) == 1)
      {
         i = GetSetBitX(phbv, 1)-1;  /* i now block number of preheader */
         lp->preheader = FindBlockByNumber(NULL, i+1);
         if ( (lp->preheader->usucc && lp->preheader->usucc != lp->header) ||
              (lp->preheader->csucc && lp->preheader->csucc != lp->header) )
            lp->preheader = NULL;
      }
/*
 *    Find the tail(s) of the loop.  Loop tails are preds of the header
 *    which are in the loop.
 */
      phbv = BlockList2BitVec(lp->header->preds);
      BitVecComb(phbv, phbv, lp->blkvec, '&');
      lp->tails = BitVec2BlockList(phbv);
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
   }
/*   prepostloops(); */
   maxdep = CalcLoopDepth();
   if (optloop)
   {
/*
 *    Majedul:
 *    loopq : the queue which keeps all the loops finding out from the CFG
 *    optloop: is the loop which is created at the parsing stage to keep track 
 *    the loop which is the candidate of most of the optimization.
 *    Here, optloop->next points the same optloop (matching the loop_body label)
 *    but found out from CFG. So, the assertion means:
 *    1. there must be an optloop found from the CFG which matches optloop.
 *    2. optloop must be the innermost loop.
 */
#if 0
      assert(optloop->next);
      assert(optloop->next->depth == maxdep);
#else
      if (optloop->next)
      {
         assert(optloop->next->depth == maxdep);
      }
      else /* no optloop anymore*/
      {
         KillFullLoop(optloop);
         optloop = NULL;
      }

#endif
   }
   SortLoops(maxdep);
   for (i=1, lp=loopq; lp; i++, lp = lp->next) lp->loopnum = i;
   CFLOOP = 1;
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

/* 
 * need comments here: initially CFLOOP==0, loopq has enrty already??
 * invalidate will throw away all but optloop info that comes from front-end
 */
   if (!CFLOOP || loopq)
      InvalidateLoopInfo();
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

/*
 * Need comments here: ??
 */
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

void ShowFlow(char *fout, BBLOCK *base)
{
   FILE *fpout;
   BBLOCK *bp;
   if (fout) 
      fpout = fopen(fout, "w");
   else
      fpout = stderr;
   fprintf(fpout, "digraph G{\n   node [shape=box]\n");
   for (bp=base; bp; bp = bp->down)
   {
#if 0
/*    This is to show conditional/unconditional jump block */
      if (bp->csucc)
         fprintf(fpout, "   block_%d -CS> block_%d\n", bp->bnum, bp->csucc->bnum);
      if (bp->usucc)
         fprintf(fpout, "   block_%d -US> block_%d\n", bp->bnum, bp->usucc->bnum);
#else
      if (bp->csucc)
         fprintf(fpout, "   block_%d -> block_%d\n", bp->bnum, bp->csucc->bnum);
      if (bp->usucc)
         fprintf(fpout, "   block_%d -> block_%d\n", bp->bnum, bp->usucc->bnum);
#endif
   }
   fprintf(fpout, "}\n");
   if (fout)
      fclose(fpout);
}

/*=============================================================================
 *                      RELATED TO PROGRAM FALL THROUGH
 *                      MAINLY DEALS WITH OPTLOOP PATHS
 *=========================================================================== */

static BLIST *FindFallThruPath(BLIST *scope, BBLOCK *head, BBLOCK *tail, 
                               BLIST *ftblks)
/*
 * checks only the fall-thru paths, returns NULL if there isn't any.
 * Only checks the path which consists of usucc.
 * NOTE: only used from this src file for internal purpose
 */
{
   if (head == tail)
   {
      ftblks = ReverseBlockList(AddBlockToList(ftblks, head));
      return ftblks;
   }
   ftblks = AddBlockToList(ftblks, head);
   if (head->usucc)
   {
      if (FindBlockInList(scope, head->usucc))
         ftblks = FindFallThruPath(scope, head->usucc, tail, ftblks);
   }
   else
   {
      fprintf(stderr,"NO FALL-THRU!! \n");
      if (ftblks) KillBlockList(ftblks);
      return NULL;
   }
   return ftblks;
}

BLIST *FindFallThruPathBlks(BLIST *scope, BBLOCK *head, BLIST *tails)
{
   BLIST *ftblks; 
   ftblks = NULL;
/*
 * consider one tail right now.
 */
   assert(tails->blk && !tails->next);
   ftblks= FindFallThruPath(scope, head, tails->blk, ftblks);
/*
 * checking whether code follows the fall-tru requence! 
 */
#if 0
   fprintf(stderr, "head=%d, tail=%d \n", head->bnum, tails->blk->bnum);
   fprintf(stderr, "Fall Through Path blks = %s\n", PrintBlockList(ftblks));

   bl = ftblks;
   while(bl && bl->next)
   {
      if (bl->blk->down != bl->next->blk)
      {
         fprintf(stderr,"CODE DOESN'T FOLLOW FALL-THRU!!!\n");
         break;
      }
      else
      {
         if (bl->next->blk == tails->blk)
         {
            fprintf(stderr, "CODE FOLLOWS THE FALL-THRU BLKS\n");
            break;
         }
         else bl = bl->next;
      }
   }
#endif
/*
 * Returns the fall thru blklist 
 */
   return (ftblks);
}

static BLIST **FindAllPathsBlks(int *np, BLIST *scope, BBLOCK *head, 
                                BBLOCK *tail, BLIST **paths, BLIST *bst)
/*
 * Returns the path (contails blist for each path) and store path count on np.
 */
{
   int i, n;
   BLIST **new;
   extern int FKO_MaxPaths;
/*
 * found tail, that completes a path. So, create new paths updated with new 
 * blocklist for that path. path[0] contains the fall-thru path
 */
   if (head == tail)
   {
      bst = AddBlockToList(bst, head);
      n = ++(*np);
      new = malloc(sizeof(BLIST*) * (n));
      assert(new);
      for (i=0; i < (n-1); i++)
      {
         new[i] = paths[i];
      }
      new[n-1] = NewReverseBlockList(bst);
#if 0
      fprintf(stderr, "path%d: %s\n",n, PrintBlockList(new[n-1]));
#endif
      if (paths) free(paths);
      paths = new;
/*
 *    bst works as a stack. so, delete the top most to avoid mem leak
 */
      free(bst);
      return paths;
   }
   bst = AddBlockToList(bst, head);
/*
 * limit path search if threshold is mentioned
 */
   if (!FKO_MaxPaths || (*np) < FKO_MaxPaths)  
   {
/*
 *    NOTE: traversing usucc first provides fall-thru path as the first path
 */
      if (head->usucc)
      {
         if (FindBlockInList(scope, head->usucc))
         {
            paths = FindAllPathsBlks(np,scope,head->usucc,tail, paths,bst);
         } 
      }
      if (head->csucc)
      {
         if (FindBlockInList(scope, head->csucc))
         {
            paths = FindAllPathsBlks(np,scope,head->csucc,tail, paths,bst);
         }
      }
   }
   free(bst);
   return paths;
}

void KillAllList(int n, BLIST** bls)
/*
 * provided the number of blist, kill all and free mem
 */
{
   int i;
   for (i=0; i < n; i++)
      KillBlockList(bls[i]);
   free(bls);
}

void SwapSuccessor(BBLOCK *blk, BBLOCK *csucc, BBLOCK *usucc)
/*
 * changes the conditional jump and the csucc and usucc without changing
 * the block structure. After this conversion, the cfg becomes messed-up
 */
{
   int k;
   INSTQ *ip;
   enum inst prebr[] = {JEQ, JNE, JLT, JLE, JGT, JGE};
   enum inst abr[]   = {JNE, JEQ, JGE, JGT, JLE, JLT};
   const int nbr = 6;
   static short lid = 0;
   short label;
   char ln[256]; /* temp as ST copied the str */

   for (ip=blk->ainst1; ip; ip=ip->next)
   {
      if(IS_COND_BRANCH(ip->inst[0]))
      {
/*
 *       add lebel at the first of previous usucc
 */
         if (usucc->ainst1->inst[0] != LABEL)
         {
            sprintf(ln, "_FKO_XCHNG_SUCC_%d", ++lid);
            usucc->ilab = label = STlabellookup(ln);
            InsNewInst(usucc, NULL, usucc->ainst1, LABEL, label, 0, 0);
         }
         else
         {
            label = usucc->ainst1->inst[1];
         }

/*
 *       chnages for the condition of the blk
 */
         for (k=0; k < nbr; k++)
            if (prebr[k] == ip->inst[0])
               break;
         assert(k != nbr);
         ip->inst[0] = abr[k]; 
         ip->inst[3] = label;
/*
 *       Now, time to change the csucc/usucc ptr which would invalidate the 
 *       CFG of bbbase
 */
         blk->csucc = usucc;
         blk->usucc = csucc;
      }
   }
}

static void InsertJmpAtEndWithLabel(BBLOCK *blk, short label)
/*
 * Add JMP to the label at the end, or update the label if exists.
 */
{
   INSTQ *ip;
   ip = blk->ainstN;
   if (ip && ip->inst[0] == JMP)
   {
      ip->inst[2] = label;
   }
   else
   {
      InsNewInst(blk, ip, NULL, JMP, -PCREG, label, 0);
   }
}
static BBLOCK *NewFallThruBasicBlocks(int *count, BBLOCK *head, BBLOCK *tail, 
                                   BBLOCK *end0)
/*
 * considering only loop blocks and single head and tail of loop.
 * will consider the multiple tails later.
 * Returns the end marker blk, end->down need to be adjusted  if there is an 
 * other flow for cleanup or loop peeling
 * NOTE: this algorithm is not costly, acts like DFS
 */
{
   extern BBLOCK *bbbase;

   static BBLOCK *end = NULL;
/*
 * count 0 indicates the 1st function call. so, initiate the start and end block
 */
   if (*count == 0)
   {
      end = end0;
      *count = 1; 
   }
#if 0
   fprintf(stderr, "Head = %d\n", head->bnum);
#endif   
/*
 * one path is complete.
 */
   if (head == tail)
   {
#if 0      
      fprintf(stderr, "One path is complete! return from %d\n",head->bnum);
#endif
      return end;
   }
/*
 * has an usucc, add this block as down if not added yet
 */
   if (head->usucc)
   {
/*
 *    usucc not yet explored, so add it below the head
 */
      if (!head->down && !head->usucc->up)
      {
         head->down = head->usucc; /* head->usucc->down is NULL */
         head->usucc->up = head;
/*
 *       delete any unconditional jmp for the fall thru
 */
         if (head->ainstN && head->ainstN->inst[0] == JMP)
            DelInst(head->ainstN);
/*
 *       Need to move the end marker if head is previous end.
 */
         if (head == end)
            end = head->usucc;
#if 0
         fprintf(stderr, "usucc %d is placed\n", head->usucc->bnum);
#endif         
         NewFallThruBasicBlocks(count, head->usucc, tail, end0);
      }
/*
 *    usucc already explored, so add jmp at the end of head
 */
      else if (!head->down && head->usucc->up)
      {
         InsertJmpAtEndWithLabel(head, head->usucc->ilab);
#if 0
         fprintf(stderr, "usucc %d is already placed before\n", 
                 head->usucc->bnum);
#endif         
      }
/*
 *    if head already placed, nothing to do
 *    How is it possible? No way!! A blk can't have 2 usucc!!! Check the CFG!
 */
      else 
      {
         if (head->down)
         {
#if 1           
            fprintf(stderr, "BLK-%d: Already placed with a down!\n",head->bnum);
            fprintf(stderr, "usucc-%d: yet to place!!!\n", head->usucc->bnum);
            assert(0);
#endif
         }
      }
   }
/*
 * has a csucc, add this at the end point
 */
   if (head->csucc)
   {
/*
 *    head is placed but csucc is not placed yet, add it at the end point
 *    NOTE: If there is a csucc, there must be a usucc. head must be placed
 *    before exploring it's csucc!
 */
      if (head->down && !head->csucc->up)
      {
         head->csucc->up = end; /* head->csucc->down is NULL*/
         end->down = head->csucc; /* end->down is lost!!! */
         end = head->csucc;
#if 0
         fprintf(stderr, "csucc %d is placed\n", head->csucc->bnum);
#endif         
         NewFallThruBasicBlocks(count, head->csucc, tail, end0);
      }
      else if (head->csucc->up)
      {
#if 0         
            fprintf(stderr, "BLK-%d: Already placed this csucc!\n", 
                    head->csucc->bnum);
#endif            
      }
/*
 *    !head->csucc->up && !head->down
 *    head->down is NULL, uplink ensures whether a blk is placed or not.
 *    Head should be placed before placing its successor. 
 *    head->usucc is placed before head through other path.
 *    So, place the csucc after the head and point the end to it. 
 */
      else
      {
         assert(head->up && !head->down);
#if 0
         fprintf(stderr, "head=%d, head->csucc= %d\n", head->bnum, 
                 head->csucc->bnum);
#endif
         head->down = head->csucc; /* head->usucc is already placed above */
         head->csucc->up = head;
         assert(end == head);
         end = head->csucc;
         NewFallThruBasicBlocks(count, head->csucc, tail, end0);
      }
   }
#if 0
   fprintf(stderr, "Exiting blk : %d, now end : %d\n", head->bnum, end->bnum);
#endif
   return end;
}

void RemakeWithNewFallThru(LOOPQ *lp)
/*
 * this function will change fall-thru only for the optloop 
 */
{
   int count;
   BLIST *bl;
   BBLOCK *bp;
   BBLOCK *end, *endnext; /* end: RET blk, endnext: other flow [if cleanup] */
   extern BBLOCK *bbbase;
   /*BLIST *ftblks, *allblks;*/
/*
 * Assume single tail for loop now
 */   
   assert(lp->tails && !lp->tails->next);
#if 0
   ftblks = FindFallThruPathBlks(lp->blocks, lp->header, lp->tails);
   fprintf(stderr, "New Fall-thru: %s\n", PrintBlockList(ftblks));
   KillBlockList(ftblks);
#endif
#if 0   
   PrintInst(stderr, bbbase);
   fflush(stderr);
   exit(0);
#endif     
#if 0
   allblks = NULL;
   for (bp=bbbase; bp; bp=bp->down)
      allblks = AddBlockToList(allblks, bp);
   allblks = ReverseBlockList(allblks);
#endif   

/*
 * find the block where the ret instruction belongs
 */
   for (bp=bbbase; bp; bp=bp->down)
   {
#if 0      
      for (ip=bp->ainst1; ip; ip=ip->next)
      {
         if (ip->inst[0] == RET)
            break;
      }
#else
      if (bp->ainstN->inst[0] == RET)
         break;
#endif
   }
   assert(bp);
   end = bp;
/*
 * find out the next of end until it is not a loop blk
 *
 */
   for (endnext=end->down; endnext && FindBlockInList(lp->blocks, endnext) ; 
        endnext=endnext->down);

/*
 * disconnect edges of loop blks except the head->up and tail->down.
 * Need isolates the blks totally from the code flow!
 */
   for (bl=lp->blocks; bl; bl=bl->next)
   {
      if (bl->blk == lp->header)
      {
         if(bl->blk->down) bl->blk->down->up = NULL; 
         bl->blk->down = NULL;
      }
      else if (bl->blk == lp->tails->blk)
      {
         if (bl->blk->up) bl->blk->up->down = NULL;
         bl->blk->up = NULL;
      }
      else
      {
/*
 *       Need to bypass the blk first, then invalidate the link
 */
         if(bl->blk->down) bl->blk->down->up = bl->blk->up;
         if(bl->blk->up) bl->blk->up->down = bl->blk->down;
         bl->blk->up = NULL;
         bl->blk->down = NULL;
      }
   }

#if 0 
   fprintf(stderr, "All Blk list: \n");
   for (bl=allblks; bl; bl=bl->next)
   {
      fprintf(stderr, "b%d: up = %d, down = %d\n", bl->blk->bnum, 
              bl->blk->up?bl->blk->up->bnum:-1, 
              bl->blk->down?bl->blk->down->bnum:-1);
   }
#endif
/*
 * Rearrange bbbase structure based on 
 * Returns the final end marker block, add the existing flow at the down of
 * end
 */
   count = 0;
   end = NewFallThruBasicBlocks(&count, lp->header, lp->tails->blk, end);
   end->down = endnext;
/*
 * checking ... ...
 */
#if 0
   fprintf(stderr, "All Blk list After Transformation : \n");
   for (bl=allblks; bl; bl=bl->next)
   {
      fprintf(stderr, "b%d: up = %d, down = %d\n", bl->blk->bnum, 
              bl->blk->up?bl->blk->up->bnum:-1, 
              bl->blk->down?bl->blk->down->bnum:-1);
   }
   KillBlockList(allblks);
#endif   
}

void TransformFallThruPath(int path)
/*
 * this function takes a path and re-structure the code to make it fall-thru
 * (it will re-structure in its own way even path is already a fall-thru)
 * NOTE: This function will work for few number of branches, can't be applied
 * after the SB of SV. cost of finding all paths would be 2^n 
 */
{
   int np;
   LOOPQ *lp;
   BLIST **paths;
   BLIST *bl, *blInvExpBlks;
   extern LOOPQ *optloop;
   
   paths = NULL;
   bl = NULL;
   blInvExpBlks = NULL;
   lp = optloop;
   np = 0;
/*
 * assumption: single tail so far!
 */
   assert(lp->tails && !lp->tails->next);
/*
 * create path list with blist for each path updating np as the path count
 * NOTE: finding all paths would be costly, specially when we call this 
 * after SB.
 */
   paths = FindAllPathsBlks(&np, lp->blocks, lp->header, lp->tails->blk, 
                            paths, bl);
#if 0   
   fprintf(stderr, "Paths=%d\n",np);
   for (i=0; i < np; i++)
   {
      fprintf(stderr,"Path%d : %s\n", i, PrintBlockList(paths[i]));
   }
#endif
/*
 * checking for the valid argument
 */
   if (path == 1)
   {
#if 0
      fprintf(stderr, "PATH1 IS ALREADY A FALL-THRU. STILL APPLYING ... ...\n");
#endif
/*
 *    Do we still need to check that!!!!
 *    Still we can apply fall-true transformation to re-structure
 */
      RemakeWithNewFallThru(lp);
      KillAllList(np, paths);
      return;
   }
   if (path > np)
   {
      fprintf(stderr, "INVALID PATH TO MAKE FALL-THRU !!!\n");
      KillAllList(np, paths);
      assert(path <= np);
      return;
   }
   
/*
 * NOTE: As we have 2 paths for most of the kernel, here I experimentally
 * test to swap the fall thru path from 0 to 1. We will parameterize it later.
 * HERE HERE, applying this after SB, there are lots of paths!!!!
 */
/*
 * Figure out the blks where exp need to change
 */
   for (bl = paths[path-1]; bl; bl=bl->next)
   {
      if (bl->blk == lp->tails->blk) break;
      if (bl->blk->csucc && (bl->blk->csucc == bl->next->blk))
         blInvExpBlks = AddBlockToList(blInvExpBlks, bl->blk);
   }
#if 0
   fprintf(stderr, "Blk need to update: %s\n", PrintBlockList(blInvExpBlks));
#endif   
/*
 * Change the cond to swap csucc and usucc
 */
   for (bl = blInvExpBlks; bl; bl=bl->next)
      SwapSuccessor(bl->blk, bl->blk->csucc, bl->blk->usucc);
   KillBlockList(blInvExpBlks);
/*
 * Re-structure the basic block according to updated information
 */
   RemakeWithNewFallThru(lp);
   KillAllList(np, paths);
   return;
}

void ReshapeFallThruCode()
/*
 * this function will reshape the code with the existing fall-thru
 */
{
   LOOPQ *lp;

   lp = optloop;
   assert(lp->tails && !lp->tails->next);
/*
 * NOTE: should apply only when there is multiple paths
 */
   RemakeWithNewFallThru(lp);
}

void PrintLoopPaths()
{
   int i, np;
   BLIST **paths;
   BLIST *bl;
   LOOPQ *lp;
   extern LOOPQ *optloop;

   np = 0;
   paths = NULL;
   bl = NULL;
   lp = optloop;

   assert(lp->tails && !lp->tails->next);

   paths = FindAllPathsBlks(&np, lp->blocks, lp->header, lp->tails->blk, 
                            paths, bl);
   assert(paths);
#if 1   
   fprintf(stderr, "**************************\n");
   fprintf(stderr, "Paths=%d\n",np);
   for (i=0; i < np; i++)
   {
      fprintf(stderr,"Path%d : %s\n", i, PrintBlockList(paths[i]));
   }
   fprintf(stderr, "**************************\n");
#endif
   KillAllList(np, paths);
}

void PrintFallThruLoopPath(LOOPQ *lp)
{
   BLIST *ftblks; 
   ftblks = FindFallThruPathBlks(lp->blocks, lp->header, lp->tails);
   fprintf(stderr, "FALL-THRU LOOP PATH: %s\n", PrintBlockList(ftblks));
   KillBlockList(ftblks);
   return;
}

/*=============================================================================
 *
 * Loop unswitching .....
 *
 *============================================================================*/


int IsAnyTail(LPLIST *l0, BBLOCK *blk)
{
   LPLIST *ll;
   for (ll=l0; ll; ll=ll->next)
   {
      if (FindBlockInList(ll->loop->tails, blk))
         return(1);
   }
   return(0); 
}

int IsVarLoopsInvariant(LPLIST *l0, short var)
{
   INSTQ *ip;
   LPLIST *ll;
   BLIST *bl;


   for (ll=l0; ll; ll=ll->next)
   {
      for (bl=ll->loop->blocks; bl; bl=bl->next)
      {
         for (ip=bl->blk->ainst1; ip; ip=ip->next)
         {
            if (  (IS_STORE(ip->inst[0]) || ip->inst[0] == NEG ||
                     ip->inst[0] == NEGS || ip->inst[0] == CVTSI)
                  && (ip->inst[1] == SToff[var-1].sa[2]) )
               return(0);
         }
      }
   }
   return(1);
}

int IsBlkBranchLoopInvariant(LPLIST *l0, BBLOCK *blk)
{
   int i;
   short reg0, reg1;
   short var0, var1;
   short op;
   INSTQ *ip, *ip0;
   
   ip = blk->ainstN;
   assert(IS_COND_BRANCH(ip->inst[0]));
   
   reg0 = reg1 = 0;
   var0 = var1 = 0;
   /*if (ip->prev->inst[0] == FCMP 
         || ip->prev->inst[0] == FCMPD 
         || ip->prev->inst[0] == CMP)*/
   ip=ip->prev;
/*
 * reg0 / var0
 */
   if (ip->inst[2] < 0)
      reg0 = ip->inst[2];
   else if (ip->inst[2])
   {
      if (!IS_CONST(STflag[ip->inst[2]-1]))
         var0 = STpts2[ip->inst[2]-1];
   }
/*
 * reg1 / var1
 */
   if (ip->inst[3] < 0)
      reg1 = ip->inst[3];
   else if (ip->inst[3])
   {
      if (!IS_CONST(STflag[ip->inst[3]-1]))
         var1 = STpts2[ip->inst[3]-1];
   }
/*
 * findout the vars
 */
   if (reg0)
   {
      if (ip->prev && IS_LOAD(ip->prev->inst[0]) && ip->prev->inst[1] == reg0)
         var0 = STpts2[ip->prev->inst[2]-1];
      else if (ip->prev->prev && IS_LOAD(ip->prev->prev->inst[0]) 
            && ip->prev->prev->inst[1] == reg0)
         var0 = STpts2[ip->prev->prev->inst[2]-1];
      else 
         fko_error(__LINE__, "found no var using reg0!");
   }
   
   if (reg1)
   {
      if (ip->prev && IS_LOAD(ip->prev->inst[0]) && ip->prev->inst[1] == reg0)
         var1 = STpts2[ip->prev->inst[2]-1];
      else if (ip->prev->prev && IS_LOAD(ip->prev->prev->inst[0]) 
            && ip->prev->prev->inst[1] == reg0)
         var1 = STpts2[ip->prev->prev->inst[2]-1];
      else 
         fko_error(__LINE__, "found no var using reg0!");
   }

   if (var0)
   { 
      if (IsVarLoopsInvariant(l0, var0))
      {
         /*fprintf(stderr, "var0=%s loop invariant!!!\n", STname[var0-1]);*/
         fko_warn(__LINE__, "var0=%s loop invariant!!!\n", STname[var0-1]);
      }
      else
         return(0);
   }
   
   if (var1)
   {
      if (IsVarLoopsInvariant(l0, var1))
      {
         /*fprintf(stderr, "var1=%s loop invariant!!!\n", STname[var1-1]);*/
         fko_warn(__LINE__, "var1=%s loop invariant!!!\n", STname[var1-1]);
      }
      else
         return(0);
   }
   return(1);
}

int IsLoopUnswitchable(LPLIST *ll)
{
   int suc, isopt;
   BBLOCK *bp, *head1, *tailn;
   BLIST *bl;
   extern LOOPQ *optloop;

#if 0
   fprintf(stderr, "loop-nest: ");
   for (l=ll; l; l=l->next)
   {
      fprintf(stderr, "%s ",STname[l->loop->body_label-1]);
   }
   exit(0);
#endif
   suc = 0;
/*
 * check whether if have only loop invariant conditional branches (except back 
 * edge of the loops)
 */
   for (bl=ll->loop->blocks; bl; bl=bl->next)
   {
      if (IsAnyTail(ll, bl->blk))
         continue; /* skip tail to skip the back edge*/
      if (bl->blk->csucc)
      {
#if 0
         PrintThisBlockInst(stderr, bl->blk);
#endif
         suc = 1;
         if (!IsBlkBranchLoopInvariant(ll, bl->blk))
            return(0);
      }
   }
/*
 * fall-thru path must contain the optloop... it will make our life easy to 
 * implement the loop unswitching
 */
   isopt = 0;
   bp = ll->loop->header;
   while(bp)
   {
      if (FindBlockInList(optloop->blocks, bp))
      {
         isopt = 1;
         break;
      }

      if (FindBlockInList(ll->loop->tails, bp))
         break;
      
      if (!FindBlockInList(ll->loop->blocks, bp))
            break;
      
      bp = bp->usucc;
   }
   if (!isopt)
      suc = 0;

   return(suc);
}

INSTQ *RemoveBlkCondBranch(BBLOCK *blk)
{
   INSTQ *ip, *ip0;
   INSTQ *iprm;

   ip = blk->ainstN;
   assert(IS_COND_BRANCH(ip->inst[0]));

   ip0 = ip->prev; //cmp, neg, negs, cvtsi 
   iprm = NewInst(NULL, NULL, NULL, ip->inst[0], ip->inst[1], ip->inst[2], 
                   ip->inst[3]);
   DelInst(ip);    // cond jump

   ip = ip0;      
   ip0 = ip->prev; // load..
   iprm = NewInst(NULL, NULL, iprm, ip->inst[0], ip->inst[1], ip->inst[2], 
                   ip->inst[3]);
   DelInst(ip);
   ip = ip0;

   while(IS_LOAD(ip->inst[0]))
   {
      ip0=ip->prev;
      iprm = NewInst(NULL, NULL, iprm, ip->inst[0], ip->inst[1], ip->inst[2], 
                      ip->inst[3]);
      DelInst(ip);
      ip = ip0;
   }
   return(iprm);
}

void LoopUnswitch(LPLIST *ll)
/*
 * NOTE: to simplify the implementation, we first consider, all the branches
 * (except back edge for loop) are loop invariant. So, we can move them all 
 * together
 */
{
   INSTQ *ip, *ip0;
   ILIST *il, *ilb;
   LOOPQ *lp;
   BLIST *bl, *dupblks, *ftheads;
   BBLOCK *bp, *bp0;
   BBLOCK *head, *tail, *newCF;
   INT_BVI iv, ivtails;
   short nlab=0, labs[4];
   short lbb, lba;
   extern BBLOCK *bbbase;
   extern INT_BVI FKO_BVTMP;

#if 0
   fprintf(stderr, "let's unswitch the loop\n");
   ShowFlow("cfg.dot", bbbase);
#endif
/*
 * step 0: duplicate the loopnest 
 * FIXME: need to duplicate upto FKO_EPILOGUE
 */
   lp = ll->loop;
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
   SetVecBit(iv, lp->header->bnum-1, 0);
   newCF = DupCFScope(lp->blkvec, iv, lp->header);
   assert(newCF->ilab);
   
   iv = BitVecCopy(iv, lp->blkvec);
   dupblks = CF2BlockList(NULL, iv, newCF);
   
   SetVecAll(iv, 0);
   ivtails = BlockList2BitVec(lp->tails);
   ftheads =FindAllFallHeads(NULL, lp->blkvec, lp->header, ivtails, iv);
   ftheads = ReverseBlockList(ftheads);

#if 0
   fprintf(stderr, "dupblks = ");
   for(bl=dupblks; bl; bl=bl->next)
      fprintf(stderr, "%d ", bl->blk->bnum);
   fprintf(stderr, "\n");
   
   fprintf(stderr, "fall-thru = ");
   for(bl=ftheads; bl; bl=bl->next)
      fprintf(stderr, "%d ", bl->blk->bnum);
   fprintf(stderr, "\n");
#endif
/*
 * step 1: remake cfg to generate expected fall-thru, which include one path of
 * optloop
 * NOTE: skipped that... we assume optloop is in fall-thru path.. we tested it 
 * in IsLoopUnswitchable() 
 */
/*
 * step 2: assuming the fall-thru path is correct, explore and fixed the
 * correct path
 * FIXME: we assume single head and tail for outter-most loop for now. many code
 * may have multiple tails.. need to have some common block to work with this 
 * approach
 */
   
#if 0
   head = ll->loop->header;
   assert(!ll->loop->tails->next);
   tail = ll->loop->tails->blk;
   bp = head;
   while(bp)
   {
      fprintf(stderr, "%d->", bp->bnum);
      if (bp == tail)
         break;
      bp = bp->usucc;
   }
   fprintf(stderr, "\n");
#endif
  
   ilb = NULL;
   lp = ll->loop;
   for (bl=lp->blocks; bl; bl=bl->next)
   {
      if (!IsAnyTail(ll, bl->blk) && bl->blk->csucc)
      {
         ip = RemoveBlkCondBranch(bl->blk);
         /*PrintThisInstQ(stderr, ip);*/
         ilb = NewIlist(ip, ilb);
/*
 *       since we will keep only one path at the end and assuming we have to 
 *       enter header of loop first, it is save to delete the label from the 
 *       conditional successor
 */
         DelInst(bl->blk->csucc->ainst1);
      }
   }

#if 0
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__,__LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
   ShowFlow("cfg2.dot", bbbase);
#endif
/*
 * where to place the conditions... placing at the end of prehead
 * BUT we need single prehead.. hopefully it will be created at repeatable opt
 */
   lp = ll->loop;
   bp = lp->preheader;
   assert(bp);

   lbb = STlabellookup("_FKO_LOOP_UNSWITCH_P2");

   ip0 = bp->ainstN;
   for (il=ilb; il; il=il->next)
   {
      for (ip=il->inst; ip; ip=ip->next)
      {
         if (IS_COND_BRANCH(ip->inst[0]))
            ip->inst[3] = lbb;
         ip0 = InsNewInst(bp, ip0, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                          ip->inst[3]);
      }
   }

/*
 * adding the duplicated blocks
 */
   for (bp0=bbbase; bp0->down; bp0=bp0->down);
   bp = NewBasicBlock(bp0, NULL);
   bp0->down = bp;
   bp->up = bp0;
   bp0 = bp;
   
   ip = InsNewInst(bp, NULL, NULL, LABEL, lbb, 0, 0);
   
   for (bl=ftheads; bl; bl=bl->next)
   {
      for (bp=bl->blk; bp; bp=bp->down)
      {
         if (!BitVecCheck(lp->blkvec, bp->bnum-1))
            break;
         bp0->down = FindBlockInListByNumber(dupblks, bp->bnum);
         bp0->down->up = bp0;
         bp0 = bp0->down;
         if (BitVecCheck(ivtails, bp->bnum-1))
            break;
         if (bp->usucc != bp->down && bp->csucc != bp->down)
            break;

      }
      bp0->down = NULL;
   }
   bp = NewBasicBlock(bp0, NULL);
   bp0->down = bp;
#if 0
   lba = STlabellookup("_IFKO_EPILOGUE");
#else
   assert(lp->posttails && !lp->posttails->next);
   assert(lp->posttails->blk->ilab);
   lba = lp->posttails->blk->ilab;
#endif
   ip = InsNewInst(bp, NULL, NULL, JMP, -PCREG, lba, 0);

#if 0
   fprintf(stdout, "inst");
   PrintInst(stdout, bbbase);
   exit(0);
#endif


#if 0
   nlab=2;
   labs[0] = STlabellookup(rout_name);
   labs[1] = STlabellookup("_IFKO_EPILOGUE");
   DoUselessLabelElim(nlab, labs);
   ShowFlow("cfg3.dot", bbbase);
#endif
/*
 * to mark prehead and posttail
 */
   InsNewInst(lp->preheader, lp->preheader->ainstN, NULL, LABEL, 
              STlabellookup("_FKO_UNSWITCH_PH"), 0, 0);
   assert(lp->posttails && !lp->posttails->next);
   InsNewInst(lp->posttails->blk, NULL, lp->posttails->blk->ainst1, LABEL, 
              STlabellookup("_FKO_UNSWITCH_PT"), 0, 0);

#if 0
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__,__LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
   ShowFlow("cfg4.dot", bbbase);
#endif
/*
 * del temporaries
 */
   KillAllIlist(ilb);
   KillBlockList(dupblks);
   KillBlockList(ftheads);
}

