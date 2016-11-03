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
ILIST *NewIlist(INSTQ *inst, ILIST *next);
ILIST *NewIlistAtEnd(INSTQ *inst, ILIST *top); 
ILIST *NewIlistInBetween(INSTQ *inst, ILIST *prev, ILIST *next);
ILIST *KillIlist(ILIST *ip);
void KillAllIlist(ILIST *ilist);
struct ptrinfo *FindPtrinfo(struct ptrinfo *base, short ptr);
void KillAllPtrinfo(struct ptrinfo *base);
struct ptrinfo *NewPtrinfo(short ptr, short flag, struct ptrinfo *next);
struct locinit *NewLI(short id, short con, struct locinit *next);
void KillAllLI(struct locinit *die);
int FindInShortList(int n, short *sp, short val);
int AddToShortList(int n, short *sp, short val);
LPLIST *NewLPlist(LPLIST *next, LOOPQ *loop);
void KillAllLPlist(LPLIST *lp);
LPLIST *FindLoopNest(LOOPQ *lp0);
#endif
