/*
 * This code written by Dave Nuechterlein based on R. Clint Whaley's
 * ATL_samm4x6x2_arm.S
 */
#ifndef ATL_GAS_ARM64
   #error "This routine requires GAS/ARM64 assembly"
#endif
#define rA0     v0
#define ra0     v1

#define rB0     v2
#define rB2     v3
#define rB4     v4
#define rb0     v5
#define rb2     v6
#define rb4     v7

#define rC00    v26
#define rC01    v27
#define rC02    v28
#define rC03    v29
#define rC04    v30
#define rC05    v31

#define NMU     w0
#define NNU0    w1
#define K0      x2
#define pA0     x3
#define pB0     x4
#define pC      x5
#define NNU     w6
#define K       x7
#define pB      x10
#define pA      x11
/*
                    r0       r1      r2         r3          0         4
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                      8           12          16
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);

*/
#ifdef BETA0
   #define F1ST( op1, op2, op3, index) fmulx  op1.4s, op2.4s, op3.4s[index]
#else
   #define F1ST( op1, op2, op3, index) fmla  op1.4s, op2.4s, op3.4s[index]
#endif
#include "atlas_asm.h"
.globl ATL_asmdecor(ATL_USERMM)
.type  ATL_asmdecor(ATL_USERMM), %function
ATL_asmdecor(ATL_USERMM):
        add     sp,sp,#-(10*16)
        stp     d8,d9,[sp,#(0*16)]
        stp     d10,d11,[sp,#(1*16)]
        stp     d12,d13,[sp,#(2*16)]
        stp     d14,d15,[sp,#(3*16)]
        stp     x18,x19,[sp,#(4*16)]
        stp     x20,x21,[sp,#(5*16)]
        stp     x22,x23,[sp,#(6*16)]
        stp     x24,x25,[sp,#(7*16)]
        stp     x26,x27,[sp,#(8*16)]
        stp     x28,x29,[sp,#(9*16)]

        ubfiz   x2,x2,0,31

   MLOOP:
      mov pB, pB0  /* point the pB pointer back to the start of B */
      mov NNU, NNU0
         ld1 {rB0.2s},[pB],#8
         ld1 {rB2.2s},[pB],#8
         #ifndef BETA0
            ld1 {rC00.4s},[pC],#16
            #ifndef BETA1
            fneg rC00.4s,rC00.4s
            #endif
         #endif
      NLOOP:
         mov pA, pA0  /* point pA to the next group of Rows */
/*
 *       1st peeled K iteration handles BETA & load of C
 */
         ld1 {rA0.4s},[pA],#16
         F1ST ( rC00, rA0, rB0, 0)
         #ifndef BETA0
            ld1 {rC01.4s},[pC],#16
            #ifndef BETA1
            fneg rC01.4s,rC01.4s
            #endif
         #endif

         F1ST ( rC01, rA0, rB0, 1)
         #ifndef BETA0
            ld1 {rC02.4s},[pC],#16
            #ifndef BETA1
            fneg rC02.4s,rC02.4s
            #endif
         #endif
         ld1 {rB4.2s},[pB],#8

         F1ST ( rC02, rA0, rB2, 0)
         #ifndef BETA0
            ld1 {rC03.4s},[pC],#16
            #ifndef BETA1
            fneg rC03.4s,rC03.4s
            #endif
         #endif
         #ifndef BETA0
            ld1 {rC04.4s},[pC],#16
            #ifndef BETA1
            fneg rC04.4s,rC04.4s
            #endif
         #endif

         F1ST ( rC03, rA0, rB2, 1)
         #ifndef BETA0
            ld1 {rC05.4s},[pC],#16
            #ifndef BETA1
            fneg rC05.4s,rC05.4s
            #endif
         #endif
         ld1 {rb0.2s},[pB],#8
         ld1 {rb2.2s},[pB],#8
         F1ST ( rC04, rA0, rB4, 0)
         #ifndef BETA0
            sub pC, pC, #96
         #endif

         F1ST ( rC05, rA0, rB4, 1)
         ld1 {rA0.4s},[pA],#16
/*
 *       2nd peeled K iteration
 */

         fmla rC00.4s, rA0.4s, rb0.4s[0]
         ld1 {rb4.2s},[pB],#8
         fmla rC01.4s, rA0.4s, rb0.4s[1]
         fmla rC02.4s, rA0.4s, rb2.4s[0]
         fmla rC03.4s, rA0.4s, rb2.4s[1]
         ld1 {rB0.2s},[pB],#8
         ld1 {rB2.2s},[pB],#8
         fmla rC04.4s, rA0.4s, rb4.4s[0]
         subs K, K0, #2
         fmla rC05.4s, rA0.4s, rb4.4s[1]
         ld1 {ra0.2s},[pA],#8
         ld1 {ra0.d}[1],[pA],#8
         beq KDONE

         KLOOP:
            subs K, K, #2
/*
 *          1st K iteration
 */
            fmla rC00.4s, ra0.4s, rB0.4s[0]
            fmla rC01.4s, ra0.4s, rB0.4s[1]
         ld1 {rB4.2s},[pB],#8

            fmla rC02.4s, ra0.4s, rB2.4s[0]
         ld1 {rb0.2s},[pB],#8
            fmla rC03.4s, ra0.4s, rB2.4s[1]

         ld1 {rb2.2s},[pB],#8
            fmla rC04.4s, ra0.4s, rB4.4s[0]
         ld1 {rA0.2s},[pA],#8
            fmla rC05.4s, ra0.4s, rB4.4s[1]
         ld1 {rA0.d}[1],[pA],#8
/*
 *          2nd K iteration
 */

            fmla rC00.4s, rA0.4s, rb0.4s[0]
            fmla rC01.4s, rA0.4s, rb0.4s[1]
         ld1 {rb4.2s},[pB],#8

            fmla rC02.4s, rA0.4s, rb2.4s[0]
         ld1 {rB0.2s},[pB],#8
            fmla rC03.4s, rA0.4s, rb2.4s[1]

         ld1 {rB2.2s},[pB],#8
            fmla rC04.4s, rA0.4s, rb4.4s[0]

         ld1 {ra0.2s},[pA],#8
            fmla rC05.4s, rA0.4s, rb4.4s[1]
         ld1 {ra0.d}[1],[pA],#8

         bne KLOOP
KDONE:
         subs NNU, NNU, #1
         st1 {rC00.4s-rC03.4s},[pC],#64
         st1 {rC04.4s-rC05.4s},[pC],#32
         #ifndef BETA0
            ld1 {rC00.4s},[pC],#16
            #ifndef BETA1
            fneg rC00.4s,rC00.4s
            #endif
         #endif

      bne NLOOP
         #ifndef BETA0
            add pC, pC, #-16
         #endif
      add pA0, pA0, K0, LSL #4
      subs NMU, NMU, #1
   bne MLOOP

        ldp     d8,d9,[sp,#(0*16)]
        ldp     d10,d11,[sp,#(1*16)]
        ldp     d12,d13,[sp,#(2*16)]
        ldp     d14,d15,[sp,#(3*16)]
        ldp     x18,x19,[sp,#(4*16)]
        ldp     x20,x21,[sp,#(5*16)]
        ldp     x22,x23,[sp,#(6*16)]
        ldp     x24,x25,[sp,#(7*16)]
        ldp     x26,x27,[sp,#(8*16)]
        ldp     x28,x29,[sp,#(9*16)]
        add     sp,sp,#(10*16)
   ret
