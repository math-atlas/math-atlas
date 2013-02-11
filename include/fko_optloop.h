#ifndef FKO_OPTLOOP_H
#define FKO_OPTLOOP_H

#define PTRF_INC	0x1
#define PTRF_CONTIG	0x2
#define PTRF_CONSTINC   0x4
#define PTRF_MIXED      0x8

#ifndef INT_BVI 
   #define INT_BVI unsigned int
#endif

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
short *DeclareMaxE(int VEC, int ne, short STi);
short *DeclareMinE(int VEC, int ne, short STi);
int DoAllAccumExpansion(LOOPQ *lp, int unroll, int vec);
short *FindAllScalarVars(BLIST *scope);
BLIST *FindAllFallHeads(BLIST *ftheads, INT_BVI iscope, BBLOCK *head, 
                        INT_BVI tails, INT_BVI inblks);
BBLOCK *DupCFScope(INT_BVI ivscp0, INT_BVI ivscp, int dupnum, BBLOCK *head);
BLIST *CF2BlockList(BLIST *bl, INT_BVI bvblks, BBLOCK *head);
INSTQ *FindCompilerFlag(BBLOCK *bp, short flag);
#endif
