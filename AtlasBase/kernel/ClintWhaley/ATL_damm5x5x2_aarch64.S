#ifndef ATL_GAS_ARM64
   #error "This routine requires GAS/AARCH64 assembly"
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

#define NMU     w0
#define NNU0    w1
#define K0      w2
#define pA0     x3
#define pB0     x4
#define pC      x5
#define NNU     w6
#define K       w7
#define pB      x10
#define pA      x11
#define incAm   x14
/*
                    w0       w1      w2         x3         x4        x5
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                      x6          x7          0
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);

 */
#include "atlas_asm.h"
.text
.align 2
.globl ATL_asmdecor(ATL_USERMM)
.type  ATL_asmdecor(ATL_USERMM), %function
ATL_asmdecor(ATL_USERMM):
        add     sp,sp,#-(10*16)
        stp     d8,d9,[sp,#(0*16)]
        stp     d10,d11,[sp,#(1*16)]
        stp     d12,d13,[sp,#(2*16)]
        stp     d14,d15,[sp,#(3*16)]
        stp     x18,x19,[sp,#(4*16)]
        stp     x20,x21,[sp,#(5*16)]
        stp     x22,x23,[sp,#(6*16)]
        stp     x24,x25,[sp,#(7*16)]
        stp     x26,x27,[sp,#(8*16)]
        stp     x28,x29,[sp,#(9*16)]
   

   mov pA, pA0
   mov x14, #0
   add   incAm,incAm,x2, LSL #3
   add   incAm,incAm,x2, LSL #3
   add   incAm,incAm,x2, LSL #3
   add   incAm,incAm,x2, LSL #3
   add   incAm,incAm,x2, LSL #3

   MLOOP:
      ldr rB0, [pB0]
      mov pB, pB0
      mov NNU, NNU0
      NLOOP:
         #ifndef BETA0
         ldp rC00, rC10, [pC,#(0*16)]
         ldp rC20, rC30, [pC,#(1*16)]
         ldp rC40, rC01, [pC,#(2*16)]
         ldp rC11, rC21, [pC,#(3*16)]
         ldp rC31, rC41, [pC,#(4*16)]
         ldp rC02, rC12, [pC,#(5*16)]
         ldp rC22, rC32, [pC,#(6*16)]
         ldp rC42, rC03, [pC,#(7*16)]
         ldp rC13, rC23, [pC,#(8*16)]
         ldp rC33, rC43, [pC,#(9*16)]
         ldp rC04, rC14, [pC,#(10*16)]
         ldp rC24, rC34, [pC,#(11*16)]
         ldr rC44,       [pC,#(12*16)]
         #endif
         mov pA, pA0
         ldp rA0, rA1, [pA,#0]
         ldp rA2, rA3, [pA,#16]
         ldr rA4,      [pA,#32]
         #ifdef BETA0
            #define F1ST( op1, op2, op3) fmul op1, op2, op3
         #elif defined(BETA1)
            #define F1ST( op1, op2, op3) fmadd op1, op2, op3, op1
         #else
            #define F1ST( op1, op2, op3) fnmsub op1, op2, op3, op1
         #endif
/*
 *       1st K iteration peeled to support BETA CASES & prefetch
 */
         F1ST(  rC00, rA0, rB0)
            ldr rB1, [pB,#8]
         F1ST(  rC10, rA1, rB0)
            /*pld [pB, #PFBDIST] */
         F1ST(  rC20, rA2, rB0)
            /*pld [pB, #PFBDIST1*PFAINC] */
         F1ST(  rC30, rA3, rB0)
            /*pld [pB, #PFBDIST2*PFAINC] */
         F1ST(  rC40, rA4, rB0)
         ldr rB0, [pB,#16]

         F1ST(  rC01, rA0, rB1)
            /*pld [pB, #PFBDIST3*PFAINC] */
         F1ST(  rC11, rA1, rB1)
            /*pld [pA, #PFADIST] */
         F1ST(  rC21, rA2, rB1)
            /*pld [pA, #PFADIST1*PFAINC] */
         F1ST(  rC31, rA3, rB1)
            /*pld [pA, #PFADIST2*PFAINC] */
         F1ST(  rC41, rA4, rB1)
         ldr rB1, [pB,#24]

         F1ST(  rC02, rA0, rB0)
            /*pld [pA, #PFADIST3*PFAINC] */
         F1ST(  rC12, rA1, rB0)
         F1ST(  rC22, rA2, rB0)
            /*pld [pfA] */
         F1ST(  rC32, rA3, rB0)
         F1ST(  rC42, rA4, rB0)
         ldr rB0, [pB,#32]

         F1ST(  rC03, rA0, rB1)
            /*pld [pfB] */
         F1ST(  rC13, rA1, rB1)
         F1ST(  rC23, rA2, rB1)
         F1ST(  rC33, rA3, rB1)
         F1ST(  rC43, rA4, rB1)
            ldr rB1, [pB,#40]

         F1ST(  rC04, rA0, rB0)
            ldr rA0, [pA,#40]
         F1ST(  rC14, rA1, rB0)
            ldr rA1, [pA,#48]
         F1ST(  rC24, rA2, rB0)
            ldr rA2, [pA,#56]
         F1ST(  rC34, rA3, rB0)
            ldr rA3, [pA,#64]
         F1ST(  rC44, rA4, rB0)
            ldr rA4, [pA,#72]
/*
 *       2nd peeled K iteration
 */
         fmadd rC00,rA0,rB1,rC00
         ldr rB0, [pB,#48]
         fmadd rC10,rA1,rB1,rC10
         fmadd rC20,rA2,rB1,rC20
            /*pld [pC,#128] */
         fmadd rC30,rA3,rB1,rC30
         fmadd rC40,rA4,rB1,rC40
         ldr rB1, [pB,#56]

         fmadd rC01,rA0,rB0,rC01
         fmadd rC11,rA1,rB0,rC11
         fmadd rC21,rA2,rB0,rC21
         fmadd rC31,rA3,rB0,rC31
         fmadd rC41,rA4,rB0,rC41
         ldr rB0, [pB,#64]

         fmadd rC02,rA0,rB1,rC02
         fmadd rC12,rA1,rB1,rC12
         fmadd rC22,rA2,rB1,rC22
         fmadd rC32,rA3,rB1,rC32
         fmadd rC42,rA4,rB1,rC42
         ldr rB1, [pB,#72]

         fmadd rC03,rA0,rB0,rC03
         add pB, pB, #80
         fmadd rC13,rA1,rB0,rC13
         add pA, pA, #80
         fmadd rC23,rA2,rB0,rC23
            /*add pfA, pfA, #64 */
         fmadd rC33,rA3,rB0,rC33
            /*add pfB, pfB, #64 */
         fmadd rC43,rA4,rB0,rC43
            ldr rB0, [pB]

         subs K, K0, #2
         fmadd rC04,rA0,rB1,rC04
            ldr rA0, [pA]
         fmadd rC14,rA1,rB1,rC14
            ldr rA1, [pA,#8]
         fmadd rC24,rA2,rB1,rC24
            ldr rA2, [pA,#16]
         fmadd rC34,rA3,rB1,rC34
            ldr rA3, [pA,#24]
         fmadd rC44,rA4,rB1,rC44
            ldr rA4, [pA,#32]
         beq KDONE

         KLOOP:
/*
 *          1st K iteration
 */
            fmadd rC00,rA0,rB0,rC00
               ldr rB1, [pB,#8]
            fmadd rC10,rA1,rB0,rC10
               /*pld [pB, #PFBDIST] */
            fmadd rC20,rA2,rB0,rC20
               /*pld [pB, #PFBDISTPFBINC] */
            fmadd rC30,rA3,rB0,rC30
               /*pld [pB, #PFBDIST2*PFBINC] */
            fmadd rC40,rA4,rB0,rC40
            ldr rB0, [pB,#16]

            fmadd rC01,rA0,rB1,rC01
               /*pld [pB, #PFBDIST3*PFBINC] */
            fmadd rC11,rA1,rB1,rC11
               /*pld [pA, #PFADIST] */
            fmadd rC21,rA2,rB1,rC21
               /*pld [pA, #PFADIST1*PFAINC] */
            fmadd rC31,rA3,rB1,rC31
               /*pld [pA, #PFADIST2*PFAINC] */
            fmadd rC41,rA4,rB1,rC41
            ldr rB1, [pB,#24]

            fmadd rC02,rA0,rB0,rC02
               /*pld [pA, #PFADIST3*PFAINC] */
            fmadd rC12,rA1,rB0,rC12
               /*pld [pA, #PFADIST4*PFAINC] */
            fmadd rC22,rA2,rB0,rC22
            fmadd rC32,rA3,rB0,rC32
            fmadd rC42,rA4,rB0,rC42
            ldr rB0, [pB,#32]

            fmadd rC03,rA0,rB1,rC03
            fmadd rC13,rA1,rB1,rC13
            fmadd rC23,rA2,rB1,rC23
            fmadd rC33,rA3,rB1,rC33
            fmadd rC43,rA4,rB1,rC43
               ldr rB1, [pB,#40]

            fmadd rC04,rA0,rB0,rC04
               ldr rA0, [pA,#40]
            fmadd rC14,rA1,rB0,rC14
               ldr rA1, [pA,#48]
            fmadd rC24,rA2,rB0,rC24
               ldr rA2, [pA,#56]
            fmadd rC34,rA3,rB0,rC34
               ldr rA3, [pA,#64]
            fmadd rC44,rA4,rB0,rC44
               ldr rA4, [pA,#72]

/*
 *          2nd K iteration
 */
            fmadd rC00,rA0,rB1,rC00
               ldr rB0, [pB,#48]
            fmadd rC10,rA1,rB1,rC10
            fmadd rC20,rA2,rB1,rC20
            fmadd rC30,rA3,rB1,rC30
            fmadd rC40,rA4,rB1,rC40
            ldr rB1, [pB,#56]

            fmadd rC01,rA0,rB0,rC01
            fmadd rC11,rA1,rB0,rC11
            fmadd rC21,rA2,rB0,rC21
            fmadd rC31,rA3,rB0,rC31
            fmadd rC41,rA4,rB0,rC41
            ldr rB0, [pB,#64]

            fmadd rC02,rA0,rB1,rC02
            fmadd rC12,rA1,rB1,rC12
            fmadd rC22,rA2,rB1,rC22
            fmadd rC32,rA3,rB1,rC32
            fmadd rC42,rA4,rB1,rC42
            ldr rB1, [pB,#72]

            fmadd rC03,rA0,rB0,rC03
               add pB, pB, #80
            fmadd rC13,rA1,rB0,rC13
               add pA, pA, #80
            fmadd rC23,rA2,rB0,rC23
            fmadd rC33,rA3,rB0,rC33
            fmadd rC43,rA4,rB0,rC43
               ldr rB0, [pB]

            subs K, K, #2
            fmadd rC04,rA0,rB1,rC04
               ldr rA0, [pA]
            fmadd rC14,rA1,rB1,rC14
               ldr rA1, [pA,#8]
            fmadd rC24,rA2,rB1,rC24
               ldr rA2, [pA,#16]
            fmadd rC34,rA3,rB1,rC34
               ldr rA3, [pA,#24]
            fmadd rC44,rA4,rB1,rC44
               ldr rA4, [pA,#32]
         bne KLOOP
KDONE:
         stp rC00, rC10, [pC],#16
         stp rC20, rC30, [pC],#16
         stp rC40, rC01, [pC],#16
         stp rC11, rC21, [pC],#16
         stp rC31, rC41, [pC],#16

         stp rC02, rC12, [pC],#16
         stp rC22, rC32, [pC],#16
         stp rC42, rC03, [pC],#16
         stp rC13, rC23, [pC],#16
         stp rC33, rC43, [pC],#16

         stp rC04, rC14, [pC],#16
         stp rC24, rC34, [pC],#16
         str rC44,       [pC],#8


         subs NNU, NNU, #1
      bne NLOOP
      add pA0, pA0, incAm
      subs NMU, NMU, #1
   bne MLOOP
DONE:
        ldp     d8,d9,[sp,#(0*16)]
        ldp     d10,d11,[sp,#(1*16)]
        ldp     d12,d13,[sp,#(2*16)]
        ldp     d14,d15,[sp,#(3*16)]
        ldp     x18,x19,[sp,#(4*16)]
        ldp     x20,x21,[sp,#(5*16)]
        ldp     x22,x23,[sp,#(6*16)]
        ldp     x24,x25,[sp,#(7*16)]
        ldp     x26,x27,[sp,#(8*16)]
        ldp     x28,x29,[sp,#(9*16)]
        add     sp,sp,#(10*16)
        ret
