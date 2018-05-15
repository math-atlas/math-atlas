#include "atlas_asm.h"

#if defined(SREAL) || defined(SCPLX)
   #define SZ 4
   #define movapd movaps
   #define vmulpd vmulps
   #define vsubpd vsubps
   #define vaddpd vaddps
   #define vfmadd231pd vfmadd231ps
   #define vbroadcastsd vbroadcastss
   #define vmovapd vmovaps
   #ifdef ATL_ARCH_XeonPHI
      #define MOVC vmovnraps
   #else
      #define MOVC vmovaps
   #endif
   #define vbroadcastf64x4 vbroadcastf32x4
#else
   #ifdef ATL_ARCH_XeonPHI
      #define MOVC vmovnrapd
   #else
      #define MOVC vmovapd
   #endif
   #define SZ 8
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
#define incBk   %rbp 
#define incC    %r12
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
#define PFDA0 1*(4*2*64)
#define PFDA2 4*PFDA0
#define PFDA3 4*PFDA2
#define PFDB0 1*(4*8*SZ)
#define PFDB2 8*PFDB0
#ifdef ATL_ARCH_XeonPHI
   #define prefA(m_) vprefetch0 m_
   #define prefB(m_) vprefetch0 m_
   #define prefC(m_) vprefetche0 m_
#else
   #define prefA(m_) prefetcht0 m_
   #define prefB(m_) prefetcht0 m_
   #define prefC(m_) prefetchw m_
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
   mov $256, r256
   mov $1024, incC
   mov pA, pA0
   mov pB, pB0
/*
 * incAm = MU*sizeof*K = 16*8*K = 128*K
 * incBm = NU*sizeof*K = 8*8*K = 64*K
 */
   shl $7, incAm           /* incAm = 128*K */
   mov KK, incB
   #if defined(SREAL) || defined(SCPLX)
      shl $5, incB
   #else
      shl $6, incB
   #endif
   mov $8*4*SZ, incBk     /* incBk = NU*KU*sizeof = 128/256 */

   ALIGN16
   MLOOP:
         vbroadcastf64x4 (pB), rB0    /* {b3,b2,b1,b0,b3,b2,b1,b0} */
         vbroadcastf64x4 4*SZ(pB), rB1
         vbroadcastf64x4 2*4*SZ(pB), rB2
         vbroadcastf64x4 3*4*SZ(pB), rB3
         vbroadcastf64x4 4*4*SZ(pB), rB4
         vbroadcastf64x4 5*4*SZ(pB), rB5
         vbroadcastf64x4 6*4*SZ(pB), rB6

      NLOOP:

         vmovapd (pA), rA0a
            add incBk, pB
         vmovapd 64(pA), rA1a
         vmovapd 128(pA), rA0b
         vmovapd 192(pA), rA1b
/*
 *       Peel K=1 to zero rCxx
 */
         vmulpd rB0{aaaa}, rA0a, rC00
            vmovapd 256(pA), rA0c
         vmulpd rB0{aaaa}, rA1a, rC10
            vmovapd 320(pA), rA1c
         vmulpd rB0{bbbb}, rA0a, rC01
            vmovapd 384(pA), rA0d
         vmulpd rB0{bbbb}, rA1a, rC11
            vmovapd 448(pA), rA1d
         vmulpd rB0{cccc}, rA0a, rC02
            lea (pA,r256,2),pA
         vmulpd rB0{cccc}, rA1a, rC12
            vbroadcastf64x4 -4*SZ(pB), rB7
         vmulpd rB0{dddd}, rA0a, rC03
            vprefetch1 PFDB2(pB)
         vmulpd rB0{dddd}, rA1a, rC13
            vprefetch1 64+PFDB2(pB)
         vmulpd rB1{aaaa}, rA0a, rC04
            vprefetch1 2*64+PFDB2(pB)
         vmulpd rB1{aaaa}, rA1a, rC14
            vprefetch1 3*64+PFDB2(pB)
         vmulpd rB1{bbbb}, rA0a, rC05
            vprefetch1 4*64+PFDB2(pB)
         vmulpd rB1{bbbb}, rA1a, rC15
            vprefetch1 5*64+PFDB2(pB)
         vmulpd rB1{cccc}, rA0a, rC06
            vprefetch1 6*64+PFDB2(pB)
         vmulpd rB1{cccc}, rA1a, rC16
            vprefetch1 7*64+PFDB2(pB)
         vmulpd rB1{dddd}, rA0a, rC07
            vmovapd  (pA), rA0a
         vmulpd rB1{dddd}, rA1a, rC17
            vmovapd 64(pA), rA1a

         FMAC rB2{aaaa}, rA0b, rC00
            vbroadcastf64x4 (pB), rB0
         FMAC rB2{aaaa}, rA1b, rC10
            vbroadcastf64x4 4*SZ(pB), rB1
         FMAC rB2{bbbb}, rA0b, rC01
            vprefetch0 PFDB0(pB)
         FMAC rB2{bbbb}, rA1b, rC11
            vprefetch0 64+PFDB0(pB)
         FMAC rB2{cccc}, rA0b, rC02
            vprefetch0 2*64+PFDB0(pB)
         FMAC rB2{cccc}, rA1b, rC12
            vprefetch0 3*64+PFDB0(pB)
         FMAC rB2{dddd}, rA0b, rC03
            vprefetch0 4*64+PFDB0(pB)
         FMAC rB2{dddd}, rA1b, rC13
            vprefetch0 5*64+PFDB0(pB)
         FMAC rB3{aaaa}, rA0b, rC04
            vprefetch0 6*64+PFDB0(pB)
         FMAC rB3{aaaa}, rA1b, rC14
            vprefetch0 7*64+PFDB0(pB)
         FMAC rB3{bbbb}, rA0b, rC05
            vprefetch1 PFDA2(pA)
         FMAC rB3{bbbb}, rA1b, rC15
            vprefetch1 64+PFDA2(pA)
         FMAC rB3{cccc}, rA0b, rC06
            vprefetch1 2*64+PFDA2(pA)
         FMAC rB3{cccc}, rA1b, rC16
            vprefetch1 3*64+PFDA2(pA)
         FMAC rB3{dddd}, rA0b, rC07
            vmovapd 128(pA), rA0b
         FMAC rB3{dddd}, rA1b, rC17
            vmovapd 192(pA), rA1b

         FMAC rB4{aaaa}, rA0c, rC00
            vbroadcastf64x4 2*4*SZ(pB), rB2
         FMAC rB4{aaaa}, rA1c, rC10
            vbroadcastf64x4 3*4*SZ(pB), rB3
         FMAC rB4{bbbb}, rA0c, rC01
            vprefetch1 4*64+PFDA2(pA)
         FMAC rB4{bbbb}, rA1c, rC11
            vprefetch1 5*64+PFDA2(pA)
         FMAC rB4{cccc}, rA0c, rC02
            vprefetch1 6*64+PFDA2(pA)
         FMAC rB4{cccc}, rA1c, rC12
            vprefetch1 7*64+PFDA2(pA)
         FMAC rB4{dddd}, rA0c, rC03
            vprefetch0 PFDA0(pA)
         FMAC rB4{dddd}, rA1c, rC13
            vprefetch0 64+PFDA0(pA)
         FMAC rB5{aaaa}, rA0c, rC04
            vprefetch0 2*64+PFDA0(pA)
         FMAC rB5{aaaa}, rA1c, rC14
            vprefetch0 3*64+PFDA0(pA)
         FMAC rB5{bbbb}, rA0c, rC05
            vprefetch0 4*64+PFDA0(pA)
         FMAC rB5{bbbb}, rA1c, rC15
            vprefetch0 5*64+PFDA0(pA)
         FMAC rB5{cccc}, rA0c, rC06
            vprefetch0 6*64+PFDA0(pA)
         FMAC rB5{cccc}, rA1c, rC16
            vprefetch0 7*64+PFDA0(pA)
         FMAC rB5{dddd}, rA0c, rC07
            vmovapd 256(pA), rA0c
         FMAC rB5{dddd}, rA1c, rC17
            vmovapd 320(pA), rA1c

         FMAC rB6{aaaa}, rA0d, rC00
            vbroadcastf64x4 4*4*SZ(pB), rB4
         FMAC rB6{aaaa}, rA1d, rC10
            vbroadcastf64x4 5*4*SZ(pB), rB5
         FMAC rB6{bbbb}, rA0d, rC01
            vprefetch2 PFDB2(pB)
         FMAC rB6{bbbb}, rA1d, rC11
            vprefetch2 1*64+PFDB2(pB)
         FMAC rB6{cccc}, rA0d, rC02
            vprefetch2 2*64+PFDB2(pB)
         FMAC rB6{cccc}, rA1d, rC12
            vprefetch2 3*64+PFDB2(pB)
         FMAC rB6{dddd}, rA0d, rC03
            vprefetch2 4*64+PFDB2(pB)
         FMAC rB6{dddd}, rA1d, rC13
            vbroadcastf64x4 6*4*SZ(pB), rB6
         FMAC rB7{aaaa}, rA0d, rC04
            vprefetch2 5*64+PFDB2(pB)
         FMAC rB7{aaaa}, rA1d, rC14
            vprefetch2 6*64+PFDB2(pB)
         FMAC rB7{bbbb}, rA0d, rC05
            vprefetch2 7*64+PFDB2(pB)
         FMAC rB7{bbbb}, rA1d, rC15
         FMAC rB7{cccc}, rA0d, rC06
            sub $4, KK
         FMAC rB7{cccc}, rA1d, rC16
         FMAC rB7{dddd}, rA0d, rC07
            vmovapd 384(pA), rA0d
         FMAC rB7{dddd}, rA1d, rC17
            vmovapd 448(pA), rA1d
         jz KDONE

         KLOOP:
               add incBk, pB
            FMAC rB0{aaaa}, rA0a, rC00
               vbroadcastf64x4 -4*SZ(pB), rB7
            FMAC rB0{aaaa}, rA1a, rC10
               vprefetch1 PFDB2(pB)
            FMAC rB0{bbbb}, rA0a, rC01
               lea (pA,r256,2),pA
            FMAC rB0{bbbb}, rA1a, rC11
               vprefetch1 64+PFDB2(pB)
            FMAC rB0{cccc}, rA0a, rC02
               vprefetch1 2*64+PFDB2(pB)
            FMAC rB0{cccc}, rA1a, rC12
               vprefetch1 3*64+PFDB2(pB)
            FMAC rB0{dddd}, rA0a, rC03
               vprefetch1 4*64+PFDB2(pB)
            FMAC rB0{dddd}, rA1a, rC13
               vprefetch1 5*64+PFDB2(pB)
            FMAC rB1{aaaa}, rA0a, rC04
               vprefetch1 6*64+PFDB2(pB)
            FMAC rB1{aaaa}, rA1a, rC14
               vprefetch1 7*64+PFDB2(pB)
            FMAC rB1{bbbb}, rA0a, rC05
               vprefetch0 PFDB0(pB)
            FMAC rB1{bbbb}, rA1a, rC15
               vprefetch0 64+PFDB0(pB)
            FMAC rB1{cccc}, rA0a, rC06
               vprefetch0 2*64+PFDB0(pB)
            FMAC rB1{cccc}, rA1a, rC16
               vprefetch0 3*64+PFDB0(pB)
            FMAC rB1{dddd}, rA0a, rC07
               vmovapd (pA), rA0a
            FMAC rB1{dddd}, rA1a, rC17
               vmovapd 64(pA), rA1a

            FMAC rB2{aaaa}, rA0b, rC00
               vbroadcastf64x4 (pB), rB0
            FMAC rB2{aaaa}, rA1b, rC10
               vbroadcastf64x4 4*SZ(pB), rB1
            FMAC rB2{bbbb}, rA0b, rC01
               vprefetch0 4*64+PFDB0(pB)
            FMAC rB2{bbbb}, rA1b, rC11
               vprefetch0 5*64+PFDB0(pB)
            FMAC rB2{cccc}, rA0b, rC02
               vprefetch0 6*64+PFDB0(pB)
            FMAC rB2{cccc}, rA1b, rC12
               vprefetch0 7*64+PFDB0(pB)
            FMAC rB2{dddd}, rA0b, rC03
               vprefetch1 PFDA2(pA)
            FMAC rB2{dddd}, rA1b, rC13
               vprefetch1 64+PFDA2(pA)
            FMAC rB3{aaaa}, rA0b, rC04
               vprefetch1 2*64+PFDA2(pA)
            FMAC rB3{aaaa}, rA1b, rC14
               vprefetch1 3*64+PFDA2(pA)
            FMAC rB3{bbbb}, rA0b, rC05
               vprefetch1 4*64+PFDA2(pA)
            FMAC rB3{bbbb}, rA1b, rC15
               vprefetch1 5*64+PFDA2(pA)
            FMAC rB3{cccc}, rA0b, rC06
               vprefetch1 6*64+PFDA2(pA)
            FMAC rB3{cccc}, rA1b, rC16
               vprefetch1 7*64+PFDA2(pA)
            FMAC rB3{dddd}, rA0b, rC07
               vmovapd 128(pA), rA0b
            FMAC rB3{dddd}, rA1b, rC17
               vmovapd 192(pA), rA1b

            FMAC rB4{aaaa}, rA0c, rC00
               vbroadcastf64x4 2*4*SZ(pB), rB2
            FMAC rB4{aaaa}, rA1c, rC10
               vbroadcastf64x4 3*4*SZ(pB), rB3
            FMAC rB4{bbbb}, rA0c, rC01
               vprefetch0 PFDA0(pA)
            FMAC rB4{bbbb}, rA1c, rC11
               vprefetch0 64+PFDA0(pA)
            FMAC rB4{cccc}, rA0c, rC02
               vprefetch0 2*64+PFDA0(pA)
            FMAC rB4{cccc}, rA1c, rC12
               vprefetch0 3*64+PFDA0(pA)
            FMAC rB4{dddd}, rA0c, rC03
               vprefetch0 4*64+PFDA0(pA)
            FMAC rB4{dddd}, rA1c, rC13
               vprefetch0 5*64+PFDA0(pA)
            FMAC rB5{aaaa}, rA0c, rC04
               vprefetch0 6*64+PFDA0(pA)
            FMAC rB5{aaaa}, rA1c, rC14
               vprefetch0 7*64+PFDA0(pA)
            FMAC rB5{bbbb}, rA0c, rC05
            FMAC rB5{bbbb}, rA1c, rC15
               vprefetch2 PFDB2(pB)
            FMAC rB5{cccc}, rA0c, rC06
               vprefetch2 1*64+PFDB2(pB)
            FMAC rB5{cccc}, rA1c, rC16
               vprefetch2 2*64+PFDB2(pB)
            FMAC rB5{dddd}, rA0c, rC07
               vmovapd 256(pA), rA0c
            FMAC rB5{dddd}, rA1c, rC17
               vmovapd 320(pA), rA1c

            FMAC rB6{aaaa}, rA0d, rC00
               vbroadcastf64x4 4*4*SZ(pB), rB4
            FMAC rB6{aaaa}, rA1d, rC10
            FMAC rB6{bbbb}, rA0d, rC01
               vbroadcastf64x4 5*4*SZ(pB), rB5
            FMAC rB6{bbbb}, rA1d, rC11
               vprefetch2 3*64+PFDB2(pB)
            FMAC rB6{cccc}, rA0d, rC02
               vprefetch2 4*64+PFDB2(pB)
            FMAC rB6{cccc}, rA1d, rC12
               vprefetch2 5*64+PFDB2(pB)
            FMAC rB6{dddd}, rA0d, rC03
               vprefetch2 6*64+PFDB2(pB)
            FMAC rB6{dddd}, rA1d, rC13
               vbroadcastf64x4 6*4*SZ(pB), rB6
            FMAC rB7{aaaa}, rA0d, rC04
               vprefetch2 7*64+PFDB2(pB)
            FMAC rB7{aaaa}, rA1d, rC14
            FMAC rB7{bbbb}, rA0d, rC05
            FMAC rB7{bbbb}, rA1d, rC15
            FMAC rB7{cccc}, rA0d, rC06
               sub $4, KK
            FMAC rB7{cccc}, rA1d, rC16
            FMAC rB7{dddd}, rA0d, rC07
               vmovapd 384(pA), rA0d
            FMAC rB7{dddd}, rA1d, rC17
               vmovapd 448(pA), rA1d
         jnz KLOOP
KDONE:
         #ifndef BETA0
            VCOP -128(pC), rC00, rC00
            VCOP -64(pC), rC10, rC10
            VCOP (pC), rC01, rC01
            VCOP 64(pC), rC11, rC11
            VCOP 128(pC), rC02, rC02
            VCOP 192(pC), rC12, rC12
            VCOP 256(pC), rC03, rC03
            VCOP 320(pC), rC13, rC13
         #endif
         MOVC rC00, -128(pC)
            prefC(-128(pC,incC))
         MOVC rC10, -64(pC)
            prefC(-64(pC,incC))
         MOVC rC01, (pC)
            prefC((pC,incC))
         MOVC rC11, 64(pC)
            prefC(64(pC,incC))
         MOVC rC02, 128(pC)
            prefC(128(pC,incC))
         MOVC rC12, 192(pC)
            prefC(192(pC,incC))
         MOVC rC03, 256(pC)
            prefC(256(pC,incC))
         MOVC rC13, 320(pC)
            prefC(320(pC,incC))
         #ifndef BETA0
            VCOP 384(pC), rC04, rC04
            VCOP 448(pC), rC14, rC14
            VCOP 512(pC), rC05, rC05
            VCOP 576(pC), rC15, rC15
            VCOP 640(pC), rC06, rC06
            VCOP 704(pC), rC16, rC16
            VCOP 768(pC), rC07, rC07
            VCOP 832(pC), rC17, rC17
         #endif
         MOVC rC04, 384(pC)
            prefC(384(pC,incC))
         MOVC rC14, 448(pC)
            prefC(448(pC,incC))
         MOVC rC05, 512(pC)
            prefC(448(pC,incC))
         MOVC rC15, 576(pC)
            prefC(576(pC,incC))
         MOVC rC06, 640(pC)
            prefC(640(pC,incC))
         MOVC rC16, 704(pC)
            prefC(704(pC,incC))
         MOVC rC07, 768(pC)
            prefC(768(pC,incC))
         MOVC rC17, 832(pC)
            prefC(832(pC,incC))

         mov KK0, KK
         add incC, pC   /* pC += MU*NU*sizeof = 16*8*8 = 1024*/
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
