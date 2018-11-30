#include "atlas_asm.h"

#define rB0     %xmm0
#define rA0     %xmm1
#define rA1     %xmm2
#define rA2     %xmm3
#define rC00    %xmm4
#define rC10    %xmm5
#define rC20    %xmm6
#define rC01    %xmm7
#define rC11    %xmm8
#define rC21    %xmm9
#define rC02    %xmm10
#define rC12    %xmm11
#define rC22    %xmm12
#define rC03    %xmm13
#define rC13    %xmm14
#define rC23    %xmm15
/*
 * Prioritize original registers for inner-loop operations, but inc regs
 * can be anything w/o changing opcode size, so use new regs for those
 */
#define KK      %rdx  /* API reg */
#define pA      %rcx  /* API reg */
#define pB      %rax  /* comes in as r9 */
#define r192    %r9   /* set after mov r9 to pC () */
/*
 * Then N-loop variables much less important, so use any orig regs left
 */
#define pA0     %r8   /* set after mov r8 to pB (rax) */
#define pC      %rsi  /* set after mov rsi to nnu () */
#define nnu     %r10  /* comes in as rsi */
#define pfA     %rbx
#define pfB     %rbp
#define incPF   %r12
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
#define prefA(m_) prefetcht0 m_
#define prefB(m_) prefetcht0 m_
#define prefC(m_) prefetcht0 m_
#define FMAC vfmadd231pd   /* FMAC m256/r256, rs1, rd */
#if defined(BETAN) || defined(BETAn)
   #define BETAN1
#endif
#ifdef BETAN1
   #define VCOP subpd
#else
   #define VCOP addpd
#endif
#define movapd movaps
.text
.global ATL_asmdecor(ATL_USERMM)
ALIGN16
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
   #ifndef ATL_KSYRK_
      mov %rsi, nnu
   #endif
   mov %r8, pB
   mov %r9, pC
   #ifdef ATL_KSYRK_
      xor nnu, nnu  /* nnu = 0, used as j counter for SYRK */
      mov $6, nnu0  /* nnu0 = mu = i+mu for outer loop diag */
   #else
      mov nnu, nnu0
   #endif
   movq 8(%rsp), pfB      /* pfB = pAn */
   movq 16(%rsp), pfA     /* pf = pBn */
   cmp pfA, pB
   CMOVE pfB, pfA
   CMOVEq 24(%rsp), pfB
   mov KK, KK0
   sub $-128, pC
   mov $192, r192
   mov pA, pA0
   mov pB, pB0
/*
 * incAm = 6*sizeof*K = 6*8*K = 3*2*8*K = 3*K*16
 */
   lea (KK, KK,2), incAm   /* incAm = 3*K */
   shl $4, incAm           /* incAm = 32*3*K */

   MLOOP:
      NLOOP:
#if 1
/*
 *       First iteration peeled to handle init of rC
 */
            movddup      (pB), rB0
            movapd (pA), rA0
            vmulpd rA0, rB0, rC00
            movapd 16(pA), rA1
            vmulpd rA1, rB0, rC10
            movapd 32(pA), rA2
            vmulpd rA2, rB0, rC20
            add $48, pA

            movddup      8(pB), rB0
            vmulpd rA0, rB0, rC01
               prefA((pfA))
               add $64, pfA
            vmulpd rA1, rB0, rC11
/*               prefB((pfB)) */
               add $64, pfB
            vmulpd rA2, rB0, rC21
               prefC((pC))

            movddup      16(pB), rB0
            vmulpd rA0, rB0, rC02
/*                prefetcht0 128(pB) */
            vmulpd rA1, rB0, rC12
/*                prefetcht0 128(pA) */
            vmulpd rA2, rB0, rC22

            movddup      24(pB), rB0
            vmulpd rA0, rB0, rC03
               movapd (pA), rA0
            vmulpd rA1, rB0, rC13
               movapd 16(pA), rA1
            vmulpd rA2, rB0, rC23
               movapd 32(pA), rA2
               add $48, pA

            add $32, pB
            sub $1, KK
            jz KDONE
#endif
         ALIGN16
         KLOOP:
            movddup      (pB), rB0
            FMAC rA0, rB0, rC00
            FMAC rA1, rB0, rC10
            FMAC rA2, rB0, rC20

            movddup      8(pB), rB0
            FMAC rA0, rB0, rC01
            FMAC rA1, rB0, rC11
            FMAC rA2, rB0, rC21

            movddup      16(pB), rB0
            FMAC rA0, rB0, rC02
            FMAC rA1, rB0, rC12
            FMAC rA2, rB0, rC22

            movddup      24(pB), rB0
            FMAC rA0, rB0, rC03
               movapd (pA), rA0
            FMAC rA1, rB0, rC13
               movapd 16(pA), rA1
            FMAC rA2, rB0, rC23
               movapd 32(pA), rA2

            add $32, pB
            add $48, pA
            sub $1, KK
         jnz KLOOP
KDONE:
         mov KK0, KK
         mov pA0, pA
         #ifndef BETA0
            VCOP -128(pC), rC00
         #endif
         movapd rC00, -128(pC)
         #ifndef BETA0
            VCOP -112(pC), rC10
         #endif
         movapd rC10, -112(pC)
         #ifndef BETA0
            VCOP -96(pC), rC20
         #endif
         movapd rC20, -96(pC)
         #ifndef BETA0
            VCOP -80(pC), rC01
         #endif
         movapd rC01, -80(pC)
         #ifndef BETA0
            VCOP -64(pC), rC11
         #endif
         movapd rC11, -64(pC)
         #ifndef BETA0
            VCOP -48(pC), rC21
         #endif
         movapd rC21, -48(pC)
         #ifndef BETA0
            VCOP -32(pC), rC02
         #endif
         movapd rC02, -32(pC)
         #ifndef BETA0
            VCOP -16(pC), rC12
         #endif
         movapd rC12, -16(pC)
         #ifndef BETA0
            VCOP (pC), rC22
         #endif
         movapd rC22, (pC)
         #ifndef BETA0
            VCOP 16(pC), rC03
         #endif
         movapd rC03, 16(pC)
         #ifndef BETA0
            VCOP 32(pC), rC13
         #endif
         movapd rC13, 32(pC)
         #ifndef BETA0
            VCOP 48(pC), rC23
         #endif
         movapd rC23, 48(pC)
         add $192, pC
   #ifdef ATL_KSYRK_
        add $4, nnu
        cmp nnu0, nnu
      jl NLOOP
      xor nnu0, nnu0
      add $6, nnu0
   #else
         sub $1, nnu
      jnz NLOOP
      mov nnu0, nnu
   #endif
      add incAm, pA0
      mov pA0, pA
      mov pB0, pB
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
