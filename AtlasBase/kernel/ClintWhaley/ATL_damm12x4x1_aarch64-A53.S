/* This file is written for Cortex-A53. Works with 12x4 access-major storage. */
/*
 * NOTES on Cortex-A53:
 *    1. in-order dual issue.
 *    2. 128-bit loads takes 2-cycle and no dual-issue.
 *    3. 64-bit loads can be issued without restriction.
 *    4. 4th cycle of FMLA cannot have any FP instruction. 
 *    5. 64-bit loads are combined with int loads and then insert.
 */
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

#define FSIZE (8*8 + 8*8) /* to save d8-d15 & x19-x26 regs */

#if defined(SREAL) || defined(SCPLX)
   #define TP 4s
#else
   #define TP 2d
#endif

#define qA0 q0
#define rA0 Mjoin(v0.,TP)
   #define dA00 d0
   #define rA01 v0.d[1]
   #define xA01 x20
#define qA1 q1
#define rA1 Mjoin(v1.,TP)
   #define dA10 d1
   #define rA11 v1.d[1]
   #define xA11 x21
#define qA2 q2
#define rA2 Mjoin(v2.,TP)
   #define dA20 d2
   #define rA21 v2.d[1]
   #define xA21 x22
#define qA3 q3
#define rA3 Mjoin(v3.,TP)
   #define dA30 d3
   #define rA31 v3.d[1]
   #define xA31 x23
#define qA4 q4
#define rA4 Mjoin(v4.,TP)
   #define dA40 d4
   #define rA41 v4.d[1]
   #define xA41 x24
#define qA5 q5
#define rA5 Mjoin(v5.,TP)
   #define dA50 d5
   #define rA51 v5.d[1]
   #define xA51 x25

#if defined(SREAL) || defined(SCPLX)
   #define qB0 q6
   #define rB0 v6.s[0]
   #define rB1 v6.s[1]
   #define rB2 v6.s[2]
   #define rB3 v6.s[3]
   #define dB0 d6
   #define xB1 x15
   #define drB1 v6.d[1]
   #define rrB0 Mjoin(v6.,TP)
   #define qB1 q7
   #define rrB1 Mjoin(v7.,TP)
   #define xB0 x19
   #define drB0 v6.d[0]
#else
   #define qB0 q6
   #define rB0 v6.d[0]
   #define rB1 v6.d[1]
   #define rrB0 Mjoin(v6.,TP)
   #define dB0 d6
   #define xB1 x15
   #define drB1 v6.d[1]

   #define qB1 q7
   #define rB2 v7.d[0]
   #define rB3 v7.d[1]
   #define rrB1 Mjoin(v7.,TP)
   #define dB2 d7
   #define xB3 x19
   #define drB3 v7.d[1]
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

/*
 * - PREFETCH IS HURTING A LOT (20%) .
 */

#ifndef BETA0
   #define LD_C(pC, off, qC) ldr qC, [pC, #(off)]
   #ifdef BETA1
      #define OP_C(rC, rT) fadd rC, rC, rT
   #else
      #define OP_C(rC, rT) fsub rC, rC, rT
   #endif
#else
   #define LD_C(pC, off, qC)
   #define OP_C(rC, rT)
#endif


#define HandleC(pC, off, rC, qC, rT, qT) \
   LD_C(pC, off, qT) ; \
   OP_C(rC, rT) ; \
   str qC, [pC, #(off)]

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
   prefA(pA, 0)
   stp d12, d13, [SP, #32]
   stp d14, d15, [SP, #48]
   stp x19, x20, [SP, #64]
   prefB(pB, 0)
   stp x21, x22, [SP, #80]
   stp x23, x24, [SP, #96]
   stp x25, x26, [SP, #112]

   /* Save registers that will be destroyed inside loops */
   mov pA, pA0
   mov pB, pB0
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
            /* 
             * Peeling the first iteration seems to provide only 0.2% speedup.
             * Removed for now to keep the code simpler and work with K=1.
             */
         #if 1
            eor v8.16b, v8.16b, v8.16b
            eor v9.16b, v9.16b, v9.16b
            eor v10.16b, v10.16b, v10.16b
            eor v11.16b, v11.16b, v11.16b
            eor v12.16b, v12.16b, v12.16b
            eor v13.16b, v13.16b, v13.16b
            eor v14.16b, v14.16b, v14.16b
            eor v15.16b, v15.16b, v15.16b
            eor v16.16b, v16.16b, v16.16b
            eor v17.16b, v17.16b, v17.16b
            eor v18.16b, v18.16b, v18.16b
            eor v19.16b, v19.16b, v19.16b
            eor v20.16b, v20.16b, v20.16b
            eor v21.16b, v21.16b, v21.16b
            eor v22.16b, v22.16b, v22.16b
            eor v23.16b, v23.16b, v23.16b
            eor v24.16b, v24.16b, v24.16b
            eor v25.16b, v25.16b, v25.16b
            eor v26.16b, v26.16b, v26.16b
            eor v27.16b, v27.16b, v27.16b
            eor v28.16b, v28.16b, v28.16b
            eor v29.16b, v29.16b, v29.16b
            eor v30.16b, v30.16b, v30.16b
            eor v31.16b, v31.16b, v31.16b
         #endif
         ;
            ldr qA0, [pA, #( 0*16)] ;
         #if defined(DREAL) || defined(DCPLX)
            ldr dB0, [pB] ;
         #else
            ldr xB0, [pB] ;
         #endif
            ldr xB1, [pB, #8] ;
            ldr qA1, [pA, #( 1*16)] ;
            ldr dA20, [pA, #( 2*16)] ;
            ldr xA21, [pA, #( 2*16+8)] ;

            subs KK, KK, #1
            beq KDONE
         /* Here, assuming rB0 full-ready. rB1 half ready. */
         /* Also, assuming rA0, rA1 full-ready. rA2 half-ready. */
         .align 5
         KLOOP:

         #if defined(DREAL) || defined(DCPLX)
            ldr dB2, [pB, #16] ;
         #else
            ins drB0, xB0
         #endif
            ins drB1, xB1 ;
         ;
            fmla rC00, rA0, rB0 ;
         #if defined(DREAL) || defined(DCPLX)
            ldr xB3, [pB, #(16+8)] ;
         #endif
            fmla rC01, rA0, rB1 ;
            fmla rC02, rA0, rB2 ;
         ;
            ldr dA30, [pA, #( 3*16)] ;
         #if defined(DREAL) || defined(DCPLX)
            ins drB3, xB3 ;
         #endif
         ;
            fmla rC03, rA0, rB3 ;
            ldr xA31, [pA, #( 3*16+8)] ;
            fmla rC10, rA1, rB0 ;
            fmla rC11, rA1, rB1 ;
         ;
            ldr dA40, [pA, #( 4*16)] ;
            ins rA21, xA21 ;
         ;
            fmla rC12, rA1, rB2 ;
            ldr xA41, [pA, #( 4*16+8)] ;
            fmla rC13, rA1, rB3 ;
            fmla rC20, rA2, rB0 ;
         ;
            ldr dA50, [pA, #( 5*16)] ;
            ins rA31, xA31 ;
         ;
            fmla rC21, rA2, rB1 ;
            ldr xA51, [pA, #( 5*16+8)] ;
            fmla rC22, rA2, rB2 ;
            add pA, pA, incAk ;
            fmla rC23, rA2, rB3 ;
         ;
            ldr dA00, [pA, #( 0*16)] ;
            ins rA41, xA41 ;
         ;
            fmla rC30, rA3, rB0 ;
            ldr xA01, [pA, #( 0*16+8)] ;
            fmla rC40, rA4, rB0 ;
            fmla rC31, rA3, rB1 ;
         ;
            ldr dA10, [pA, #( 1*16)] ;
            ins rA51, xA51 ;
         ;
            fmla rC50, rA5, rB0 ;
            ldr xA11, [pA, #( 1*16+8)] ;
            fmla rC41, rA4, rB1 ;
            add pB, pB, incBk ;
            fmla rC51, rA5, rB1 ;
         ;
            ldr dA20, [pA, #( 2*16)] ;
            ins rA01, xA01 ;
         ;
            fmla rC32, rA3, rB2 ;
            ldr xA21, [pA, #( 2*16+8)] ;
            fmla rC42, rA4, rB2 ;
            subs KK, KK, #1
            fmla rC52, rA5, rB2 ;
         ;
         #if defined(DREAL) || defined(DCPLX)
            ldr dB0, [pB] ;
         #else
            ldr xB0, [pB] ;
         #endif
            ins rA11, xA11 ;
         ;
            fmla rC33, rA3, rB3 ;
            ldr xB1, [pB, #8] ;
            fmla rC43, rA4, rB3 ;
            fmla rC53, rA5, rB3 ;
         ;

         bne KLOOP

      KDONE:
      /* tail for the last iteration */
         ;
         #if defined(DREAL) || defined(DCPLX)
            ldr dB2, [pB, #16] ;
         #else
            ins drB0, xB0
         #endif
            ins drB1, xB1 ;
         ;
            fmla rC00, rA0, rB0 ;
         #if defined(DREAL) || defined(DCPLX)
            ldr xB3, [pB, #(16+8)] ;
         #endif
            fmla rC01, rA0, rB1 ;
            fmla rC02, rA0, rB2 ;
         ;
            ldr dA30, [pA, #( 3*16)] ;
         #if defined(DREAL) || defined(DCPLX)
            ins drB3, xB3 ;
         #endif
         ;
            fmla rC03, rA0, rB3 ;
            ldr xA31, [pA, #( 3*16+8)] ;
            fmla rC10, rA1, rB0 ;
            fmla rC11, rA1, rB1 ;
         ;
            ldr dA40, [pA, #( 4*16)] ;
            ins rA21, xA21 ;
         ;
            fmla rC12, rA1, rB2 ;
            ldr xA41, [pA, #( 4*16+8)] ;
            fmla rC13, rA1, rB3 ;
            fmla rC20, rA2, rB0 ;
         ;
            ldr dA50, [pA, #( 5*16)] ;
            ins rA31, xA31 ;
         ;
            fmla rC21, rA2, rB1 ;
            ldr xA51, [pA, #( 5*16+8)] ;
            fmla rC22, rA2, rB2 ;
            add pA, pA, incAk ;
            fmla rC23, rA2, rB3 ;
         ;
            ins rA41, xA41 ;
         ;
            fmla rC30, rA3, rB0 ;
            fmla rC40, rA4, rB0 ;
            fmla rC31, rA3, rB1 ;
         ;
            ins rA51, xA51 ;
         ;
            fmla rC50, rA5, rB0 ;
            fmla rC41, rA4, rB1 ;
            add pB, pB, incBk ;
            fmla rC51, rA5, rB1 ;
         ;
         ;
            fmla rC32, rA3, rB2 ;
            fmla rC42, rA4, rB2 ;
            fmla rC52, rA5, rB2 ;
         ;
         ;
            fmla rC33, rA3, rB3 ;
            fmla rC43, rA4, rB3 ;
            fmla rC53, rA5, rB3
      /* tail for the last iteration is done now. */
         mov KK, KK0
         mov pA, pA0
         #if 0 /* this loses 1% of performance */
            HandleC(pC, ( 0*16), rC00, qC00, rA0, qA0)
            HandleC(pC, ( 1*16), rC10, qC10, rA0, qA0)
            HandleC(pC, ( 2*16), rC20, qC20, rA0, qA0)
            HandleC(pC, ( 3*16), rC30, qC30, rA0, qA0)
            HandleC(pC, ( 4*16), rC40, qC40, rA0, qA0)
            HandleC(pC, ( 5*16), rC50, qC50, rA0, qA0)
            HandleC(pC, ( 6*16), rC01, qC01, rA0, qA0)
            HandleC(pC, ( 7*16), rC11, qC11, rA0, qA0)
            HandleC(pC, ( 8*16), rC21, qC21, rA0, qA0)
            HandleC(pC, ( 9*16), rC31, qC31, rA0, qA0)
            HandleC(pC, (10*16), rC41, qC41, rA0, qA0)
            HandleC(pC, (11*16), rC51, qC51, rA0, qA0)
            HandleC(pC, (12*16), rC02, qC02, rA0, qA0)
            HandleC(pC, (13*16), rC12, qC12, rA0, qA0)
            HandleC(pC, (14*16), rC22, qC22, rA0, qA0)
            HandleC(pC, (15*16), rC32, qC32, rA0, qA0)
            HandleC(pC, (16*16), rC42, qC42, rA0, qA0)
            HandleC(pC, (17*16), rC52, qC52, rA0, qA0)
            HandleC(pC, (18*16), rC03, qC03, rA0, qA0)
            HandleC(pC, (19*16), rC13, qC13, rA0, qA0)
            HandleC(pC, (20*16), rC23, qC23, rA0, qA0)
            HandleC(pC, (21*16), rC33, qC33, rA0, qA0)
            HandleC(pC, (22*16), rC43, qC43, rA0, qA0)
            HandleC(pC, (23*16), rC53, qC53, rA0, qA0)
         #else
            LD_C(pC, ( 0*16), qA0)
            LD_C(pC, ( 1*16), qA1)
            LD_C(pC, ( 2*16), qA2)
            LD_C(pC, ( 3*16), qA3)
            LD_C(pC, ( 4*16), qA4)
            LD_C(pC, ( 5*16), qA5)
            LD_C(pC, ( 6*16), qB0)
            LD_C(pC, ( 7*16), qB1)
            OP_C(rC00, rA0)
            OP_C(rC10, rA1)
            OP_C(rC20, rA2)
            OP_C(rC30, rA3)
            OP_C(rC40, rA4)
            OP_C(rC50, rA5)
            OP_C(rC01, rrB0)
            OP_C(rC11, rrB1)
            str qC00, [pC, #( 0*16)]
            str qC10, [pC, #( 1*16)]
            str qC20, [pC, #( 2*16)]
            str qC30, [pC, #( 3*16)]
            str qC40, [pC, #( 4*16)]
            str qC50, [pC, #( 5*16)]
            str qC01, [pC, #( 6*16)]
            str qC11, [pC, #( 7*16)]
            LD_C(pC, ( 8*16), qA0)
            LD_C(pC, ( 9*16), qA1)
            LD_C(pC, (10*16), qA2)
            LD_C(pC, (11*16), qA3)
            LD_C(pC, (12*16), qA4)
            LD_C(pC, (13*16), qA5)
            LD_C(pC, (14*16), qB0)
            LD_C(pC, (15*16), qB1)
            OP_C(rC21, rA0)
            OP_C(rC31, rA1)
            OP_C(rC41, rA2)
            OP_C(rC51, rA3)
            OP_C(rC02, rA4)
            OP_C(rC12, rA5)
            OP_C(rC22, rrB0)
            OP_C(rC32, rrB1)
            str qC21, [pC, #( 8*16)]
            str qC31, [pC, #( 9*16)]
            str qC41, [pC, #(10*16)]
            str qC51, [pC, #(11*16)]
            str qC02, [pC, #(12*16)]
            str qC12, [pC, #(13*16)]
            str qC22, [pC, #(14*16)]
            str qC32, [pC, #(15*16)]
            LD_C(pC, (16*16), qA0)
            LD_C(pC, (17*16), qA1)
            LD_C(pC, (18*16), qA2)
            LD_C(pC, (19*16), qA3)
            LD_C(pC, (20*16), qA4)
            LD_C(pC, (21*16), qA5)
            LD_C(pC, (22*16), qB0)
            LD_C(pC, (23*16), qB1)
            OP_C(rC42, rA0)
            OP_C(rC52, rA1)
            OP_C(rC03, rA2)
            OP_C(rC13, rA3)
            OP_C(rC23, rA4)
            OP_C(rC33, rA5)
            OP_C(rC43, rrB0)
            OP_C(rC53, rrB1)
            str qC42, [pC, #(16*16)]
            str qC52, [pC, #(17*16)]
            str qC03, [pC, #(18*16)]
            str qC13, [pC, #(19*16)]
            str qC23, [pC, #(20*16)]
            str qC33, [pC, #(21*16)]
            str qC43, [pC, #(22*16)]
            str qC53, [pC, #(23*16)]
         #endif
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
   ldp d10, d11, [SP, #16]
   prefA(pAn, 0)
   ldp d12, d13, [SP, #32]
   ldp d14, d15, [SP, #48]
   prefB(pBn, 0)
   ldp x19, x20, [SP, #64]
   ldp x21, x22, [SP, #80]
   prefC(pCn, 0)
   ldp x23, x24, [SP, #96]
   ldp x25, x26, [SP, #112]
   add SP, SP, #FSIZE
   ret
