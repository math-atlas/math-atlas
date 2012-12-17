#include "atlas_asm.h"

#define movapd movaps
#define m0      %xmm0
#define rA0     %xmm1
#define rB0     %xmm2
#define rC0     %xmm3
#define rC1     %xmm4
#define rC2     %xmm5
#define rC3     %xmm6
#define rC4     %xmm7
#define rC5     %xmm8
#define rC6     %xmm9
#define rC7     %xmm10
#define rC8     %xmm11
#define rC9     %xmm12
#define rC10    %xmm13
#define rC11    %xmm14
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
#define PFBDIST 192
#define PFADIST 128
#define prefA(m_) prefetcht0 m_
#define prefB(m_) prefetcht0 m_
#define prefC(m_) prefetcht0 m_
#ifdef BETAN1
   #define VCOP subpd
#else
   #define VCOP addpd
#endif
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
   mov %rsi, nnu
   mov %r8, pB
   mov %r9, pC
   mov nnu, nnu0
   movq 8(%rsp), pfB      /* pfB = pAn */
   movq 16(%rsp), pfA     /* pf = pBn */
   cmp pfA, pB
   CMOVE pfB, pfA
   CMOVEq 24(%rsp), pfB
   mov KK, KK0
   sub $-128, pC
   sub $-128, pA
   mov $192, r192
   mov pA, pA0
   mov pB, pB0
/*
 * incAm = 24*sizeof*K = 24*8*K = 3*8*8*K = 3*K*64
 */
   lea (KK, KK,2), incAm   /* incAm = 3*K */
   shl $6, incAm           /* incAm = 64*3*K */

   MLOOP:
      NLOOP:
/*
         prefA((pA,r192))
         prefA(127(pA,r192))
         prefC((pC))
*/
/*
 *       First iteration peeled to handle init of rC
 */
#if 1
         movddup (pB), rC11
         movapd -128(pA), rC0
         mulpd rC11, rC0
         movapd -112(pA), rC1
         prefC((pC))
         mulpd rC11, rC1
         movapd -96(pA), rC2
         mulpd rC11, rC2
         movapd -80(pA), rC3
         mulpd rC11, rC3
         movapd -64(pA), rC4
         mulpd rC11, rC4
         movapd -48(pA), rC5
         mulpd rC11, rC5
         movapd -32(pA), rC6
         mulpd rC11, rC6
         movapd -16(pA), rC7
         mulpd rC11, rC7
         movapd (pA), rC8
         mulpd rC11, rC8
         movapd 16(pA), rC9
         mulpd rC11, rC9
         movapd 32(pA), rC10
         prefC(128(pC))
         mulpd rC11, rC10
         mulpd 48(pA), rC11

         add r192, pA
         add $8, pB
         sub $8, KK
         jz KDONE

#endif
         KLOOP:
            movddup (pB), rB0
            movapd -128(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA), rA0
            prefB((pB,r192))
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA), rA0
            mulpd 48(pA), rB0
            addpd rB0, rC11

            movddup 8(pB), rB0
            movapd -128(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA,r192), rA0
            mulpd 48(pA,r192), rB0
            addpd rB0, rC11
            lea (pA, r192,2), pA

            movddup 16(pB), rB0
            movapd -128(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA), rA0
            mulpd 48(pA), rB0
            addpd rB0, rC11

            movddup 24(pB), rB0
            movapd -128(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA,r192), rA0
            mulpd 48(pA,r192), rB0
            addpd rB0, rC11
            lea (pA, r192,2), pA

            movddup 32(pB), rB0
            movapd -128(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA), rA0
            mulpd 48(pA), rB0
            addpd rB0, rC11

            movddup 40(pB), rB0
            movapd -128(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA,r192), rA0
            mulpd 48(pA,r192), rB0
            addpd rB0, rC11
            lea (pA, r192,2), pA

            movddup 48(pB), rB0
            movapd -128(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA), rA0
            mulpd 48(pA), rB0
            addpd rB0, rC11

            movddup 56(pB), rB0
            movapd -128(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA,r192), rA0
            mulpd 48(pA,r192), rB0
            addpd rB0, rC11
            lea (pA, r192,2), pA

            add $64, pB
            sub $8, KK
         jnz KLOOP
         KDONE:
            movddup (pB), rB0
            movapd -128(pA), rA0
            mulpd rB0, rA0
         prefA(-128(pA,r192,2))
            addpd rA0, rC0
            movapd -112(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA), rA0
            mulpd 48(pA), rB0
            addpd rB0, rC11

            movddup 8(pB), rB0
            movapd -128(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA,r192), rA0
         prefA((pA,r192,2))
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA,r192), rA0
            mulpd 48(pA,r192), rB0
            addpd rB0, rC11
            lea (pA, r192,2), pA

            movddup 16(pB), rB0
            movapd -128(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA), rA0
               prefetcht2 (pfB)
            mulpd rB0, rA0
            addpd rA0, rC1
               add $32, pfB
            movapd -96(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA), rA0
            mulpd 48(pA), rB0
            addpd rB0, rC11

            movddup 24(pB), rB0
            movapd -128(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA,r192), rA0
            mulpd 48(pA,r192), rB0
            addpd rB0, rC11
            lea (pA, r192,2), pA

            movddup 32(pB), rB0
            movapd -128(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
            movapd -112(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            movapd -96(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            movapd -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            movapd -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            movapd -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            movapd -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            movapd -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            movapd (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            movapd 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            movapd 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            movapd 48(pA), rA0
            mulpd 48(pA), rB0
            addpd rB0, rC11

            movddup 40(pB), rB0
            movapd -128(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
         #ifndef BETA0
            VCOP -128(pC), rC0
         #endif
            movapd -112(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
         #ifndef BETA0
            VCOP -112(pC), rC1
         #endif
            movapd -96(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
         #ifndef BETA0
            VCOP -96(pC), rC2
         #endif
            movapd -80(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
         #ifndef BETA0
            VCOP -80(pC), rC3
         #endif
            movapd -64(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
         #ifndef BETA0
            VCOP -64(pC), rC4
         #endif
            movapd -48(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
         #ifndef BETA0
            VCOP -48(pC), rC5
         #endif
            movapd -32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
         #ifndef BETA0
            VCOP -32(pC), rC6
         #endif
            movapd -16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
         #ifndef BETA0
            VCOP -16(pC), rC7
         #endif
            movapd (pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
         #ifndef BETA0
            VCOP (pC), rC8
         #endif
            movapd 16(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
         #ifndef BETA0
            VCOP 16(pC), rC9
         #endif
            movapd 32(pA,r192), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
         #ifndef BETA0
            VCOP 32(pC), rC10
         #endif
            movapd 48(pA,r192), rA0
            mulpd 48(pA,r192), rB0
            addpd rB0, rC11
         #ifndef BETA0
            VCOP 48(pC), rC11
         #endif
            lea (pA, r192,2), pA

            movddup 48(pB), rB0
            movapd -128(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC0
         movapd rC0, -128(pC)
            movapd -112(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
         movapd rC1, -112(pC)
            movapd -96(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
         movapd rC2, -96(pC)
            movapd -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
         movapd rC3, -80(pC)
            movapd -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
         movapd rC4, -64(pC)
            movapd -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
         movapd rC5, -48(pC)
            movapd -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
         movapd rC6, -32(pC)
            movapd -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
         movapd rC7, -16(pC)
            movapd (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
         movapd rC8, (pC)
            movapd 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
         movapd rC9, 16(pC)
            movapd 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
         movapd rC10, 32(pC)
            movapd 48(pA), rA0
            mulpd 48(pA), rB0
            addpd rB0, rC11
         movapd rC11, 48(pC)
         add $56, pB
         mov KK0, KK
         mov pA0, pA
         add r192, pC
         sub $1, nnu
      jnz NLOOP
      mov nnu0, nnu
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
