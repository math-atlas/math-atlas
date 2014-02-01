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
#define pfB     %rbp
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
#define PFBDIST 128
#define PFADIST 128
#define prefA(m_) vprefetch2 m_
#define prefB(m_) vprefetch2 m_
#define prefC(m_) vprefetche1 m_
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
   movq 8(%rsp), pfA      /* pfA = pAn */
   movq 16(%rsp), pfB     /* pfB = pBn */
/*   cmp pfB, pB */
/*   CMOVE pfA, pfB */
   mov KK, incAm
   shr $1, KK             /* KK = K/2 */
   mov KK, KK0
   sub $-128, pC
   sub $-128, pA
//   sub $-128, pfA
//   sub $-128, pfB
   mov $256, r256
   mov $192, r192
   mov pA, pA0
   mov pB, pB0
/*
 * incAm = MU*sizeof*K = 32*8*K = 256*K
 */
   shl $8, incAm           /* incAm = 256*K */

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
               vmovapd (pA), rA2
               vmovapd 64(pA), rA3
               vmovapd 128(pA), ra0
               vmovapd 192(pA), ra1
               vmovapd 256(pA), ra2
               vmovapd 320(pA), ra3
               vbroadcastsd 7*SZ(pB), rb3

            vmulpd rA0, rB0, rC00
//              prefC(-128(pC))
            vmulpd rA1, rB0, rC10
            vmulpd rA2, rB0, rC20
            vmulpd rA3, rB0, rC30

            vmulpd rA0, rB1, rC01
//              prefC((pC))
//              prefC(128-256(pC,r256))
            vmulpd rA1, rB1, rC11
            vmulpd rA2, rB1, rC21
            vmulpd rA3, rB1, rC31

            vmulpd rA0, rB2, rC02
            vmulpd rA1, rB2, rC12
            vmulpd rA2, rB2, rC22
            vmulpd rA3, rB2, rC32

            vmulpd rA0, rB3, rC03
            vmulpd rA1, rB3, rC13
            vmulpd rA2, rB3, rC23
            vmulpd rA3, rB3, rC33
/*
 *          2nd peeled K iteration
 */
               add $512, pA

            FMAC ra0, rb0, rC00
            FMAC ra1, rb0, rC10
            FMAC ra2, rb0, rC20
            FMAC ra3, rb0, rC30

            FMAC ra0, rb1, rC01
            FMAC ra1, rb1, rC11
            FMAC ra2, rb1, rC21
            FMAC ra3, rb1, rC31

            FMAC ra0, rb2, rC02
            FMAC ra1, rb2, rC12
            FMAC ra2, rb2, rC22
            FMAC ra3, rb2, rC32

            FMAC ra0, rb3, rC03
            FMAC ra1, rb3, rC13
            FMAC ra2, rb3, rC23
            FMAC ra3, rb3, rC33
               vmovapd -128(pA), rA0
               vmovapd -64(pA), rA1
               vmovapd (pA), rA2
               vmovapd 64(pA), rA3
               vmovapd 128-256(pA,r256), ra0
               vmovapd 192-256(pA,r256), ra1
               vmovapd (pA,r256), ra2
               vmovapd 320-256(pA,r256), ra3

               vbroadcastsd 8*SZ(pB), rB0
               vbroadcastsd 9*SZ(pB), rB1
               vbroadcastsd 10*SZ(pB), rB2
               vbroadcastsd 11*SZ(pB), rB3
               vbroadcastsd 12*SZ(pB), rb0
               vbroadcastsd 13*SZ(pB), rb1
               vbroadcastsd 14*SZ(pB), rb2
               add $16*SZ, pB
               add $512, pA
               dec %edx
               jz KDONE

         KLOOP:

            FMAC rA0, rB0, rC00
               vbroadcastsd -SZ(pB), rb3
            FMAC rA1, rB0, rC10
            FMAC rA2, rB0, rC20
            FMAC rA3, rB0, rC30
               vbroadcastsd (pB), rB0

            FMAC rA0, rB1, rC01
               vprefetch0 (pfA)
            FMAC rA1, rB1, rC11
            FMAC rA2, rB1, rC21
               add $64, pfA
            FMAC rA3, rB1, rC31
               vbroadcastsd SZ(pB), rB1

            FMAC rA0, rB2, rC02
            FMAC rA1, rB2, rC12
            FMAC rA2, rB2, rC22
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
            FMAC ra2, rb0, rC20
            FMAC ra3, rb0, rC30
               vbroadcastsd 4*SZ(pB), rb0
            FMAC ra0, rb1, rC01
            FMAC ra1, rb1, rC11
            FMAC ra2, rb1, rC21
            FMAC ra3, rb1, rC31
               vbroadcastsd 5*SZ(pB), rb1


            FMAC ra0, rb2, rC02
               add $8*SZ, pB
            FMAC ra1, rb2, rC12
               lea (pA, r256,2), pA
            FMAC ra2, rb2, rC22
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
