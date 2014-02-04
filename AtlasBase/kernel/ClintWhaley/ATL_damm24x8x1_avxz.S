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
#define nnu     %r10  /* comes in as rsi */
#define pfA     %rbx
#define incB    %rbp
#define r192    %r12
#define KK0     %rdi
/*
 * We could give a rat's ass about what registers used in outer (M-) loop
 */
#define nmu     %r11  /* comes in as rdi */
#define incAm   %r13
#define nnu0    %r14
#define pB0     %r15
/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */
#define PFDISTA 448
#define PFDISTB 320
#define prefA(m_) vprefetch0 m_
#define prefB(m_) vprefetch0 m_
#define prefC(m_) vprefetche0 m_
#define FMAC vfmadd231pd   /* FMAC m256/r256, rs1, rd */
#if defined(BETAN) || defined(BETAn)
   #define BETAN1
#endif
#ifdef BETAN1
   #define VCOP vsubpd
#else
   #define VCOP vaddpd
#endif
#define vmovapd vmovaps
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
   mov %rsi, nnu
   mov %r8, pB
   mov %r9, pC
   mov nnu, nnu0
      vprefetch0 (pB)
   movq 8(%rsp), pfA      /* pfA = pAn */
   mov KK, incAm
   mov KK, KK0
   sub $-128, pC
   sub $-128, pA
   mov $256, r256
   mov $192, r192
   mov pA, pA0
   mov pB, pB0
/*
 * incAm = MU*sizeof*K = 24*8*K = 64*3*K
 * incB = NU*sizeof*K = 8*8*K = 64*K
 */
   lea (KK, KK, 2), incAm  /* incAm = 3*K */
   shl $6, incAm           /* incAm = 64*3*K */
   mov KK, incB
   #ifdef SREAL
      shl $5, incB
   #else
      shl $6, incB
   #endif

   ALIGN16
   MLOOP:
         vbroadcastsd (pB), rB0
      NLOOP:
/*
 *       Peel K=1 to zero rCxx
 */
            vmovapd -128(pA), rA0
         vmulpd rA0, rB0, rC00
            vmovapd -64(pA), rA1
         vmulpd rA1, rB0, rC10
            vmovapd (pA), rA2
         vmulpd rA2, rB0, rC20

            vbroadcastsd SZ(pB), rB1
         vmulpd rA0, rB1, rC01
            vbroadcastsd 2*SZ(pB), rB2
         vmulpd rA1, rB1, rC11
            vbroadcastsd 3*SZ(pB), rB3
         vmulpd rA2, rB1, rC21

            vbroadcastsd 4*SZ(pB), rB0
         vmulpd rA0, rB2, rC02
            vbroadcastsd 5*SZ(pB), rB1
         vmulpd rA1, rB2, rC12
         vmulpd rA2, rB2, rC22
           vbroadcastsd 6*SZ(pB), rB2

         vmulpd rA0, rB3, rC03
         vmulpd rA1, rB3, rC13
            add r192, pA
         vmulpd rA2, rB3, rC23
           vbroadcastsd 7*SZ(pB), rB3

         vmulpd rA0, rB0, rC04
            vprefetche0 (pC)
         vmulpd rA1, rB0, rC14
         vmulpd rA2, rB0, rC24
           vbroadcastsd 8*SZ(pB), rB0

         vmulpd rA0, rB1, rC05
         vmulpd rA1, rB1, rC15
         vmulpd rA2, rB1, rC25
           vbroadcastsd 9*SZ(pB), rB1

         vmulpd rA0, rB2, rC06
         vmulpd rA1, rB2, rC16
         dec %edx
         vmulpd rA2, rB2, rC26
           vbroadcastsd 10*SZ(pB), rB2

         vmulpd rA0, rB3, rC07
           vmovapd -128(pA), rA0
         vmulpd rA1, rB3, rC17
           vmovapd -64(pA), rA1
         vmulpd rA2, rB3, rC27

         jz KDONE
           vmovapd (pA), rA2
           vbroadcastsd 11*SZ(pB), rB3

         KLOOP:
            FMAC rA0, rB0, rC00
               add $8*SZ, pB
            FMAC rA1, rB0, rC10
               add r192, pA
            FMAC rA2, rB0, rC20
               vbroadcastsd 4*SZ(pB), rB0

            FMAC rA0, rB1, rC01
               prefB(PFDISTB(pB))
            FMAC rA1, rB1, rC11
               prefB(64+PFDISTB(pB))
            FMAC rA2, rB1, rC21
               vbroadcastsd 5*SZ(pB), rB1

            FMAC rA0, rB2, rC02
               prefB(128+PFDISTB(pB))
            FMAC rA1, rB2, rC12
               prefA(PFDISTA(pA))
            FMAC rA2, rB2, rC22
               vbroadcastsd 6*SZ(pB), rB2

            FMAC rA0, rB3, rC03
               prefA(64+PFDISTA(pA))
            FMAC rA1, rB3, rC13
               prefA(128+PFDISTA(pA))
            FMAC rA2, rB3, rC23
              vbroadcastsd 7*SZ(pB), rB3

            FMAC rA0, rB0, rC04
               prefA(192+PFDISTA(pA))
            FMAC rA1, rB0, rC14
               vprefetch1 -64(pB,incB)
            FMAC rA2, rB0, rC24
              vbroadcastsd 8*SZ(pB), rB0

            FMAC rA0, rB1, rC05
               vprefetch1 (pB,incB)
            FMAC rA1, rB1, rC15
               vprefetch1 64(pB,incB)
            FMAC rA2, rB1, rC25
              vbroadcastsd 9*SZ(pB), rB1

            FMAC rA0, rB2, rC06
               vprefetch1 128(pB,incB)
            FMAC rA1, rB2, rC16
               vprefetch2 -128(pA,incAm)
            FMAC rA2, rB2, rC26
              vbroadcastsd 10*SZ(pB), rB2

            FMAC rA0, rB3, rC07
              vmovapd -128(pA), rA0
            FMAC rA1, rB3, rC17
              vmovapd -64(pA), rA1
            FMAC rA2, rB3, rC27
              vmovapd (pA), rA2

            sub $1, %rdx
              vbroadcastsd 11*SZ(pB), rB3
         jnz KLOOP
KDONE:
         mov KK0, KK
         #ifndef BETA0
            VCOP -128(pC), rC00, rC00
            VCOP -64(pC), rC10, rC10
            VCOP (pC), rC20, rC20
         #endif
         vmovapd rC00, -128(pC)
         vmovapd rC10, -64(pC)
         vmovapd rC20, (pC)
         #ifndef BETA0
            VCOP 64(pC), rC01, rC01
            VCOP 128-256(pC,r256), rC11, rC11
            VCOP 192-256(pC,r256), rC21, rC21
         #endif
         vmovapd rC01, 64(pC)
         vmovapd rC11, 128-256(pC,r256)
         vmovapd rC21, 192-256(pC,r256)
         #ifndef BETA0
            VCOP (pC,r256), rC02, rC02
            VCOP 320-256(pC,r256), rC12, rC12
            VCOP 384-512(pC,r256,2), rC22, rC22
         #endif
         vmovapd rC02, (pC,r256)
         vmovapd rC12, 320-256(pC,r256)
         vmovapd rC22, 384-512(pC,r256,2)
         #ifndef BETA0
            VCOP 448-512(pC,r256,2), rC03, rC03
            VCOP (pC,r256,2), rC13, rC13
            VCOP 576-512(pC,r256,2), rC23, rC23
         #endif
         vmovapd rC03, 448-512(pC,r256,2)
         vmovapd rC13, (pC,r256,2)
         vmovapd rC23, 576-512(pC,r256,2)
         #ifndef BETA0
            VCOP 640-768(pC,r192,4), rC04, rC04
            VCOP 704-768(pC,r192,4), rC14, rC14
            VCOP (pC,r192,4), rC24, rC24
         #endif
         vmovapd rC04, 640-768(pC,r192,4)
         vmovapd rC14, 704-768(pC,r192,4)
         vmovapd rC24, (pC,r192,4)
         #ifndef BETA0
            VCOP 832-768(pC,r192,4), rC05, rC05
            VCOP 896-1024(pC,r256,4), rC15, rC15
            VCOP 960-1024(pC,r256,4), rC25, rC25
         #endif
         vmovapd rC05, 832-768(pC,r192,4)
         vmovapd rC15, 896-1024(pC,r256,4)
         vmovapd rC25, 960-1024(pC,r256,4)
         #ifndef BETA0
            VCOP (pC,r256,4), rC06, rC06
            VCOP 1088-1024(pC,r256,4), rC16, rC16
            VCOP 1152(pC), rC26, rC26
         #endif
         vmovapd rC06, (pC,r256,4)
         vmovapd rC16, 1088-1024(pC,r256,4)
         vmovapd rC26, 1152(pC)
         #ifndef BETA0
            VCOP 1216(pC), rC07, rC07
            VCOP 1280(pC), rC17, rC17
            VCOP 1344(pC), rC27, rC27
         #endif
         vmovapd rC07, 1216(pC)
         vmovapd rC17, 1280(pC)
         vmovapd rC27, 1344(pC)

         add $8*SZ, pB
         lea (pC, r192,8), pC /* pC += MU*NU*sizeof = 24*8*8 = 1536 = 192*8 */
         sub $1, nnu
         mov pA0, pA
      jnz NLOOP
      mov nnu0, nnu
      add incAm, pA0
      mov pB0, pB
      mov pA0, pA
      sub $1, nmu
      mov KK0, KK
   jnz MLOOP
 DONE:
   movq -8(%rsp), %rbp
   movq -16(%rsp), %rbx
   movq -24(%rsp), %r12
   movq -32(%rsp), %r13
   movq -40(%rsp), %r14
   movq -48(%rsp), %r15
   ret
