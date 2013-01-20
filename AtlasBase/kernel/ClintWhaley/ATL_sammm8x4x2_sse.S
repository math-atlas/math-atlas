#include "atlas_asm.h"
#ifndef ATL_SSE1
   #error "This kernel requires SSE1"
#endif

#define rm0     %xmm0
#define rB0     %xmm1
#define rB1     %xmm2
#define rB2     %xmm3
#define rB3     %xmm4
#define rb3     %xmm5
#define rA0     %xmm6
#define rA1     %xmm7
#define rC00    %xmm8
#define rC10    %xmm9
#define rC01    %xmm10
#define rC11    %xmm11
#define rC02    %xmm12
#define rC12    %xmm13
#define rC03    %xmm14
#define rC13    %xmm15
/*
 * Prioritize original registers for inner-loop operations, but inc regs
 * can be anything w/o changing opcode size, so use new regs for those
 */
#define KK      %edx  /* API reg */
   #define rK      %rdx  /* API reg */
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
#define KK0     %edi
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
#define pref prefetcht1
#define PFBDIST 128
#define PFADIST 128
#define prefA(m_) prefetcht0 m_
#define prefB(m_) prefetcht0 m_
#define prefC(m_) prefetcht0 m_
#define FMAC vfmadd231pd   /* FMAC m256/r256, rs1, rd */
#ifdef BETAN1
   #define VCOP subps
#elif defined(BETA1)
   #define VCOP addps
#endif
.text
.global ATL_asmdecor(ATL_USERMM)
ALIGN16
ATL_asmdecor(ATL_USERMM):
/*
 * Save callee-saved iregs
 */
   #define FSIZE 6*8
   sub $FSIZE, %rsp
   movq %rbp, (%rsp)
   movq %rbx, 8(%rsp)
   movq %r12, 16(%rsp)
   movq %r13, 24(%rsp)
   movq %r14, 32(%rsp)
   movq %r15, 40(%rsp)
/*
 * Load paramaters
 */
   mov %rdi, nmu
   mov %rsi, nnu
   mov %r8, pB
   mov %r9, pC
   mov nnu, nnu0
   movq FSIZE+8(%rsp), pfA  
   movq FSIZE+16(%rsp), pfB
   shr $1, KK
   mov KK, KK0
   mov pB, pB0
/*
 * incAm = 8*sizeof*K = 8*4*K = 32*K 
 */
   mov rK, incAm
   shl $6, incAm           /* incAm = 32*K */
   add $64, pA
   mov pA, pA0
   mov $8*2*4, incPF         /* incPF = mu*nu*sizeof */

   ALIGN32
   MNLOOP:
/*      NLOOP: */
/*
 *          Unroll first two iterations to zero C
 */
            movaps (pB), rB3        /* port 2, 3 bytes */
            movaps rB3, rB0         /* port 5, 3 bytes */
            shufps $0x00, rB0, rB0  /* port 5, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            movaps -64(pA), rC00    /* port 2, 5 bytes */


            prefetcht0 (pC)         /* port 2, 3 bytes */
            movaps rC00, rC01       /* port 5, 4 bytes */
            mulps rB0, rC00         /* port 0, 4 bytes */
            movaps -48(pA), rC10    /* port 2, 5 bytes */

            movaps rC10, rC11       /* port 5, 4 bytes */
            mulps rB0, rC10         /* port 0, 4 bytes */
            movaps rB3, rB1         /* port 5, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            shufps $0x55, rB1, rB1  /* port 5, 4 bytes */

            prefetcht0 64(pC)       /* port 2, 4 bytes */
            movaps rC01, rC02       /* port 5, 4 bytes */
            mulps  rB1, rC01        /* port 0, 4 bytes */
            movaps rC11, rC12       /* port 5, 4 bytes */

            add incPF, pfA          /* port 5, 3 bytes */
            pref (pfB)              /* port 2, 3 bytes */
            mulps  rB1, rC11        /* port 0, 4 bytes */
            movaps rB3, rB2         /* port 5, 3 bytes */
            add incPF, pfB          /* port 0, 3 bytes */


            pref -128(pfA)          /* port 2, 4 bytes */
            shufps $0xAA, rB2, rB2  /* port 5, 4 bytes */
            movaps rC02, rC03       /* port 5, 4 bytes */
            mulps  rB2, rC02        /* port 0, 4 bytes */

            movaps rC12, rC13       /* port 5, 4 bytes */
            mulps  rB2, rC12        /* port 0, 4 bytes */
               movaps 16(pB), rb3   /* port 2, 4 bytes */
            shufps $0xFF, rB3, rB3  /* port 5, 4 bytes */

            mulps rB3, rC03         /* port 0, 4 bytes */
            movaps rb3, rB0         /* port 5, 3 bytes */
            mulps rB3, rC13         /* port 0, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            shufps $0x00, rB0, rB0  /* port 5, 4 bytes */

/*
 *          2nd K iteration
 */
            movaps -32(pA), rA0     /* port 2, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            movaps rB0, rm0         /* port 5, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rA0, rm0          /* port 0, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            movaps rb3, rB1         /* port 5, 3 bytes */
            
            movaps -16(pA), rA1     /* port 2, 4 bytes */
            addps rm0, rC00         /* port 1, 4 bytes */
            shufps $0x55, rB1, rB1  /* port 5, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rA1, rB0          /* port 0, 3 bytes */

            addps rB0, rC10         /* port 1, 4 bytes */
               movaps 32(pB), rB3   /* port 2, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            movaps rA0, rm0         /* port 5, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rB1, rm0          /* port 0, 3 bytes */

            movaps rB3, rB0         /* port 5, 3 bytes */
            mulps rA1, rB1          /* port 0, 3 bytes */
            addps rm0, rC01         /* port 1, 4 bytes */
            movaps rb3, rB2         /* port 5, 3 bytes */
            movaps rA0, rm0         /* port 5, 3 bytes */

            shufps $0xAA, rB2, rB2  /* port 5, 4 bytes */
            addps rB1, rC11         /* port 1, 4 bytes */
            mulps rB2, rm0          /* port 0, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            addps rm0, rC02         /* port 1, 4 bytes */

            shufps $0xFF, rb3, rb3  /* port 5, 4 bytes */
            mulps rA1, rB2          /* port 0, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            addps rB2, rC12         /* port 1, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rb3, rA0          /* port 0, 3 bytes */

            shufps $0x00, rB0, rB0  /* port 5, 4 bytes */
            add $32, pB             /* port 5, 4 bytes */
            addps rA0, rC03         /* port 1, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rA1, rb3          /* port 0, 3 bytes */

            movaps rB3, rB1         /* port 5, 3 bytes */
            addps rb3, rC13         /* port 1, 4 bytes */
            dec KK                  /* port 0, 2 bytes */
            jz KDONE                /* port 5, 6 bytes */
         KLOOP:
            movaps rB0, rm0         /* port 5, 3 bytes */
            movaps rB3, rB2         /* port 5, 3 bytes */
            movaps (pA), rA0        /* port 2, 3 bytes */
            mulps rA0, rm0          /* port 0, 3 bytes */
            addps rm0, rC00         /* port 1, 4 bytes */

            movaps 16(pA), rA1      /* port 2, 4 bytes */
            mulps rA1, rB0          /* port 0, 3 bytes */
            addps rB0, rC10         /* port 1, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            shufps $0x55, rB1, rB1  /* port 5, 4 bytes */

            movaps rA0, rm0         /* port 5, 3 bytes */
            mulps rB1, rm0          /* port 0, 3 bytes */
               .byte 0x3e           /* no prt, 1 bytes */
            addps rm0, rC01         /* port 1, 4 bytes */
               .byte 0x3e           /* no prt, 1 bytes */
            shufps $0xAA, rB2, rB2  /* port 5, 4 bytes */

            mulps  rA1, rB1         /* port 0, 3 bytes */
            addps  rB1, rC11        /* port 1, 4 bytes */
               movaps 16(pB), rb3   /* port 2, 4 bytes */
               .byte 0x3e           /* no prt, 1 bytes */
            movaps rA0, rm0         /* port 5, 4 bytes */

            mulps rB2, rm0          /* port 0, 3 bytes */
            addps rm0, rC02         /* port 1, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            shufps $0xFF, rB3, rB3  /* port 5, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            movaps rb3, rB0         /* port 5, 3 bytes */

            mulps rA1, rB2          /* port 0, 3 bytes */
            addps rB2, rC12         /* port 1, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            shufps $0x00, rB0, rB0  /* port 5, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rB3, rA0          /* port 0, 3 bytes */

            addps rA0, rC03         /* port 1, 4 bytes */
            movaps rB0, rm0         /* port 5, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            movaps 32(pA), rA0      /* port 2, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rA1, rB3          /* port 0, 3 bytes */
/*
 * HERE HERE
 */
            addps rB3, rC13         /* port 1, 4 bytes */
            movaps 48(pA), rA1      /* port 2, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rA0, rm0          /* port 0, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            movaps rb3, rB1         /* port 5, 3 bytes */
            
            addps rm0, rC00         /* port 1, 4 bytes */
            shufps $0x55, rB1, rB1  /* port 5, 4 bytes */
            mulps rA1, rB0          /* port 0, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            addps rB0, rC10         /* port 1, 4 bytes */


               movaps 32(pB), rB3   /* port 2, 4 bytes */
            movaps rA0, rm0         /* port 5, 3 bytes */
            mulps rB1, rm0          /* port 0, 3 bytes */
            movaps rB3, rB0         /* port 5, 3 bytes */
            mulps rA1, rB1          /* port 0, 3 bytes */

            addps rm0, rC01         /* port 1, 4 bytes */
            movaps rb3, rB2         /* port 5, 3 bytes */
            shufps $0xAA, rB2, rB2  /* port 5, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            addps rB1, rC11         /* port 1, 4 bytes */

            movaps rA0, rm0         /* port 5, 3 bytes */
            mulps rB2, rm0          /* port 0, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            addps rm0, rC02         /* port 1, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            shufps $0xFF, rb3, rb3  /* port 5, 4 bytes */

            mulps rA1, rB2          /* port 0, 3 bytes */
            addps rB2, rC12         /* port 1, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rb3, rA0          /* port 0, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            shufps $0x00, rB0, rB0  /* port 5, 4 bytes */

            add $32, pB             /* port 5, 4 bytes */
            addps rA0, rC03         /* port 1, 4 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            mulps rA1, rb3          /* port 0, 3 bytes */
            .byte 0x3e              /* no prt, 1 bytes */
            movaps rB3, rB1         /* port 5, 3 bytes */

            add $64, pA             /* port 5, 4 bytes */
            addps rb3, rC13         /* port 1, 4 bytes */
            dec KK                  /* port 0, 2 bytes */
         jnz KLOOP                  /* port 5, 6 bytes */
         .byte 0x66                 /* no prt, 1 bytes */
         nop                        /* port 5, 1 bytes */
KDONE:
         mov KK0, KK
         mov pA0, pA

         #ifndef BETA0
            VCOP (pC), rC00
         #endif
         movaps rC00, (pC)
         #ifndef BETA0
            VCOP 16(pC), rC10
         #endif
         movaps rC10, 16(pC)
         #ifndef BETA0
            VCOP 32(pC), rC01
         #endif
         movaps rC01, 32(pC)
         #ifndef BETA0
            VCOP 48(pC), rC11
         #endif
         movaps rC11, 48(pC)
         #ifndef BETA0
            VCOP 64(pC), rC02
         #endif
         movaps rC02, 64(pC)
         #ifndef BETA0
            VCOP 80(pC), rC12
         #endif
         movaps rC12, 80(pC)
         #ifndef BETA0
            VCOP 96(pC), rC03
         #endif
         movaps rC03, 96(pC)
         #ifndef BETA0
            VCOP 112(pC), rC13
         #endif
         movaps rC13, 112(pC)

         add $128, pC
         sub $1, nnu
      jnz MNLOOP
      mov nnu0, nnu
      add incAm, pA0
      mov pA0, pA
      mov pB0, pB
      sub $1, nmu
   jnz MNLOOP
 DONE:
   movq (%rsp), %rbp
   movq 8(%rsp), %rbx
   movq 16(%rsp), %r12
   movq 24(%rsp), %r13
   movq 32(%rsp), %r14
   movq 40(%rsp), %r15
   add $FSIZE, %rsp
   ret
