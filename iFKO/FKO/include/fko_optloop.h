#ifndef FKO_OPTLOOP_H
#define FKO_OPTLOOP_H

#define PTRF_INC	0x1
#define PTRF_CONTIG	0x2
#define PTRF_CONSTINC   0x4
#define PTRF_MIXED      0x8

#ifndef INT_BVI 
   #define INT_BVI unsigned int
#endif
/*
 * Majedul: used in RC transformation
 */
#define IN_IF_ONLY 0x1
#define IN_ELSE_ONLY 0x2
#define IN_BOTH_IF_ELSE 0x3


void AddLoopControl(LOOPQ *lp, INSTQ *ipinit, INSTQ *ipupdate, INSTQ *ippost,
                    INSTQ *iptest);
void KillLoopControl(LOOPQ *lp);
ILIST *FindIndexRef(BLIST *scope, short I);
ILIST *FindIndexRefInArray(BLIST *scope, short I);
void UpdatePointerLoads(BLIST *scope, struct ptrinfo *pbase, int UR, int unroll);
INSTQ *KillPointerUpdates(struct ptrinfo *pbase, int UR);
struct ptrinfo *FindMovingPointers(BLIST *scope);
void UnrollCleanup(LOOPQ *lp, int unroll);
void UnrollCleanup2(LOOPQ *lp, int unroll);
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
/*BBLOCK *DupCFScope(INT_BVI ivscp0, INT_BVI ivscp, int dupnum, BBLOCK *head);*/
BBLOCK *DupCFScope(INT_BVI ivscp0, INT_BVI ivscp, BBLOCK *head);
BLIST *CF2BlockList(BLIST *bl, INT_BVI bvblks, BBLOCK *head);
INSTQ *FindCompilerFlag(BBLOCK *bp, short flag);
BBLOCK *DupBlock(BBLOCK *bold);
int VarIsAccumulator(BLIST *scope, int var);
int VarIsMaxOrMin(BLIST *scope, short var, int maxcheck, int mincheck);
int MovMaxMinVarsOut(int movmax, int movmin);
int ElimMaxMinIf(int maxelim, int minelim);
void FeedbackLoopInfo();
void UpdateOptLoopWithMaxMinVars();
int IterativeRedCom();
void FinalizeVectorCleanup(LOOPQ *lp, int unroll);
int DoAllScalarExpansion(LOOPQ *lp, int unroll, int vec);
void UpdateUnrolledIndices(BLIST *scope, short I, int UR);
int NonLocalDeref(short dt);
void Set_OL_NEINC_One();
int Get_OL_NEINC();
INSTQ *GetSEHeadTail(LOOPQ *lp, short se, short ne, short *ses, int vec,
                     int sflag);
void AddInstToPrehead(LOOPQ *lp, INSTQ *iadd, short type, short r0, short r1);
void AddInstToPosttail(LOOPQ *lp, INSTQ *iadd, short type, short r0, short r1);
int DoScalExpansOnLoop(LOOPQ *lp, short type, short se, short *ses);
int DelLoopControl(LOOPQ *lp);
int CountUnrollFactor(LOOPQ *lp);
short *UpdateDeref(INSTQ *ip, int ireg, int inc);
struct ptrinfo *FindConstMovingPtr(BBLOCK *bp);
int FindNumIFs(BLIST *scope);
#endif
