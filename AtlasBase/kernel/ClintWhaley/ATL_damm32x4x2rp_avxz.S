#include "atlas_asm.h"

#if defined(SREAL) || defined(SCPLX)
   #define SZ 4
   #define movapd movaps
   #define vmulpd vmulps
   #define vsubpd vsubps
   #define vaddpd vaddps
   #define vfmadd231pd vfmadd231ps
   #define vbroadcastsd vbroadcastss
#else
   #define vmovapd vmovaps
   #define SZ 8
#endif

#define rB0     %zmm0
#define rB1     %zmm1
#define rB2     %zmm2
#define rB3     %zmm3
#define rA0     %zmm4
#define rA1     %zmm5
#define rA2     %zmm6
#define rA3     %zmm7
#define rb0     %zmm8
#define rb1     %zmm9
#define rb2     %zmm10
#define rb3     %zmm11
#define ra0     %zmm12
#define ra1     %zmm13
#define ra2     %zmm14
#define ra3     %zmm15
#define rC00    %zmm16
#define rC10    %zmm17
#define rC20    %zmm18
#define rC30    %zmm19
#define rC01    %zmm20
#define rC11    %zmm21
#define rC21    %zmm22
#define rC31    %zmm23
#define rC02    %zmm24
#define rC12    %zmm25
#define rC22    %zmm26
#define rC32    %zmm27
#define rC03    %zmm28
#define rC13    %zmm29
#define rC23    %zmm30
#define rC33    %zmm31

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
     prefA((pA))
   mov %rsi, nnu
   mov %r8, pB
     prefB((pB))
   mov %r9, pC
     prefA(64(pA))
   mov $256, r256
     prefA(128(pA))
   mov KK, incAm
     prefA(192-256(pA,r256))
   shr $1, KK             /* KK = K/2 */
     prefA((pA,r256))
   mov KK, KK0
     prefA(320-256(pA,r256))
   sub $-128, pC
     prefA(384-512(pA,r256,2))
   sub $-128, pA
   mov nnu, nnu0
     prefA(448-512(pA,r256,2))
   mov $192, r192
     prefA((pA,r256,2))
   mov pA, pA0
     prefA(576-512(pA,r256,2))
   mov pB, pB0
/*
 * incAm = MU*sizeof*K = 32*8*K = 256*K
 * incB = NU*sizeof*K = 4*8*K = 32*K
 */
   shl $8, incAm           /* incAm = 256*K */
     prefA(640-768(pA,r192,4))
   mov KK, incB
     prefA(704-768(pA,r192,4))
   shl $5, incB

   ALIGN16
   MLOOP:
               vbroadcastsd (pB), rB0
               vbroadcastsd SZ(pB), rB1
               vbroadcastsd 2*SZ(pB), rB2
               vbroadcastsd 3*SZ(pB), rB3
               vbroadcastsd 4*SZ(pB), rb0
               vbroadcastsd 5*SZ(pB), rb1
               vbroadcastsd 6*SZ(pB), rb2
      NLOOP:
/*
 *       First iteration peeled to handle init of rC
 */
            vmovapd -128(pA), rA0
            vmovapd -64(pA), rA1

         vmulpd rA0, rB0, rC00
            vmovapd (pA), rA2
         vmulpd rA1, rB0, rC10
            vmovapd 64(pA), rA3
         vmulpd rA2, rB0, rC20
            vmovapd 128(pA), ra0
         vmulpd rA3, rB0, rC30
            vmovapd 192(pA), ra1

         vmulpd rA0, rB1, rC01
            vmovapd 256(pA), ra2
         vmulpd rA1, rB1, rC11
            vmovapd 320(pA), ra3
         vmulpd rA2, rB1, rC21
            vbroadcastsd 7*SZ(pB), rb3
         vmulpd rA3, rB1, rC31

         vmulpd rA0, rB2, rC02
            add $512, pA
         vmulpd rA1, rB2, rC12
            vbroadcastsd 8*SZ(pB), rB0
         vmulpd rA2, rB2, rC22
            vbroadcastsd 9*SZ(pB), rB1
         vmulpd rA3, rB2, rC32
            vbroadcastsd 10*SZ(pB), rB2

         vmulpd rA0, rB3, rC03
            vmovapd -128(pA), rA0
         vmulpd rA1, rB3, rC13
            vmovapd -64(pA), rA1
         vmulpd rA2, rB3, rC23
            vmovapd (pA), rA2
         vmulpd rA3, rB3, rC33
            vmovapd 64(pA), rA3
/*
 *       2nd peeled K iteration
 */

         FMAC ra0, rb0, rC00
            vbroadcastsd 11*SZ(pB), rB3
         FMAC ra1, rb0, rC10
            prefC(-128(pC))
         FMAC ra2, rb0, rC20
            prefC(-64(pC))
         FMAC ra3, rb0, rC30
            vbroadcastsd 12*SZ(pB), rb0

         FMAC ra0, rb1, rC01
            prefC((pC))
         FMAC ra1, rb1, rC11
            prefC(64(pC))
         FMAC ra2, rb1, rC21
            prefC(128-256(pC,r256))
         FMAC ra3, rb1, rC31
            vbroadcastsd 13*SZ(pB), rb1

         FMAC ra0, rb2, rC02
            prefC(192-256(pC,r256))
         FMAC ra1, rb2, rC12
            prefC((pC,r256))
         FMAC ra2, rb2, rC22
            prefC(320-256(pC,r256))
         FMAC ra3, rb2, rC32
            vbroadcastsd 14*SZ(pB), rb2

         FMAC ra0, rb3, rC03
            vmovapd 128-256(pA,r256), ra0
         FMAC ra1, rb3, rC13
            vmovapd 192-256(pA,r256), ra1
         FMAC ra2, rb3, rC23
            vmovapd (pA,r256), ra2
         FMAC ra3, rb3, rC33
            vmovapd 320-256(pA,r256), ra3

         add $16*SZ, pB
         add $512, pA
         dec %edx
           prefC(384-512(pC,r256,2))
         jz KDONE
/*
 *       Next 4 K-its unrolled to finish prefetch of C
 */
         FMAC rA0, rB0, rC00
         vbroadcastsd -SZ(pB), rb3
         FMAC rA1, rB0, rC10
            prefB(PFDISTB(pB))
         FMAC rA2, rB0, rC20
           prefC(448-512(pC,r256,2))
         FMAC rA3, rB0, rC30
            vbroadcastsd (pB), rB0

         FMAC rA0, rB1, rC01
            prefA(PFDISTA(pA))
         FMAC rA1, rB1, rC11
            prefA(64+PFDISTA(pA))
         FMAC rA2, rB1, rC21
            prefA(128+PFDISTA(pA))
         FMAC rA3, rB1, rC31
            vbroadcastsd SZ(pB), rB1

         FMAC rA0, rB2, rC02
            prefA(192+PFDISTA(pA))
         FMAC rA1, rB2, rC12
            prefA(256+PFDISTA(pA))
         FMAC rA2, rB2, rC22
            prefA(320+PFDISTA(pA))
         FMAC rA3, rB2, rC32
            vbroadcastsd 2*SZ(pB), rB2

         FMAC rA0, rB3, rC03
            vmovapd -128(pA), rA0
         FMAC rA1, rB3, rC13
            vmovapd -64(pA), rA1
         FMAC rA2, rB3, rC23
            vmovapd (pA), rA2
         FMAC rA3, rB3, rC33
            vmovapd 64(pA), rA3

         FMAC ra0, rb0, rC00
            vbroadcastsd 3*SZ(pB), rB3
         FMAC ra1, rb0, rC10
            prefA(384+PFDISTA(pA))
         FMAC ra2, rb0, rC20
           prefC((pC,r256,2))
         FMAC ra3, rb0, rC30
            vbroadcastsd 4*SZ(pB), rb0
         FMAC ra0, rb1, rC01
           prefC(576-512(pC,r256,2))
         FMAC ra1, rb1, rC11
            vprefetch1 (pB,incB)
         FMAC ra2, rb1, rC21
            vprefetch2 -128(pA,incAm)
         FMAC ra3, rb1, rC31
            vbroadcastsd 5*SZ(pB), rb1


         FMAC ra0, rb2, rC02
            prefA(448+PFDISTA(pA))
         FMAC ra1, rb2, rC12
            add $8*SZ, pB
         FMAC ra2, rb2, rC22
            lea (pA, r256,2), pA
         FMAC ra3, rb2, rC32
            vbroadcastsd -2*SZ(pB), rb2

         FMAC ra0, rb3, rC03
            vmovapd 128-512(pA), ra0
         FMAC ra1, rb3, rC13
            vmovapd 192-512(pA), ra1
         FMAC ra2, rb3, rC23
            vmovapd 256-512(pA), ra2
         FMAC ra3, rb3, rC33
            vmovapd 320-512(pA), ra3

         dec %edx   /* dec KK */
           prefC(640-768(pC,r192,4))
         jz KDONE

         FMAC rA0, rB0, rC00
         vbroadcastsd -SZ(pB), rb3
         FMAC rA1, rB0, rC10
            prefB(PFDISTB(pB))
         FMAC rA2, rB0, rC20
           prefC(704-768(pC,r192,4))
         FMAC rA3, rB0, rC30
            vbroadcastsd (pB), rB0

         FMAC rA0, rB1, rC01
            prefA(PFDISTA(pA))
         FMAC rA1, rB1, rC11
            prefA(64+PFDISTA(pA))
         FMAC rA2, rB1, rC21
            prefA(128+PFDISTA(pA))
         FMAC rA3, rB1, rC31
            vbroadcastsd SZ(pB), rB1

         FMAC rA0, rB2, rC02
            prefA(192+PFDISTA(pA))
         FMAC rA1, rB2, rC12
            prefA(256+PFDISTA(pA))
         FMAC rA2, rB2, rC22
            prefA(320+PFDISTA(pA))
         FMAC rA3, rB2, rC32
            vbroadcastsd 2*SZ(pB), rB2

         FMAC rA0, rB3, rC03
            vmovapd -128(pA), rA0
         FMAC rA1, rB3, rC13
            vmovapd -64(pA), rA1
         FMAC rA2, rB3, rC23
            vmovapd (pA), rA2
         FMAC rA3, rB3, rC33
            vmovapd 64(pA), rA3

         FMAC ra0, rb0, rC00
            vbroadcastsd 3*SZ(pB), rB3
         FMAC ra1, rb0, rC10
            prefA(384+PFDISTA(pA))
         FMAC ra2, rb0, rC20
           prefC(678-768(pC,r192,4))
         FMAC ra3, rb0, rC30
            vbroadcastsd 4*SZ(pB), rb0
         FMAC ra0, rb1, rC01
           prefC(832-768(pC,r192,4))
         FMAC ra1, rb1, rC11
            vprefetch1 (pB,incB)
         FMAC ra2, rb1, rC21
            vprefetch2 -128(pA,incAm)
         FMAC ra3, rb1, rC31
            vbroadcastsd 5*SZ(pB), rb1


         FMAC ra0, rb2, rC02
            prefA(448+PFDISTA(pA))
         FMAC ra1, rb2, rC12
            add $8*SZ, pB
         FMAC ra2, rb2, rC22
            lea (pA, r256,2), pA
         FMAC ra3, rb2, rC32
            vbroadcastsd -2*SZ(pB), rb2

         FMAC ra0, rb3, rC03
            vmovapd 128-512(pA), ra0
         FMAC ra1, rb3, rC13
            vmovapd 192-512(pA), ra1
         FMAC ra2, rb3, rC23
            vmovapd 256-512(pA), ra2
         FMAC ra3, rb3, rC33
            vmovapd 320-512(pA), ra3

         dec %edx   /* dec KK */
         jz KDONE

         KLOOP:
            FMAC rA0, rB0, rC00
               vbroadcastsd -SZ(pB), rb3
            FMAC rA1, rB0, rC10
               prefB(PFDISTB(pB))
            FMAC rA2, rB0, rC20
            FMAC rA3, rB0, rC30
               vbroadcastsd (pB), rB0

            FMAC rA0, rB1, rC01
               prefA(PFDISTA(pA))
            FMAC rA1, rB1, rC11
               prefA(64+PFDISTA(pA))
            FMAC rA2, rB1, rC21
               prefA(128+PFDISTA(pA))
            FMAC rA3, rB1, rC31
               vbroadcastsd SZ(pB), rB1

            FMAC rA0, rB2, rC02
               prefA(192+PFDISTA(pA))
            FMAC rA1, rB2, rC12
               prefA(256+PFDISTA(pA))
            FMAC rA2, rB2, rC22
               prefA(320+PFDISTA(pA))
            FMAC rA3, rB2, rC32
               vbroadcastsd 2*SZ(pB), rB2

            FMAC rA0, rB3, rC03
               vmovapd -128(pA), rA0
            FMAC rA1, rB3, rC13
               vmovapd -64(pA), rA1
            FMAC rA2, rB3, rC23
               vmovapd (pA), rA2
            FMAC rA3, rB3, rC33
               vmovapd 64(pA), rA3

            FMAC ra0, rb0, rC00
               vbroadcastsd 3*SZ(pB), rB3
            FMAC ra1, rb0, rC10
               prefA(384+PFDISTA(pA))
            FMAC ra2, rb0, rC20
            FMAC ra3, rb0, rC30
               vbroadcastsd 4*SZ(pB), rb0
            FMAC ra0, rb1, rC01
            FMAC ra1, rb1, rC11
               vprefetch1 (pB,incB)
            FMAC ra2, rb1, rC21
               vprefetch2 -128(pA,incAm)
            FMAC ra3, rb1, rC31
               vbroadcastsd 5*SZ(pB), rb1


            FMAC ra0, rb2, rC02
               prefA(448+PFDISTA(pA))
            FMAC ra1, rb2, rC12
               add $8*SZ, pB
            FMAC ra2, rb2, rC22
               lea (pA, r256,2), pA
            FMAC ra3, rb2, rC32
               vbroadcastsd -2*SZ(pB), rb2

            FMAC ra0, rb3, rC03
               vmovapd 128-512(pA), ra0
            FMAC ra1, rb3, rC13
               vmovapd 192-512(pA), ra1
            FMAC ra2, rb3, rC23
               vmovapd 256-512(pA), ra2
            FMAC ra3, rb3, rC33

            dec %edx   /* dec KK */
            vmovapd 320-512(pA), ra3
         jnz KLOOP
KDONE:

         #ifndef BETA0
            VCOP -128(pC), rC00, rC00
            VCOP -64(pC), rC10, rC10
            VCOP (pC), rC20, rC20
            VCOP 64(pC), rC30, rC30
         #endif
         vmovapd rC00, -128(pC)
         vmovapd rC10, -64(pC)
         vmovapd rC20, (pC)
         vmovapd rC30, 64(pC)

         #ifndef BETA0
            VCOP 128-256(pC,r256), rC01, rC01
            VCOP 192-256(pC,r256), rC11, rC11
            VCOP (pC,r256), rC21, rC21
            VCOP 320-256(pC,r256), rC31, rC31
         #endif
         vmovapd rC01, 128-256(pC,r256)
         vmovapd rC11, 192-256(pC,r256)
         vmovapd rC21, (pC,r256)
         vmovapd rC31, 320-256(pC,r256)

         #ifndef BETA0
            VCOP 384-512(pC,r256,2), rC02, rC02
            VCOP 448-512(pC,r256,2), rC12, rC12
            VCOP 512-512(pC,r256,2), rC22, rC22
            VCOP 576-512(pC,r256,2), rC32, rC32
         #endif
         vmovapd rC02, 384-512(pC,r256,2)
         vmovapd rC12, 448-512(pC,r256,2)
         vmovapd rC22, (pC,r256,2)
         vmovapd rC32, 576-512(pC,r256,2)

         #ifndef BETA0
            VCOP 640(pC), rC03, rC03
            VCOP 704(pC), rC13, rC13
            VCOP 768(pC), rC23, rC23
            VCOP 832(pC), rC33, rC33
         #endif
         vmovapd rC03, 640(pC)
         vmovapd rC13, 704(pC)
         vmovapd rC23, 768(pC)
         vmovapd rC33, 832(pC)

         mov KK0, KK
         mov pA0, pA
         sub $8*SZ, pB
         add $1024, pC  /* pC += MU*NU*sizeof = 32*4*8 = 1024 */
         sub $1, nnu
      jnz NLOOP
      mov nnu0, nnu
      add incAm, pA0
      mov pA0, pA
      mov pB0, pB
      mov KK0, KK
      sub $1, nmu
   jnz MLOOP
 DONE:
   movq -8(%rsp), %rbp
   movq -16(%rsp), %rbx
   movq -24(%rsp), %r12
   movq -32(%rsp), %r13
   movq -40(%rsp), %r14
   movq -48(%rsp), %r15
   ret
