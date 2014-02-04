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
#define rB4     %zmm4
#define rB5     %zmm5
#define rB6     %zmm6
#define rB7     %zmm7
#define rA0a    %zmm8
#define rA1a    %zmm9
#define rA0b    %zmm10
#define rA1b    %zmm11
#define rA0c    %zmm12
#define rA1c    %zmm13
#define rA0d    %zmm14
#define rA1d    %zmm15
#define rC00    %zmm16
#define rC10    %zmm17
#define rC01    %zmm18
#define rC11    %zmm19
#define rC02    %zmm20
#define rC12    %zmm21
#define rC03    %zmm22
#define rC13    %zmm23
#define rC04    %zmm24
#define rC14    %zmm25
#define rC05    %zmm26
#define rC15    %zmm27
#define rC06    %zmm28
#define rC16    %zmm29
#define rC07    %zmm30
#define rC17    %zmm31

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
#define incB    %rbx
/* #define pfB     %rbp */
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
#define PFDISTB 512
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
   mov KK, incAm
   mov KK, KK0
   sub $-128, pC
   sub $-128, pA
   sub $-128, pB
   mov $256, r256
   mov $192, r192
   mov pA, pA0
   mov pB, pB0
/*
 * incAm = MU*sizeof*K = 16*8*K = 128*K
 * incBm = NU*sizeof*K = 8*8*K = 64*K 
 */
   shl $7, incAm           /* incAm = 128*K */
   mov KK, incB
   #ifdef SREAL
      shl $5, incB
   #else
      shl $6, incB
   #endif

   ALIGN16
   MLOOP:
         vbroadcastsd -128(pB), rB0
      NLOOP:
/*
 *       Peel K=1 to zero rCxx
 */
            vmovapd -128(pA), rA0a
         vmulpd rA0a, rB0, rC00
            vmovapd -64(pA), rA1a
         vmulpd rA1a, rB0, rC10

            vbroadcastsd SZ-128(pB), rB1
         vmulpd rA0a, rB1, rC01
            vbroadcastsd 2*SZ-128(pB), rB2
         vmulpd rA1a, rB1, rC11

            vbroadcastsd 3*SZ-128(pB), rB3
         vmulpd rA0a, rB2, rC02
            vbroadcastsd 4*SZ-128(pB), rB4
         vmulpd rA1a, rB2, rC12

            vbroadcastsd 5*SZ-128(pB), rB5
         vmulpd rA0a, rB3, rC03
           vbroadcastsd 6*SZ-128(pB), rB6
         vmulpd rA1a, rB3, rC13

           vbroadcastsd 7*SZ-128(pB), rB7
         vmulpd rA0a, rB4, rC04
           vbroadcastsd 8*SZ-128(pB), rB0
         vmulpd rA1a, rB4, rC14

           vmovapd (pA), rA0b
         vmulpd rA0a, rB5, rC05
           vmovapd 64(pA), rA1b
         vmulpd rA1a, rB5, rC15

           vbroadcastsd 9*SZ-128(pB), rB1
         vmulpd rA0a, rB6, rC06
           vbroadcastsd 10*SZ-128(pB), rB2
         vmulpd rA1a, rB6, rC16

           vbroadcastsd 11*SZ-128(pB), rB3
         vmulpd rA0a, rB7, rC07
           vbroadcastsd 12*SZ-128(pB), rB4
         vmulpd rA1a, rB7, rC17


           vbroadcastsd 13*SZ-128(pB), rB5
         FMAC rA0b, rB0, rC00
           vbroadcastsd 14*SZ-128(pB), rB6
         FMAC rA1b, rB0, rC10
           vbroadcastsd 15*SZ-128(pB), rB7
         FMAC rA0b, rB1, rC01
           vmovapd 128(pA), rA0c
         FMAC rA1b, rB1, rC11
           vmovapd 192(pA), rA1c
         FMAC rA0b, rB2, rC02
           vbroadcastsd 16*SZ-128(pB), rB0
         FMAC rA1b, rB2, rC12
           vbroadcastsd 17*SZ-128(pB), rB1
         FMAC rA0b, rB3, rC03
           vbroadcastsd 18*SZ-128(pB), rB2
         FMAC rA1b, rB3, rC13
           vbroadcastsd 19*SZ-128(pB), rB3
         FMAC rA0b, rB4, rC04
           vmovapd 256(pA), rA0d
         FMAC rA1b, rB4, rC14
           vbroadcastsd 20*SZ-128(pB), rB4
         FMAC rA0b, rB5, rC05
           vmovapd 320(pA), rA1d
         FMAC rA1b, rB5, rC15
           vbroadcastsd 21*SZ-128(pB), rB5
         FMAC rA0b, rB6, rC06
            lea (pA,r256,2),pA
         FMAC rA1b, rB6, rC16
           vbroadcastsd 22*SZ-128(pB), rB6
         FMAC rA0b, rB7, rC07
           vmovapd -128(pA), rA0a
         FMAC rA1b, rB7, rC17
           vbroadcastsd 23*SZ-128(pB), rB7

         FMAC rA0c, rB0, rC00
           vmovapd -64(pA), rA1a
         FMAC rA1c, rB0, rC10
           vbroadcastsd 24*SZ-128(pB), rB0
         FMAC rA0c, rB1, rC01
           vmovapd (pA), rA0b
         FMAC rA1c, rB1, rC11
           vbroadcastsd 25*SZ-128(pB), rB1
         FMAC rA0c, rB2, rC02
           vmovapd 64(pA), rA1b
         FMAC rA1c, rB2, rC12
           vbroadcastsd 26*SZ-128(pB), rB2
         FMAC rA0c, rB3, rC03
            prefC(-128(pC))
         FMAC rA1c, rB3, rC13
           vbroadcastsd 27*SZ-128(pB), rB3
         FMAC rA0c, rB4, rC04
            prefC(-64(pC))
         FMAC rA1c, rB4, rC14
           vbroadcastsd 28*SZ-128(pB), rB4
         FMAC rA0c, rB5, rC05
            prefC((pC))
         FMAC rA1c, rB5, rC15
           vbroadcastsd 29*SZ-128(pB), rB5
         FMAC rA0c, rB6, rC06
            prefC(64(pC))
         FMAC rA1c, rB6, rC16
           vbroadcastsd 30*SZ-128(pB), rB6
         FMAC rA0c, rB7, rC07
           vmovapd 128(pA), rA0c
         FMAC rA1c, rB7, rC17
           vbroadcastsd 31*SZ-128(pB), rB7

         FMAC rA0d, rB0, rC00
            add r256, pB
         FMAC rA1d, rB0, rC10
           vbroadcastsd -128(pB), rB0
         FMAC rA0d, rB1, rC01
           vmovapd 192(pA), rA1c
         FMAC rA1d, rB1, rC11
           vbroadcastsd SZ-128(pB), rB1
         FMAC rA0d, rB2, rC02
            prefC(128(pC))
         FMAC rA1d, rB2, rC12
           vbroadcastsd 2*SZ-128(pB), rB2
         FMAC rA0d, rB3, rC03
            prefC(192(pC))
         FMAC rA1d, rB3, rC13
           vbroadcastsd 3*SZ-128(pB), rB3
         FMAC rA0d, rB4, rC04
            prefC(256(pC))
         FMAC rA1d, rB4, rC14
           vbroadcastsd 4*SZ-128(pB), rB4
         FMAC rA0d, rB5, rC05
            prefC(320(pC))
         FMAC rA1d, rB5, rC15
           vbroadcastsd 5*SZ-128(pB), rB5
         FMAC rA0d, rB6, rC06
            prefC(384(pC))
         FMAC rA1d, rB6, rC16
           vbroadcastsd 6*SZ-128(pB), rB6
         FMAC rA0d, rB7, rC07
           vmovapd 256(pA), rA0d
         FMAC rA1d, rB7, rC17

         sub $4, KK
            vbroadcastsd 7*SZ-128(pB), rB7
         jz KDONE

         KLOOP:
            FMAC rA0a, rB0, rC00
               vmovapd 320(pA), rA1d
            FMAC rA1a, rB0, rC10
               vbroadcastsd  8*SZ-128(pB), rB0
            FMAC rA0a, rB1, rC01
               lea (pA,r256,2),pA
            FMAC rA1a, rB1, rC11
               vbroadcastsd  9*SZ-128(pB), rB1
            FMAC rA0a, rB2, rC02
               prefB(PFDISTB(pB))
            FMAC rA1a, rB2, rC12
               vbroadcastsd 10*SZ-128(pB), rB2
            FMAC rA0a, rB3, rC03
               prefB(64+PFDISTB(pB))
            FMAC rA1a, rB3, rC13
               vbroadcastsd 11*SZ-128(pB), rB3
            FMAC rA0a, rB4, rC04
               prefB(128+PFDISTB(pB))
            FMAC rA1a, rB4, rC14
               vbroadcastsd 12*SZ-128(pB), rB4
            FMAC rA0a, rB5, rC05
               prefB(192+PFDISTB(pB))
            FMAC rA1a, rB5, rC15
               vbroadcastsd 13*SZ-128(pB), rB5
            FMAC rA0a, rB6, rC06
               prefB(256+PFDISTB(pB))
            FMAC rA1a, rB6, rC16
               vbroadcastsd 14*SZ-128(pB), rB6
            FMAC rA0a, rB7, rC07
               vmovapd -128(pA), rA0a
            FMAC rA1a, rB7, rC17
               vbroadcastsd 15*SZ-128(pB), rB7

            FMAC rA0b, rB0, rC00
               vmovapd -64(pA), rA1a
            FMAC rA1b, rB0, rC10
               vbroadcastsd 16*SZ-128(pB), rB0
            FMAC rA0b, rB1, rC01
               prefA(PFDISTA(pA))
            FMAC rA1b, rB1, rC11
               vbroadcastsd 17*SZ-128(pB), rB1
            FMAC rA0b, rB2, rC02
               prefA(64+PFDISTA(pA))
            FMAC rA1b, rB2, rC12
               vbroadcastsd 18*SZ-128(pB), rB2
            FMAC rA0b, rB3, rC03
               prefA(128+PFDISTA(pA))
            FMAC rA1b, rB3, rC13
               vbroadcastsd 19*SZ-128(pB), rB3
            FMAC rA0b, rB4, rC04
               prefA(192+PFDISTA(pA))
            FMAC rA1b, rB4, rC14
               vbroadcastsd 20*SZ-128(pB), rB4
            FMAC rA0b, rB5, rC05
               prefA(256+PFDISTA(pA))
            FMAC rA1b, rB5, rC15
               vbroadcastsd 21*SZ-128(pB), rB5
            FMAC rA0b, rB6, rC06
               prefA(320+PFDISTA(pA))
            FMAC rA1b, rB6, rC16
               vbroadcastsd 22*SZ-128(pB), rB6
            FMAC rA0b, rB7, rC07
               vmovapd (pA), rA0b
            FMAC rA1b, rB7, rC17
               vbroadcastsd 23*SZ-128(pB), rB7

            FMAC rA0c, rB0, rC00
               vmovapd 64(pA), rA1b
            FMAC rA1c, rB0, rC10
               vbroadcastsd 24*SZ-128(pB), rB0
            FMAC rA0c, rB1, rC01
            FMAC rA1c, rB1, rC11
               vbroadcastsd 25*SZ-128(pB), rB1
            FMAC rA0c, rB2, rC02
            FMAC rA1c, rB2, rC12
               vbroadcastsd 26*SZ-128(pB), rB2
            FMAC rA0c, rB3, rC03
            FMAC rA1c, rB3, rC13
               vbroadcastsd 27*SZ-128(pB), rB3
            FMAC rA0c, rB4, rC04
            FMAC rA1c, rB4, rC14
               vbroadcastsd 28*SZ-128(pB), rB4
            FMAC rA0c, rB5, rC05
            FMAC rA1c, rB5, rC15
               vbroadcastsd 29*SZ-128(pB), rB5
            FMAC rA0c, rB6, rC06
            FMAC rA1c, rB6, rC16
               vbroadcastsd 30*SZ-128(pB), rB6
            FMAC rA0c, rB7, rC07
               vmovapd 128(pA), rA0c
            FMAC rA1c, rB7, rC17
               vbroadcastsd 31*SZ-128(pB), rB7

            FMAC rA0d, rB0, rC00
               vmovapd 192(pA), rA1c
            FMAC rA1d, rB0, rC10
               add r256, pB
            FMAC rA0d, rB1, rC01
               vbroadcastsd -128(pB), rB0
            FMAC rA1d, rB1, rC11
               vbroadcastsd SZ-128(pB), rB1
            FMAC rA0d, rB2, rC02
            FMAC rA1d, rB2, rC12
               vbroadcastsd 2*SZ-128(pB), rB2
            FMAC rA0d, rB3, rC03
            FMAC rA1d, rB3, rC13
               vbroadcastsd 3*SZ-128(pB), rB3
            FMAC rA0d, rB4, rC04
               vprefetch1 (pB,incB)
            FMAC rA1d, rB4, rC14
               vbroadcastsd 4*SZ-128(pB), rB4
            FMAC rA0d, rB5, rC05
               vprefetch2 -128(pA,incAm)
            FMAC rA1d, rB5, rC15
               vbroadcastsd 5*SZ-128(pB), rB5
            FMAC rA0d, rB6, rC06
               sub $4, KK
            FMAC rA1d, rB6, rC16
               vbroadcastsd 6*SZ-128(pB), rB6
            FMAC rA0d, rB7, rC07
               vmovapd 256(pA), rA0d
            FMAC rA1d, rB7, rC17
               vbroadcastsd 7*SZ-128(pB), rB7
         jnz KLOOP
KDONE:
         #ifndef BETA0
            VCOP -128(pC), rC00, rC00
            VCOP -64(pC), rC10, rC10
            VCOP (pC), rC01, rC01
            VCOP 64(pC), rC11, rC11
            VCOP 128-256(pC,r256), rC02, rC02
            VCOP 192-256(pC,r256), rC12, rC12
            VCOP (pC,r256), rC03, rC03
            VCOP 320-256(pC,r256), rC13, rC13
            VCOP 384-512(pC,r256,2), rC04, rC04
            VCOP 448-512(pC,r256,2), rC14, rC14
            VCOP (pC,r256,2), rC05, rC05
            VCOP 576-512(pC,r256,2), rC15, rC15
            VCOP 640-768(pC,r192,4), rC06, rC06
            VCOP 704-768(pC,r192,4), rC16, rC16
            VCOP (pC,r192,4), rC07, rC07
            VCOP 832-768(pC,r192,4), rC17, rC17
         #endif
         vmovapd rC00, -128(pC)
         vmovapd rC10, -64(pC)
         vmovapd rC01, (pC)
         vmovapd rC11, 64(pC)
         vmovapd rC02, 128-256(pC,r256)
         vmovapd rC12, 192-256(pC,r256)
         vmovapd rC03, (pC,r256)
         vmovapd rC13, 320-256(pC,r256)
         vmovapd rC04, 384-512(pC,r256,2)
         vmovapd rC14, 448-512(pC,r256,2)
         vmovapd rC05, (pC,r256,2)
         vmovapd rC15, 576-512(pC,r256,2)
         vmovapd rC06, 640-768(pC,r192,4)
         vmovapd rC16, 704-768(pC,r192,4)
         vmovapd rC07, (pC,r192,4)
         vmovapd rC17, 832-768(pC,r192,4)

         mov KK0, KK
         lea (pC,r256,4), pC /* pC += MU*NU*sizeof = 16*8*8 = 1024*/
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
