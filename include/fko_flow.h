#ifndef FKO_FLOW_H
#define FKO_FLOW_H

BLIST *NewBlockList(BBLOCK *blk);
void KillAllBlockList(BLIST *lp);
BBLOCK *NewBasicBlock(BBLOCK *up, BBLOCK *down);
void KillAllBasicBlocks(BBLOCK *base);
BBLOCK *FindBasicBlocks(BBLOCK *base0);
void AddBlockComments(BBLOCK *bp);

#endif
