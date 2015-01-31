#include "atlas_asm.h"

#define rB      %ymm0
#define rA0     %ymm1
#define rA1     %ymm2
#define rB0     %ymm3
#define rC00    %ymm4
#define rC10    %ymm5
#define rC01    %ymm6
#define rC11    %ymm7
#define rC02    %ymm8
#define rC12    %ymm9
#define rC03    %ymm10
#define rC13    %ymm11
#define rC04    %ymm12
#define rC14    %ymm13
#define rC05    %ymm14
#define rC15    %ymm15

/*
 * Prioritize original registers for inner-loop operations, but inc regs
 * can be anything w/o changing opcode size, so use new regs for those
 */
#define KK      %rdx  /* API reg */
#define pA      %rcx  /* API reg */
#define pB      %rax  /* comes in as r8 */
#define incBn   %rsi  
#define incAm   %rdi
/*
 * Then N-loop variables much less important, so use any orig regs left
 */
#define pA0     %r8   /* set after mov r8 to pB (rax) */
#define pC      %r9   /* API reg */
#define nnu     %r10  /* comes in as rsi */
#define pfA     %rbx
#define pfB     %r12
#define incC    %rbp
#define KK0     %r13
/*
 * We could give a rat's ass about what registers used in outer (M-) loop
 */
#define nmu     %r11  /* comes in as rdi */
#define nnu0    %r14
#define pB0     %r15
/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */
#define PFBDIST 1*128
#define PFADIST 10*128
#ifdef ATL_3DNow
   #define prefC(m_) prefetchw m_
#else
   #define prefC(m_) prefetcht0 m_
#endif
#define prefA(m_) prefetcht0 m_
#define prefB(m_) prefetcht0 m_
#if defined(ATL_MOVEC)
   #define prefA2(m_) prefetcht2 m_
   #define prefB2(m_) prefetcht2 m_
   #define PFNEXT 0
#else
   #define PFNEXT 1
   #define prefA2(m_) prefetcht2 m_
   #define prefB2(m_) prefetcht2 m_
#endif
#define FMAC vfmadd231pd   /* FMAC m256/r256, rs1, rd */
#if defined(BETAN) || defined(BETAn)
   #define BETAN1
#endif
#ifdef BETAN1
   #define FMACI vfmsub231pd
#elif defined(BETA0)
   #define FMACI vmulpd
#else
   #define FMACI FMAC
#endif
#define vmovapd vmovaps
#define vxorpd vxorps
.text
.global ATL_asmdecor(ATL_USERMM)
ALIGN16
ATL_asmdecor(ATL_USERMM):
   #if !defined(BETA0) && defined(FORCE_MACC)
      #define BETAOFF -56
      fld1         /* st = {1.0} */
      #ifdef BETAN1
         fchs      /* st = {-1.0} */
      #endif
      fstpl BETAOFF(%rsp)   /* st = {} */
  #endif
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
   mov nnu, nnu0
   #if PFNEXT
      movq 8(%rsp), pfA      /* pfA = pAn */
      movq 16(%rsp), pfB     /* pfB = pBn */
   #endif
   mov KK, KK0
   sub $-128, pA
   sub $-128, pB
   sub $-128, pC
   mov pA, pA0
   mov pB, pB0
   lea (KK, KK, 2), incBn /* incBn = 3*K */
   shl $4, incBn          /* incBn = K*nu*sz = K*6*8 = 16*(3*K) */
   mov KK, incAm          /* incAm = K */
   shl $6, incAm          /* incAm = mu*K*sizeof = 8*K*8 = K*64 */
   mov $384, incC         /* incC = mu*nu*sz = 8*6*8 = 384 */

   #ifndef BETA0
      vmovapd -128(pC), rC00
      vmovapd  -96(pC), rC10
      vmovapd  -64(pC), rC01
      vmovapd  -32(pC), rC11
      vmovapd     (pC), rC02
      vmovapd   32(pC), rC12
      vmovapd   64(pC), rC03
      vmovapd   96(pC), rC13
      vmovapd   128(pC), rC04
      vmovapd   160(pC), rC14
      vmovapd   192(pC), rC05
      vmovapd   224(pC), rC15
   #endif
            vmovapd -128(pA), rA0
            vmovapd -96(pA), rA1
            vmovapd -128(pB), rB
   .local MNLOOP
   ALIGN32
   MNLOOP:
/*
 *       First iteration peeled to handle correct application of beta,
 *       and perform some prefetch on next blocks
 */
         vpermpd $0x00, rB, rB0
	    prefC(-128(pC,incC))
         FMACI rA0, rB0, rC00
         FMACI rA1, rB0, rC10
         vpermpd $0x55, rB, rB0
	    prefC((pC,incC))
         FMACI rA0, rB0, rC01
         FMACI rA1, rB0, rC11
         vpermpd $0xAA, rB, rB0
	    prefC(128(pC,incC))
         FMACI rA0, rB0, rC02
         FMACI rA1, rB0, rC12
         vpermpd $0xFF, rB, rB0
         vmovapd -128+32(pB), rB
         FMACI rA0, rB0, rC03
         FMACI rA1, rB0, rC13
         vpermpd $0x00, rB, rB0
         #if PFNEXT && defined(ATL_MOVEA)
	    prefA2(-128(pfA))
	 #endif
         FMACI rA0, rB0, rC04
         FMACI rA1, rB0, rC14
         vpermpd $0x55, rB, rB0
         #if PFNEXT && defined(ATL_MOVEA)
	    prefA2((pfA))
	 #endif
         FMACI rA0, rB0, rC05
         vmovapd -128+64(pA), rA0
         FMACI rA1, rB0, rC15
         vmovapd -96+64(pA), rA1

         vpermpd $0xAA, rB, rB0
         #if PFNEXT && defined(ATL_MOVEA)
	    prefA2(128(pfA))
	    add incC, pfA
         #endif
         FMAC rA0, rB0, rC00
         FMAC rA1, rB0, rC10
         vpermpd $0xFF, rB, rB0
         vmovapd -96+32(pB), rB
         FMAC rA0, rB0, rC01
         FMAC rA1, rB0, rC11
         vpermpd $0x00, rB, rB0
         #if PFNEXT && defined(ATL_MOVEB)
	    prefB2(-128(pfB))
         #endif
         FMAC rA0, rB0, rC02
         FMAC rA1, rB0, rC12
         vpermpd $0x55, rB, rB0
         #if PFNEXT && defined(ATL_MOVEB)
	    prefB2((pfB))
         #endif
         FMAC rA0, rB0, rC03
         FMAC rA1, rB0, rC13
         vpermpd $0xAA, rB, rB0
         #if PFNEXT && defined(ATL_MOVEB)
	    prefB2(128(pfB))
	 #endif
         FMAC rA0, rB0, rC04
         FMAC rA1, rB0, rC14
         vpermpd $0xFF, rB, rB0
            vmovapd -64+32(pB), rB
         #if PFNEXT && defined(ATL_MOVEB)
	    add incC, pfB
         #endif
         FMAC rA0, rB0, rC05
            vmovapd (pA), rA0
            FMAC rA1, rB0, rC15
               vmovapd -32+64(pA), rA1
	    add $192, pA
            #if PFNEXT
               prefB(PFBDIST-128(pB))
            #endif
	    sub $-128, pB
            #if PFNEXT
               prefA(PFADIST-192(pA))
            #endif
            sub $2, %edx
	    jz KDONE

      nop
   ALIGN16
         KLOOP:
            vpermpd $0x00, rB, rB0
               prefB(PFBDIST-128(pB))
            FMAC rA0, rB0, rC00
            FMAC rA1, rB0, rC10
            vpermpd $0x55, rB, rB0
               prefA(PFADIST-128(pA))
            FMAC rA0, rB0, rC01
            FMAC rA1, rB0, rC11
            vpermpd $0xAA, rB, rB0
            FMAC rA0, rB0, rC02
            FMAC rA1, rB0, rC12
            vpermpd $0xFF, rB, rB0
            vmovapd -128(pB), rB
            FMAC rA0, rB0, rC03
            FMAC rA1, rB0, rC13
            vpermpd $0x00, rB, rB0
            FMAC rA0, rB0, rC04
            FMAC rA1, rB0, rC14
            vpermpd $0x55, rB, rB0
            FMAC rA0, rB0, rC05
            vmovapd -128(pA), rA0
            FMAC rA1, rB0, rC15
            vmovapd -96(pA), rA1

            vpermpd $0xAA, rB, rB0
            FMAC rA0, rB0, rC00
            FMAC rA1, rB0, rC10
            vpermpd $0xFF, rB, rB0
            vmovapd -96(pB), rB
            FMAC rA0, rB0, rC01
            FMAC rA1, rB0, rC11
            vpermpd $0x00, rB, rB0
            FMAC rA0, rB0, rC02
            FMAC rA1, rB0, rC12
            vpermpd $0x55, rB, rB0
            FMAC rA0, rB0, rC03
            FMAC rA1, rB0, rC13
            vpermpd $0xAA, rB, rB0
            FMAC rA0, rB0, rC04
            FMAC rA1, rB0, rC14
            vpermpd $0xFF, rB, rB0
               vmovapd -64(pB), rB
            FMAC rA0, rB0, rC05
               vmovapd -64(pA), rA0
            FMAC rA1, rB0, rC15
               vmovapd -32(pA), rA1

	    add $96, pB
	    sub $-128, pA
            sub $2, %edx
         jnz KLOOP
      KDONE:
         #define MOVC vmovapd
//         #define MOVC vmovntpd
         MOVC rC00, -128(pC)
         #ifndef BETA0
            vmovapd -128(pC,incC), rC00
	 #endif
         MOVC rC10,  -96(pC)
         #ifndef BETA0
            vmovapd  -96(pC,incC), rC10
	 #endif
         MOVC rC01,  -64(pC)
         #ifndef BETA0
            vmovapd  -64(pC,incC), rC01
	 #endif
         MOVC rC11,  -32(pC)
         #ifndef BETA0
            vmovapd  -32(pC,incC), rC11
	 #endif
         MOVC rC02,     (pC)
         #ifndef BETA0
            vmovapd     (pC,incC), rC02
	 #endif
         MOVC rC12,   32(pC)
         #ifndef BETA0
            vmovapd   32(pC,incC), rC12
	 #endif
         MOVC rC03,   64(pC)
         #ifndef BETA0
            vmovapd   64(pC,incC), rC03
	 #endif
         MOVC rC13,   96(pC)
         #ifndef BETA0
            vmovapd   96(pC,incC), rC13
	 #endif
         MOVC rC04,  128(pC)
         #ifndef BETA0
            vmovapd  128(pC,incC), rC04
	 #endif
         MOVC rC14,  160(pC)
         #ifndef BETA0
            vmovapd  160(pC,incC), rC14
	 #endif
         MOVC rC05,  192(pC)
         #ifndef BETA0
            vmovapd  192(pC,incC), rC05
	 #endif
         MOVC rC15,  224(pC)
         #ifndef BETA0
            vmovapd  224(pC,incC), rC15
	 #endif
	 sub $32, pB
            vmovapd -128(pA0), rA0
	 add incC, pC
            vmovapd -96(pA0), rA1
         mov KK0, KK
         mov pA0, pA
         sub $1, nnu
      jnz MNLOOP
      add incAm, pA0
         vmovapd -128(pB0), rB
      mov nnu0, nnu
         vmovapd -128(pA0), rA0
      mov pA0, pA
         vmovapd -96(pA0), rA1
      mov pB0, pB
      sub $1, nmu
   jnz MNLOOP
 DONE:
   movq -8(%rsp), %rbp
   movq -16(%rsp), %rbx
   movq -24(%rsp), %r12
   movq -32(%rsp), %r13
   movq -40(%rsp), %r14
   movq -48(%rsp), %r15
   ret
#if 1
.global findSize
findSize:
mov $SS1-SS0, %rax
ret
SS0:
   vmovaps (%rax,%r8), %ymm15
SS1:
#endif

