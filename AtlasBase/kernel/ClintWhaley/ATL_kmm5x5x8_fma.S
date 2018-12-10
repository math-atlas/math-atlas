/*
 * This code debugged to work with ATL_VLENB=64 only, but perf not so great.
 * Originally designed to used in syrk, but perf blows because ipmen is 
 * using KB=62, which is too small for a KVEC kernel.  Shows that tuning
 * using ipmen won't always get great perf
 */
#include "atlas_asm.h"
#define ATL_VLENB 64
#if ATL_VLENB == 16
   #define vmovapd movaps
   #define vmovaps movaps
#endif
#if defined(SREAL) || defined(SCPLX)
   #define shSZ 2
   #define SZ 4
   #define vbroadcastsd vbroadcastss
   #define vfmadd231pd vfmadd231ps
   #define vaddpd vaddps
   #define vsubpd vsubps
   #define vmulpd vmulps
   #define vmovapd vmovaps
#else
   #define vmovapd vmovaps
   #define movapd movaps
   #define SZ 8
   #define shSZ 3
#endif
#define FMAC vfmadd231pd   /* FMAC m256/r256, rs1, rd */
#if ATL_VLENB == 64
   #define rB0 %zmm0
      #define rB0y %ymm0
      #define rB0x %xmm0
   #define rB1 %zmm1
      #define rB1y %ymm1
      #define rB1x %xmm1
   #define rA0 %zmm2
      #define rA0y %ymm2
      #define rA0x %xmm2
   #define rA1 %zmm3
      #define rA1y %ymm3
      #define rA1x %xmm3
   #define rA2 %zmm4
      #define rA2y %ymm4
      #define rA2x %xmm4
   #define rA3 %zmm5
      #define rA3y %ymm5
      #define rA3x %xmm5
   #define rA4 %zmm6
      #define rA4y %ymm6
      #define rA4x %xmm6
   #define rC0_0 %zmm7
      #define rC0_0y %ymm7
      #define rC0_0x %xmm7
   #define rC1_0 %zmm8
      #define rC1_0y %ymm8
      #define rC1_0x %xmm8
   #define rC2_0 %zmm9
      #define rC2_0y %ymm9
      #define rC2_0x %xmm9
   #define rC3_0 %zmm10
      #define rC3_0y %ymm10
      #define rC3_0x %xmm10
   #define rC4_0 %zmm11
      #define rC4_0y %ymm11
      #define rC4_0x %xmm11
   #define rC0_1 %zmm12
      #define rC0_1y %ymm12
      #define rC0_1x %xmm12
   #define rC1_1 %zmm13
      #define rC1_1y %ymm13
      #define rC1_1x %xmm13
   #define rC2_1 %zmm14
      #define rC2_1y %ymm14
      #define rC2_1x %xmm14
   #define rC3_1 %zmm15
      #define rC3_1y %ymm15
      #define rC3_1x %xmm15
   #define rC4_1 %zmm16
      #define rC4_1y %ymm16
      #define rC4_1x %xmm16
   #define rC0_2 %zmm17
      #define rC0_2y %ymm17
      #define rC0_2x %xmm17
   #define rC1_2 %zmm18
      #define rC1_2y %ymm18
      #define rC1_2x %xmm18
   #define rC2_2 %zmm19
      #define rC2_2y %ymm19
      #define rC2_2x %xmm19
   #define rC3_2 %zmm20
      #define rC3_2y %ymm20
      #define rC3_2x %xmm20
   #define rC4_2 %zmm21
      #define rC4_2y %ymm21
      #define rC4_2x %xmm21
   #define rC0_3 %zmm22
      #define rC0_3y %ymm22
      #define rC0_3x %xmm22
   #define rC1_3 %zmm23
      #define rC1_3y %ymm23
      #define rC1_3x %xmm23
   #define rC2_3 %zmm24
      #define rC2_3y %ymm24
      #define rC2_3x %xmm24
   #define rC3_3 %zmm25
      #define rC3_3y %ymm25
      #define rC3_3x %xmm25
   #define rC4_3 %zmm26
      #define rC4_3y %ymm26
      #define rC4_3x %xmm26
   #define rC0_4 %zmm27
      #define rC0_4y %ymm27
      #define rC0_4x %xmm27
   #define rC1_4 %zmm28
      #define rC1_4y %ymm28
      #define rC1_4x %xmm28
   #define rC2_4 %zmm29
      #define rC2_4y %ymm29
      #define rC2_4x %xmm29
   #define rC3_4 %zmm30
      #define rC3_4y %ymm30
      #define rC3_4x %xmm30
   #define rC4_4 %zmm31
      #define rC4_4y %ymm31
      #define rC4_4x %xmm31
#elif ATL_VLENB == 32
   #define rB0 %ymm0
      #define rB0x %xmm0
   #define rB1 %ymm1
      #define rB1x %xmm1
   #define rA0 %ymm2
      #define rA0x %xmm2
   #define rA1 %ymm3
      #define rA1x %xmm3
   #define rA2 %ymm4
      #define rA2x %xmm4
   #define rA3 %ymm5
      #define rA3x %xmm5
   #define rA4 %ymm6
      #define rA4x %xmm6
   #define rC0_0 %ymm7
      #define rC0_0x %xmm7
   #define rC1_0 %ymm8
      #define rC1_0x %xmm8
   #define rC2_0 %ymm9
      #define rC2_0x %xmm9
   #define rC3_0 %ymm10
      #define rC3_0x %xmm10
   #define rC4_0 %ymm11
      #define rC4_0x %xmm11
   #define rC0_1 %ymm12
      #define rC0_1x %xmm12
   #define rC1_1 %ymm13
      #define rC1_1x %xmm13
   #define rC2_1 %ymm14
      #define rC2_1x %xmm14
   #define rC3_1 %ymm15
      #define rC3_1x %xmm15
   #define rC4_1 %ymm16
      #define rC4_1x %xmm16
   #define rC0_2 %ymm17
      #define rC0_2x %xmm17
   #define rC1_2 %ymm18
      #define rC1_2x %xmm18
   #define rC2_2 %ymm19
      #define rC2_2x %xmm19
   #define rC3_2 %ymm20
      #define rC3_2x %xmm20
   #define rC4_2 %ymm21
      #define rC4_2x %xmm21
   #define rC0_3 %ymm22
      #define rC0_3x %xmm22
   #define rC1_3 %ymm23
      #define rC1_3x %xmm23
   #define rC2_3 %ymm24
      #define rC2_3x %xmm24
   #define rC3_3 %ymm25
      #define rC3_3x %xmm25
   #define rC4_3 %ymm26
      #define rC4_3x %xmm26
   #define rC0_4 %ymm27
      #define rC0_4x %xmm27
   #define rC1_4 %ymm28
      #define rC1_4x %xmm28
   #define rC2_4 %ymm29
      #define rC2_4x %xmm29
   #define rC3_4 %ymm30
      #define rC3_4x %xmm30
   #define rC4_4 %ymm31
      #define rC4_4x %xmm31
#else
   #define rB0 %xmm0
   #define rB1 %xmm1
   #define rA0 %xmm2
   #define rA1 %xmm3
   #define rA2 %xmm4
   #define rA3 %xmm5
   #define rA4 %xmm6
   #define rC0_0 %xmm7
   #define rC1_0 %xmm8
   #define rC2_0 %xmm9
   #define rC3_0 %xmm10
   #define rC4_0 %xmm11
   #define rC0_1 %xmm12
   #define rC1_1 %xmm13
   #define rC2_1 %xmm14
   #define rC3_1 %xmm15
   #define rC4_1 %xmm16
   #define rC0_2 %xmm17
   #define rC1_2 %xmm18
   #define rC2_2 %xmm19
   #define rC3_2 %xmm20
   #define rC4_2 %xmm21
   #define rC0_3 %xmm22
   #define rC1_3 %xmm23
   #define rC2_3 %xmm24
   #define rC3_3 %xmm25
   #define rC4_3 %xmm26
   #define rC0_4 %xmm27
   #define rC1_4 %xmm28
   #define rC2_4 %xmm29
   #define rC3_4 %xmm30
   #define rC4_4 %xmm31
#endif
/*
 * Prioritize original registers for inner-loop operations, but inc regs
 * can be anything w/o changing opcode size, so use new regs for those
 */
#define KK      %rdx  /* API reg */
#define pA      %rcx  /* API reg */
#define pB      %rax  /* comes in as r9 */
#define r256    %r9   /* set after mov r9 to pC () */
/*
 * Then N-loop variables much less important, so use any orig regs left
 */
#define pA0     %r8   /* set after mov r8 to pB (rax) */
#define pC      %rsi  /* set after mov rsi to nnu () */
#define iC      %r10  /* comes in as rsi */
#define incK    %rbp
#define pfA     %rbx
#define r192    %r12
#define KK0     %rdi
/*
 * We could give a rat's ass about what registers used in outer (M-) loop
 */
#define nmu     %r11  /* comes in as rdi */
#define incAn   %r13 /* %rbp */
#define iC0     %r14
#define pB0     %r15
/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */
#define PFBDIST 128
#define PFADIST 128
#ifdef ATL_ARCH_XeonPHI
   #define prefA(m_) vprefetch0 m_
   #define prefB(m_) vprefetch0 m_
   #define prefC(m_) vprefetche0 m_
#else
   #define prefA(m_) prefetcht0 m_
   #define prefB(m_) prefetcht0 m_
   #define prefC(m_) prefetchw m_
#endif
#if defined(BETAN) || defined(BETAn)
   #define BETAN1
#endif
#ifdef BETAN1
   #define VCOP vsubpd
#else
   #define VCOP vaddpd
#endif
.text
ALIGN16
.globl ATL_asmdecor(ATL_USERMM)
ATL_asmdecor(ATL_USERMM):
/*
 * Save callee-saved iregs
 */
   movq %rbp, -8(%rsp)
   movq %rbx, -16(%rsp)
   movq %r12, -24(%rsp)
   movq %r13, -32(%rsp)
   movq %r14, -40(%rsp)
   movq %r15, -48(%rsp)
/*
 * Load paramaters
 */
   mov %rdi, nmu
     prefA((pA))
   #ifndef ATL_KSYRK_
      mov %rsi, iC
   #endif
   lea 128(%r8), pB
     prefA(64(pA))
   lea 128(%r9), pC
     prefA(128(pA))
   movq 8(%rsp), pfA      /* pfA = pAn */
      prefB(-128(pB))
   mov $256, r256
      prefA(192(pA))
   mov $192, r192
      prefA((pA,r256))
   mov pB, pB0
/*
 * incAn = NU*sizeof*K = 5*8*K 
 * incK = NU*VLENB = 5*64 = 320
 */
   shl $shSZ, KK           /* KK = K*sizeof */
   lea (KK, KK, 4), KK     /* KK = nu*sizeof*K */
   mov KK, incAn
   lea 128(pA,KK), pA
   mov pA, pA0
   neg KK
   mov KK, KK0
   mov $5*ATL_VLENB, incK
   #ifdef ATL_KSYRK_
      xor iC, iC         /* nnu = 0, used as j counter for SYRK */
      mov $5, iC0   /* nnu0 = mu = i+mu for outer loop diag */
   #else
/*
 *    blksz = ((mu*nu*sizeof+VLENB-1)/VLENB)*VLENB=((5*5*8+63)/64)*64=256;
 *    blksz = 256 = 1 * 2^{8}
 *    iC = -nblks*blksz = -nnu*blksz = (nnu*1)*2^{8};
 */
      #ifdef SREAL
         shl $7, iC
      #else
         shl $8, iC
      #endif
   #endif
   add iC, pC
   neg iC
   mov iC, iC0

   mov KK, KK0
   ALIGN16
   MLOOP:
      NLOOP:
/*
 *       Peel K=1 iteration to zero rCxx
 */
            vmovapd -128+0*ATL_VLENB(pA,KK), rA0
            vmovapd -128+0*ATL_VLENB(pB), rB0
         vmulpd rA0, rB0, rC0_0
            vmovapd -128+1*ATL_VLENB(pA,KK), rA1
         vmulpd rA1, rB0, rC1_0
            vmovapd -128+2*ATL_VLENB(pA,KK), rA2
         vmulpd rA2, rB0, rC2_0
            vmovapd -128+3*ATL_VLENB(pA,KK), rA3
         vmulpd rA3, rB0, rC3_0
            vmovapd -128+4*ATL_VLENB(pA,KK), rA4
         vmulpd rA4, rB0, rC4_0
            vmovapd -128+1*ATL_VLENB(pB), rB0
         vmulpd rA0, rB0, rC0_1
         vmulpd rA1, rB0, rC1_1
         vmulpd rA2, rB0, rC2_1
         vmulpd rA3, rB0, rC3_1
         vmulpd rA4, rB0, rC4_1
            vmovapd -128+2*ATL_VLENB(pB), rB0
         vmulpd rA0, rB0, rC0_2
         vmulpd rA1, rB0, rC1_2
         vmulpd rA2, rB0, rC2_2
         vmulpd rA3, rB0, rC3_2
         vmulpd rA4, rB0, rC4_2
            vmovapd -128+3*ATL_VLENB(pB), rB0
         vmulpd rA0, rB0, rC0_3
         vmulpd rA1, rB0, rC1_3
         vmulpd rA2, rB0, rC2_3
         vmulpd rA3, rB0, rC3_3
         vmulpd rA4, rB0, rC4_3
            vmovapd -128+4*ATL_VLENB(pB), rB0
            add incK, pB
            add incK, KK
         vmulpd rA0, rB0, rC0_4
            vmovapd -128+0*ATL_VLENB(pA, KK), rA0
         vmulpd rA1, rB0, rC1_4
            vmovapd -128+1*ATL_VLENB(pA, KK), rA1
         vmulpd rA2, rB0, rC2_4
            vmovapd -128+2*ATL_VLENB(pA, KK), rA2
         vmulpd rA3, rB0, rC3_4
            vmovapd -128+3*ATL_VLENB(pA, KK), rA3
         vmulpd rA4, rB0, rC4_4
            vmovapd -128+4*ATL_VLENB(pA, KK), rA4
         jz KDONE

         KLOOP:
               vmovapd -128+0*ATL_VLENB(pB), rB0
            FMAC rA0, rB0, rC0_0
            FMAC rA1, rB0, rC1_0
            FMAC rA2, rB0, rC2_0
            FMAC rA3, rB0, rC3_0
            FMAC rA4, rB0, rC4_0
               vmovapd -128+1*ATL_VLENB(pB), rB0
            FMAC rA0, rB0, rC0_1
            FMAC rA1, rB0, rC1_1
            FMAC rA2, rB0, rC2_1
            FMAC rA3, rB0, rC3_1
            FMAC rA4, rB0, rC4_1
               vmovapd -128+2*ATL_VLENB(pB), rB0
            FMAC rA0, rB0, rC0_2
            FMAC rA1, rB0, rC1_2
            FMAC rA2, rB0, rC2_2
            FMAC rA3, rB0, rC3_2
            FMAC rA4, rB0, rC4_2
               vmovapd -128+3*ATL_VLENB(pB), rB0
            FMAC rA0, rB0, rC0_3
            FMAC rA1, rB0, rC1_3
            FMAC rA2, rB0, rC2_3
            FMAC rA3, rB0, rC3_3
            FMAC rA4, rB0, rC4_3
               vmovapd -128+4*ATL_VLENB(pB), rB0
               add incK, pB
            FMAC rA0, rB0, rC0_4
               add incK, KK
               vmovapd -128+0*ATL_VLENB(pA,KK), rA0
            FMAC rA1, rB0, rC1_4
               vmovapd -128+1*ATL_VLENB(pA,KK), rA1
            FMAC rA2, rB0, rC2_4
               vmovapd -128+2*ATL_VLENB(pA,KK), rA2
            FMAC rA3, rB0, rC3_4
               vmovapd -128+3*ATL_VLENB(pA,KK), rA3
            FMAC rA4, rB0, rC4_4
               vmovapd -128+4*ATL_VLENB(pA,KK), rA4
         jnz KLOOP
KDONE:
         mov KK0, KK
         #ifndef BETA0
            vmovapd -128+0*64(pC,iC), rB0
            vmovapd -128+1*64(pC,iC), rB1
         #endif
/*
 *       Reduce rC0_0y={rC3_0,rC2_0,rC1_0,rC0_0}
 */
         vextractf64x4 $1, rC0_0, rA0y
         vaddpd rA0y, rC0_0y, rA0y
         vextractf64x4 $1, rC1_0, rA1y
         vaddpd rA1y, rC1_0y, rA1y
         vextractf64x4 $1, rC2_0, rA2y
         vaddpd rA2y, rC2_0y, rA2y
         vextractf64x4 $1, rC3_0, rA3y
         vaddpd rA3y, rC3_0y, rA3y
         vhaddpd rA1y, rA0y, rA0y
         vhaddpd rA3y, rA2y, rA2y
         vperm2f128 $0x31, rA2y, rA0y, rA1y
         vperm2f128 $0x20, rA2y, rA0y, rA0y
         vaddpd rA1y, rA0y, rC0_0y /*rC0_0y={rC3_0,rC2_0,rC1_0,rC0_0}*/
         #ifndef BETA0
            vmovapd -128+2*64(pC,iC), rC1_0
            movsd -128+3*64(pC,iC), rC2_0x
         #endif
/*
 *       Reduce rC4_0y={rC2_1,rC1_1,rC0_1,rC4_0}
 */
         vextractf64x4 $1, rC4_0, rA0y
         vaddpd rA0y, rC4_0y, rA0y
         vextractf64x4 $1, rC0_1, rA1y
         vaddpd rA1y, rC0_1y, rA1y
         vextractf64x4 $1, rC1_1, rA2y
         vaddpd rA2y, rC1_1y, rA2y
         vextractf64x4 $1, rC2_1, rA3y
         vaddpd rA3y, rC2_1y, rA3y
         vhaddpd rA1y, rA0y, rA0y
         vhaddpd rA3y, rA2y, rA2y
         vperm2f128 $0x31, rA2y, rA0y, rA1y
         vperm2f128 $0x20, rA2y, rA0y, rA0y
         vaddpd rA1y, rA0y, rC4_0y /*rC4_0y={rC2_1,rC1_1,rC0_1,rC4_0}*/
         vinsertf64x4 $1, rC4_0y, rC0_0, rC0_0
/*
 *       Reduce rC3_1y={rC1_2,rC0_2,rC4_1,rC3_1}
 */
         vextractf64x4 $1, rC3_1, rA0y
         vaddpd rA0y, rC3_1y, rA0y
         vextractf64x4 $1, rC4_1, rA1y
         vaddpd rA1y, rC4_1y, rA1y
         vextractf64x4 $1, rC0_2, rA2y
         vaddpd rA2y, rC0_2y, rA2y
         vextractf64x4 $1, rC1_2, rA3y
         vaddpd rA3y, rC1_2y, rA3y
         vhaddpd rA1y, rA0y, rA0y
         vhaddpd rA3y, rA2y, rA2y
         vperm2f128 $0x31, rA2y, rA0y, rA1y
         vperm2f128 $0x20, rA2y, rA0y, rA0y
         vaddpd rA1y, rA0y, rC3_1y /*rC3_1y={rC1_2,rC0_2,rC4_1,rC3_1}*/
/*
 *       Reduce rC2_2y={rC0_3,rC4_2,rC3_2,rC2_2}
 */
         vextractf64x4 $1, rC2_2, rA0y
         vaddpd rA0y, rC2_2y, rA0y
         vextractf64x4 $1, rC3_2, rA1y
         vaddpd rA1y, rC3_2y, rA1y
         vextractf64x4 $1, rC4_2, rA2y
         vaddpd rA2y, rC4_2y, rA2y
         vextractf64x4 $1, rC0_3, rA3y
         vaddpd rA3y, rC0_3y, rA3y
         vhaddpd rA1y, rA0y, rA0y
         vhaddpd rA3y, rA2y, rA2y
         vperm2f128 $0x31, rA2y, rA0y, rA1y
         vperm2f128 $0x20, rA2y, rA0y, rA0y
         vaddpd rA1y, rA0y, rC2_2y /*rC2_2y={rC0_3,rC4_2,rC3_2,rC2_2}*/
         vinsertf64x4 $1, rC2_2y, rC3_1, rC3_1
/*
 *       Reduce rC1_3y={rC4_3,rC3_3,rC2_3,rC1_3}
 */
         vextractf64x4 $1, rC1_3, rA0y
         vaddpd rA0y, rC1_3y, rA0y
         vextractf64x4 $1, rC2_3, rA1y
         vaddpd rA1y, rC2_3y, rA1y
         vextractf64x4 $1, rC3_3, rA2y
         vaddpd rA2y, rC3_3y, rA2y
         vextractf64x4 $1, rC4_3, rA3y
         vaddpd rA3y, rC4_3y, rA3y
         vhaddpd rA1y, rA0y, rA0y
         vhaddpd rA3y, rA2y, rA2y
         vperm2f128 $0x31, rA2y, rA0y, rA1y
         vperm2f128 $0x20, rA2y, rA0y, rA0y
         vaddpd rA1y, rA0y, rC1_3y /*rC1_3y={rC4_3,rC3_3,rC2_3,rC1_3}*/
/*
 *       Reduce rC0_4y={rC3_4,rC2_4,rC1_4,rC0_4}
 */
         vextractf64x4 $1, rC0_4, rA0y
         vaddpd rA0y, rC0_4y, rA0y
         vextractf64x4 $1, rC1_4, rA1y
         vaddpd rA1y, rC1_4y, rA1y
         vextractf64x4 $1, rC2_4, rA2y
         vaddpd rA2y, rC2_4y, rA2y
         vextractf64x4 $1, rC3_4, rA3y
         vaddpd rA3y, rC3_4y, rA3y
         vhaddpd rA1y, rA0y, rA0y
         vhaddpd rA3y, rA2y, rA2y
         vperm2f128 $0x31, rA2y, rA0y, rA1y
         vperm2f128 $0x20, rA2y, rA0y, rA0y
         vaddpd rA1y, rA0y, rC0_4y /*rC0_4y={rC3_4,rC2_4,rC1_4,rC0_4}*/
         vinsertf64x4 $1, rC0_4y, rC1_3, rC1_3
/*
 *       reduce odd-man out rC4_4
 */
         vextractf64x4 $1, rC4_4, rA0y
         vaddpd rA0y, rC4_4y, rA0y     /*{Cd, Cc, Ca, Cd} */
         vhaddpd rA0y, rA0y, rA0y      /*{Ccd,Ccd,Cad,Cad} */
         vextractf128 $1, rA0y, rA1x   /*{XX , XX,Ccd,Ccd} */
         addsd rA1x, rA0x

         #ifdef BETA1
            vaddpd rB0, rC0_0, rC0_0
            vaddpd rB1, rC3_1, rC3_1
            vaddpd rC1_0, rC1_3, rC1_3
            addsd rC2_0x, rA0x
         #elif !defined(BETA0)
            vsubpd rB0, rC0_0, rC0_0
            vsubpd rB1, rC3_1, rC3_1
            vsubpd rC1_0, rC1_3, rC1_3
            subsd rC2_0x, rA0x
         #endif
         vmovapd rC0_0, -128+0*64(pC,iC)
         vmovapd rC3_1, -128+1*64(pC,iC)
         vmovapd rC1_3, -128+2*64(pC,iC)
         movsd rA0x, -128+3*64(pC,iC)
      #ifdef SREAL
         sub $-128, iC
      #else
         add $256, iC
      #endif
      jnz NLOOP
      mov iC0, iC
      add incAn, pA
      mov pB0, pB
      sub iC0, pC
      dec nmu
   jnz MLOOP

 DONE:
   movq -8(%rsp), %rbp
   movq -16(%rsp), %rbx
   movq -24(%rsp), %r12
   movq -32(%rsp), %r13
   movq -40(%rsp), %r14
   movq -48(%rsp), %r15
   ret
