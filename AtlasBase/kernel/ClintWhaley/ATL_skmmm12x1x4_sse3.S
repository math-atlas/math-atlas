#include "atlas_asm.h"

#define rm0     %xmm0
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

#define KK      %rdx   /* API register */
#define pA      %rcx   /* API reg */
#define pB      %rax   /* comes in as r8 */
#define NMU     %rdi   /* API reg */
#define NNU     %rsi   /* API reg */
#define pC      %r9    /* API reg */
#define pfB     %r10
#define pfA     %r8

#define NNU0    %r11
#define incA    %r12
#define pB0     %r13
#define r192    %r14
#define K0      %r15
#define FSIZE 6*8
#ifdef BETAN1
   #define VOP subps
#elif defined(BETA1)
   #define VOP addps
#elif defined(VOP)
   #undef VOP
#endif
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
 * Set incA = 12*K*sizeof = 12*K*4 = 48*K = 3*16*K
 */
   lea (KK, KK, 2), incA        /* incA = 3*K */
   shl $4, incA                 /* incA = 16*3*K = 48*K */
   shr $2, KK                   /* KK /= VLEN */
   mov KK, K0
   mov $192, r192
   mov NNU, NNU0
   
   ALIGN16
   MLOOP:
//      NLOOP:
         movaps (pB), rC11
         movaps -128(pA), rC0
         mulps rC11, rC0
         movaps -112(pA), rC1
            prefetchw (pC)
         mulps rC11, rC1
         movaps -96(pA), rC2
            prefetcht0 (pfB)
         mulps rC11, rC2
         movaps -80(pA), rC3
            prefetcht0 (pfA)
         mulps rC11, rC3
         movaps -64(pA), rC4
         mulps rC11, rC4
         movaps -48(pA), rC5
         mulps rC11, rC5
         movaps -32(pA), rC6
         mulps rC11, rC6
         movaps -16(pA), rC7
         mulps rC11, rC7
         movaps (pA), rC8
         mulps rC11, rC8
         movaps 16(pA), rC9
         mulps rC11, rC9
         movaps 32(pA), rC10
         mulps rC11, rC10
         mulps 48(pA), rC11

         add $16, pB
         add r192, pA
            add $48, pfB
            add $48, pfA
         sub $1, KK
         jz KDONE
         ALIGN16
         KLOOP:
            movaps (pB), rB0
            movaps -128(pA), rm0
            mulps rB0, rm0
            addps rm0, rC0
            movaps -112(pA), rm0
            mulps rB0, rm0
            addps rm0, rC1
            movaps -96(pA), rm0
            mulps rB0, rm0
            addps rm0, rC2
            movaps -80(pA), rm0
            mulps rB0, rm0
            addps rm0, rC3
            movaps -64(pA), rm0
            mulps rB0, rm0
            addps rm0, rC4
            movaps -48(pA), rm0
            mulps rB0, rm0
            addps rm0, rC5
            movaps -32(pA), rm0
            mulps rB0, rm0
            addps rm0, rC6
            movaps -16(pA), rm0
            mulps rB0, rm0
            addps rm0, rC7
            movaps    (pA), rm0
            mulps rB0, rm0
            addps rm0, rC8
            movaps  16(pA), rm0
            mulps rB0, rm0
            addps rm0, rC9
            movaps  32(pA), rm0
            mulps rB0, rm0
            addps rm0, rC10
            mulps 48(pA), rB0
            addps rB0, rC11

            add $16, pB
            add r192, pA
            sub $1, KK
         jnz KLOOP
KDONE:
/*
 *       Sum up rCx regs
 */
         haddps rC1, rC0        /* rC0 = {c1cd,c1ab,c0cd,c0ab} */
         haddps rC3, rC2        /* rC2 = {c3cd,c3ab,c2cd,c2ab} */
         haddps rC2, rC0        /* rC0 = {c3abcd,c2abcd,c1abcd,c0abcd} */
         #ifdef VOP
            VOP (pC), rC0
         #endif
         movaps rC0, (pC)
         haddps rC5, rC4        /* rC4 = {c5cd,c5ab,c4cd,c4ab} */
         haddps rC7, rC6        /* rC6 = {c7cd,c7ab,c6cd,c6ab} */
         haddps rC6, rC4        /* rC4 = {c3abcd,c2abcd,c1abcd,c0abcd} */
         #ifdef VOP
            VOP 16(pC), rC4
         #endif
         movaps rC4, 16(pC)
         haddps rC9, rC8        /* rC8 = {c9cd,c9ab,c8cd,c8ab} */
         haddps rC11, rC10      /* rC10 = {c11cd,c11ab,c10cd,c10ab} */
         haddps rC10, rC8       /* rC8 = {c11abcd,c10abcd,c9abcd,c8abcd} */
         #ifdef VOP
            VOP 32(pC), rC8
         #endif
         movaps rC8, 32(pC)
         add $48, pC
         sub incA, pA           /* pA -= 12*K*sizeof */
         mov K0, KK
         sub $1, NNU
      jnz MLOOP
      add incA, pA   /* pA += 12*K*sizeof */
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
