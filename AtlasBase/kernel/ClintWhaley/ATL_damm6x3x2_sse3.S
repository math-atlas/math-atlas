#include "atlas_asm.h"
#ifndef KB
   #define KB 0
#endif
/*
 * innermost (K-) loop items get priority on 1st 7 regs
 */
#define pA      %rcx
#define pB      %rdi
#define KK0     %rax
#define KK      %rdx
#define incA    %rsi
#define incB    %rbx
#define pA0     %r11
/*
 * Second (N-) loop items get next level of priority on good regs
 */
#define pC      %rbp
#define pfA     %r8
#define pfB     %r9
#define incPF   %r10
#define nnu     %r12
/*
 * Outer- (M-) loop variables assigned to any regs
 */
#define nmu     %r13
#define pB0     %r14
#define nnu0    %r15
/*
 * floating point registers
 */
#define m0   %xmm0
#define rA0  %xmm1
#define rA1  %xmm2
#define rA2  %xmm3
#define rB0  %xmm4
#define rB1  %xmm5
#define rB2  %xmm6
#define rC00 %xmm7
#define rC10 %xmm8
#define rC20 %xmm9
#define rC01 %xmm10
#define rC11 %xmm11
#define rC21 %xmm12
#define rC02 %xmm13
#define rC12 %xmm14
#define rC22 %xmm15
/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */

#define FSIZE 6*8
#ifndef prefA
   #define prefA prefetcht0
#endif
#ifndef prefB
   #define prefB prefetcht2
#endif
#ifndef prefC
   #ifdef ATL_3DNow
      #define prefC prefetchw
   #else
      #define prefC prefetcht0
   #endif
#endif
#ifdef BETAN1
   #define BETCOP subpd
#else
   #define BETCOP addpd
#endif
#define MOVAPD movaps
/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */
.text
ALIGN64
.global ATL_asmdecor(ATL_USERMM)
ATL_asmdecor(ATL_USERMM):
/*
 * Save callee-saved iregs
 */
   sub $FSIZE, %rsp
   movq    %rbp, 0(%rsp)
   movq    %rbx, 8(%rsp)
  .byte 0x66
  .byte 0x66
   nop
   movq    %r12, 16(%rsp)
   movq    %r13, 24(%rsp)
   .byte 0x3e
   movq    %r14, 32(%rsp)
   movq    %r15, 40(%rsp)
/*
 * Load paramaters
 */
   mov %rdi, nmu
   mov %rsi, nnu
   mov %r8, pB
   .byte 0x66
   nop
   mov %r9, pC
   mov nnu, nnu0
   movq FSIZE+8(%rsp), pfB      /* pfB = pAn */
   movq FSIZE+16(%rsp), pfA     /* pf = pBn */
   cmp pfA, pB
   CMOVE pfB, pfA
   CMOVEq FSIZE+24(%rsp), pfB
   .byte 0x66
   .byte 0x66
   nop
/*
 * Extend range of 1-byte offsets  by starting at -128
 */
   sub $-128, pA
   sub $-128, pB
   .byte 0x3e
   mov $6*3*8, incPF           /* incPF = mu*nu*sizeof */
   sub $-128, pC
   sub $-128, pfA
   .byte 0x3e
   sub $-128, pfB
   movq pB, pB0
   mov pA, pA0
   mov KK, KK0
            .byte 0x3e
            movddup -128(pB), rB0
  .byte 0x66
  .byte 0x66
  .byte 0x66
   nop
   mov $48, incB
            movddup -120(pB), rB1
   lea (incB, incB), incA
#if 1
  .byte 0x66
  nop
  .byte 0x66
  .byte 0x66
   nop 
#else
   ALIGN4
#endif
   .local MNLOOP
   MNLOOP:
/*
      .local NLOOP
      NLOOP:
*/
/*
 *       Peel first iteration of K loop to initialize rCx
 */
         MOVAPD -128(pA), rC02
         MOVAPD rC02, rC00
         mulpd rB0, rC00
  .byte 0x66
  .byte 0x66
   nop
         MOVAPD -112(pA), rC12
         MOVAPD rC12, rC10
         mulpd rB0, rC10
  .byte 0x66
   nop
         MOVAPD -96(pA), rC22
         MOVAPD rC22, rC20
         mulpd rB0, rC20
  .byte 0x66
   nop

         MOVAPD rC02, rC01
         mulpd rB1, rC01
            movddup -112(pB), rB2
  .byte 0x66
   nop 
         MOVAPD rC12, rC11
         mulpd rB1, rC11
         MOVAPD rC22, rC21
  .byte 0x66
  .byte 0x66
   nop
         mulpd rB1, rC21
            prefC -128(pC)

         mulpd rB2, rC02
         #if KB > 1
            movddup -104(pB), rB0
         #endif
  .byte 0x66
   nop
         mulpd rB2, rC12
         #if KB > 1
            movddup -96(pB), rB1
         #endif
         mulpd rB2, rC22

         MOVAPD -80(pA), rA0
  .byte 0x66
   nop
         MOVAPD rA0, m0
         mulpd rB0, m0
         addpd m0, rC00
         MOVAPD -64(pA), rA1
   nop
         MOVAPD rA1, m0
         mulpd rB0, m0
         addpd m0, rC10
         MOVAPD -48(pA), rA2
         mulpd rA2, rB0
         addpd rB0, rC20

            prefC 64(pC)
         MOVAPD rA0, m0
         mulpd rB1, m0
         addpd m0, rC01
            prefA (pfA)
         MOVAPD rA1, m0
         mulpd rB1, m0
         addpd m0, rC11
            prefA -128(pfA)
  .byte 0x66
   nop;
         mulpd rA2, rB1
         addpd rB1, rC21

         movddup -88(pB), rB2
  .byte 0x66
   nop;
         mulpd rB2, rA0
         addpd rA0, rC02
            movddup -80(pB), rB0
  .byte 0x66
   nop;
         mulpd rB2, rA1
         addpd rA1, rC12
            movddup -72(pB), rB1
  .byte 0x66
   nop;
         mulpd rB2, rA2
         addpd rA2, rC22

         MOVAPD -32(pA), rA0
         MOVAPD rA0, m0
         mulpd rB0, m0
         addpd m0, rC00
         MOVAPD -16(pA), rA1
         MOVAPD rA1, m0
   nop
         mulpd rB0, m0
         addpd m0, rC10
         MOVAPD (pA), rA2
         MOVAPD rA0, m0
         nop
         mulpd rA2, rB0
         addpd rB0, rC20

            prefA -64(pfA)
  .byte 0x66
   nop
         mulpd rB1, m0
         addpd m0, rC01
         MOVAPD rA1, m0
            prefB (pfB)
         mulpd rB1, m0
         addpd m0, rC11
            movddup -56(pB), rB0
  .byte 0x66
   nop
         mulpd rA2, rB1
         addpd rB1, rC21

         movddup -64(pB), rB2
  .byte 0x66
   nop
         mulpd rB2, rA0
         addpd rA0, rC02
            movddup -48(pB), rB1
  .byte 0x66
   nop
         mulpd rB2, rA1
         addpd rA1, rC12
         MOVAPD 16(pA), rA0
  .byte 0x66
  .byte 0x66
   nop
         mulpd rB2, rA2
         addpd rA2, rC22

         MOVAPD rA0, m0
         MOVAPD 32(pA), rA1
         mulpd rB0, m0
         addpd m0, rC00
         MOVAPD rA1, m0
         MOVAPD 48(pA), rA2
   nop
         mulpd rB0, m0
         addpd m0, rC10
            prefB -128(pfB)
  .byte 0x66
   nop
         mulpd rA2, rB0
         addpd rB0, rC20

         MOVAPD rA0, m0
            lea (pA, incA, 2), pA
         mulpd rB1, m0
         addpd m0, rC01
            prefB -64(pfB)
  .byte 0x66
   nop 
         MOVAPD rA1, m0
         mulpd rB1, m0
         addpd m0, rC11
prefetcht0 (pC)
         mulpd rA2, rB1
         addpd rB1, rC21

         movddup -40(pB), rB2
  .byte 0x66
  nop 
         mulpd rB2, rA0
         addpd rA0, rC02
            movddup -32(pB), rB0
  .byte 0x66
  nop 
         mulpd rB2, rA1
         addpd rA1, rC12
            movddup -24(pB), rB1
         .byte 0x66
         nop 
//         lea (pB, incB, 2), pB
         add incB, pB
         add $48, pB
         mulpd rB2, rA2
         addpd rA2, rC22
         sub $4, KK
         jz KLOOPDONE

// SS0:
         ALIGN16
         KLOOP:
            MOVAPD -128(pA), rA0
            MOVAPD rA0, m0
            mulpd rB0, m0
            addpd m0, rC00
            nop
            MOVAPD -112(pA), rA1
            MOVAPD rA1, m0
            mulpd rB0, m0
            addpd m0, rC10
            MOVAPD -96(pA), rA2
               prefetcht0 (pA)
            mulpd rA2, rB0
            addpd rB0, rC20

            MOVAPD rA0, m0
               prefetcht0 96(pB)
            mulpd rB1, m0
            addpd m0, rC01
            MOVAPD rA1, m0
            mulpd rB1, m0
            addpd m0, rC11
            .byte 0x66
            .byte 0x66
            .byte 0x66
            nop
            mulpd rA2, rB1
            addpd rB1, rC21

            movddup -112(pB), rB2
            .byte 0x66
            nop
            mulpd rB2, rA0
            addpd rA0, rC02
               movddup -104(pB), rB0
            .byte 0x66
            nop
/*          =============================================================
 *          Everything above this aligned in 16-byte chunks, no adv lower
 *          for some reason!
 *          =============================================================
 */
            mulpd rB2, rA1
            addpd rA1, rC12
               movddup -96(pB), rB1
            mulpd rB2, rA2
            addpd rA2, rC22

            MOVAPD -80(pA), rA0
            MOVAPD rA0, m0
            mulpd rB0, m0
            addpd m0, rC00
            MOVAPD -64(pA), rA1
            MOVAPD rA1, m0
            mulpd rB0, m0
            addpd m0, rC10
            MOVAPD -48(pA), rA2
            mulpd rA2, rB0
            addpd rB0, rC20
            add incA, pA

            MOVAPD rA0, m0
            mulpd rB1, m0
            addpd m0, rC01
            MOVAPD rA1, m0
            mulpd rB1, m0
            addpd m0, rC11
            mulpd rA2, rB1
            addpd rB1, rC21

            movddup -88(pB), rB2
            mulpd rB2, rA0
            addpd rA0, rC02
               movddup -80(pB), rB0
            mulpd rB2, rA1
            addpd rA1, rC12
               movddup -72(pB), rB1
            mulpd rB2, rA2
            add incB, pB
            addpd rA2, rC22

         sub $2, KK
         jnz KLOOP
//SS1:
         ALIGN16
         KLOOPDONE:
         add incPF, pfA
         add incPF, pfB
/*
 *       Write answer back out to C
 */
         #ifdef BETA0
            MOVAPD rC00, -128(pC)
            MOVAPD rC10, -112(pC)
            MOVAPD rC20, -96(pC)
            MOVAPD rC01, -80(pC)
            MOVAPD rC11, -64(pC)
            MOVAPD rC21, -48(pC)
            MOVAPD rC02, -32(pC)
            MOVAPD rC12, -16(pC)
            MOVAPD rC22, (pC)
/*
 *          Add running sum in rCx with original C, then store back out
 */
         #else
            BETCOP -128(pC), rC00
            MOVAPD rC00, -128(pC)
            .byte 0x3e
            BETCOP -112(pC), rC10
            MOVAPD rC10, -112(pC)
            BETCOP -96(pC), rC20
            MOVAPD rC20, -96(pC)
            BETCOP -80(pC), rC01
            movapd rC01, -80(pC)
            .byte 0x66
            .byte 0x66
            .byte 0x66
            nop
            BETCOP -64(pC), rC11
            .byte 0x3e
            MOVAPD rC11, -64(pC)
            .byte 0x66
            .byte 0x66
            .byte 0x66
            nop
            BETCOP -48(pC), rC21
            .byte 0x3e
            MOVAPD rC21, -48(pC)
            .byte 0x66
            .byte 0x66
            .byte 0x66
            nop
            BETCOP -32(pC), rC02
            .byte 0x3e
            MOVAPD rC02, -32(pC)
            .byte 0x66
            .byte 0x66
            .byte 0x66
            nop
            BETCOP -16(pC), rC12
            .byte 0x3e
            MOVAPD rC12, -16(pC)
            .byte 0x66
            .byte 0x66
            .byte 0x66
            nop
            BETCOP (pC), rC22
            .byte 0x3e
            MOVAPD rC22, (pC)
            .byte 0x66
            .byte 0x66
            .byte 0x66
            nop
         #endif
         add $6*3*8, pC        /* pC += MU*NU*sizeof */
         .byte 0x3e
         movddup -128(pB), rB0
         mov pA0, pA
         .byte 0x3e
         movddup -120(pB), rB1
            .byte 0x66
            .byte 0x66
            nop
         mov KK0, KK
            .byte 0x66
            .byte 0x66
            .byte 0x66
            nop
      sub $1, nnu
      jnz MNLOOP

         movddup -128(pB0), rB0
      mov nnu0, nnu
      mov pB0, pB
      lea (KK, KK, 4), pA      /* pA = 5*K */
         movddup -120(pB0), rB1
      add KK, pA               /* pA = 6*K = MU*K */
      shl $3, pA               /* pA = MU*K*sizeof */
      add pA0, pA              /* pA = A + MU*K*sizeof */
      mov pA, pA0
   sub $1, nmu
   jnz MNLOOP
/* DONE: */
   movq    (%rsp), %rbp
   movq    8(%rsp), %rbx
   movq    16(%rsp), %r12
   movq    24(%rsp), %r13
   movq    32(%rsp), %r14
   movq    40(%rsp), %r15
   add $FSIZE, %rsp
   ret
#if 0
ALIGN16
.global findSize
findSize:
mov $SS1-SS0, %rax
ret
#endif
