#ifndef ATL_GAS_ARM
   #error "This routine requires GAS/ARM assembly"
#endif
#define rA0     d0
#define rA1     d1
#define rA2     d2
#define rA3     d3
#define rA4     d4
#define rB0     d5
#define rB1     d6
#define rC00    d7
#define rC10    d8
#define rC20    d9
#define rC30    d10
#define rC40    d11
#define rC01    d12
#define rC11    d13
#define rC21    d14
#define rC31    d15
#define rC41    d16
#define rC02    d17
#define rC12    d18
#define rC22    d19
#define rC32    d20
#define rC42    d21
#define rC03    d22
#define rC13    d23
#define rC23    d24
#define rC33    d25
#define rC43    d26
#define rC04    d27
#define rC14    d28
#define rC24    d29
#define rC34    d30
#define rC44    d31

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
#define incAm   r14
/*
                    r0       r1      r2         r3          0         4
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                      8           12          16
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);

 */
#define PFBDIST 64
#define PFBINC  128
#define PFADIST 64
#define PFAINC 128
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
   fstmDBd SP!, {d8-d15}        /* save all fp regs */
   stmDB SP!, {r4-r11,r14}      /* save all iregs */
   #define FSIZE 9*4 + 8*8
   add r12, SP, #FSIZE
   ldmIA r12, {pB0, pC, pfA, pfB}

   mov incAm, K0, LSL #5         /* incAm = 4*K*sizeof = K*32 = K << 5*/
         fldd rB0, [pB0]
   add incAm, incAm, K0, LSL #3  /* incAm = 5*K*sizeof = */
   pld [pB0, #64]
   cmp pfB, pB
   mov pB, pB0
   movNE pfA, pfB
   cmp pfA, pA
   mov NNU, NNU0
   ldrEQ pfA, [r13, #FSIZE+16]

   MLOOP:
      NLOOP:
         fldmiad pA0, {rA0-rA4}
         #ifndef BETA0
            add PTR, pC, #120
            fldmiad pC, {rC00-rC42}
            fldmiad PTR, {rC03-rC44}
         #endif
         mov pA, pA0
         #ifdef BETA0
            #define F1ST fmuld
         #elif defined(BETA1)
            #define F1ST fmacd
         #else
            #define F1ST fmscd
         #endif
/*
 *       1st K iteration peeled to support BETA CASES & prefetch
 */
         subs K, K0, #1
         F1ST  rC00, rA0, rB0
            fldd rB1, [pB,#8]
         F1ST  rC10, rA1, rB0
            pld [pB,#64]
         F1ST  rC20, rA2, rB0
            pld [pB,#128]
         F1ST  rC30, rA3, rB0
            pld [pB,#192]
         F1ST  rC40, rA4, rB0
         fldd   rB0, [pB,#16]

         F1ST  rC01, rA0, rB1
            pld [pB,#256]
         F1ST  rC11, rA1, rB1
            pld [pB,#320]
         F1ST  rC21, rA2, rB1
            pld [pB,#384]
         F1ST  rC31, rA3, rB1
         F1ST  rC41, rA4, rB1
         fldd   rB1, [pB,#24]

         F1ST  rC02, rA0, rB0
         F1ST  rC12, rA1, rB0
         F1ST  rC22, rA2, rB0
         F1ST  rC32, rA3, rB0
         F1ST  rC42, rA4, rB0
         fldd   rB0, [pB,#32]

         F1ST  rC03, rA0, rB1
         F1ST  rC13, rA1, rB1
         F1ST  rC23, rA2, rB1
         F1ST  rC33, rA3, rB1
         F1ST  rC43, rA4, rB1
            fldd   rB1, [pB,#40]

         F1ST  rC04, rA0, rB0
            flddNE rA0, [pA,#40]
         F1ST  rC14, rA1, rB0
            flddNE rA1, [pA,#48]
         F1ST  rC24, rA2, rB0
            flddNE rA2, [pA,#56]
         F1ST  rC34, rA3, rB0
            flddNE rA3, [pA,#64]
         F1ST  rC44, rA4, rB0
         bEQ KDONE2
            fldd   rA4, [pA,#72]
/*
 *       2nd peeled K iteration
 */
         fmacd rC00, rA0, rB1
         fldd rB0, [pB,#48]
         fmacd rC10, rA1, rB1
         fmacd rC20, rA2, rB1
            pld [pfA]
         fmacd rC30, rA3, rB1
         fmacd rC40, rA4, rB1
         fldd rB1, [pB,#56]

         fmacd rC01, rA0, rB0
         fmacd rC11, rA1, rB0
         fmacd rC21, rA2, rB0
         fmacd rC31, rA3, rB0
         fmacd rC41, rA4, rB0
         fldd rB0, [pB,#64]

         fmacd rC02, rA0, rB1
            pld [pC, #200]
         fmacd rC12, rA1, rB1
            pld [pC, #264]
         fmacd rC22, rA2, rB1
            pld [pC, #328]
         fmacd rC32, rA3, rB1
            pld [pC, #392]
         fmacd rC42, rA4, rB1
         fldd rB1, [pB,#72]

         fmacd rC03, rA0, rB0
         add pB, pB, #80
         fmacd rC13, rA1, rB0
         add pA, pA, #80
         fmacd rC23, rA2, rB0
            add pfA, pfA, #64
         fmacd rC33, rA3, rB0
         subs K, K, #1
         fmacd rC43, rA4, rB0
            fldd   rB0, [pB]

         fmacd rC04, rA0, rB1
            flddNE rA0, [pA]
         fmacd rC14, rA1, rB1
            flddNE rA1, [pA,#8]
         fmacd rC24, rA2, rB1
            flddNE rA2, [pA,#16]
         fmacd rC34, rA3, rB1
            flddNE rA3, [pA,#24]
         fmacd rC44, rA4, rB1
            flddNE rA4, [pA,#32]
         bEQ KDONE

         KLOOP:
/*
 *          1st K iteration
 */
            fmacd rC00, rA0, rB0
               fldd rB1, [pB,#8]
            fmacd rC10, rA1, rB0
               pld [pB,#64]
            fmacd rC20, rA2, rB0
               pld [pA,#64]
            fmacd rC30, rA3, rB0
               pld [pB,#128]
            fmacd rC40, rA4, rB0
            fldd rB0, [pB,#16]

            fmacd rC01, rA0, rB1
               pld [pA,#128]
            fmacd rC11, rA1, rB1
               pld [pB,#192]
            fmacd rC21, rA2, rB1
               pld [pA,#192]
            fmacd rC31, rA3, rB1
               pld [pA,#256]
            fmacd rC41, rA4, rB1
            fldd rB1, [pB,#24]

            fmacd rC02, rA0, rB0
               pld [pB,#256]
            fmacd rC12, rA1, rB0
               pld [pA,#320]
            fmacd rC22, rA2, rB0
               pld [pA,#384]
            fmacd rC32, rA3, rB0
            subs K, K, #1
            fmacd rC42, rA4, rB0
            fldd rB0, [pB,#32]

            fmacd rC03, rA0, rB1
            fmacd rC13, rA1, rB1
            fmacd rC23, rA2, rB1
            fmacd rC33, rA3, rB1
            fmacd rC43, rA4, rB1
               fldd   rB1, [pB,#40]

            fmacd rC04, rA0, rB0
               flddNE rA0, [pA,#40]
            fmacd rC14, rA1, rB0
               flddNE rA1, [pA,#48]
            fmacd rC24, rA2, rB0
               flddNE rA2, [pA,#56]
            fmacd rC34, rA3, rB0
               flddNE rA3, [pA,#64]
            fmacd rC44, rA4, rB0
               flddNE rA4, [pA,#72]
            bEQ KDONE2

/*
 *          2nd K iteration
 */
            fmacd rC00, rA0, rB1
               fldd rB0, [pB,#48]
            fmacd rC10, rA1, rB1
            fmacd rC20, rA2, rB1
            fmacd rC30, rA3, rB1
            fmacd rC40, rA4, rB1
            fldd rB1, [pB,#56]

            fmacd rC01, rA0, rB0
            fmacd rC11, rA1, rB0
            fmacd rC21, rA2, rB0
            fmacd rC31, rA3, rB0
            fmacd rC41, rA4, rB0
            fldd rB0, [pB,#64]

            fmacd rC02, rA0, rB1
            fmacd rC12, rA1, rB1
            fmacd rC22, rA2, rB1
            fmacd rC32, rA3, rB1
            fmacd rC42, rA4, rB1
            fldd rB1, [pB,#72]

            fmacd rC03, rA0, rB0
               add pB, pB, #80
            fmacd rC13, rA1, rB0
               add pA, pA, #80
            fmacd rC23, rA2, rB0
            fmacd rC33, rA3, rB0
            subs K, K, #1
            fmacd rC43, rA4, rB0
               fldd   rB0, [pB]

            fmacd rC04, rA0, rB1
               flddNE rA0, [pA]
            fmacd rC14, rA1, rB1
               flddNE rA1, [pA,#8]
            fmacd rC24, rA2, rB1
               flddNE rA2, [pA,#16]
            fmacd rC34, rA3, rB1
               flddNE rA3, [pA,#24]
            fmacd rC44, rA4, rB1
               flddNE rA4, [pA,#32]
         bne KLOOP
KDONE:

         subs NNU, NNU, #1
         fstmIAd pC!, {rC00-rC42}
         fstmIAd pC!, {rC03-rC44}

      bne NLOOP
      subs NMU, NMU, #1
         fldd rB0, [pB0]
      mov pB, pB0
      mov NNU, NNU0
      add pA0, pA0, incAm
   bne MLOOP
DONE:
   ldmIA SP!, {r4-r11,r14}  /* restore all iregs */
   fldmIAd SP!, {d8-d15}    /* restore all fpregs */
   bx lr
KDONE2:
   fcpyd rB0, rB1
   add pB, #40
   add pA, #40
   b KDONE


.size ATL_asmdecor(ATL_USERMM),.-ATL_asmdecor(ATL_USERMM)
