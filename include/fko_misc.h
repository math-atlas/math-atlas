#ifndef FKO_MISC_H
#define FKO_MISC_H
#include "fko_inst.h"

INSTQ *PrintComment(BBLOCK *blk, INSTQ *prev, INSTQ *next, ...);
INSTQ *PrintMajorComment(BBLOCK *blk, INSTQ *prev, INSTQ *next, ...);
void fko_error(int errno, ...);
void fko_warn(int errno, ...);
int const2shift(int c);
struct loopq *NewLoop(int flag);
struct locinit *NewLocinit(short id, short con, struct locinit *next);
void KillAllLocinit(struct locinit *libase);
void *NewPtrTable(int *n, void *old, int chunk);
short *NewShortTable(int *n, short *old, int chunk);

#endif
