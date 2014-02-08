#include "atlas_asm.h"

#ifdef SREAL
   #define SZ 4
   #define movapd movaps
   #define vmulpd vmulps
   #define vsubpd vsubps
   #define vaddpd vaddps
   #define vfmadd231pd vfmadd231ps
   #define vbroadcastsd vbroadcastss
#else
   #define SZ 8
   #define vmovapd vmovaps
#endif
#if !defined(KB) || KB == 0
   #ifdef KB
      #undef KB
   #endif
   #if defined(NB) && NB >= 24
      #define KB NB
   #elif defined(MB) && MB >= 24
      #define KB MB
   #else
      #define KB 48
   #endif
#endif
#define FMAC vfmadd231pd   /* FMAC m256/r256, rs1, rd */
#if defined(BETAN) || defined(BETAn)
   #define BETAN1
#endif
#ifdef BETAN1
   #define VCOP vsubpd
#else
   #define VCOP vaddpd
#endif
#ifdef BETA0
   #define prefC(m_) vprefetche1 m_
#else
   #define prefC(m_) vprefetche0 m_
#endif

#define rB0     %zmm0
#define rB1     %zmm1
#define rB2     %zmm2
#define rB3     %zmm3
#define rA0     %zmm4
#define rA1     %zmm5
#define rA2     %zmm6
#define rC00    %zmm7
#define rC10    %zmm8
#define rC20    %zmm9
#define rC01    %zmm10
#define rC11    %zmm11
#define rC21    %zmm12
#define rC02    %zmm13
#define rC12    %zmm14
#define rC22    %zmm15
#define rC03    %zmm16
#define rC13    %zmm17
#define rC23    %zmm18
#define rC04    %zmm19
#define rC14    %zmm20
#define rC24    %zmm21
#define rC05    %zmm22
#define rC15    %zmm23
#define rC25    %zmm24
#define rC06    %zmm25
#define rC16    %zmm26
#define rC26    %zmm27
#define rC07    %zmm28
#define rC17    %zmm29
#define rC27    %zmm30

/*
 * rbp, rbx, r12 & r15 callee-saved, so use them last
 * 15-4 = 11 regs can be used w/o save
 */

/*
 * ABI-mandated registers: 6 registers --> 5 regs left
 */
#define nmu %rdi
#define nnu %rsi
#define KK  %rdx
#define pA  %rcx
#define pB  %r8
#define pC  %r9
/*
 * regs of choice: 5 regs --> 
 */
#define KK0 %r10
#define nnu0 %r11
#define incAm %r13
#define pB0 %rax
#define pA0 %r14
/*
 * Regs that must be saved and restored:rbp,rbx,r12,r15
 */
#define incBn %r15
#define incK  %r12
#define incC  %rbx
/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */
.text
ALIGN16
.globl ATL_asmdecor(ATL_USERMM)
ATL_asmdecor(ATL_USERMM):
      vprefetch0 (pB)
   movq %r15, -8(%rsp)
      vprefetch0 64(pB)
   movq %r14, -16(%rsp)
      vprefetch0 128(pB)
   movq %r13, -24(%rsp)
      vprefetch0 192(pB)
   movq %r12, -32(%rsp)
      vprefetch0 256(pB)
   movq %rbp, -40(%rsp)
      vprefetch0 320(pB)
   movq %rbx, -48(%rsp)
      vprefetch0 384(pB)
/*
 * incAm = MU*sizeof*K = 24*8*K = 64*3*K
 */
   lea (KK, KK,2), incAm   /* incAm = 3*K */
      vprefetch0 448(pB)
   shl $6, incAm           /* incAm = 64*3*K */
      vprefetch0 512(pB)
   mov nnu, nnu0
      vprefetch0 576(pB)
   mov pA, pA0
      vprefetch0 640(pB)
/*
 * incBn = NU*SZ*K = 8*SZ*K
 */
   mov KK, incBn
      vprefetch0 704(pB)
   #ifdef SREAL
      shl $5, incBn
   #else
      shl $6, incBn
   #endif
      vprefetch0 768(pB)
   mov $1536, incC
      vprefetch0 789(pB)
/*
 * Adjust pB and loop bounds so that we can use loop bound to index pB
 */
   mov incBn, KK
      vprefetch0 (pA)
   add incBn, pB
      vprefetch0 64(pA)
   neg KK
      vprefetch0 128(pA)
   mov pB, pB0
      vprefetch0 3*64(pA)
   mov KK, KK0
      vprefetch0 4*64(pA)
   mov $8*SZ, incK  /* incK = NU*SZ */
      vprefetch0 5*64(pA)

#define PFDA0 384
#define PFDA2 1152
#define PFDA3 2304
   MLOOP:
         vbroadcastsd (pB,KK), rB0
         vbroadcastsd SZ(pB,KK), rB1
         vbroadcastsd 2*SZ(pB,KK), rB2
      NLOOP:
/*
 *       1st iteration peeled to init C
 */
            vmovapd (pA), rA0
            vmovapd 64(pA), rA1
         vmulpd rA0, rB0, rC00
            vmovapd 2*64(pA), rA2
         vmulpd rA1, rB0, rC10
            vbroadcastsd 3*SZ(pB,KK), rB3
         vmulpd rA2, rB0, rC20
            vbroadcastsd 4*SZ(pB,KK), rB0
         vmulpd rA0, rB1, rC01
            vprefetch2 KB*SZ*8(pB,KK)
         vmulpd rA1, rB1, rC11
            vprefetch0 64(pB,KK)
         vmulpd rA2, rB1, rC21
            vbroadcastsd 5*SZ(pB,KK), rB1
         vmulpd rA0, rB2, rC02
               vprefetch1 PFDA2(pA)
         vmulpd rA1, rB2, rC12
               vprefetch1 64+PFDA2(pA)
         vmulpd rA2, rB2, rC22
            vbroadcastsd 6*SZ(pB,KK), rB2
         vmulpd rA0, rB3, rC03
               vprefetch1 128+PFDA2(pA)
         vmulpd rA1, rB3, rC13
               vprefetch0 PFDA0(pA)
         vmulpd rA2, rB3, rC23
            vbroadcastsd 7*SZ(pB,KK), rB3
         vmulpd rA0, rB0, rC04
               vprefetch0 64+PFDA0(pA)
         vmulpd rA1, rB0, rC14
               vprefetch0 128+PFDA0(pA)
         vmulpd rA2, rB0, rC24
            vbroadcastsd 8*SZ(pB,KK), rB0
         vmulpd rA0, rB1, rC05
         vmulpd rA1, rB1, rC15
         vmulpd rA2, rB1, rC25
            vbroadcastsd 9*SZ(pB,KK), rB1
         vmulpd rA0, rB2, rC06
            add $192, pA
         vmulpd rA1, rB2, rC16
            add incK, KK
         vmulpd rA2, rB2, rC26
            vbroadcastsd 2*SZ(pB,KK), rB2
         vmulpd rA0, rB3, rC07
            vmovaps (pA), rA0
         vmulpd rA1, rB3, rC17
            vmovaps 64(pA), rA1
         vmulpd rA2, rB3, rC27
            vmovaps 128(pA), rA2
         jz KDONE

         KLOOP:
               vbroadcastsd 3*SZ(pB,KK), rB3
            FMAC rA0, rB0, rC00
               vprefetch2 KB*SZ*8(pB,KK)
            FMAC rA1, rB0, rC10
               vprefetch0 64(pB,KK)
            FMAC rA2, rB0, rC20
               vbroadcastsd 4*SZ(pB,KK), rB0
            FMAC rA0, rB1, rC01
               vprefetch1 PFDA2(pA)
            FMAC rA1, rB1, rC11
               vprefetch1 64+PFDA2(pA)
            FMAC rA2, rB1, rC21
               vbroadcastsd 5*SZ(pB,KK), rB1
            FMAC rA0, rB2, rC02
               vprefetch1 2*64+PFDA2(pA)
            FMAC rA1, rB2, rC12
               vprefetch0 PFDA0(pA)
            FMAC rA2, rB2, rC22
               vbroadcastsd 6*SZ(pB,KK), rB2
            FMAC rA0, rB3, rC03
               vprefetch0 64+PFDA0(pA)
            FMAC rA1, rB3, rC13
               vprefetch0 128+PFDA0(pA)
            FMAC rA2, rB3, rC23
               vbroadcastsd 7*SZ(pB,KK), rB3
            FMAC rA0, rB0, rC04
               vprefetch2 PFDA3(pA)
            FMAC rA1, rB0, rC14
               vprefetch2 64+PFDA3(pA)
            FMAC rA2, rB0, rC24
               vbroadcastsd 8*SZ(pB,KK), rB0
            FMAC rA0, rB1, rC05
               vprefetch2 128+PFDA3(pA)
            FMAC rA1, rB1, rC15
            FMAC rA2, rB1, rC25
               vbroadcastsd 9*SZ(pB,KK), rB1
            FMAC rA0, rB2, rC06
               add $192, pA
            FMAC rA1, rB2, rC16
               add incK, KK
            FMAC rA2, rB2, rC26
               vbroadcastsd 2*SZ(pB,KK), rB2
            FMAC rA0, rB3, rC07
               vmovaps (pA), rA0
            FMAC rA1, rB3, rC17
               vmovaps 64(pA), rA1
            FMAC rA2, rB3, rC27
               vmovaps 128(pA), rA2
         jnz KLOOP
         KDONE:
         #ifndef BETA0
            VCOP (pC), rC00, rC00
            VCOP 64(pC), rC10, rC10
            VCOP 128(pC), rC20, rC20
            VCOP 192(pC), rC01, rC01
            VCOP 256(pC), rC11, rC11
            VCOP 320(pC), rC21, rC21
            VCOP 384(pC), rC02, rC02
            VCOP 448(pC), rC12, rC12
         #endif
         vmovapd rC00, (pC)
            prefC(1536(pC))
         vmovapd rC10, 64(pC)
            prefC(64+1536(pC))
         vmovapd rC20, 2*64(pC)
            prefC(2*64+1536(pC))
         vmovapd rC01, 3*64(pC)
            prefC(3*64+1536(pC))
         vmovapd rC11, 4*64(pC)
            prefC(4*64+1536(pC))
         vmovapd rC21, 5*64(pC)
            prefC(5*64+1536(pC))
         vmovapd rC02, 6*64(pC)
            prefC(6*64+1536(pC))
         vmovapd rC12, 7*64(pC)
            prefC(7*64+1536(pC))
         #ifndef BETA0
            VCOP 512(pC), rC22, rC22
            VCOP 576(pC), rC03, rC03
            VCOP 640(pC), rC13, rC13
            VCOP 704(pC), rC23, rC23
            VCOP 768(pC), rC04, rC04
            VCOP 832(pC), rC14, rC14
            VCOP 896(pC), rC24, rC24
            VCOP 960(pC), rC05, rC05
         #endif
         vmovapd rC22, 8*64(pC)
            prefC(8*64+1536(pC))
         vmovapd rC03, 9*64(pC)
            prefC(9*64+1536(pC))
         vmovapd rC13, 10*64(pC)
            prefC(10*64+1536(pC))
         vmovapd rC23, 11*64(pC)
            prefC(11*64+1536(pC))
         vmovapd rC04, 12*64(pC)
            prefC(12*64+1536(pC))
         vmovapd rC14, 13*64(pC)
            prefC(13*64+1536(pC))
         vmovapd rC24, 14*64(pC)
            prefC(14*64+1536(pC))
         vmovapd rC05, 15*64(pC)
            prefC(15*64+1536(pC))
         #ifndef BETA0
            VCOP 1024(pC), rC15, rC15
            VCOP 1088(pC), rC25, rC25
            VCOP 1152(pC), rC06, rC06
            VCOP 1216(pC), rC16, rC16
            VCOP 1280(pC), rC26, rC26
            VCOP 1344(pC), rC07, rC07
            VCOP 1408(pC), rC17, rC17
            VCOP 1472(pC), rC27, rC27
         #endif
         vmovapd rC15, 16*64(pC)
            prefC(16*64+1536(pC))
         vmovapd rC25, 17*64(pC)
            prefC(17*64+1536(pC))
         vmovapd rC06, 18*64(pC)
            prefC(18*64+1536(pC))
         vmovapd rC16, 19*64(pC)
            prefC(19*64+1536(pC))
         vmovapd rC26, 20*64(pC)
            prefC(20*64+1536(pC))
         vmovapd rC07, 21*64(pC)
            prefC(21*64+1536(pC))
         vmovapd rC17, 22*64(pC)
            prefC(22*64+1536(pC))
         vmovapd rC27, 23*64(pC)
            prefC(23*64+1536(pC))
         add incC, pC
         mov KK0, KK
         add incBn, pB
         dec nnu
         mov pA0, pA
      jnz NLOOP
      mov pB0, pB
      add incAm, pA0
      dec nmu
      mov pA0, pA
      mov nnu0, nnu
   jnz MLOOP


DONE:
   movq  -8(%rsp), %r15
   movq -16(%rsp), %r14
   movq -24(%rsp), %r13
   movq -32(%rsp), %r12
   movq -40(%rsp), %rbp
   movq -48(%rsp), %rbx
   ret
