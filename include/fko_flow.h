#ifndef FKO_FLOW_H
#define FKO_FLOW_H
#include "ifko.h"

BLIST *NewBlockList(BBLOCK *blk, BLIST *next);
void KillAllBlockList(BLIST *lp);
BBLOCK *NewBasicBlock(BBLOCK *up, BBLOCK *down);
void KillAllBasicBlocks(BBLOCK *base);
BBLOCK *FindBasicBlocks(BBLOCK *base0);
void AddBlockComments(BBLOCK *bp);
#if IFKO_DEBUG_LEVEL > 0
   void CheckFlow(BBLOCK *bbase, char *file, int line);
#else
   #define CheckFlow(arg1_, arg2_, arg3)
#endif

#endif
