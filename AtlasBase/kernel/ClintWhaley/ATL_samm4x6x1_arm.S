#define rA0     s0
#define rA1     s1
#define rA2     s2
#define rA3     s3
#define rB0     s4
#define rB1     s5
#define rB2     s6
#define rB3     s7
#define rC00    s8
#define rC10    s9
#define rC20    s10
#define rC30    s11
#define rC01    s12
#define rC11    s13
#define rC21    s14
#define rC31    s15
#define rC02    s16
#define rC12    s17
#define rC22    s18
#define rC32    s19
#define rC03    s20
#define rC13    s21
#define rC23    s22
#define rC33    s23
#define rC04    s24
#define rC14    s25
#define rC24    s26
#define rC34    s27
#define rC05    s28
#define rC15    s29
#define rC25    s30
#define rC35    s31

#define NMU     r0
#define NNU0    r1
#define K0      r2
#define pA0     r3
#define pB0     r4
#define pC      r5
#define NNU     r6
#define K       r7
#define pfA     r8
#define pfB     r9
#define pB      r10
#define pA      r11
#define PTR     r12
/*
                    r0       r1      r2         r3          0         4
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                      8           12          16
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);

 */
#define PFBDIST 128
#define PFBINC  64
#define PFADIST 64
#define PFAINC 64
#ifdef BETA0
   #define F1ST fmuls
#elif defined(BETA1)
   #define F1ST fmacs
#else
   #define F1ST fmscs
#endif
#include "atlas_asm.h"
.code 32
.fpu vfpv3
#ifdef ATL_ARM_HARDFP
.eabi_attribute 28, 1
#endif
.text
.align 2
.globl ATL_asmdecor(ATL_USERMM)
.type  ATL_asmdecor(ATL_USERMM), %function
ATL_asmdecor(ATL_USERMM):
   pld [pA]
   fstmDBs SP!, {s16-s31}       /* save all fp regs */
   stmDB SP!, {r4-r11,r14}      /* save all iregs */
   #define FSIZE 9*4 + 16*4
   add r12, SP, #FSIZE
   ldmIA r12, {pB0, pC, pfA, pfB}

   cmp pfA, pA
   ldrEQ pfA, [r13, #FSIZE+16]
   cmp pfB, pB
   ldrEQ pfB, [r13, #FSIZE+16]
   mov pA, pA0
   cmp K0, #120
   movGE pfA, pfB

   MLOOP:
      mov pB, pB0
      mov NNU, NNU0
         fldmIAs pB!, {rB0-rB1}
         #ifndef BETA0
            fldmIAs pC!, {rC00,rC10}
         #endif
      NLOOP:
         mov pA, pA0
/* 
 *       1st peeled K iteration handles BETA & load of C
 */
         fldmIAs pA!, {rA0-rA1}
         F1ST  rC00, rA0, rB0
         fldmIAs pA!, {rA2-rA3}
         F1ST  rC10, rA1, rB0
         #ifndef BETA0
            fldmIAs pC!, {rC20,rC30}
         #endif
         F1ST  rC20, rA2, rB0
         #ifndef BETA0
            fldmIAs pC!, {rC01,rC11}
         #endif
         F1ST  rC30, rA3, rB0
         #ifndef BETA0
            fldmIAs pC!, {rC21,rC31}
         #endif

         F1ST  rC01, rA0, rB1
         fldmIAs pB!, {rB2, rB3}
         F1ST  rC11, rA1, rB1
         #ifndef BETA0
            fldmIAs pC!, {rC02,rC12}
         #endif
         F1ST  rC21, rA2, rB1
         #ifndef BETA0
            fldmIAs pC!, {rC22,rC32}
         #endif
         F1ST  rC31, rA3, rB1
         fldmIAs pB!, {rB0, rB1}

         F1ST  rC02, rA0, rB2
         #ifndef BETA0
            fldmIAs pC!, {rC03,rC13}
         #endif
         F1ST  rC12, rA1, rB2
         #ifndef BETA0
            fldmIAs pC!, {rC23,rC33}
         #endif
         F1ST  rC22, rA2, rB2
         #ifndef BETA0
            fldmIAs pC!, {rC04,rC14}
         #endif
         F1ST  rC32, rA3, rB2
         #ifndef BETA0
            fldmIAs pC!, {rC24,rC34}
         #endif

         F1ST  rC03, rA0, rB3
         #ifndef BETA0
            fldmIAs pC!, {rC05,rC15}
         #endif
         F1ST  rC13, rA1, rB3
         #ifndef BETA0
            fldmIAs pC!, {rC25,rC35}
         #endif
         F1ST  rC23, rA2, rB3
         F1ST  rC33, rA3, rB3
             fldmIAs pB!, {rB2, rB3}

         F1ST  rC04, rA0, rB0
         #ifndef BETA0
            sub pC, pC, #96
         #endif
         F1ST  rC14, rA1, rB0
         subs K, K0, #1
         F1ST  rC24, rA2, rB0
         F1ST  rC34, rA3, rB0

         F1ST  rC05, rA0, rB1
         F1ST  rC15, rA1, rB1
             fldmiasNE pA!, {rA0, rA1}
         F1ST  rC25, rA2, rB1
         F1ST  rC35, rA3, rB1
         bEQ KDONE2
             fldmias pA!, {rA2, rA3}
/* 
 *       2nd peeled K iteration
 */

         fmacs rC00, rA0, rB2
             fldmias pB!, {rB0, rB1}
         fmacs rC10, rA1, rB2
         fmacs rC20, rA2, rB2
         fmacs rC30, rA3, rB2

         fmacs rC01, rA0, rB3
         fmacs rC11, rA1, rB3
         fmacs rC21, rA2, rB3
         fmacs rC31, rA3, rB3
             fldmias pB!, {rB2, rB3}

         fmacs rC02, rA0, rB0
         fmacs rC12, rA1, rB0
         fmacs rC22, rA2, rB0
         fmacs rC32, rA3, rB0

         fmacs rC03, rA0, rB1
         #ifdef BETA0
            pld [pC]
         #else
            pld [pC, #128]
         #endif
         fmacs rC13, rA1, rB1
         #ifdef BETA0
            pld [pC, #64]
         #endif
         fmacs rC23, rA2, rB1
         fmacs rC33, rA3, rB1
             fldmIAs pB!, {rB0, rB1}

         fmacs rC04, rA0, rB2
            pld [pfA]
         fmacs rC14, rA1, rB2
            pld [pfB]
         fmacs rC24, rA2, rB2
            add pfA, pfA, #64
         fmacs rC34, rA3, rB2
            add pfB, pfB, #64

         subs K, K, #1
         fmacs rC05, rA0, rB3
         fmacs rC15, rA1, rB3
             fldmiasNE pA!, {rA0, rA1}
         fmacs rC25, rA2, rB3
         fmacs rC35, rA3, rB3
             fldmiasNE pA!, {rA2, rA3}
         beq KDONE

         KLOOP:
            subs K, K, #1
/* 
 *          1st K iteration
 */
            fmacs rC00, rA0, rB0
                fldmias pB!, {rB2, rB3}
            fmacs rC10, rA1, rB0
            pld [pB, #PFBDIST]
            fmacs rC20, rA2, rB0
            pld [pB, #PFBDIST+PFBINC]
            fmacs rC30, rA3, rB0

            fmacs rC01, rA0, rB1
            pld [pB, #PFBDIST+2*PFBINC]
            fmacs rC11, rA1, rB1
            fmacs rC21, rA2, rB1
            fmacs rC31, rA3, rB1
                fldmias pB!, {rB0, rB1}

            fmacs rC02, rA0, rB2
            pld [pB, #PFBDIST+3*PFBINC-8]
            fmacs rC12, rA1, rB2
            fmacs rC22, rA2, rB2
            pld [pA, #PFADIST]
            fmacs rC32, rA3, rB2

            fmacs rC03, rA0, rB3
            pld [pA, #PFADIST+1*PFAINC]
            fmacs rC13, rA1, rB3
            fmacs rC23, rA2, rB3
            fmacs rC33, rA3, rB3
                fldmias pB!, {rB2, rB3}

            fmacs rC04, rA0, rB0
            fmacs rC14, rA1, rB0
            fmacs rC24, rA2, rB0
            fmacs rC34, rA3, rB0

            fmacs rC05, rA0, rB1
            fmacs rC15, rA1, rB1
                fldmiasNE pA!, {rA0, rA1}
            fmacs rC25, rA2, rB1
            fmacs rC35, rA3, rB1
            bEQ KDONE2
                fldmias pA!, {rA2, rA3}
/* 
 *          2nd K iteration
 */
            subs K, K, #1

            fmacs rC00, rA0, rB2
                fldmias pB!, {rB0, rB1}
            fmacs rC10, rA1, rB2
            fmacs rC20, rA2, rB2
            fmacs rC30, rA3, rB2

            fmacs rC01, rA0, rB3
            fmacs rC11, rA1, rB3
            fmacs rC21, rA2, rB3
            fmacs rC31, rA3, rB3
                fldmias pB!, {rB2, rB3}

            fmacs rC02, rA0, rB0
            fmacs rC12, rA1, rB0
            fmacs rC22, rA2, rB0
            fmacs rC32, rA3, rB0

            fmacs rC03, rA0, rB1
            fmacs rC13, rA1, rB1
            fmacs rC23, rA2, rB1
            fmacs rC33, rA3, rB1
                fldmias pB!, {rB0, rB1}

            fmacs rC04, rA0, rB2
            fmacs rC14, rA1, rB2
            fmacs rC24, rA2, rB2
            fmacs rC34, rA3, rB2

            fmacs rC05, rA0, rB3
            fmacs rC15, rA1, rB3
                fldmiasNE pA!, {rA0, rA1}
            fmacs rC25, rA2, rB3
            fmacs rC35, rA3, rB3
                fldmiasNE pA!, {rA2, rA3}

         bne KLOOP
KDONE:
         subs NNU, NNU, #1
         fstmIAs pC!, {rC00-rC35}
         #ifndef BETA0
            fldmIAsNE pC!, {rC00,rC10}
         #endif

      bne NLOOP
      add pA0, pA0, K0, LSL #4
      subs NMU, NMU, #1
   bne MLOOP

   ldmIA SP!, {r4-r11,r14}  /* restore all iregs */
   fldmIAs SP!, {s16-s31}   /* restore all fpregs */
   bx lr
KDONE2:
   fcpys rB0, rB2
   fcpys rB1, rB3
   b KDONE
.size ATL_asmdecor(ATL_USERMM),.-ATL_asmdecor(ATL_USERMM)
