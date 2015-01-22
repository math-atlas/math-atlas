#ifndef OPTSIMD_H
#define OPTSIMD_H

/*
 * this is for different Simd vectorization methods
 */
#define VECT_NCONTRL 0x1    /* simple loop with no control flow */
#define VECT_SV 0x2       /* speculative vectorization */
#define VECT_SHADOW_VRC 0x4       /* speculative vectorization */

#if 0
   #define VECT_VRC 0x4       /* vector redundant computation */
   #define VECT_VEM 0x8       /* Vectorization after replacing with Max var */
#endif

int Type2Vlen(int type);
BLIST *AddLoopDupBlks(LOOPQ *lp, BBLOCK *up, BBLOCK *down);
void UnalignLoopSpecialization(LOOPQ *lp);
#endif
