#ifndef FKO_OPTLOOP_H
#define FKO_OPTLOOP_H

#define PTRF_INC	0x1
#define PTRF_CONTIG	0x2
#define PTRF_CONSTINC   0x4
#define PTRF_MIXED      0x8

void AddLoopControl(LOOPQ *lp, INSTQ *ipinit, INSTQ *ipupdate, INSTQ *ippost,
                    INSTQ *iptest);
void KillLoopControl(LOOPQ *lp);
ILIST *FindIndexRef(BLIST *scope, short I);
void UpdatePointerLoads(BLIST *scope, struct ptrinfo *pbase, int UR);
INSTQ *KillPointerUpdates(struct ptrinfo *pbase, int UR);
struct ptrinfo *FindMovingPointers(BLIST *scope);
void UnrollCleanup(LOOPQ *lp, int unroll);
void GenCleanupLoop(LOOPQ *lp);
void OptimizeLoopControl(LOOPQ *lp, int unroll, int NeedKilling, INSTQ *ippost);
int UnrollLoop(LOOPQ *lp, int unroll);
void AddPrefetch(LOOPQ *lp, int unroll);
short *DeclareAE(int VEC, int ne, short STi);
int DoAllAccumExpansion(LOOPQ *lp, int unroll, int vec);
BLIST *FindAllFallHeads(BLIST *ftheads, int iscope, BBLOCK *head, int tails,
                        int inblks);
BBLOCK *DupCFScope(short ivscp0, short ivscp, int dupnum, BBLOCK *head);
BLIST *CF2BlockList(BLIST *bl, short bvblks, BBLOCK *head);
INSTQ *FindCompilerFlag(BBLOCK *bp, short flag);
#endif
