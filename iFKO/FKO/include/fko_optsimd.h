#ifndef OPTSIMD_H
#define OPTSIMD_H

/*
 * this is for different Simd vectorization methods
 */
#define VECT_NCONTRL 0x1      /* simple loop with no control flow */
#define VECT_SV 0x2           /* speculative vectorization */
#define VECT_SHADOW_VRC 0x4   /* shadow vrc vectorization  */
#define VECT_INTRINSIC  0x8   /* user defined intrinsic vectorization*/
#define VECT_SLP  0x10        /* SLP vctorization */

#if 0
   #define VECT_VRC 0x4       /* vector redundant computation */
   #define VECT_VEM 0x8       /* Vectorization after replacing with Max var */
#endif

int Type2Vlen(int type);
BLIST *AddLoopDupBlks(LOOPQ *lp, BBLOCK *up, BBLOCK *down);
void UnalignLoopSpecialization(LOOPQ *lp);
short FindReadUseType(INSTQ *ip, short var, INT_BVI blkvec);
void UpdateVecLoop(LOOPQ *lp);
int IsSpeculationNeeded();
int IsSIMDalignLoopPeelable(LOOPQ *lp);
int SpeculativeVectorAnalysis();
int SpecSIMDLoop(int SB_UR);
int RcVectorAnalysis();
int RcVectorization();
int SlpVectorization();
void FindPaths(BBLOCK *head, BLIST *loopblocks, LOOPQ *lp, BLIST *blkstack);
void KillPathTable();
int PathVectorizable(int pnum);
int FindNumPaths(LOOPQ *lp);
int LoopNestVec();
#endif
