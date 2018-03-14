#define rA0 d0
#define rA1 d1
#define rA2 d2
#define rA3 d3
#define ra0 d4
#define ra1 d5
#define ra2 d6
#define ra3 d7
#define rB0 d8
#define rB1 d9
#define rB2 d10
#define rB3 d11
#define rb0 d12
#define rb1 d13
#define rb2 d14
#define rb3 d15
#define rC00 d16
#define rC10 d17
#define rC20 d18
#define rC30 d19
#define rC01 d20
#define rC11 d21
#define rC21 d22
#define rC31 d23
#define rC02 d24
#define rC12 d25
#define rC22 d26
#define rC32 d27
#define rC03 d28
#define rC13 d29
#define rC23 d30
#define rC33 d31
#define nmu 	r0
#define nnu 	r1
#define KK  	r2
#define pA 	r3
#define pB 	r4
#define pC 	r5
#define nnu0	r6
#define pfB	r7
#define pfA	r8
#define pfC  	r9
#define KK0	r10
#define pA0     r11
#define pB0     r12
#define Kp      r14 
#ifndef KB
   #define KB 0
#endif
/*
                    r0       r1      r2         r3          0         4
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                      8           12          16
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);

 */
#include "atlas_asm.h"
.code 32
.fpu vfpv3
#ifdef ATL_ARM_HARDFP
.eabi_attribute 28, 1
#endif
.text
.align 2
.globl ATL_asmdecor(ATL_USERMM)
.type  ATL_asmdecor(ATL_USERMM), %function
ATL_asmdecor(ATL_USERMM):
   fstmDBd SP!, {d8-d15}    /* save all fp regs */
   stmDB SP!, {r4-r11,r14}  /* save all iregs */
   #define FSIZE 9*4+8*8
   add r12, SP, #FSIZE
   ldmIA r12, {pB, pC, pfB, pfA, pfC}

   mov nnu0, nnu
   mov pA0, pA
   mov pB0, pB
/*
 * If K < 6, use rolled loop 
 */
#if KB > 0 && KB < 6
   b   SHORTK
#elif KB == 0
   cmp KK, #6
   bLT SHORTK
#endif
/*
 * Peel off first and last 2 iterations, and any odd iteration, store in Kp
 * NOTE: must have jumped before reaching this point if K < 6
 */
   and  Kp, KK, #1
   add  Kp, Kp, #4
   sub KK, KK, Kp
   mov KK0, KK
   MLOOP:
      fldd rA0, [pA]
      fldd rB0, [pB]
      NLOOP:
/* 
 *       K=1 iteration peeled to initialize C
 */
         fmuld rC00, rA0, rB0
           fldd rA1, [pA,#8]
         fmuld rC10, rA1, rB0
           fldd rA2, [pA,#16]
         fmuld rC20, rA2, rB0
           fldd rA3, [pA,#24]
         fmuld rC30, rA3, rB0
           fldd rB1, [pB,#8]
         fmuld rC01, rA0, rB1
           fldd rB2, [pB,#16]
         fmuld rC11, rA1, rB1
           fldd rB3, [pB,#24]
         fmuld rC21, rA2, rB1
           fldd rb0, [pB,#32]
         fmuld rC31, rA3, rB1
           fldd ra0, [pA,#32]
         fmuld rC02, rA0, rB2
           fldd ra1, [pA,#40]
         fmuld rC12, rA1, rB2
           fldd ra2, [pA,#48]
         fmuld rC22, rA2, rB2
           fldd ra3, [pA,#56]
         fmuld rC32, rA3, rB2
           fldd rb1, [pB,#40]
         fmuld rC03, rA0, rB3
           fldd rb2, [pB,#48]
         fmuld rC13, rA1, rB3
           fldd rb3, [pB,#56]
         fmuld rC23, rA2, rB3
           fldd rA0, [pA,#64]
         fmuld rC33, rA3, rB3
           fldd rB0, [pB,#64]
/*
 *       K=2 peeled to complete fetch of A/B, prefetch C
 */
         fmacd rC00, ra0, rb0
           fldd rA1, [pA,#72]
         fmacd rC10, ra1, rb0
           fldd rA2, [pA,#80]
         fmacd rC20, ra2, rb0
           fldd rA3, [pA,#88]
         fmacd rC30, ra3, rb0
           fldd rB1, [pB,#72]
         fmacd rC01, ra0, rb1
           fldd rB2, [pB,#80]
         fmacd rC11, ra1, rb1
           fldd rB3, [pB,#88]
         fmacd rC21, ra2, rb1
           fldd rb0, [pB,#96]
         fmacd rC31, ra3, rb1
           fldd rb1, [pB,#104]
         fmacd rC02, ra0, rb2
/*            pld [pC,#64] */
         fmacd rC12, ra1, rb2
            add pB, pB, #64
         fmacd rC22, ra2, rb2
            add pA, pA, #64
         fmacd rC32, ra3, rb2
           fldd rb2, [pB,#112-64]
         fmacd rC03, ra0, rb3
           fldd ra0, [pA,#96-64]
         fmacd rC13, ra1, rb3
           fldd ra1, [pA,#104-64]
         fmacd rC23, ra2, rb3
           fldd ra2, [pA,#112-64]
         fmacd rC33, ra3, rb3
           fldd ra3, [pA,#120-64]
#if KB == 0
   #if defined(MB) && MB > 0
      #if MB > 100
         #define PFO 100*4*8
      #else
         #define PFO MB*4*8
      #endif
   #elif defined(NB) && NB > 0
      #if NB > 100
         #define PFO 100*4*8
      #else
         #define PFO NB*4*8
      #endif
   #else
      #define PFO 60*4*8
   #endif
#else
   #if KB > 100
      #define PFO 100*4*8
   #else
      #define PFO KB*4*8
   #endif
#endif
         KLOOP:
            subs KK, KK, #2
            fmacd rC00, rA0, rB0
               fldd rb3, [pB, #56]
            fmacd rC10, rA1, rB0
               pld [pA, #128]
               add   pB, pB, #64
            fmacd rC20, rA2, rB0
               pld [pB, #64]
            add   pA, pA, #64
            fmacd rC30, rA3, rB0
               fldd rB0, [pB]
      
            fmacd rC01, rA0, rB1
               pld [pB,#PFO]
            fmacd rC11, rA1, rB1
               pld [pB,#PFO+64]
            fmacd rC21, rA2, rB1
            fmacd rC31, rA3, rB1
               fldd rB1, [pB,#8]
      
            fmacd rC02, rA0, rB2
               pld [pB,#PFO+128]
            fmacd rC12, rA1, rB2
               pld [pB,#PFO+192]
            fmacd rC22, rA2, rB2
               pld [pB,#PFO+256]
            fmacd rC32, rA3, rB2
               fldd rB2, [pB,#16]
      
            fmacd rC03, rA0, rB3
               fldd rA0, [pA]
            fmacd rC13, rA1, rB3
               fldd rA1, [pA,#8]
            fmacd rC23, rA2, rB3
               fldd rA2, [pA,#16]
            fmacd rC33, rA3, rB3
               fldd rA3, [pA,#24]
      
            fmacd rC00, ra0, rb0
               fldd rB3, [pB,#24]
            fmacd rC10, ra1, rb0
               pld [pB,#PFO+320]
            fmacd rC20, ra2, rb0
               pld [pB,#PFO+384]
            fmacd rC30, ra3, rb0
               fldd rb0, [pB,#32]
      
            fmacd rC01, ra0, rb1
               pld [pB,#PFO+448]
            fmacd rC11, ra1, rb1
               pld [pB,#PFO+512]
            fmacd rC21, ra2, rb1
               pld [pB,#PFO+576]
            fmacd rC31, ra3, rb1
               fldd rb1, [pB, #40]
      
            fmacd rC02, ra0, rb2
               pld [pB,#PFO+640]
            fmacd rC12, ra1, rb2
               pld [pB,#PFO+704]
            fmacd rC22, ra2, rb2
               pld [pB,#PFO+768]
            fmacd rC32, ra3, rb2
               fldd rb2, [pB, #48]
      
            fmacd rC03, ra0, rb3
               fldd ra0, [pA,#32]
            fmacd rC13, ra1, rb3
               fldd ra1, [pA, #40]
            fmacd rC23, ra2, rb3
               fldd ra2, [pA, #48]
            fmacd rC33, ra3, rb3
               fldd ra3, [pA, #56]
         bne KLOOP
         KDONE:
         #if KB > 0 && (KB/2)*2 != KB
            b ODDK
         #elif KB == 0
            tst Kp, #1
            bNE ODDK
         #endif
/*
 *       Drain fetch pipeline during last 2 K its; fetch C for BETA!=0
 */
            fmacd rC00, rA0, rB0
               fldd rb3, [pB, #56]
            fmacd rC10, rA1, rB0
               add   pB, pB, #64
            fmacd rC20, rA2, rB0
            add   pA, pA, #64
            fmacd rC30, rA3, rB0
            #ifndef BETA0
               fldd rB0, [pC]
            #endif
      
            fmacd rC01, rA0, rB1
               pld [pfA]
            fmacd rC11, rA1, rB1
               pld [pfA,#64]
            fmacd rC21, rA2, rB1
               pld [pfA,#128]
            fmacd rC31, rA3, rB1
            #ifndef BETA0
               fldd rB1, [pC,#8]
            #endif
      
            fmacd rC02, rA0, rB2
            fmacd rC12, rA1, rB2
            fmacd rC22, rA2, rB2
                add pfA, pfA, #4*4*8   /* pfB += MU*NU*sizeof */
            fmacd rC32, rA3, rB2
            #ifndef BETA0
               fldd rB2, [pC,#16]
            #endif
      
            fmacd rC03, rA0, rB3
            #ifndef BETA0
               fldd rA0, [pC,#24]
            #endif
            fmacd rC13, rA1, rB3
            #ifndef BETA0
               fldd rA1, [pC,#32]
            #endif
            fmacd rC23, rA2, rB3
            #ifndef BETA0
               fldd rA2, [pC,#40]
            #endif
            fmacd rC33, rA3, rB3
            #ifndef BETA0
               fldd rA3, [pC,#48]
            #endif
      
            fmacd rC00, ra0, rb0
            #ifndef BETA0
               fldd rB3, [pC,#56]
            #endif
            fmacd rC10, ra1, rb0
            fmacd rC20, ra2, rb0
            fmacd rC30, ra3, rb0
            #ifndef BETA0
               fldd rb0, [pC,#64]
            #endif
      
            fmacd rC01, ra0, rb1
                pld [pfB]
            fmacd rC11, ra1, rb1
                pld [pfB,#64]
            fmacd rC21, ra2, rb1
                pld [pfB,#128]
            fmacd rC31, ra3, rb1
            #ifndef BETA0
               fldd rb1, [pC,#72]
            #endif
      
            fmacd rC02, ra0, rb2
               pld [pC,#128]
            fmacd rC12, ra1, rb2
               pld [pC,#192]
            fmacd rC22, ra2, rb2
                add pfB, pfB, #4*4*8   /* pfB += MU*NU*sizeof */
            fmacd rC32, ra3, rb2
            #ifndef BETA0
               fldd rb2, [pC,#80]
            #endif
      
            fmacd rC03, ra0, rb3
            #ifndef BETA0
               fldd ra0, [pC,#88]
            #endif
            fmacd rC13, ra1, rb3
            #ifndef BETA0
               fldd ra1, [pC,#96]
            #endif
            fmacd rC23, ra2, rb3
            #ifndef BETA0
               fldd ra2, [pC,#104]
            #endif
            fmacd rC33, ra3, rb3
            #ifndef BETA0
               fldd ra3, [pC,#112]
            #endif

         #ifndef BETA0
            #ifdef BETA1
               #define FOPC faddd
            #else
               #define FOPC fsubd
            #endif
            FOPC rC00, rC00, rB0
               fldd rb3, [pC,#120]
            FOPC rC10, rC10, rB1
               subs nnu, nnu, #1
            FOPC rC20, rC20, rB2
               flddNE rB0, [pB]
            FOPC rC30, rC30, rA0
               flddNE rA0, [pA0]
            FOPC rC01, rC01, rA1
               mov pA, pA0
            FOPC rC11, rC11, rA2
               mov KK, KK0
            FOPC rC21, rC21, rA3
                pld [pA0,#1920]
            FOPC rC31, rC31, rB3

            FOPC rC02, rC02, rb0
                pld [pB0,#1920]
            FOPC rC12, rC12, rb1
            FOPC rC22, rC22, rb2
            FOPC rC32, rC32, ra0
            FOPC rC03, rC03, ra1
            FOPC rC13, rC13, ra2
            FOPC rC23, rC23, ra3
                pld [pB,#128]
            FOPC rC33, rC33, rb3
         #endif
         #ifdef BETA0
            subs nnu, nnu, #1
         #endif
         fstmIAd pC!, {rC00-rC33}
         #ifdef BETA0
         mov KK, KK0
            flddNE rA0, [pA0]
         mov pA, pA0
            flddNE rB0, [pB]
         #endif
      bne NLOOP
      mov nnu, nnu0
      add pA0, pA0, KK0, LSL #5
      add pA0, pA0, Kp, LSL #5
      mov pA, pA0
      mov pB, pB0
      subs nmu, nmu, #1
   bne MLOOP

DONE:
   ldmIA SP!, {r4-r11,r14}  /* restore all iregs */
   fldmIAd SP!, {d8-d15}    /* restore all fpregs */
   bx lr
/*
 * When we have to drain 3 iterations, rather than 2
 */
ODDK:
            fmacd rC00, rA0, rB0
               fldd rb3, [pB, #56]
            fmacd rC10, rA1, rB0
               add   pB, pB, #64
            fmacd rC20, rA2, rB0
            add   pA, pA, #64
            fmacd rC30, rA3, rB0
               fldd rB0, [pB]
      
            fmacd rC01, rA0, rB1
               pld [pC,#128]
            fmacd rC11, rA1, rB1
               pld [pC,#192]
            fmacd rC21, rA2, rB1
               pld [pC,#256]
            fmacd rC31, rA3, rB1
               fldd rB1, [pB,#8]
      
            fmacd rC02, rA0, rB2
                pld [pfB]
            fmacd rC12, rA1, rB2
                pld [pfB,#64]
            fmacd rC22, rA2, rB2
                pld [pfB,#128]
            fmacd rC32, rA3, rB2
               fldd rB2, [pB,#16]
      
            fmacd rC03, rA0, rB3
               fldd rA0, [pA]
            fmacd rC13, rA1, rB3
               fldd rA1, [pA,#8]
            fmacd rC23, rA2, rB3
               fldd rA2, [pA,#16]
            fmacd rC33, rA3, rB3
               fldd rA3, [pA,#24]
      
            fmacd rC00, ra0, rb0
               fldd rB3, [pB,#24]
            fmacd rC10, ra1, rb0
                pld [pfA]
            fmacd rC20, ra2, rb0
                pld [pfA,#64]
            fmacd rC30, ra3, rb0
            #ifndef BETA0
               fldd rb0, [pC]
            #endif
      
            fmacd rC01, ra0, rb1
                pld [pfA,#128]
            fmacd rC11, ra1, rb1
               add pB, pB, #32
            fmacd rC21, ra2, rb1
               add pA, pA, #32
            fmacd rC31, ra3, rb1
            #ifndef BETA0
               fldd rb1, [pC,#8]
            #endif
      
            fmacd rC02, ra0, rb2
                add pfB, pfB, #4*4*8   /* pfB += MU*NU*sizeof */
            fmacd rC12, ra1, rb2
                add pfA, pfA, #4*4*8   /* pfA += MU*NU*sizeof */
            fmacd rC22, ra2, rb2
            fmacd rC32, ra3, rb2
            #ifndef BETA0
               fldd rb2, [pC,#16]
            #endif
      
            fmacd rC03, ra0, rb3
            #ifndef BETA0
               fldd ra0, [pC,#24]
            #endif
            fmacd rC13, ra1, rb3
            #ifndef BETA0
               fldd ra1, [pC,#32]
            #endif
            fmacd rC23, ra2, rb3
            #ifndef BETA0
               fldd ra2, [pC,#40]
            #endif
            fmacd rC33, ra3, rb3
            #ifndef BETA0
               fldd ra3, [pC,#48]
            #endif

            fmacd rC00, rA0, rB0
            #ifndef BETA0
               fldd rb3, [pC,#56]
            #endif
            fmacd rC10, rA1, rB0
            fmacd rC20, rA2, rB0
            fmacd rC30, rA3, rB0
            #ifndef BETA0
               fldd rB0, [pC,#64]
            #endif
      
            fmacd rC01, rA0, rB1
            fmacd rC11, rA1, rB1
            fmacd rC21, rA2, rB1
            fmacd rC31, rA3, rB1
            #ifndef BETA0
               fldd rB1, [pC,#72]
            #endif
      
            fmacd rC02, rA0, rB2
            fmacd rC12, rA1, rB2
            fmacd rC22, rA2, rB2
            fmacd rC32, rA3, rB2
            #ifndef BETA0
               fldd rB2, [pC,#80]
            #endif
      
            fmacd rC03, rA0, rB3
            #ifndef BETA0
               fldd rA0, [pC,#88]
            #endif
            fmacd rC13, rA1, rB3
            #ifndef BETA0
               fldd rA1, [pC,#96]
            #endif
            fmacd rC23, rA2, rB3
            #ifndef BETA0
               fldd rA2, [pC,#104]
            #endif
            fmacd rC33, rA3, rB3
            #ifndef BETA0
               fldd rA3, [pC,#112]
            #endif
            #ifndef BETA0
               FOPC rC00, rC00, rb0
                 fldd rB3, [pC,#120]
               FOPC rC10, rC10, rb1
                  subs nnu, nnu, #1
               FOPC rC20, rC20, rb2
               FOPC rC30, rC30, ra0

               FOPC rC01, rC01, ra1
               FOPC rC11, rC11, ra2
                  mov KK, KK0
               FOPC rC21, rC21, ra3
                  mov pA, pA0
               FOPC rC31, rC31, rb3

               FOPC rC02, rC02, rB0
                  flddNE rB0, [pB]
               FOPC rC12, rC12, rB1
               FOPC rC22, rC22, rB2
               FOPC rC32, rC32, rA0
                  flddNE rA0, [pA0]

               FOPC rC03, rC03, rA1
               FOPC rC13, rC13, rA2
               FOPC rC23, rC23, rA3
               FOPC rC33, rC33, rB3
            #endif
         #ifdef BETA0
            subs nnu, nnu, #1
         #endif
         fstmIAd pC!, {rC00-rC33}
         #ifdef BETA0
         mov KK, KK0
            flddNE rA0, [pA0]
         mov pA, pA0
            flddNE rB0, [pB]
         #endif
      bne NLOOP
      mov nnu, nnu0
      add pA0, pA0, KK0, LSL #5
      add pA0, pA0, Kp, LSL #5
      mov pA, pA0
      mov pB, pB0
      subs nmu, nmu, #1
   bne MLOOP
   b DONE
/*
 * Special case for short-K just use simplist rolled loop
 */
#if KB == 0 || KB < 6
SHORTK:
   mov KK0, KK
   mov pfC, #0
   MLOOPK:
      NLOOPK:
         fldmIAd pA, {rA0-rA3}
         fldmIAd pB, {rB0-rB2}
         #ifdef BETA0
            fmdhr rC00, pfC
            fmdlr rC00, pfC
            fcpyd rC10, rC00
            fcpyd rC20, rC00
            fcpyd rC30, rC00
            fcpyd rC01, rC00
            fcpyd rC11, rC00
            fcpyd rC21, rC00
            fcpyd rC31, rC00
            fcpyd rC02, rC00
            fcpyd rC12, rC00
            fcpyd rC22, rC00
            fcpyd rC32, rC00
            fcpyd rC03, rC00
            fcpyd rC13, rC00
            fcpyd rC23, rC00
            fcpyd rC33, rC00
         #else
            fldmIAd pC, {rC00-rC33}
            #ifndef BETA1
               fnegd rC00, rC00
               fnegd rC10, rC10
               fnegd rC20, rC20
               fnegd rC30, rC30
               fnegd rC01, rC01
               fnegd rC11, rC11
               fnegd rC21, rC21
               fnegd rC31, rC31
               fnegd rC02, rC02
               fnegd rC12, rC12
               fnegd rC22, rC22
               fnegd rC32, rC32
               fnegd rC03, rC03
               fnegd rC13, rC13
               fnegd rC23, rC23
               fnegd rC33, rC33
            #endif
         #endif
         KLOOPK:
            subs KK, KK, #1
            fmacd rC00, rA0, rB0
               fldd rB3, [pB,#24]
            fmacd rC10, rA1, rB0
               add pB, pB, #32
            fmacd rC20, rA2, rB0
               add pA, pA, #32
            fmacd rC30, rA3, rB0
               flddNE rB0, [pB]

            fmacd rC01, rA0, rB1
            fmacd rC11, rA1, rB1
            fmacd rC21, rA2, rB1
            fmacd rC31, rA3, rB1
               flddNE rB1, [pB,#8]

            fmacd rC02, rA0, rB2
            fmacd rC12, rA1, rB2
            fmacd rC22, rA2, rB2
            fmacd rC32, rA3, rB2
               flddNE rB2, [pB,#16]

            fmacd rC03, rA0, rB3
               flddNE rA0, [pA]
            fmacd rC13, rA1, rB3
               flddNE rA1, [pA,#8]
            fmacd rC23, rA2, rB3
               flddNE rA2, [pA,#16]
            fmacd rC33, rA3, rB3
               flddNE rA3, [pA,#24]
         bne KLOOPK
         fstmIAd pC!, {rC00-rC33}
         mov KK, KK0
         mov pA, pA0
         subs nnu, nnu, #1
      bne NLOOPK
      mov nnu, nnu0
      add pA0, pA0, KK0, LSL #5   /* pA0 += K*MU*sizeof = K*4*8 = K*32 = K<<5 */
      mov pA, pA0
      mov pB, pB0
      subs nmu, nmu, #1
   bne MLOOPK
   b DONE
#endif
.size ATL_asmdecor(ATL_USERMM),.-ATL_asmdecor(ATL_USERMM)
