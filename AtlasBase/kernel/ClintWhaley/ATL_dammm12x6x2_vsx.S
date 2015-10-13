#include "atlas_asm.h"
#define pB0 r0
#define NNU r2
#define NMU r3
#define NN0 r4
#define KK  r5
#define pA  r6
#define pB  r7
#define pC  r8
#define pfA r9
#define pfB r10
#define incA r11
/* #define Kb0 r14 */
#define i16 r12
   #define i16_1 i16
#define i32 r15 
   #define i16_2 i32
#define i48 r16 
   #define i16_3 i48
#define i64 r17
   #define i16_4 i64
#define i80 r18 
   #define i16_5 i80
#define i96 r19 
   #define i16_6 i96
#define i112 r20
   #define i16_7 i112
#define i128 r22
   #define i16_8 i128
#define i144 r23
   #define i16_9 i144
#define i160 r24
   #define i16_10 i160
#define i176 r25
   #define i16_11 i176
#define i192 r26
   #define i16_12 i192
#define i208 r27
   #define i16_13 i208
#define i224 r28
   #define i16_14 i224
#define i240 r29
   #define i16_15 i240

#define rC00 v0
#define rC10 v1
#define rC20 v2
#define rC30 v3
#define rC40 v4
#define rC50 v5
#define rC01 v6
#define rC11 v7
#define rC21 v8
#define rC31 v9
#define rC41 v10
#define rC51 v11
#define rC02 v12
#define rC12 v13
#define rC22 v14
#define rC32 v15
#define rC42 v16
#define rC52 v17
#define rC03 v18
#define rC13 v19
#define rC23 v20
#define rC33 v21
#define rC43 v22
#define rC53 v23
#define rC04 v24
#define rC14 v25
#define rC24 v26
#define rC34 v27
#define rC44 v28
#define rC54 v29
#define rC05 v30
#define rC15 v31
#define rC25 v32
#define rC35 v33
#define rC45 v34
#define rC55 v35
#define rA0 v36
#define rA1 v37
#define rA2 v38
#define rA3 v39
#define rA4 v40
#define rA5 v41
#define ra0 v42
#define ra1 v43
#define ra2 v44
#define ra3 v45
#define ra4 v46
#define ra5 v47
#define rB0 v48
#define rB1 v49
#define rB2 v50
#define rB3 v51
#define rB4 v52
#define rB5 v53
#define rb0 v54
#define rb1 v55
#define rb2 v56
#define rb3 v57
#define rb4 v58
#define rb5 v59
/*
 * NOTE: 1st param at 96(r1), because caller must reserve space for 8 reg-passed
 * params in param passing space on stack.  ABI says parm passing space at
 * 32(r1) -> 32+64 -> 96(r1).
 * param save space can be written to by callee.
 */
/*
                     r3       r4     r5         r6         r7        r8
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                       r9          r10     96(r1)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */
.local MLOOP
.local NLOOP
.local KLOOP
.globl ATL_USERMM
ATL_USERMM:
/*
 * Must save & restore:
 *    r1&r2 -> in sframe tail
 *    r14-r31 -> 18*8 = 144 bytes (if used)
 *    f14-f31 -> vsr14-vsr31 -> 18*8 = 144 bytes
 *    v20-v31 -> vsr52-vsr63 -> 12*16 = 192 bytes
 */
#define ISZ 16*8
#define FSZ 144
#define VSZ 192
#define LSZ 0
#define MSZ 32      /* mandatory 32 bytes for stack frame tail */
#define VOFF (MSZ+LSZ)
#define FOFF (VOFF+VSZ)
#define IOFF (FOFF+FSZ)
#define FSIZE (MSZ+LSZ+VSZ+FSZ+ISZ)
/*
 * Allocate local stack frame in order to save & restore 
 */
   stdu r1, -FSIZE(r1)   /* allocate stack & store old stack ptr */
/*
 * Save iregs, UNUSED: r14
 */
   std r2, 24(r1)        /* TOC reg always at 24(sp) */
/*   std r14, IOFF(r1) */
   std r15, 8+IOFF(r1)
   std r16, 8*2+IOFF(r1)
   std r17, 8*3+IOFF(r1)
   std r18, 8*4+IOFF(r1)
   std r19, 8*5+IOFF(r1)
   std r20, 8*6+IOFF(r1)
   std r21, 8*7+IOFF(r1)
   std r22, 8*8+IOFF(r1)
   std r23, 8*9+IOFF(r1)
   std r24, 8*10+IOFF(r1)
   std r25, 8*11+IOFF(r1)
   std r26, 8*12+IOFF(r1)
   std r27, 8*13+IOFF(r1)
   std r28, 8*14+IOFF(r1)
   std r29, 8*15+IOFF(r1)
/*   std r30, 8*16+IOFF(r1) */
/*   std r31, 8*17+IOFF(r1) */
/*
 * Initialize iregs
 */
   li i16_1, 16*1
   li i16_2, 16*2
   li i16_3, 16*3
   li i16_4, 16*4
   li i16_5, 16*5
   li i16_6, 16*6
   li i16_7, 16*7
   li i16_8, 16*8
   li i16_9, 16*9
   li i16_10, 16*10
   li i16_11, 16*11
   li i16_12, 16*12
   li i16_13, 16*13
   li i16_14, 16*14
   li i16_15, 16*15
/*
 * Save f14-f31
 */
   stfd f14, FOFF(r1)
   stfd f15, 8+FOFF(r1)
   stfd f16, 16+FOFF(r1)
   stfd f17, 24+FOFF(r1)
   stfd f18, 32+FOFF(r1)
   stfd f19, 40+FOFF(r1)
   stfd f20, 48+FOFF(r1)
   stfd f21, 56+FOFF(r1)
   stfd f22, 64+FOFF(r1)
   stfd f23, 72+FOFF(r1)
   stfd f24, 80+FOFF(r1)
   stfd f25, 88+FOFF(r1)
   stfd f26, 96+FOFF(r1)
   stfd f27, 104+FOFF(r1)
   stfd f28, 112+FOFF(r1)
   stfd f29, 120+FOFF(r1)
   stfd f30, 128+FOFF(r1)
   stfd f31, 136+FOFF(r1)
/*
 * Save vec regs, use VSX so we don't rely on Altivec 
 */
   mr pB0, pB
   addi pB, r1, VOFF
   stxvd2x v52, 0, pB
   stxvd2x v53, i16_1, pB
   stxvd2x v54, i16_2, pB
   stxvd2x v55, i16_3, pB
   stxvd2x v56, i16_4, pB
   stxvd2x v57, i16_5, pB
   stxvd2x v58, i16_6, pB
   stxvd2x v59, i16_7, pB
   #if 0
   stxvd2x v60, i16_8, pB
   stxvd2x v61, i16_9, pB
   stxvd2x v62, i16_10, pB
   stxvd2x v63, i16_11, pB
   #endif
/*
 * Use 12x6 register block, incA = K*mu*sizeof = K*12*8 = K*3*32
 */
   add incA, KK, KK     /* incA = KK*2 */
   add incA, incA, KK  /* incA = 3*KK */
   sldi incA, incA, 5  /* incA = (3*KK)*32 */
   add incA, incA, i16_12
   srdi KK, KK, 1        /* KK = KK/KU = KK/2 */

   ALIGN16
   MLOOP:
      mr pB, pB0
      mr NNU, NN0
      NLOOP:
/*
 *       Need to peel to handle beta=0,-1, but I'm too lazy to do it yet
 */
         #ifdef BETA0
            xxlxor rC00, rC00, rC00
            xxlxor rC10, rC10, rC10
            xxlxor rC20, rC20, rC20
            xxlxor rC30, rC30, rC30
            xxlxor rC40, rC40, rC40
            xxlxor rC50, rC50, rC50
            xxlxor rC01, rC01, rC01
            xxlxor rC11, rC11, rC11
            xxlxor rC21, rC21, rC21
            xxlxor rC31, rC31, rC31
            xxlxor rC41, rC41, rC41
            xxlxor rC51, rC51, rC51
            xxlxor rC02, rC02, rC02
            xxlxor rC12, rC12, rC12
            xxlxor rC22, rC22, rC22
            xxlxor rC32, rC32, rC32
            xxlxor rC42, rC42, rC42
            xxlxor rC52, rC52, rC52
            xxlxor rC03, rC03, rC03
            xxlxor rC13, rC13, rC13
            xxlxor rC23, rC23, rC23
            xxlxor rC33, rC33, rC33
            xxlxor rC43, rC43, rC43
            xxlxor rC53, rC53, rC53
            xxlxor rC04, rC04, rC04
            xxlxor rC14, rC14, rC14
            xxlxor rC24, rC24, rC24
            xxlxor rC34, rC34, rC34
            xxlxor rC44, rC44, rC44
            xxlxor rC54, rC54, rC54
            xxlxor rC05, rC05, rC05
            xxlxor rC15, rC15, rC15
            xxlxor rC25, rC25, rC25
            xxlxor rC35, rC35, rC35
            xxlxor rC45, rC45, rC45
            xxlxor rC55, rC55, rC55
         #else
            lxvd2x rC00, 0, pC
            lxvd2x rC10, i16, pC
            lxvd2x rC20, i16_2, pC
            lxvd2x rC30, i16_3, pC
            lxvd2x rC40, i16_4, pC
            lxvd2x rC50, i16_5, pC
            lxvd2x rC01, i16_6, pC
            lxvd2x rC11, i16_7, pC
            lxvd2x rC21, i16_8, pC
            lxvd2x rC31, i16_9, pC
            lxvd2x rC41, i16_10, pC
            lxvd2x rC51, i16_11, pC
            lxvd2x rC02, i16_12, pC
            lxvd2x rC12, i16_13, pC
            lxvd2x rC22, i16_14, pC
            lxvd2x rC32, i16_15, pC
            addi pC, pC, 256
            lxvd2x rC42, 0, pC
            lxvd2x rC52, i16_1, pC
            lxvd2x rC03, i16_2, pC
            lxvd2x rC13, i16_3, pC
            lxvd2x rC23, i16_4, pC
            lxvd2x rC33, i16_5, pC
            lxvd2x rC43, i16_6, pC
            lxvd2x rC53, i16_7, pC
            lxvd2x rC04, i16_8, pC
            lxvd2x rC14, i16_9, pC
            lxvd2x rC24, i16_10, pC
            lxvd2x rC34, i16_11, pC
            lxvd2x rC44, i16_12, pC
            lxvd2x rC54, i16_13, pC
            lxvd2x rC05, i16_14, pC
            lxvd2x rC15, i16_15, pC
            addi pC, pC, 256
            lxvd2x rC25, 0, pC
            lxvd2x rC35, i16_1, pC
            lxvd2x rC45, i16_2, pC
            lxvd2x rC55, i16_3, pC
            addi pC, pC, -512
            #ifndef BETA1
               xvnegdp rC00, rC00
               xvnegdp rC10, rC10
               xvnegdp rC20, rC20
               xvnegdp rC30, rC30
               xvnegdp rC40, rC40
               xvnegdp rC50, rC50
               xvnegdp rC01, rC01
               xvnegdp rC11, rC11
               xvnegdp rC21, rC21
               xvnegdp rC31, rC31
               xvnegdp rC41, rC41
               xvnegdp rC51, rC51
               xvnegdp rC02, rC02
               xvnegdp rC12, rC12
               xvnegdp rC22, rC22
               xvnegdp rC32, rC32
               xvnegdp rC42, rC42
               xvnegdp rC52, rC52
               xvnegdp rC03, rC03
               xvnegdp rC13, rC13
               xvnegdp rC23, rC23
               xvnegdp rC33, rC33
               xvnegdp rC43, rC43
               xvnegdp rC53, rC53
               xvnegdp rC04, rC04
               xvnegdp rC14, rC14
               xvnegdp rC24, rC24
               xvnegdp rC34, rC34
               xvnegdp rC44, rC44
               xvnegdp rC54, rC54
               xvnegdp rC05, rC05
               xvnegdp rC15, rC15
               xvnegdp rC25, rC25
               xvnegdp rC35, rC35
               xvnegdp rC45, rC45
               xvnegdp rC55, rC55
            #endif
         #endif
         lxvd2x rA0, 0, pA
         lxvd2x rA1, i16_1, pA
         lxvd2x rA2, i16_2, pA
         lxvd2x rA3, i16_3, pA
         lxvd2x rA4, i16_4, pA
         lxvd2x rA5, i16_5, pA

         lxvd2x ra0, i16_6, pA
         lxvd2x ra1, i16_7, pA
         lxvd2x ra2, i16_8, pA
         lxvd2x ra3, i16_9, pA
         lxvd2x ra4, i16_10, pA
         lxvd2x ra5, i16_11, pA
         add pA, pA, i16_12
         lxvd2x rb0, 0, pB
            xxspltd rB0, rb0, 0
            xxspltd rB1, rb0, 1
         lxvd2x rb1, i16_1, pB
            xxspltd rB2, rb1, 0
            xxspltd rB3, rb1, 1
         lxvd2x rb2, i16_2, pB
            xxspltd rB4, rb2, 0
            xxspltd rB5, rb2, 1
         lxvd2x rb3, i16_3, pB
         lxvd2x rb4, i16_4, pB
         lxvd2x rb5, i16_5, pB
         lxvd2x rb0, i16_6, pB
         lxvd2x rb1, i16_7, pB
         lxvd2x rb2, i16_8, pB
         add pB, pB, i16_9

         mtctr KK
         KLOOP:
            xvmaddadp rC00, rA0, rB0
            xvmaddadp rC10, rA1, rB0
            xvmaddadp rC20, rA2, rB0
            xvmaddadp rC30, rA3, rB0
            xvmaddadp rC40, rA4, rB0
            xvmaddadp rC50, rA5, rB0
               xxspltd rB0, rb3, 0
 
            xvmaddadp rC01, rA0, rB1
            xvmaddadp rC11, rA1, rB1
            xvmaddadp rC21, rA2, rB1
            xvmaddadp rC31, rA3, rB1
            xvmaddadp rC41, rA4, rB1
            xvmaddadp rC51, rA5, rB1
               xxspltd rB1, rb3, 1
               lxvd2x rb3, 0, pB

            xvmaddadp rC02, rA0, rB2
            xvmaddadp rC12, rA1, rB2
            xvmaddadp rC22, rA2, rB2
            xvmaddadp rC32, rA3, rB2
            xvmaddadp rC42, rA4, rB2
            xvmaddadp rC52, rA5, rB2
               xxspltd rB2, rb4, 0
 
            xvmaddadp rC03, rA0, rB3
            xvmaddadp rC13, rA1, rB3
            xvmaddadp rC23, rA2, rB3
            xvmaddadp rC33, rA3, rB3
            xvmaddadp rC43, rA4, rB3
            xvmaddadp rC53, rA5, rB3
               xxspltd rB3, rb4, 1
               lxvd2x rb4, i16_1, pB

            xvmaddadp rC04, rA0, rB4
            xvmaddadp rC14, rA1, rB4
            xvmaddadp rC24, rA2, rB4
            xvmaddadp rC34, rA3, rB4
            xvmaddadp rC44, rA4, rB4
            xvmaddadp rC54, rA5, rB4
               xxspltd rB4, rb5, 0
 
            xvmaddadp rC05, rA0, rB5
               lxvd2x rA0, 0, pA
            xvmaddadp rC15, rA1, rB5
               lxvd2x rA1, i16_1, pA
            xvmaddadp rC25, rA2, rB5
               lxvd2x rA2, i16_2, pA
            xvmaddadp rC35, rA3, rB5
               lxvd2x rA3, i16_3, pA
            xvmaddadp rC45, rA4, rB5
                lxvd2x rA4, i16_4, pA
            xvmaddadp rC55, rA5, rB5
               lxvd2x rA5, i16_5, pA
               xxspltd rB5, rb5, 1
               lxvd2x rb5, i16_2, pB


            xvmaddadp rC00, ra0, rB0
            xvmaddadp rC10, ra1, rB0
            xvmaddadp rC20, ra2, rB0
            xvmaddadp rC30, ra3, rB0
            xvmaddadp rC40, ra4, rB0
            xvmaddadp rC50, ra5, rB0
               xxspltd rB0, rb0, 0
 
            xvmaddadp rC01, ra0, rB1
            xvmaddadp rC11, ra1, rB1
            xvmaddadp rC21, ra2, rB1
            xvmaddadp rC31, ra3, rB1
            xvmaddadp rC41, ra4, rB1
            xvmaddadp rC51, ra5, rB1
               xxspltd rB1, rb0, 1
               lxvd2x rb0, i16_3, pB
 
            xvmaddadp rC02, ra0, rB2
            xvmaddadp rC12, ra1, rB2
            xvmaddadp rC22, ra2, rB2
            xvmaddadp rC32, ra3, rB2
            xvmaddadp rC42, ra4, rB2
            xvmaddadp rC52, ra5, rB2
               xxspltd rB2, rb1, 0

            xvmaddadp rC03, ra0, rB3
            xvmaddadp rC13, ra1, rB3
            xvmaddadp rC23, ra2, rB3
            xvmaddadp rC33, ra3, rB3
            xvmaddadp rC43, ra4, rB3
            xvmaddadp rC53, ra5, rB3
               xxspltd rB3, rb1, 1
               lxvd2x rb1, i16_4, pB

            xvmaddadp rC04, ra0, rB4
            xvmaddadp rC14, ra1, rB4
            xvmaddadp rC24, ra2, rB4
            xvmaddadp rC34, ra3, rB4
            xvmaddadp rC44, ra4, rB4
            xvmaddadp rC54, ra5, rB4
               xxspltd rB4, rb2, 0

            xvmaddadp rC05, ra0, rB5
               lxvd2x ra0, i16_6, pA
            xvmaddadp rC15, ra1, rB5
               lxvd2x ra1, i16_7, pA
            xvmaddadp rC25, ra2, rB5
               lxvd2x ra2, i16_8, pA
            xvmaddadp rC35, ra3, rB5
               lxvd2x ra3, i16_9, pA
            xvmaddadp rC45, ra4, rB5
               lxvd2x ra4, i16_10, pA
            xvmaddadp rC55, ra5, rB5
               lxvd2x ra5, i16_11, pA
               xxspltd rB5, rb2, 1
               lxvd2x rb2, i16_5, pB
            add pB, pB, i16_6
            add pA, pA, i16_12
         bdnz+ KLOOP
         stxvd2x rC00, 0, pC
         stxvd2x rC10, i16_1, pC
         stxvd2x rC20, i16_2, pC
         stxvd2x rC30, i16_3, pC
         stxvd2x rC40, i16_4, pC
         stxvd2x rC50, i16_5, pC

         stxvd2x rC01, i16_6, pC
         stxvd2x rC11, i16_7, pC
         stxvd2x rC21, i16_8, pC
         stxvd2x rC31, i16_9, pC
         stxvd2x rC41, i16_10, pC
         stxvd2x rC51, i16_11, pC

         stxvd2x rC02, i16_12, pC
         stxvd2x rC12, i16_13, pC
         stxvd2x rC22, i16_14, pC
         stxvd2x rC32, i16_15, pC
         addi   pC, pC, 256
         stxvd2x rC42, 0, pC
         stxvd2x rC52, i16_1, pC

         stxvd2x rC03, i16_2, pC
         stxvd2x rC13, i16_3, pC
         stxvd2x rC23, i16_4, pC
         stxvd2x rC33, i16_5, pC
         stxvd2x rC43, i16_6, pC
         stxvd2x rC53, i16_7, pC

         stxvd2x rC04, i16_8, pC
         stxvd2x rC14, i16_9, pC
         stxvd2x rC24, i16_10, pC
         stxvd2x rC34, i16_11, pC
         stxvd2x rC44, i16_12, pC
         stxvd2x rC54, i16_13, pC

         stxvd2x rC05, i16_14, pC
         stxvd2x rC15, i16_15, pC
         addi   pC, pC, 256
         stxvd2x rC25, 0, pC
         stxvd2x rC35, i16_1, pC
         stxvd2x rC45, i16_2, pC
         stxvd2x rC55, i16_3, pC
         add    pC, pC, i16_4
         subf pB, i16_9, pB
         subf pA, incA, pA
         addic. NNU, NNU, -1
      bne+ NLOOP
      add pA, pA, incA
      subf pA, i16_12, pA
      addic. NMU, NMU, -1
   bne+ MLOOP
/*
 * Restore fpregs
 */
   lfd f14, FOFF(r1)
   lfd f15, 8+FOFF(r1)
   lfd f16, 16+FOFF(r1)
   lfd f17, 24+FOFF(r1)
   lfd f18, 32+FOFF(r1)
   lfd f19, 40+FOFF(r1)
   lfd f20, 48+FOFF(r1)
   lfd f21, 56+FOFF(r1)
   lfd f22, 64+FOFF(r1)
   lfd f23, 72+FOFF(r1)
   lfd f24, 80+FOFF(r1)
   lfd f25, 88+FOFF(r1)
   lfd f26, 96+FOFF(r1)
   lfd f27, 104+FOFF(r1)
   lfd f28, 112+FOFF(r1)
   lfd f29, 120+FOFF(r1)
   lfd f30, 128+FOFF(r1)
   lfd f31, 136+FOFF(r1)
/*
 * Restore vec regs 
 */
   addi pB, r1, VOFF
   lxvd2x v52, 0, pB
   lxvd2x v53, i16_1, pB
   lxvd2x v54, i16_2, pB
   lxvd2x v55, i16_3, pB
   lxvd2x v56, i16_4, pB
   lxvd2x v57, i16_5, pB
   lxvd2x v58, i16_6, pB
   lxvd2x v59, i16_7, pB
   #if 0
   lxvd2x v60, i16_8, pB
   lxvd2x v61, i16_9, pB
   lxvd2x v62, i16_10, pB
   lxvd2x v63, i16_11, pB
   #endif
/*
 * Restore iregs
 */
   ld r2, 24(r1)        /* TOC reg always at 24(sp) */
/*   ld r14, IOFF(r1) */
   ld r15, 8+IOFF(r1)
   ld r16, 8*2+IOFF(r1)
   ld r17, 8*3+IOFF(r1)
   ld r18, 8*4+IOFF(r1)
   ld r19, 8*5+IOFF(r1)
   ld r20, 8*6+IOFF(r1)
   ld r21, 8*7+IOFF(r1)
   ld r22, 8*8+IOFF(r1)
   ld r23, 8*9+IOFF(r1)
   ld r24, 8*10+IOFF(r1)
   ld r25, 8*11+IOFF(r1)
   ld r26, 8*12+IOFF(r1)
   ld r27, 8*13+IOFF(r1)
   ld r28, 8*14+IOFF(r1)
   ld r29, 8*15+IOFF(r1)
/*   ld r30, 8*16+IOFF(r1) */
/*   ld r31, 8*17+IOFF(r1) */
/*
 * Dealloc stack frame & return
 */
   addi r1, r1, FSIZE
   blr
