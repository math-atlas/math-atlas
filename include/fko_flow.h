#ifndef FKO_FLOW_H
#define FKO_FLOW_H
#include "ifko.h"

BLIST *NewBlockList(BBLOCK *blk, BLIST *next);
BLIST *AddBlockToList(BLIST *list, BBLOCK *blk);
static BLIST *KillBlockListEntry(BLIST *lp);
BLIST *RemoveBlockFromList(BLIST *list, BBLOCK *blk);
int BlockList2BitVec(BLIST *lp);
BLIST *BitVec2BlockList(int iv);
BLIST *FindInList(BLIST *lp, BBLOCK *blk);
BBLOCK *FindBlockInList(BLIST *lp, BBLOCK *blk);
BBLOCK *FindBlockInListByNumber(BLIST *lp, ushort bnum);
BLIST *MergeBlockLists(BLIST *l1, BLIST *l2);
void KillBlockList(BLIST *lp);
BLIST *ReverseBlockList(BLIST *list);
BLIST *MergeBlockLists(BLIST *l1, BLIST *l2);
BLIST *FindBlockListWithLabel(BLIST *list, int ilab);

BBLOCK *NewBasicBlock(BBLOCK *up, BBLOCK *down);
void KillAllBasicBlocks(BBLOCK *base);
BBLOCK *FindBasicBlocks(BBLOCK *base0);
void AddBlockComments(BBLOCK *bp);
void SetBlocksActiveInst(BBLOCK *bp);
BBLOCK *NewBasicBlocks(BBLOCK *base0);
BBLOCK *FindBlockByNumber(BBLOCK *bp, const short bnum);
BBLOCK *FindBlockWithLabel(BBLOCK *bp, int ilab);

#if IFKO_DEBUG_LEVEL > 0
   void CheckFlow(BBLOCK *bbase, char *file, int line);
#else
   #define CheckFlow(arg1_, arg2_, arg3)
#endif

#endif
