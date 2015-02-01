#include "atlas_asm.h"
/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */

#define FSIZE 6*8
#define PFADIST 448
#define PFBDIST 448
#ifndef prefA
   #define prefA prefetcht0
#endif
#ifndef prefB
   #define prefB prefetcht0
#endif
#ifndef prefC
   #ifdef ATL_3DNow
      #define prefC prefetchw
   #else
      #define prefC prefetcht0
   #endif
#endif
#if defined(SREAL) || defined(SCPLX)
   #define vmovapd vmovaps
   #define vsubpd vsubps
   #define vaddpd vaddps
   #define vmulpd vmulps
   #define vbroadcastsd vbroadcastss
   #define FMAC vfmadd231ps   /* FMAC m256/r256, rs1, rd */
   #define SZ 4
#else
   #define FMAC vfmadd231pd   /* FMAC m256/r256, rs1, rd */
   #define vmovapd vmovaps
   #define SZ 8
#endif
#ifdef BETAN1
   #define BETCOP vsubpd
#else
   #define BETCOP vaddpd
#endif
/*
 * floating point registers
 */
#define m0   %ymm0
#define rA0  %ymm1
#define rA1  %ymm2
#define rA2  %ymm3
#define rB0  %ymm4
#define rB1  %ymm5
#define rB2  %ymm6
#define rC00 %ymm7
#define rC10 %ymm8
#define rC20 %ymm9
#define rC01 %ymm10
#define rC11 %ymm11
#define rC21 %ymm12
#define rC02 %ymm13
#define rC12 %ymm14
#define rC22 %ymm15
/*
 * Prioritize original registers for inner-loop operations, but inc regs
 * can be anything w/o changing opcode size, so use new regs for those
 */
#define KK      %rdx  /* API reg */
#define pA      %rcx  /* API reg */
#define pB      %rax  /* comes in as r9 */
#define incBk   %r8   /* set after mov r8 to pB (rax) */
/*
 * Then N-loop variables much less important, so use any orig regs left
 */
#define pC      %rsi  /* set after mov rsi to nnu () */
#define nnu     %r10  /* comes in as rsi */
#define pfA     %rbx
#define r96     %rbp
#define incPF   %r12
#define KK0     %rdi
/*
 * We could give a rat's ass about what registers used in outer (M-) loop
 */
#define nmu     %r11  /* comes in as rdi */
#define incAm   %r13
#define nnu0    %r9 /* %r14 */
#define pB0     %r14

/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */
.text
.global ATL_asmdecor(ATL_USERMM)
ALIGN16
ATL_asmdecor(ATL_USERMM):
/*
 * Save callee-saved iregs
 */
   sub  $FSIZE, %rsp
   movq %rbp, (%rsp)
   movq %rbx, 8(%rsp)
   movq %r12, 16(%rsp)
   movq %r13, 24(%rsp)
   movq %r14, 32(%rsp)
/*
 * Load paramaters
 */
   mov %rdi, nmu
      prefetcht0 (pA)
   mov %rsi, nnu
   mov %r8, pB
      prefetcht0 64(pA)
   mov %r9, pC
   mov nnu, nnu0
   mov $96, r96
      prefetcht0 128(pA)
   movq FSIZE+16(%rsp), pfA     /* pfA = pBn */
   cmp pfA, pB                  /* if (pBn == pB) */
   CMOVEq FSIZE+8(%rsp), pfA    /*    pfA = pAn */
   cmp pfA, pA                  /* if (pAn == pA) */
   CMOVEq FSIZE+24(%rsp), pfA   /*    pfA = pCn */
   sub $-128, pfA
   sub $-128, pC                 /* extend range of 1-byte offsets */
      prefetcht0 192(pA)
/*
 * Set constants
 */
   mov $12*3*8, incPF           /* incPF = mu*nu*sizeof */
   mov $3*SZ, incBk             /* 24 = NU*sizeof = 3*8 = 24 */
      prefetcht0 256(pA)
   mov pB, pB0
/*
 * incAm = MU*sizeof*K = 12*8*K = 3*32*K
 */
   lea (KK, KK, 2), KK          /* KK = 3*K */
      prefetcht0 (pB)
   shl $5, KK                   /* KK = 3*32*K = 12*8*K = MU*sizeof*K */
   mov KK, incAm                /* incAm = MU*sizeof*K */
      prefetcht0 64(pB)
   add KK, pA                   /* pA[-kk] will access A */
   neg KK                       /* KK = -NU*sizeof*K */
   mov KK, KK0
   vbroadcastsd (pB), rB0
   ALIGN32
   MNLOOP:
/*
 *          First peeled iteration gets us to preloading next iter's data for
 *          loop while only doing 1 load/flop.  It does no adds to avoid
 *          having to zero the C registers.
 */

            vmovapd (pA,KK), rA0
            vmulpd rB0, rA0, rC00
            vmovapd 32(pA,KK), rA1
            vmulpd rB0, rA1, rC10
            vmovapd 64(pA,KK), rA2
            vmulpd rB0, rA2, rC20

            vbroadcastsd SZ(pB), rB1
            vmulpd rB1, rA0, rC01
            vbroadcastsd 2*SZ(pB), rB2
            vmulpd rB1, rA1, rC11
               vbroadcastsd 3*SZ(pB), rB0
            vmulpd rB1, rA2, rC21
               vbroadcastsd 4*SZ(pB), rB1
            add incBk, pB

            vmulpd rB2, rA0, rC02
               vmovapd 96(pA,KK), rA0
            vmulpd rB2, rA1, rC12
               vmovapd 128(pA,KK), rA1
            vmulpd rB2, rA2, rC22
               vmovapd 160(pA,KK), rA2
            add r96, KK
            jz KDONE_NOFINALPEEL
/*
 *          Stop loop on iteration early to set up for C, so 2nd peel is
 *          from end (bottom) of loop
 */
               prefC -128(pC)
            add incBk, pB
               prefC -64(pC)
            add r96, KK
            jz KLOOPDRAIN
/*
 *          Now peel a 3rd iteration (already peeled one from top and bottom)
 *          in order to do some prefetch.  This peel is the exact code from
 *          loop, with some prefetch commands added.
 */
            FMAC rB0, rA0, rC00
            vbroadcastsd -SZ(pB), rB2
            FMAC rB0, rA1, rC10
               prefA -128(pfA)
            FMAC rB0, rA2, rC20
               vbroadcastsd (pB), rB0

            FMAC rB1, rA0, rC01
               prefA (pfA)
            FMAC rB1, rA1, rC11
               prefC (pC)
               prefC 64(pC)
            FMAC rB1, rA2, rC21
               vbroadcastsd SZ(pB), rB1
            add incBk, pB

            FMAC rB2, rA0, rC02
               vmovapd (pA,KK), rA0
               add incPF, pfA
            FMAC rB2, rA1, rC12
               vmovapd 32(pA,KK), rA1
            FMAC rB2, rA2, rC22
               vmovapd 64(pA,KK), rA2
            add r96, KK
            jz KLOOPDRAIN
               prefC 32(pC,r96)
/*
 *        Finally, start actual loop
 */
          ALIGN32
          KLOOP:
               prefetcht0 PFBDIST(pB)
            FMAC rB0, rA0, rC00
            vbroadcastsd -SZ(pB), rB2
            FMAC rB0, rA1, rC10
               prefetcht0 PFADIST(pA,KK)
            FMAC rB0, rA2, rC20
               vbroadcastsd (pB), rB0

            FMAC rB1, rA0, rC01
               prefetcht0 PFADIST+64(pA,KK)
            FMAC rB1, rA1, rC11
            FMAC rB1, rA2, rC21
               vbroadcastsd SZ(pB), rB1
            add incBk, pB

            FMAC rB2, rA0, rC02
               vmovapd (pA,KK), rA0
            FMAC rB2, rA1, rC12
               vmovapd 32(pA,KK), rA1
            FMAC rB2, rA2, rC22
               vmovapd 64(pA,KK), rA2
            add r96, KK
         jnz KLOOP
/*
 *       Last iteration peeled off bottom to allow store of C; this should
 *       strongly improve BETA=0, but may hurt BETA=1.  This kernel written
 *       primarily for K-cleanup, which is always BETA=0, so do it.
 */
.local KLOOPDRAIN
KLOOPDRAIN:
#ifdef BETAN1
   #define VCOP vsubpd
#else
   #define VCOP vaddpd
#endif
            vbroadcastsd -SZ(pB), rB2
            FMAC rB0, rA0, rC00
            #ifndef BETA0
               VCOP  -128(pC), rC00, rC00
            #endif
            vmovapd rC00, -128(pC)
            FMAC rB0, rA1, rC10
            #ifndef BETA0
               VCOP  -96(pC), rC10, rC10
            #endif
            vmovapd rC10, -96(pC)
            FMAC rB0, rA2, rC20
            #ifndef BETA0
               VCOP  -64(pC), rC20, rC20
            #endif
            vmovapd rC20, -64(pC)

            FMAC rB1, rA0, rC01
            #ifndef BETA0
               VCOP  -32(pC), rC01, rC01
            #endif
            vmovapd rC01, -32(pC)
            FMAC rB1, rA1, rC11
            #ifndef BETA0
               VCOP  (pC), rC11, rC11
            #endif
            vmovapd rC11, (pC)
            FMAC rB1, rA2, rC21
            #ifndef BETA0
               VCOP  32(pC), rC21, rC21
            #endif
            vmovapd rC21, 32(pC)

            FMAC rB2, rA0, rC02
            #ifndef BETA0
               VCOP  64(pC), rC02, rC02
            #endif
            vmovapd rC02, 64(pC)
            FMAC rB2, rA1, rC12
            #ifndef BETA0
               VCOP  96(pC), rC12, rC12
            #endif
            vmovapd rC12, 96(pC)
            FMAC rB2, rA2, rC22
            #ifndef BETA0
               VCOP  32(pC,r96), rC22, rC22
            #endif
            vmovapd rC22, 32(pC,r96)
.local KLOOPDONE
KLOOPDONE:
         mov KK0, KK
         vbroadcastsd (pB), rB0
         add incPF, pC                /* pC += MU*NU*sizeof */
      sub $1, nnu
      jnz MNLOOP
      vbroadcastsd (pB0), rB0
      mov nnu0, nnu
      mov pB0, pB
      add incAm, pA                     /* pA += MU*sizeof*K */
   sub $1, nmu
   jnz MNLOOP
/* DONE: */
   movq (%rsp), %rbp
   movq 8(%rsp), %rbx
   movq 16(%rsp), %r12
   movq 24(%rsp), %r13
   movq 32(%rsp), %r14
   add  $FSIZE, %rsp
   ret
/*
 * got answers in rCxx, just need to apply them to memory
 */
KDONE_NOFINALPEEL:
/*
 *       Write answer back out to C
 */
         #ifdef BETA0
            vmovapd rC00, -128(pC)
            vmovapd rC10, -96(pC)
            vmovapd rC20, -64(pC)
            vmovapd rC01, -32(pC)
            vmovapd rC11, (pC)
            vmovapd rC21, 32(pC)
            vmovapd rC02, 64(pC)
            vmovapd rC12, 96(pC)
            vmovapd rC22, 128(pC)
/*
 *          Add running sum in rCx with original C, then store back out
 */
         #else
            BETCOP -128(pC), rC00, rC00
            vmovapd rC00, -128(pC)
            BETCOP -96(pC), rC10, rC10
            vmovapd rC10, -96(pC)
            BETCOP -64(pC), rC20, rC20
            vmovapd rC20, -64(pC)
            BETCOP -32(pC), rC01, rC01
            vmovapd rC01, -32(pC)
            BETCOP (pC), rC11, rC11
            vmovapd rC11, (pC)
            BETCOP 32(pC), rC21, rC21
            vmovapd rC21, 32(pC)
            BETCOP 64(pC), rC02, rC02
            vmovapd rC02, 64(pC)
            BETCOP 96(pC), rC12, rC12
            vmovapd rC12, 96(pC)
            BETCOP 128(pC), rC22, rC22
            vmovapd rC22, 128(pC)
         #endif
         jmp KLOOPDONE
