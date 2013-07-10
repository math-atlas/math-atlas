#include "atlas_asm.h"

#define rA0     %xmm0
#define rB0     %xmm1
#define rC0     %xmm2
#define rC1     %xmm3
#define rC2     %xmm4
#define rC3     %xmm5
#define rC4     %xmm6
#define rC5     %xmm7
#define rC6     %xmm8
#define rC7     %xmm9
#define rC8     %xmm10
#define rC9     %xmm11
#define rC10    %xmm12
#define rC11    %xmm13
#define rC12    %xmm14
#define rC13    %xmm15

#define KK      %rdx   /* API register */
#define pA      %rcx   /* API reg */
#define pB      %rax   /* comes in as r8 */
#define NMU     %rdi   /* API reg */
#define NNU     %rsi   /* API reg */
#define pC      %r9    /* API reg */
#define pfB     %r10
#define pfA     %r8
#define r224    %rbx

#define NNU0    %r11
#define incA    %r12
#define pB0     %r13
#define r112    %r14
#define K0      %r15
#define FSIZE 6*8
#ifdef BETAN1
   #define VOP subpd
#elif defined(BETA1)
   #define VOP addpd
#elif defined(VOP)
   #undef VOP
#endif
#ifdef ATL_3DNow
   #define prefC prefetchw
#else
   #define prefC prefetcht0
#endif
#if KB <= 40
   #define prefB(m_) prefetcht0 m_
#else
   #define prefB(m_) prefetcht2 m_
#endif
#if KB > 120
   #define prefA(m_)
#elif KB > 40
   #define prefA(m_) prefetcht2 m_
#else
   #define prefA(m_) prefetcht0 m_
#endif
#define MOVAPD movaps
/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */
.text
.global ATL_asmdecor(ATL_USERMM)
ALIGN16
ATL_asmdecor(ATL_USERMM):
/*
 * Save callee-saved iregs
 */
   sub  $FSIZE, %rsp
   movq %rbp, (%rsp)
   movq %rbx, 8(%rsp)
   movq %r12, 16(%rsp)
   movq %r13, 24(%rsp)
   movq %r14, 32(%rsp)
   movq %r15, 40(%rsp)

   mov %r8, pB
   sub $-128, pA
   mov pB, pB0
   movq FSIZE+16(%rsp), pfB
   movq FSIZE+8(%rsp), pfA
/*
 * incA = 14*K*sizeof = 14*K*8 = 7*16*K
 */
   lea (KK, KK,4), incA   /* incA = 5*K */
   lea (incA, KK,2), incA /* incA = 7*K */
   shl $4, incA           /* incA = 7*16*K */
   mov NNU, NNU0
   mov $112, r112
   mov $224, r224
   mov KK, K0

   ALIGN16
   MLOOP:
/*      NLOOP: */
         MOVAPD (pB), rC13
         MOVAPD -128(pA), rC0
         mulpd rC13, rC0
         MOVAPD -112(pA), rC1
         mulpd rC13, rC1
         MOVAPD -96(pA), rC2
         mulpd rC13, rC2
         MOVAPD -80(pA), rC3
         mulpd rC13, rC3
         MOVAPD -64(pA), rC4
         mulpd rC13, rC4
         MOVAPD -48(pA), rC5
         mulpd rC13, rC5
         MOVAPD -32(pA), rC6
         mulpd rC13, rC6
         MOVAPD -16(pA), rC7
         mulpd rC13, rC7
         MOVAPD (pA), rC8
         mulpd rC13, rC8
         MOVAPD 16(pA), rC9
         mulpd rC13, rC9
         MOVAPD 32(pA), rC10
         mulpd rC13, rC10
         MOVAPD 48(pA), rC11
         mulpd rC13, rC11
         MOVAPD 64(pA), rC12
            prefC (pC)
         mulpd rC13, rC12
         mulpd  80(pA), rC13
         add r224, pA
            prefC 64(pC)
         add $16, pB
         sub $2, KK
         jz KDONE

            MOVAPD -128(pA), rA0
            MOVAPD (pB), rB0
            mulpd rB0, rA0
            addpd rA0, rC0
            MOVAPD -112(pA), rA0
               prefB((pfB))
            mulpd rB0, rA0
            addpd rA0, rC1
            MOVAPD -96(pA), rA0
               prefB(64(pfB))
            mulpd rB0, rA0
            addpd rA0, rC2
            MOVAPD -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            MOVAPD -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            MOVAPD -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            MOVAPD -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            MOVAPD -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            MOVAPD (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            MOVAPD 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            MOVAPD 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            MOVAPD 48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC11
            MOVAPD 64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC12
            mulpd  80(pA), rB0
               prefA((pfA))
            addpd rB0, rC13
         add r224, pA
               prefA(64(pfA))
         add $16, pB
         sub $2, KK
         jz KDONE
         ALIGN16
         KLOOP: 
            MOVAPD -128(pA), rA0
            MOVAPD (pB), rB0
            mulpd rB0, rA0
            addpd rA0, rC0
            MOVAPD -112(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC1
            MOVAPD -96(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC2
            MOVAPD -80(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC3
            MOVAPD -64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC4
            MOVAPD -48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC5
            MOVAPD -32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC6
            MOVAPD -16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC7
            MOVAPD (pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC8
            MOVAPD 16(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC9
            MOVAPD 32(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC10
            MOVAPD 48(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC11
            MOVAPD 64(pA), rA0
            mulpd rB0, rA0
            addpd rA0, rC12
            mulpd  80(pA), rB0
            addpd rB0, rC13
         add r224, pA
         add $16, pB
         sub $2, KK
         jnz KLOOP
KDONE:
         mov K0, KK
/*
 *       Sum up rCx regs
 */
         haddpd rC1, rC0        /* rC0 = {c1ab,c0ab} */
         #ifdef VOP
            VOP (pC), rC0
         #endif
         MOVAPD rC0, (pC)
         haddpd rC3, rC2        /* rC2 = {c3ab,c2ab} */
         #ifdef VOP
            VOP 16(pC), rC2
         #endif
         MOVAPD rC2, 16(pC)
         haddpd rC5, rC4        /* rC4 = {c5ab,c4ab} */
         #ifdef VOP
            VOP 32(pC), rC4
         #endif
         MOVAPD rC4, 32(pC)
         haddpd rC7, rC6        /* rC6 = {c7ab,c6ab} */
         #ifdef VOP
            VOP 48(pC), rC6
         #endif
         MOVAPD rC6, 48(pC)
         haddpd rC9, rC8        /* rC8 = {c9ab,c8ab} */
         #ifdef VOP
            VOP 64(pC), rC8
         #endif
         MOVAPD rC8, 64(pC)
         haddpd rC11, rC10      /* rC10 = {c11ab,c10ab} */
         #ifdef VOP
            VOP 80(pC), rC10
         #endif
         MOVAPD rC10, 80(pC)
         haddpd rC13, rC12      /* rC12 = {c13ab,c12ab} */
         #ifdef VOP
            VOP 96(pC), rC12
         #endif
         MOVAPD rC12, 96(pC)

            add r112, pfB
            add r112, pfA
         add r112, pC
         sub incA, pA
         sub $1, NNU
      jnz MLOOP
      add incA, pA   /* pA += 14*K*sizeof */
      mov pB0, pB
      sub $1, NMU
      mov NNU0, NNU
   jnz MLOOP

DONE:
   movq (%rsp), %rbp
   movq 8(%rsp), %rbx
   movq 16(%rsp), %r12
   movq 24(%rsp), %r13
   movq 32(%rsp), %r14
   movq 40(%rsp), %r15
   add  $FSIZE, %rsp
   ret
