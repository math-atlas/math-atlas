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
#define rA0     %zmm2
#define rA1     %zmm3
#define rA2     %zmm4
#define rA3     %zmm5
#define rC00    %zmm6
#define rC10    %zmm7
#define rC20    %zmm8
#define rC30    %zmm9
#define rC01    %zmm10
#define rC11    %zmm11
#define rC21    %zmm12
#define rC31    %zmm13
#define rC02    %zmm14
#define rC12    %zmm15
#define rC22    %zmm16
#define rC32    %zmm17
#define rC03    %zmm18
#define rC13    %zmm19
#define rC23    %zmm20
#define rC33    %zmm21
#define rC04    %zmm22
#define rC14    %zmm23
#define rC24    %zmm24
#define rC34    %zmm25
#define rC05    %zmm26
#define rC15    %zmm27
#define rC25    %zmm28
#define rC35    %zmm29

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
#define prefA(m_) vprefetche2 m_
#define prefB(m_) vprefetche2 m_
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
   movq 16(%rsp), pfB     /* pfB = pBn */
/*   cmp pfB, pB */
/*   CMOVE pfA, pfB */
   mov KK, incAm
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
//               vbroadcastsd -128(pB), rB0

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
            vmovapd 64(pA), rA3
         vmulpd rA3, rB0, rC30

            vbroadcastsd SZ(pB), rB1
         vmulpd rA0, rB1, rC01
            vbroadcastsd 2*SZ(pB), rB0
         vmulpd rA1, rB1, rC11
         vmulpd rA2, rB1, rC21
         vmulpd rA3, rB1, rC31
           vbroadcastsd 3*SZ(pB), rB1

         vmulpd rA0, rB0, rC02
            vprefetche0 -128(pC)
         vmulpd rA1, rB0, rC12
         vmulpd rA2, rB0, rC22
         vmulpd rA3, rB0, rC32
           vbroadcastsd 4*SZ(pB), rB0

         vmulpd rA0, rB1, rC03
            vprefetche0 -64(pC)
         vmulpd rA1, rB1, rC13
            add r256, pA
         vmulpd rA2, rB1, rC23
         vmulpd rA3, rB1, rC33
           vbroadcastsd 5*SZ(pB), rB1

         vmulpd rA0, rB0, rC04
            vprefetche0 (pC)
         vmulpd rA1, rB0, rC14
         vmulpd rA2, rB0, rC24
         dec %edx
         vmulpd rA3, rB0, rC34
           vbroadcastsd 6*SZ(pB), rB0

         vmulpd rA0, rB1, rC05
           vmovapd -128(pA), rA0
         vmulpd rA1, rB1, rC15
           vmovapd -64(pA), rA1
         vmulpd rA2, rB1, rC25
           vmovapd (pA), rA2
         vmulpd rA3, rB1, rC35
         jz KDONE
           vmovapd 64(pA), rA3
           vbroadcastsd 7*SZ(pB), rB1

         KLOOP:
            FMAC rA0, rB0, rC00
               add $6*SZ, pB
            FMAC rA1, rB0, rC10
               add r256, pA
            FMAC rA2, rB0, rC20
            FMAC rA3, rB0, rC30
              vbroadcastsd 2*SZ(pB), rB0

            FMAC rA0, rB1, rC01
            FMAC rA1, rB1, rC11
            FMAC rA2, rB1, rC21
            FMAC rA3, rB1, rC31
              vbroadcastsd 3*SZ(pB), rB1

            FMAC rA0, rB0, rC02
            FMAC rA1, rB0, rC12
            FMAC rA2, rB0, rC22
            FMAC rA3, rB0, rC32
              vbroadcastsd 4*SZ(pB), rB0

            FMAC rA0, rB1, rC03
               vprefetch2 (pfA)
            FMAC rA1, rB1, rC13
            FMAC rA2, rB1, rC23
               add $48, pfA
            FMAC rA3, rB1, rC33
              vbroadcastsd 5*SZ(pB), rB1

            FMAC rA0, rB0, rC04
            FMAC rA1, rB0, rC14
            FMAC rA2, rB0, rC24
            sub $1, %rdx
            FMAC rA3, rB0, rC34
              vbroadcastsd 6*SZ(pB), rB0

            FMAC rA0, rB1, rC05
              vmovapd -128(pA), rA0
            FMAC rA1, rB1, rC15
              vmovapd -64(pA), rA1
            FMAC rA2, rB1, rC25
              vmovapd (pA), rA2
            FMAC rA3, rB1, rC35
              vmovapd 64(pA), rA3
              vbroadcastsd 7*SZ(pB), rB1
         jnz KLOOP
KDONE:
         mov KK0, KK
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

         #ifndef BETA0
            VCOP 896-1024(pC,r256,4), rC04, rC04
            VCOP 960-1024(pC,r256,4), rC14, rC14
            VCOP (pC,r256,4), rC24, rC24
            VCOP 1088-1024(pC,r256,4), rC34, rC34
         #endif
         vmovapd rC04, 896-1024(pC,r256,4)
         vmovapd rC14, 960-1024(pC,r256,4)
         vmovapd rC24, (pC,r256,4)
         vmovapd rC34, 1088-1024(pC,r256,4)

         #ifndef BETA0
            VCOP 1152(pC), rC05, rC05
            VCOP 1216(pC), rC15, rC15
            VCOP 1280(pC), rC25, rC25
            VCOP 1344(pC), rC35, rC35
         #endif
         vmovapd rC05, 1152(pC)
         vmovapd rC15, 1216(pC)
         vmovapd rC25, 1280(pC)
         vmovapd rC35, 1344(pC)


         add $6*SZ, pB
         add $1536, pC  /* pC += MU*NU*sizeof = 32*6*8 = 1536 */
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
