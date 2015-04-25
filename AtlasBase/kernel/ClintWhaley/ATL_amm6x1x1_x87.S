#include "atlas_asm.h"

#ifdef ATL_GAS_x8632
   #define nmu  %edi
   #define nnu  %esi
   #define K    %ecx
   #define pA   %eax
   #define pB   %ebx
   #define pC   %ebp
   #define rtmp %edx
   #define pB0  FSIZE+20(%esp)
   #define nnu0 20(%esp)
   #define K0   24(%esp)
   #define FSIZE 8*4
#elif defined(ATL_GAS_x8664)
   #define nmu %rdi
   #define nnu %rsi
   #define K   %rdx
   #define pA  %rcx
   #define pB  %rax
   #define pC  %r9
   #define nn0 %r8
   #define pB0 %r10
   #define K0  %r11
   #define nnu0 %r12
#else
   #error "This file requires x86 assembly!"
#endif
#ifdef SCPLX
   #define SREAL 1
#elif defined(DCPLX)
   #define DREAL 1
#endif
#ifdef SREAL
   #define fmull fmuls
   #define fstpl fstps
   #define faddl fadds
   #define fsubl fsubs
   #define fldl flds
   #define SZ 4
#else
   #define SZ 8
#endif
/*
                  4/rdi    8/rsi 12/rdx    16/ rcx     20/ r8    24/ r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                28/8(%rsp) 32/16(%rsp)  36/24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */

.text
.global ATL_asmdecor(ATL_USERMM)
ALIGN16
ATL_asmdecor(ATL_USERMM):
#ifdef ATL_GAS_x8632
   sub $FSIZE, %esp
   movl %ebp, (%esp)
   movl %ebx, 4(%esp)
   movl %esi, 8(%esp)
   movl %edi, 16(%esp)

   movl  4+FSIZE(%esp), nmu
   movl  8+FSIZE(%esp), nnu
   movl 12+FSIZE(%esp), K  
   movl 16+FSIZE(%esp), pA
   movl 20+FSIZE(%esp), pB
   movl 24+FSIZE(%esp), pC
   #define movq movl
#else
   mov %r8, pB
   movq %r12, -8(%rsp)
#endif
/*
 * Compute K = -6*K*sizeof = -6*8*K = 3*16*K = 48*K
 */
   lea (K, K, 2), K   /* K = 3*K */
   #ifdef SREAL
      shl $3, K          /* K = 24K */
   #else
      shl $4, K          /* K = 48K */
   #endif
   neg K              /* K = -48*K */
   movq K, K0
   movq nnu, nnu0 
   movq pB, pB0

   MLOOP:
      sub K, pA                         /* pA += 6*K*sizeof */
      movq nnu0, nnu
      movq pB0, pB
      NLOOP:
         fldz                           /* ST={0} */
         fldz                           /* ST={0,0} */
         fldz                           /* ST={0,0,0} */
         fldz                           /* ST={0,0,0,0} */
         fldz                           /* ST={0,0,0,0,0} */
         fldz                           /* ST={0,0,0,0,0,0} */
         movq K0, K
         KLOOP:
            fldl (pB)                   /* ST={b0,c0,c0,c0,c0,c0,c0} */
            add $SZ, pB     
            fldl (pA,K)                 /* ST={a0,b0,c0,c1,c2,c3,c4,c5} */
            fmul  %st(1), %st           /* ST={c0,b0,c0,c1,c2,c3,c4,c5} */
            faddp  %st, %st(2)          /* ST={b0,c0,c1,c2,c3,c4,c5} */
            fldl SZ(pA,K)               /* ST={a1,b0,c0,c1,c2,c3,c4,c5} */
            fmul  %st(1), %st           /* ST={c1,b0,c0,c1,c2,c3,c4,c5} */
            faddp  %st, %st(3)          /* ST={b0,c0,c1,c2,c3,c4,c5} */
            fldl 2*SZ(pA,K)             /* ST={a2,b0,c0,c1,c2,c3,c4,c5} */
            fmul  %st(1), %st           /* ST={c2,b0,c0,c1,c2,c3,c4,c5} */
            faddp  %st, %st(4)          /* ST={b0,c0,c1,c2,c3,c4,c5} */
            fldl 3*SZ(pA,K)             /* ST={a3,b0,c0,c1,c2,c3,c4,c5} */
            fmul  %st(1), %st           /* ST={c3,b0,c0,c1,c2,c3,c4,c5} */
            faddp  %st, %st(5)          /* ST={b0,c0,c1,c2,c3,c4,c5} */
            fldl 4*SZ(pA,K)             /* ST={a4,b0,c0,c1,c2,c3,c4,c5} */
            fmul  %st(1), %st           /* ST={c4,b0,c0,c1,c2,c3,c4,c5} */
            faddp  %st, %st(6)          /* ST={b0,c0,c1,c2,c3,c4,c5} */
            fmull 5*SZ(pA,K)            /* ST={c5,c0,c1,c2,c3,c4,c5} */
            faddp  %st, %st(6)          /* ST={c0,c1,c2,c3,c4,c5} */
            add $6*SZ, K
         jnz KLOOP
         #ifdef BETA0
            fstpl (pC)                  /* ST={c1,c2,c3,c4,c5} */
            fstpl SZ(pC)                /* ST={c2,c3,c4,c5} */
            fstpl 2*SZ(pC)              /* ST={c3,c4,c5} */
            fstpl 3*SZ(pC)              /* ST={c4,c5} */
            fstpl 4*SZ(pC)              /* ST={c5} */
            fstpl 5*SZ(pC)              /* ST={} */
         #elif defined(BETA1)
            faddl (pC)                  /* ST={c0,c1,c2,c3,c4,c5} */
            fstpl (pC)                  /* ST={c1,c2,c3,c4,c5} */
            faddl SZ(pC)                /* ST={c1,c2,c3,c4,c5} */
            fstpl SZ(pC)                /* ST={c2,c3,c4,c5} */
            faddl 2*SZ(pC)              /* ST={c2,c3,c4,c5} */
            fstpl 2*SZ(pC)              /* ST={c3,c4,c5} */
            faddl 3*SZ(pC)              /* ST={c3,c4,c5} */
            fstpl 3*SZ(pC)              /* ST={c4,c5} */
            faddl 4*SZ(pC)              /* ST={c4,c5} */
            fstpl 4*SZ(pC)              /* ST={c5} */
            faddl 5*SZ(pC)              /* ST={c5} */
            fstpl 5*SZ(pC)              /* ST={} */
         #else
            fsubl (pC)                  /* ST={c0,c1,c2,c3,c4,c5} */
            fstpl (pC)                  /* ST={c1,c2,c3,c4,c5} */
            fsubl SZ(pC)                /* ST={c1,c2,c3,c4,c5} */
            fstpl SZ(pC)                /* ST={c2,c3,c4,c5} */
            fsubl 2*SZ(pC)              /* ST={c2,c3,c4,c5} */
            fstpl 2*SZ(pC)              /* ST={c3,c4,c5} */
            fsubl 3*SZ(pC)              /* ST={c3,c4,c5} */
            fstpl 3*SZ(pC)              /* ST={c4,c5} */
            fsubl 4*SZ(pC)              /* ST={c4,c5} */
            fstpl 4*SZ(pC)              /* ST={c5} */
            fsubl 5*SZ(pC)              /* ST={c5} */
            fstpl 5*SZ(pC)              /* ST={} */
         #endif
         add $6*SZ, pC
         dec nnu
      jnz NLOOP
      movq K0, K
      dec nmu
   jnz MLOOP

DONE:
#ifdef ATL_GAS_x8632
   movl (%esp), %ebp
   movl 4(%esp), %ebx
   movl 8(%esp), %esi
   movl 16(%esp), %edi
   add $FSIZE, %esp
#else
   movq -8(%rsp), %r12
#endif
   ret
