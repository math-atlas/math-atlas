/* This file is written for Cortex-A57. Works with 12x4 access-major storage. */
#ifndef ATL_GAS_ARM64
   #error "This routine requires GAS/AARCH64 assembly."
#endif

#include "atlas_asm.h"

/*
                     x0       x1     x2         x3         x4        x5
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC, 
                  CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
                         x6           x7         0
 */

#define FSIZE (8*8) /* to save d8-d15 regs */

#if defined(SREAL) || defined(SCPLX)
   #define TP 4s
#else
   #define TP 2d
#endif

#define qA0 q0
#define rA0 Mjoin(v0.,TP)
#define qA1 q1
#define rA1 Mjoin(v1.,TP)
#define qA2 q2
#define rA2 Mjoin(v2.,TP)
#define qA3 q3
#define rA3 Mjoin(v3.,TP)
#define qA4 q4
#define rA4 Mjoin(v4.,TP)
#define qA5 q5
#define rA5 Mjoin(v5.,TP)

#if defined(SREAL) || defined(SCPLX)
   #define qB0 q6
   #define rB0 v6.s[0]
   #define rB1 v6.s[1]
   #define rB2 v6.s[2]
   #define rB3 v6.s[3]
#else
   #define qB0 q6
   #define rB0 v6.d[0]
   #define rB1 v6.d[1]

   #define qB1 q7
   #define rB2 v7.d[0]
   #define rB3 v7.d[1]
#endif

#define qC00 q8
#define rC00 Mjoin( v8.,TP)
#define qC01 q9
#define rC01 Mjoin( v9.,TP)
#define qC02 q10
#define rC02 Mjoin(v10.,TP)
#define qC03 q11
#define rC03 Mjoin(v11.,TP)

#define qC10 q12
#define rC10 Mjoin(v12.,TP)
#define qC11 q13
#define rC11 Mjoin(v13.,TP)
#define qC12 q14
#define rC12 Mjoin(v14.,TP)
#define qC13 q15
#define rC13 Mjoin(v15.,TP)

#define qC20 q16
#define rC20 Mjoin(v16.,TP)
#define qC21 q17
#define rC21 Mjoin(v17.,TP)
#define qC22 q18
#define rC22 Mjoin(v18.,TP)
#define qC23 q19
#define rC23 Mjoin(v19.,TP)

#define qC30 q20
#define rC30 Mjoin(v20.,TP)
#define qC31 q21
#define rC31 Mjoin(v21.,TP)
#define qC32 q22
#define rC32 Mjoin(v22.,TP)
#define qC33 q23
#define rC33 Mjoin(v23.,TP)

#define qC40 q24
#define rC40 Mjoin(v24.,TP)
#define qC41 q25
#define rC41 Mjoin(v25.,TP)
#define qC42 q26
#define rC42 Mjoin(v26.,TP)
#define qC43 q27
#define rC43 Mjoin(v27.,TP)

#define qC50 q28
#define rC50 Mjoin(v28.,TP)
#define qC51 q29
#define rC51 Mjoin(v29.,TP)
#define qC52 q30
#define rC52 Mjoin(v30.,TP)
#define qC53 q31
#define rC53 Mjoin(v31.,TP)

/* Define integer registers as needed. */

#define nmu       x0 /* arg */
#define nnu0      x1 /* arg */
#define KK0       x2 /* arg */
#define pA0       x3 /* arg */
#define pB0       x4 /* arg */
#define pC        x5 /* arg */
#define pAn       x6 /* arg */
#define pBn       x7 /* arg */
#define pCn       x9 /* arg: load from SP(0) (shouldn't use x8) */

#if defined(SREAL) || defined(SCPLX)
   #define incBk     #16   /* incBk = NU*sizeof */
#else
   #define incBk     #32   /* incBk = NU*sizeof */
#endif

#define incAk     #96   /* incAk = MU*sizeof */
#define incCb     #384  /* incCb = MU*NU*sizeof */
#define incAm     x10   /* incAm = MU*sizeof*K */

/* Now define the dupliate registers */
#define KK       x11 /* copy from args */
#define nnu      x12 /* copy from args */
#define pA       x13 /* copy from args */
#define pB       x14 /* copy from args */

/* Now define the prefetch instructions/regs/constants */
#ifndef PFADIST
   #define PFADIST 288
#endif
#ifndef PFBDIST
   #define PFBDIST 96
#endif
#ifndef PFCDIST
   #define PFCDIST 96
#endif

#ifndef prefA
   #define prefA(base, off) prfm PLDL1KEEP, [base, #(off)]
#endif
#ifndef prefB
   #define prefB(base, off) prfm PLDL1KEEP, [base, #(off)]
#endif
#ifndef prefC
   #define prefC(base, off) prfm PLDL2STRM, [base, #(off)]
#endif

#if defined(DREAL) || defined(DCPLX)
   #define DBL_LD ldr qB1, [pB, #16] 
#else 
   #define DBL_LD 
#endif

/*
 * Re-arranging unrolled instructions doesn't help. So each iteration 
 * is replaced by this macro. The macro was parameterized for the 
 * first iteration.
 * - PREFETCH INSIDE IS HURTING A LOT (20%) .
 */

#define KLOOP_ITER1(op) \
   ldr qB0, [pB] ; \
   ldp qA0, qA1, [pA, #(0*16)] ; \
   op rC00, rA0, rB0 ; \
   op rC01, rA0, rB1 ; \
   DBL_LD ; \
   op rC02, rA0, rB2 ; \
   op rC03, rA0, rB3 ; \
 ; \
   ldp qA2, qA3, [pA, #(2*16)] ; \
   op rC10, rA1, rB0 ; \
   op rC11, rA1, rB1 ; \
   op rC12, rA1, rB2 ; \
   op rC13, rA1, rB3 ; \
 ; \
   ldp qA4, qA5, [pA, #(4*16)] ; \
   op rC20, rA2, rB0 ; \
   op rC21, rA2, rB1 ; \
   op rC22, rA2, rB2 ; \
   op rC23, rA2, rB3 ; \
   add pB, pB, incBk ; \
 ; \
   op rC30, rA3, rB0 ; \
   op rC31, rA3, rB1 ; \
   op rC32, rA3, rB2 ; \
   op rC33, rA3, rB3 ; \
   add pA, pA, incAk ; \
 ; \
   op rC40, rA4, rB0 ; \
   op rC41, rA4, rB1 ; \
   op rC42, rA4, rB2 ; \
   op rC43, rA4, rB3 ; \
 ; \
   op rC50, rA5, rB0 ; \
   op rC51, rA5, rB1 ; \
   op rC52, rA5, rB2 ; \
   op rC53, rA5, rB3

#ifndef BETA0
   #define LD_C(pC, off, qT0, qT1) ldp qT0, qT1, [pC, #(off)]
   #ifdef BETA1
      #define OP_C(rC, rT) fadd rC, rC, rT
   #else
      #define OP_C(rC, rT) fsub rC, rC, rT
   #endif
#else
   #define LD_C(pC, off, qT0, qT1)
   #define OP_C(rC, rT)
#endif


#define HandleC(pC, off, rC0, qC0, rC1, qC1, rT0, qT0, rT1, qT1) \
   LD_C(pC, off, qT0, qT1) ; \
   OP_C(rC0, rT0) ; \
   OP_C(rC1, rT1) ; \
   stp qC0, qC1, [pC, #(off)]

.text
.align 5
.global ATL_asmdecor(ATL_USERMM)
.type ATL_asmdecor(ATL_USERMM), %function
ATL_asmdecor(ATL_USERMM):
/*
 * Save callee-saved regs
 */
   sub SP, SP, #FSIZE
   stp  d8,  d9, [SP,  #0]
   stp d10, d11, [SP, #16]
   stp d12, d13, [SP, #32]
   stp d14, d15, [SP, #48]

   /* Save registers that will be destroyed inside loops */
   mov pA, pA0
   prefA(pA0, 0)
   mov pB, pB0
   prefB(pB0, 0)
   mov KK, KK0
   mov nnu, nnu0

   /* Load the remaining args from stack */
   ldr pCn, [SP, #FSIZE]

   /* incAm = MU*sizeof*K = 12*8*K = 3*32*K */
   add incAm, KK, KK, LSL #1     /* incAm = 3*K */
   lsl incAm, incAm, #5          /* incAm = (3*K) * 32 */


   .align 5
   MLOOP:
      NLOOP:
            /* to avoid zeroing, peel the first iteration */
            KLOOP_ITER1(fmul)
            KLOOP_ITER1(fmla)
            KLOOP_ITER1(fmla)
            KLOOP_ITER1(fmla)
            KLOOP_ITER1(fmla)
            KLOOP_ITER1(fmla)

            subs KK, KK, #6
            beq KDONE

         .align 5
         KLOOP:

            KLOOP_ITER1(fmla)
            KLOOP_ITER1(fmla)
            KLOOP_ITER1(fmla)
            KLOOP_ITER1(fmla)
            KLOOP_ITER1(fmla)
            KLOOP_ITER1(fmla)

            subs KK, KK, #6
         bne KLOOP

      KDONE:
         mov KK, KK0
         mov pA, pA0
            HandleC(pC, ( 0*16), rC00,qC00, rC10,qC10, rA0,qA0, rA1,qA1)
            HandleC(pC, ( 2*16), rC20,qC20, rC30,qC30, rA2,qA2, rA3,qA3)
            HandleC(pC, ( 4*16), rC40,qC40, rC50,qC50, rA4,qA4, rA5,qA5)
            HandleC(pC, ( 6*16), rC01,qC01, rC11,qC11, rA0,qA0, rA1,qA1)
            HandleC(pC, ( 8*16), rC21,qC21, rC31,qC31, rA2,qA2, rA3,qA3)
            HandleC(pC, (10*16), rC41,qC41, rC51,qC51, rA4,qA4, rA5,qA5)
            HandleC(pC, (12*16), rC02,qC02, rC12,qC12, rA0,qA0, rA1,qA1)
            HandleC(pC, (14*16), rC22,qC22, rC32,qC32, rA2,qA2, rA3,qA3)
            HandleC(pC, (16*16), rC42,qC42, rC52,qC52, rA4,qA4, rA5,qA5)
            HandleC(pC, (18*16), rC03,qC03, rC13,qC13, rA0,qA0, rA1,qA1)
            HandleC(pC, (20*16), rC23,qC23, rC33,qC33, rA2,qA2, rA3,qA3)
            HandleC(pC, (22*16), rC43,qC43, rC53,qC53, rA4,qA4, rA5,qA5)
            
         add pC, pC, incCb

         subs nnu, nnu, #1
      bne NLOOP

   NDONE:
      mov nnu, nnu0
      add pA, pA0, incAm
      add pA0, pA0, incAm
      mov pB, pB0

      subs nmu, nmu, #1
   bne MLOOP

DONE:
   ldp  d8,  d9, [SP,  #0]
   prefB(pBn, 0)
   ldp d10, d11, [SP, #16]
   prefA(pAn, 0)
   ldp d12, d13, [SP, #32]
   prefC(pCn, 0)
   ldp d14, d15, [SP, #48]
   add SP, SP, #FSIZE
   ret
