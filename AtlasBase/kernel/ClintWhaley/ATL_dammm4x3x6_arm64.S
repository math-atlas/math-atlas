#include "atlas_asm.h"
#define rA0  d0
#define rA1  d1
#define rA2  d2
#define rA3  d3
#define ra0  d4
#define ra1  d5
#define ra2  d6
#define ra3  d7
#define Ra0  d8
#define Ra1  d9
#define Ra2  d10
#define Ra3  d11
#define rB0  d14
#define rB1  d15
#define rB2  d16
#define rb0  d17
#define rb1  d18
#define rb2  d19
#define rC00 d20
#define rC10 d21
#define rC20 d22
#define rC30 d23
#define rC01 d24
#define rC11 d25
#define rC21 d26
#define rC31 d27
#define rC02 d28
#define rC12 d29
#define rC22 d30
#define rC32 d31

#define nmu x0
#define nnu x1
#define K0  x2
#define pA  x3
#define pB  x4
#define pC  x5
#define pAn x6
#define pBn x7
#define pCn x8
#define JJ  x9
#define KK  x10
#define pB0 x11
#define pA0 x12
#define rc00 x13
#define rc10 x14
#define rc20 x15
#define rc21 x16
/*
 *                   x0,      x1,    x2,       x3,        x4,        x5
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 *                      x6,       x7,        (SP)
 */
#define FSIZE 8*8   /* save & restore d8-d15 */
.text
.globl ATL_asmdecor(ATL_USERMM)
ATL_asmdecor(ATL_USERMM):
   sub sp, sp, #FSIZE      /* allocate local frame */
   str d8, [sp]
   str d9, [sp,#8]
   str d10, [sp,#2*8]
   str d11, [sp,#3*8]
   str d12, [sp,#4*8]
   str d13, [sp,#5*8]
   str d14, [sp,#6*8]
   str d15, [sp,#7*8]
   mov pA0, pA
   mov pB0, pB

   MLOOP:
      mov JJ, nnu
      ldr rA0, [pA]
      ldr rA1, [pA,#8]
      ldr rA2, [pA,#8*2]
      ldr rA3, [pA,#8*3]
      ldr rB0, [pB]
      ldr rB1, [pB,#8]
      ldr ra0, [pA,#8*4]
      ldr ra1, [pA,#8*5]
      ldr ra2, [pA,#8*6]
      ldr ra3, [pA,#8*7]
      NLOOP:
/*
 *       First 6 K-iterations peeled to handle beta & load/use pipe
 */
      ldr rB2, [pB,#8*2]
         subs KK, K0, #12
         #ifdef BETA0 /* def BETA0 */
      ldr rb0, [pB,#8*3]
            fmul rC00, rA0, rB0
      ldr rb1, [pB,#8*4]
            fmul rC10, rA1, rB0
      ldr rb2, [pB,#8*5]
            fmul rC20, rA2, rB0
      ldr Ra0, [pA,#8*8]
            fmul rC30, rA3, rB0
               ldr rB0, [pB,#8*6]!
            fmul rC01, rA0, rB1
      ldr Ra1, [pA,#8*9]
            fmul rC11, rA1, rB1
      ldr Ra2, [pA,#8*10]
            fmul rC21, rA2, rB1
      ldr Ra3, [pA,#8*11]
            fmul rC31, rA3, rB1
               ldr rB1, [pB,#8]
            fmul rC02, rA0, rB2
               ldr rA0, [pA,#8*12]!
            fmul rC12, rA1, rB2
               ldr rA1, [pA,#8]
            fmul rC22, rA2, rB2
               ldr rA2, [pA,#8*2]
            fmul rC32, rA3, rB2
               ldr rB2, [pB,#8*2]
         #else
      ldr rb0, [pB,#8*3]
      ldr rb1, [pB,#8*4]
      ldr rb2, [pB,#8*5]
      ldr Ra0, [pA,#8*8]
      ldr Ra1, [pA,#8*9]
      ldr Ra2, [pA,#8*10]
      ldr Ra3, [pA,#8*11]
            #ifdef BETA1
               #define F1ST fmadd
            #else
               #define F1ST fnmsub
            #endif
            ldr rC00, [pC]
            ldr rC10, [pC,#8]
            ldr rC20, [pC,#8*2]
            ldr rC30, [pC,#8*3]
            F1ST rC00, rA0, rB0, rC00
            ldr rC01, [pC,#8*4]
            F1ST rC10, rA1, rB0, rC10
            ldr rC11, [pC,#8*5]
            F1ST rC20, rA2, rB0, rC20
            ldr rC21, [pC,#8*6]
            F1ST rC30, rA3, rB0, rC30
               ldr rB0, [pB,#8*6]!
            F1ST rC01, rA0, rB1, rC01
            ldr rC31, [pC,#8*7]
            F1ST rC11, rA1, rB1, rC11
            F1ST rC21, rA2, rB1, rC21
            F1ST rC31, rA3, rB1, rC31
            ldr rC02, [pC,#8*8]
               ldr rB1, [pB,#8]
            F1ST rC02, rA0, rB2, rC02
               ldr rA0, [pA,#8*12]!
            ldr rC12, [pC,#8*9]
            F1ST rC12, rA1, rB2, rC12
               ldr rA1, [pA,#8]
            ldr rC22, [pC,#8*10]
            F1ST rC22, rA2, rB2, rC22
               ldr rA2, [pA,#8*2]
            ldr rC32, [pC,#8*11]
            F1ST rC32, rA3, rB2, rC32
               ldr rB2, [pB,#8*2]
         #endif

            fmadd rC00, ra0, rb0, rC00
               ldr rA3, [pA,#8*3]
            fmadd rC10, ra1, rb0, rC10
            fmadd rC20, ra2, rb0, rC20
            fmadd rC30, ra3, rb0, rC30
               ldr rb0, [pB,#8*3]
            fmadd rC01, ra0, rb1, rC01
            #ifdef BETA0
            #endif
            fmadd rC11, ra1, rb1, rC11
            fmadd rC21, ra2, rb1, rC21
            fmadd rC31, ra3, rb1, rC31
               ldr rb1, [pB,#8*4]
            fmadd rC02, ra0, rb2, rC02
               ldr ra0, [pA,#8*4]
            fmadd rC12, ra1, rb2, rC12
               ldr ra1, [pA,#8*5]
            fmadd rC22, ra2, rb2, rC22
               ldr ra2, [pA,#8*6]
            fmadd rC32, ra3, rb2, rC32
               ldr rb2, [pB,#8*5]


            fmadd rC00, Ra0, rB0, rC00
               ldr ra3, [pA,#8*7]
            fmadd rC10, Ra1, rB0, rC10
            fmadd rC20, Ra2, rB0, rC20
            fmadd rC30, Ra3, rB0, rC30
               ldr rB0, [pB,#8*6]
            fmadd rC01, Ra0, rB1, rC01
            fmadd rC11, Ra1, rB1, rC11
            fmadd rC21, Ra2, rB1, rC21
            fmadd rC31, Ra3, rB1, rC31
               ldr rB1, [pB,#8*7]
            fmadd rC02, Ra0, rB2, rC02
               ldr Ra0, [pA,#8*8]
            fmadd rC12, Ra1, rB2, rC12
               ldr Ra1, [pA,#8*9]
            fmadd rC22, Ra2, rB2, rC22
               ldr Ra2, [pA,#8*10]
            fmadd rC32, Ra3, rB2, rC32
               ldr rB2, [pB,#8*8]

            fmadd rC00, rA0, rb0, rC00
               ldr Ra3, [pA,#8*11]
            fmadd rC10, rA1, rb0, rC10
            fmadd rC20, rA2, rb0, rC20
            fmadd rC30, rA3, rb0, rC30
               ldr rb0, [pB,#8*9]
            fmadd rC01, rA0, rb1, rC01
            fmadd rC11, rA1, rb1, rC11
            fmadd rC21, rA2, rb1, rC21
            fmadd rC31, rA3, rb1, rC31
               ldr rb1, [pB,#8*10]
            fmadd rC02, rA0, rb2, rC02
               ldr rA0, [pA,#8*12]
            fmadd rC12, rA1, rb2, rC12
               ldr rA1, [pA,#8*13]
            fmadd rC22, rA2, rb2, rC22
               ldr rA2, [pA,#8*14]
            fmadd rC32, rA3, rb2, rC32
               ldr rb2, [pB,#8*11]

            fmadd rC00, ra0, rB0, rC00
               ldr rA3, [pA,#8*15]
            fmadd rC10, ra1, rB0, rC10
            fmadd rC20, ra2, rB0, rC20
            fmadd rC30, ra3, rB0, rC30
               ldr rB0, [pB,#8*12]
            fmadd rC01, ra0, rB1, rC01
            fmadd rC11, ra1, rB1, rC11
            fmadd rC21, ra2, rB1, rC21
            fmadd rC31, ra3, rB1, rC31
               ldr rB1, [pB,#8*13]
            fmadd rC02, ra0, rB2, rC02
               ldr ra0, [pA,#8*16]
            fmadd rC12, ra1, rB2, rC12
               ldr ra1, [pA,#8*17]
            fmadd rC22, ra2, rB2, rC22
               ldr ra2, [pA,#8*18]
            fmadd rC32, ra3, rB2, rC32
               ldr rB2, [pB,#8*14]

            fmadd rC00, Ra0, rb0, rC00
               ldr ra3, [pA,#8*19]
            fmadd rC10, Ra1, rb0, rC10
            fmadd rC20, Ra2, rb0, rC20
            fmadd rC30, Ra3, rb0, rC30
               ldr rb0, [pB,#8*15]
            fmadd rC01, Ra0, rb1, rC01
            fmadd rC11, Ra1, rb1, rC11
            fmadd rC21, Ra2, rb1, rC21
            fmadd rC31, Ra3, rb1, rC31
               ldr rb1, [pB,#8*16]
            fmadd rC02, Ra0, rb2, rC02
               ldr Ra0, [pA,#8*20]
            fmadd rC12, Ra1, rb2, rC12
               ldr Ra1, [pA,#8*21]
            fmadd rC22, Ra2, rb2, rC22
               ldr Ra2, [pA,#8*22]
            fmadd rC32, Ra3, rb2, rC32
               ldr rb2, [pB,#8*17]
            b.eq KDONE
         KLOOP:
            subs KK, KK, #6
               ldr Ra3, [pA,#8*23]
            fmadd rC00, rA0, rB0, rC00
            fmadd rC10, rA1, rB0, rC10
            fmadd rC20, rA2, rB0, rC20
            fmadd rC30, rA3, rB0, rC30
               ldr rB0, [pB,#8*18]!
            fmadd rC01, rA0, rB1, rC01
            fmadd rC11, rA1, rB1, rC11
            fmadd rC21, rA2, rB1, rC21
            fmadd rC31, rA3, rB1, rC31
               ldr rB1, [pB,#8]
            fmadd rC02, rA0, rB2, rC02
               ldr rA0, [pA,#8*24]!
            fmadd rC12, rA1, rB2, rC12
               ldr rA1, [pA,#8]
            fmadd rC22, rA2, rB2, rC22
               ldr rA2, [pA,#8*2]
            fmadd rC32, rA3, rB2, rC32
               ldr rB2, [pB,#8*2]

            fmadd rC00, ra0, rb0, rC00
               ldr rA3, [pA,#8*3]
            fmadd rC10, ra1, rb0, rC10
            fmadd rC20, ra2, rb0, rC20
            fmadd rC30, ra3, rb0, rC30
               ldr rb0, [pB,#8*3]
            fmadd rC01, ra0, rb1, rC01
            fmadd rC11, ra1, rb1, rC11
            fmadd rC21, ra2, rb1, rC21
            fmadd rC31, ra3, rb1, rC31
               ldr rb1, [pB,#8*4]
            fmadd rC02, ra0, rb2, rC02
               ldr ra0, [pA,#8*4]
            fmadd rC12, ra1, rb2, rC12
               ldr ra1, [pA,#8*5]
            fmadd rC22, ra2, rb2, rC22
               ldr ra2, [pA,#8*6]
            fmadd rC32, ra3, rb2, rC32
               ldr rb2, [pB,#8*5]

            fmadd rC00, Ra0, rB0, rC00
               ldr ra3, [pA,#8*7]
            fmadd rC10, Ra1, rB0, rC10
            fmadd rC20, Ra2, rB0, rC20
            fmadd rC30, Ra3, rB0, rC30
               ldr rB0, [pB,#8*6]
            fmadd rC01, Ra0, rB1, rC01
            fmadd rC11, Ra1, rB1, rC11
            fmadd rC21, Ra2, rB1, rC21
            fmadd rC31, Ra3, rB1, rC31
               ldr rB1, [pB,#8*7]
            fmadd rC02, Ra0, rB2, rC02
               ldr Ra0, [pA,#8*8]
            fmadd rC12, Ra1, rB2, rC12
               ldr Ra1, [pA,#8*9]
            fmadd rC22, Ra2, rB2, rC22
               ldr Ra2, [pA,#8*10]
            fmadd rC32, Ra3, rB2, rC32
               ldr rB2, [pB,#8*8]

            fmadd rC00, rA0, rb0, rC00
               ldr Ra3, [pA,#8*11]
            fmadd rC10, rA1, rb0, rC10
            fmadd rC20, rA2, rb0, rC20
            fmadd rC30, rA3, rb0, rC30
               ldr rb0, [pB,#8*9]
            fmadd rC01, rA0, rb1, rC01
            fmadd rC11, rA1, rb1, rC11
            fmadd rC21, rA2, rb1, rC21
            fmadd rC31, rA3, rb1, rC31
               ldr rb1, [pB,#8*10]
            fmadd rC02, rA0, rb2, rC02
               ldr rA0, [pA,#8*12]
            fmadd rC12, rA1, rb2, rC12
               ldr rA1, [pA,#8*13]
            fmadd rC22, rA2, rb2, rC22
               ldr rA2, [pA,#8*14]
            fmadd rC32, rA3, rb2, rC32
               ldr rb2, [pB,#8*11]

            fmadd rC00, ra0, rB0, rC00
               ldr rA3, [pA,#8*15]
            fmadd rC10, ra1, rB0, rC10
            fmadd rC20, ra2, rB0, rC20
            fmadd rC30, ra3, rB0, rC30
               ldr rB0, [pB,#8*12]
            fmadd rC01, ra0, rB1, rC01
            fmadd rC11, ra1, rB1, rC11
            fmadd rC21, ra2, rB1, rC21
            fmadd rC31, ra3, rB1, rC31
               ldr rB1, [pB,#8*13]
            fmadd rC02, ra0, rB2, rC02
               ldr ra0, [pA,#8*16]
            fmadd rC12, ra1, rB2, rC12
               ldr ra1, [pA,#8*17]
            fmadd rC22, ra2, rB2, rC22
               ldr ra2, [pA,#8*18]
            fmadd rC32, ra3, rB2, rC32
               ldr rB2, [pB,#8*14]

            fmadd rC00, Ra0, rb0, rC00
               ldr ra3, [pA,#8*19]
            fmadd rC10, Ra1, rb0, rC10
            fmadd rC20, Ra2, rb0, rC20
            fmadd rC30, Ra3, rb0, rC30
               ldr rb0, [pB,#8*15]
            fmadd rC01, Ra0, rb1, rC01
            fmadd rC11, Ra1, rb1, rC11
            fmadd rC21, Ra2, rb1, rC21
            fmadd rC31, Ra3, rb1, rC31
               ldr rb1, [pB,#8*16]
            fmadd rC02, Ra0, rb2, rC02
               ldr Ra0, [pA,#8*20]
            fmadd rC12, Ra1, rb2, rC12
               ldr Ra1, [pA,#8*21]
            fmadd rC22, Ra2, rb2, rC22
               ldr Ra2, [pA,#8*22]
            fmadd rC32, Ra3, rb2, rC32
               ldr rb2, [pB,#8*17]
         b.ne KLOOP
         KDONE:
/*
 *       Last 6 K-iterations peeled to drain load/use pipline & apply C
 */
            fmadd rC00, rA0, rB0, rC00
               ldr Ra3, [pA,#8*23]
            fmadd rC10, rA1, rB0, rC10
            fmadd rC20, rA2, rB0, rC20
            fmadd rC30, rA3, rB0, rC30
               ldr rB0, [pB,#8*18]!
            fmadd rC01, rA0, rB1, rC01
            fmadd rC11, rA1, rB1, rC11
            fmadd rC21, rA2, rB1, rC21
            fmadd rC31, rA3, rB1, rC31
               ldr rB1, [pB,#8]
            fmadd rC02, rA0, rB2, rC02
               ldr rA0, [pA,#8*24]!
            fmadd rC12, rA1, rB2, rC12
               ldr rA1, [pA,#8]
            fmadd rC22, rA2, rB2, rC22
               ldr rA2, [pA,#8*2]
            fmadd rC32, rA3, rB2, rC32
               ldr rB2, [pB,#8*2]

            fmadd rC00, ra0, rb0, rC00
               ldr rA3, [pA,#8*3]
            fmadd rC10, ra1, rb0, rC10
            fmadd rC20, ra2, rb0, rC20
            fmadd rC30, ra3, rb0, rC30
               ldr rb0, [pB,#8*3]
            fmadd rC01, ra0, rb1, rC01
            fmadd rC11, ra1, rb1, rC11
            fmadd rC21, ra2, rb1, rC21
            fmadd rC31, ra3, rb1, rC31
               ldr rb1, [pB,#8*4]
            fmadd rC02, ra0, rb2, rC02
               ldr ra0, [pA,#8*4]
            fmadd rC12, ra1, rb2, rC12
               ldr ra1, [pA,#8*5]
            fmadd rC22, ra2, rb2, rC22
               ldr ra2, [pA,#8*6]
            fmadd rC32, ra3, rb2, rC32
               ldr rb2, [pB,#8*5]

            fmadd rC00, Ra0, rB0, rC00
               ldr ra3, [pA,#8*7]
            fmadd rC10, Ra1, rB0, rC10
            fmadd rC20, Ra2, rB0, rC20
            fmadd rC30, Ra3, rB0, rC30
               ldr rB0, [pB,#8*6]
            fmadd rC01, Ra0, rB1, rC01
            fmadd rC11, Ra1, rB1, rC11
            fmadd rC21, Ra2, rB1, rC21
            fmadd rC31, Ra3, rB1, rC31
               ldr rB1, [pB,#8*7]
            fmadd rC02, Ra0, rB2, rC02
               ldr Ra0, [pA,#8*8]
            fmadd rC12, Ra1, rB2, rC12
               ldr Ra1, [pA,#8*9]
            fmadd rC22, Ra2, rB2, rC22
               ldr Ra2, [pA,#8*10]
            fmadd rC32, Ra3, rB2, rC32
               ldr rB2, [pB,#8*8]

            fmadd rC00, rA0, rb0, rC00
               ldr Ra3, [pA,#8*11]
            fmadd rC10, rA1, rb0, rC10
            fmadd rC20, rA2, rb0, rC20
            fmadd rC30, rA3, rb0, rC30
               ldr rb0, [pB,#8*9]
            fmadd rC01, rA0, rb1, rC01
            fmadd rC11, rA1, rb1, rC11
            fmadd rC21, rA2, rb1, rC21
            fmadd rC31, rA3, rb1, rC31
               ldr rb1, [pB,#8*10]
            fmadd rC02, rA0, rb2, rC02
      ldr rA0, [pA0]
            fmadd rC12, rA1, rb2, rC12
      ldr rA1, [pA0,#8]
            fmadd rC22, rA2, rb2, rC22
      ldr rA2, [pA0,#8*2]
            fmadd rC32, rA3, rb2, rC32
               ldr rb2, [pB,#8*11]

            fmadd rC00, ra0, rB0, rC00
      ldr rA3, [pA0,#8*3]
            fmadd rC10, ra1, rB0, rC10
            fmadd rC20, ra2, rB0, rC20
            fmadd rC30, ra3, rB0, rC30
      ldr rB0, [pB,#8*12]!
            fmadd rC01, ra0, rB1, rC01
            fmadd rC11, ra1, rB1, rC11
            fmadd rC21, ra2, rB1, rC21
            fmadd rC31, ra3, rB1, rC31
      ldr rB1, [pB,#8]
            fmadd rC02, ra0, rB2, rC02
      ldr ra0, [pA0,#8*4]
            fmadd rC12, ra1, rB2, rC12
      ldr ra1, [pA0,#8*5]
            fmadd rC22, ra2, rB2, rC22
      ldr ra2, [pA0,#8*6]
            fmadd rC32, ra3, rB2, rC32
      ldr ra3, [pA0,#8*7]

            fmadd rC00, Ra0, rb0, rC00
         str rC00, [pC]
            fmadd rC10, Ra1, rb0, rC10
         str rC10, [pC,#8]
            fmadd rC20, Ra2, rb0, rC20
         str rC20, [pC,#8*2]
            fmadd rC30, Ra3, rb0, rC30
         str rC30, [pC,#8*3]
            fmadd rC01, Ra0, rb1, rC01
         str rC01, [pC,#8*4]
            fmadd rC11, Ra1, rb1, rC11
         str rC11, [pC,#8*5]
            fmadd rC21, Ra2, rb1, rC21
         str rC21, [pC,#8*6]
            fmadd rC31, Ra3, rb1, rC31
         str rC31, [pC,#8*7]
            fmadd rC02, Ra0, rb2, rC02
         str rC02, [pC,#8*8]
            fmadd rC12, Ra1, rb2, rC12
         str rC12, [pC,#8*9]
            fmadd rC22, Ra2, rb2, rC22
         str rC22, [pC,#8*10]
            fmadd rC32, Ra3, rb2, rC32
         str rC32, [pC,#8*11]
         add pC, pC, #8*12
         subs JJ, JJ, #1
//         add pB, pB, #8*12
         mov pA, pA0
      b.ne NLOOP
      add pA0, pA0, K0, LSL #5
      subs nmu, nmu, #1
      mov pA, pA0
      mov pB, pB0
   b.ne MLOOP

DONE:
   ldr d8, [sp]
   ldr d9, [sp,#8]
   ldr d10, [sp,#2*8]
   ldr d11, [sp,#3*8]
   ldr d12, [sp,#4*8]
   ldr d13, [sp,#5*8]
   ldr d14, [sp,#6*8]
   ldr d15, [sp,#7*8]
   add sp, sp, #FSIZE      /* deallocate local frame */
   ret
