#ifndef FKO_FLOW_H
#define FKO_FLOW_H
#include "fko.h"

BLIST *NewBlockList(BBLOCK *blk, BLIST *next);
BLIST *AddBlockToList(BLIST *list, BBLOCK *blk);
BLIST *RemoveBlockFromList(BLIST *list, BBLOCK *blk);
BLIST *RemoveBlksFromList(BLIST *l1, BLIST *l2); /* delete l2 from l1 */
INT_BVI BlockList2BitVec(BLIST *lp);
INT_BVI BlockList2BitVecFlagged(BLIST *lp, int init);
BLIST *BitVec2BlockList(INT_BVI iv);
BLIST *FindInList(BLIST *lp, BBLOCK *blk);
BBLOCK *FindBlockInList(BLIST *lp, BBLOCK *blk);
BBLOCK *FindBlockInListByNumber(BLIST *lp, ushort bnum);
BLIST *MergeBlockLists(BLIST *l1, BLIST *l2);
void KillBlockList(BLIST *lp);
LOOPQ *KillLoop(LOOPQ *lp);
LOOPQ *KillFullLoop(LOOPQ *lp);
void KillAllLoops();
BLIST *ReverseBlockList(BLIST *list);
BLIST *NewReverseBlockList(BLIST *list);
BLIST *CopyBlockList(BLIST *list);
BLIST *MergeBlockLists(BLIST *l1, BLIST *l2);
BLIST *FindBlockListWithLabel(BLIST *list, int ilab);
BLIST *GetGlobalScope();
BBLOCK *NewBasicBlock(BBLOCK *up, BBLOCK *down);
void KillAllBasicBlocks(BBLOCK *base);
BBLOCK *FindBasicBlocks(BBLOCK *base0);
void AddBlockComments(BBLOCK *bp);
void SetBlocksActiveInst(BBLOCK *bp);
BBLOCK *NewBasicBlocks(BBLOCK *base0);
BBLOCK *FindBlockByNumber(BBLOCK *bp, const short bnum);
BBLOCK *FindBlockWithLabel(BBLOCK *bp, int ilab);
char *PrintBlockList(BLIST *lp);
BBLOCK *DelBlock(BBLOCK *delblk);
void InvalidateLoopInfo(void);
void FindLoops();
void TransformFallThruPath(int path);
void ReshapeFallThruCode();
BBLOCK *KillBlock(BBLOCK *base, BBLOCK *killme);
#if IFKO_DEBUG_LEVEL > 0
   void CheckFlow(BBLOCK *bbase, char *file, int line);
#else
   #define CheckFlow(arg1_, arg2_, arg3)
#endif
int IsLoopUnswitchable(LPLIST *ll);
void LoopUnswitch(LPLIST *ll);

#endif
