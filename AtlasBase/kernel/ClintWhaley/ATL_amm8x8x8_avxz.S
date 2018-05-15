#include "atlas_asm.h"

#if defined(SREAL) || defined(SCPLX)
   #define SZ 4
   #define movapd movaps
   #define vmulpd vmulps
   #define vsubpd vsubps
   #define vaddpd vaddps
   #define vfmadd231pd vfmadd231ps
   #define vbroadcastsd vbroadcastss
   #define vmovapd vmovaps
   #ifdef ATL_ARCH_XeonPHI
      #define MOVC vmovnraps
   #else
      #define MOVC vmovaps
   #endif
   #define vbroadcastf64x4 vbroadcastf32x4
#else
   #ifdef ATL_ARCH_XeonPHI
      #define MOVC vmovnrapd
   #else
      #define MOVC vmovapd
   #endif
   #define SZ 8
#endif

#define rA0     %zmm0
#define rA1     %zmm1
#define rA2     %zmm2
#define rA3     %zmm3
#define rA4     %zmm4
#define rA5     %zmm5
#define rA6     %zmm6
#define rA7     %zmm7
#define rB0     %zmm8
#define rB1     %zmm9
#define rB2     %zmm10
#define rB3     %zmm11
#define rB4     %zmm12
#define rB5     %zmm13
#define rB6     %zmm14
#define rB7     %zmm15
#define rB8     %zmm16
#define rB9     %zmm17
#define rB10    %zmm18
#define rB11    %zmm19
#define rB12    %zmm20
#define rB13    %zmm21
#define rB14    %zmm22
#define rB15    %zmm23
#define rC00    %zmm24
#define rC01    %zmm25
#define rC02    %zmm26
#define rC03    %zmm27
#define rC04    %zmm28
#define rC05    %zmm29
#define rC06    %zmm30
#define rC07    %zmm31

#define nmu %rdi
#define nnu %rsi
#define pA  %rcx
#define pB  %r8
#define pC  %r9
#define nnu0 %rdx
#define pB0 %rax
#define r512 %r10
#define incBn %r11
/*
                    rdi      rsi    rdx        rcx         r8        r9
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                  8(%rsp)    16(%rsp)     24(%rsp)
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn);
 */
#define FMAC vfmadd231pd   /* FMAC m256/r256, rs1, rd */
#if defined(BETAN) || defined(BETAn)
   #define BETAN1
#endif
#ifdef BETAN1
   #define VCOP vsubpd
#else
   #define VCOP vaddpd
#endif
.text
ALIGN16
.globl ATL_asmdecor(ATL_USERMM)
ATL_asmdecor(ATL_USERMM):
      vprefetch0 (pA)
   mov nnu, nnu0
      vprefetch0 64(pA)
   mov $512, r512
      vprefetch0 2*64(pA)
   mov pB, pB0
      vprefetch0 3*64(pA)
   mov $8*8*SZ, incBn  /* incBn = NU*KU*sizeof = 256/512 */
      vprefetch0 (pB)

   ALIGN16
   MLOOP:
/*
 *    Entire KB*NU elts of A are preloaded to registers
 */
      vmovapd (pA), rA0
         vprefetch0 64(pB)
      vmovapd 64(pA), rA1
         vprefetch0 2*64(pB)
      vmovapd 2*64(pA), rA2
         vprefetch0 3*64(pB)
      vmovapd 3*64(pA), rA3
         vprefetch0 4*64(pB)
      vmovapd 4*64(pA), rA4
      vmovapd 5*64(pA), rA5
      vmovapd 6*64(pA), rA6
      vmovapd 7*64(pA), rA7

      NLOOP:
/*
 *       1st K iteration zeros rC0x
 */
            vbroadcastf64x4 (pB), rB0  
            vbroadcastf64x4 4*SZ(pB), rB1
            vbroadcastf64x4 2*4*SZ(pB), rB2
            vbroadcastf64x4 3*4*SZ(pB), rB3
         vmulpd rB0{aaaa}, rA0, rC00
            vprefetche0 (pC)
         vmulpd rB0{bbbb}, rA0, rC01
            vprefetche0 64(pC)
         vmulpd rB0{cccc}, rA0, rC02
            vprefetche0 2*64(pC)
         vmulpd rB0{dddd}, rA0, rC03
            vprefetche0 3*64(pC)
         vmulpd rB1{aaaa}, rA0, rC04
            vprefetche0 4*64(pC)
         vmulpd rB1{bbbb}, rA0, rC05
            vprefetche0 5*64(pC)
         vmulpd rB1{cccc}, rA0, rC06
            vprefetche0 6*64(pC)
         vmulpd rB1{dddd}, rA0, rC07
            vprefetche0 7*64(pC)
/*
 *       2nd K iteration
 */
         FMAC rB2{aaaa}, rA1, rC00
            vbroadcastf64x4 4*4*SZ(pB), rB4
         FMAC rB2{bbbb}, rA1, rC01
            vbroadcastf64x4 5*4*SZ(pB), rB5
         FMAC rB2{cccc}, rA1, rC02
            vbroadcastf64x4 6*4*SZ(pB), rB6
         FMAC rB2{dddd}, rA1, rC03
            vbroadcastf64x4 7*4*SZ(pB), rB7
         FMAC rB3{aaaa}, rA1, rC04
            vbroadcastf64x4 8*4*SZ(pB), rB8
         FMAC rB3{bbbb}, rA1, rC05
            vbroadcastf64x4 9*4*SZ(pB), rB9
         FMAC rB3{cccc}, rA1, rC06
            vbroadcastf64x4 10*4*SZ(pB), rB10
         FMAC rB3{dddd}, rA1, rC07
            vbroadcastf64x4 11*4*SZ(pB), rB11
/*
 *       3rd K iteration
 */
         FMAC rB4{aaaa}, rA2, rC00
            vbroadcastf64x4 12*4*SZ(pB), rB12
         FMAC rB4{bbbb}, rA2, rC01
            vbroadcastf64x4 13*4*SZ(pB), rB13
         FMAC rB4{cccc}, rA2, rC02
            vbroadcastf64x4 14*4*SZ(pB), rB14
         FMAC rB4{dddd}, rA2, rC03
            vbroadcastf64x4 15*4*SZ(pB), rB15
         FMAC rB5{aaaa}, rA2, rC04
            add incBn, pB
         FMAC rB5{bbbb}, rA2, rC05
            add r512, pC   /* pC += MU*NU*sizeof = 8*8*8 = 512 */
         FMAC rB5{cccc}, rA2, rC06
         FMAC rB5{dddd}, rA2, rC07
/*
 *       4th K iteration
 */
         FMAC rB6{aaaa}, rA3, rC00
         FMAC rB6{bbbb}, rA3, rC01
         FMAC rB6{cccc}, rA3, rC02
         FMAC rB6{dddd}, rA3, rC03
         FMAC rB7{aaaa}, rA3, rC04
         FMAC rB7{bbbb}, rA3, rC05
         FMAC rB7{cccc}, rA3, rC06
         FMAC rB7{dddd}, rA3, rC07
/*
 *       5th K iteration
 */
         FMAC rB8{aaaa}, rA4, rC00
         FMAC rB8{bbbb}, rA4, rC01
         FMAC rB8{cccc}, rA4, rC02
         FMAC rB8{dddd}, rA4, rC03
         FMAC rB9{aaaa}, rA4, rC04
         FMAC rB9{bbbb}, rA4, rC05
         FMAC rB9{cccc}, rA4, rC06
         FMAC rB9{dddd}, rA4, rC07
/*
 *       6th K iteration
 */
         FMAC rB10{aaaa}, rA5, rC00
         FMAC rB10{bbbb}, rA5, rC01
         FMAC rB10{cccc}, rA5, rC02
         FMAC rB10{dddd}, rA5, rC03
         FMAC rB11{aaaa}, rA5, rC04
         FMAC rB11{bbbb}, rA5, rC05
         FMAC rB11{cccc}, rA5, rC06
         FMAC rB11{dddd}, rA5, rC07
/*
 *       7th K iteration
 */
         FMAC rB12{aaaa}, rA6, rC00
         FMAC rB12{bbbb}, rA6, rC01
         FMAC rB12{cccc}, rA6, rC02
         FMAC rB12{dddd}, rA6, rC03
         FMAC rB13{aaaa}, rA6, rC04
         FMAC rB13{bbbb}, rA6, rC05
         FMAC rB13{cccc}, rA6, rC06
         FMAC rB13{dddd}, rA6, rC07
/*
 *       8th K iteration
 */
         FMAC rB14{aaaa}, rA7, rC00
         FMAC rB14{bbbb}, rA7, rC01
         FMAC rB14{cccc}, rA7, rC02
         FMAC rB14{dddd}, rA7, rC03
         FMAC rB15{aaaa}, rA7, rC04
         FMAC rB15{bbbb}, rA7, rC05
         FMAC rB15{cccc}, rA7, rC06
         FMAC rB15{dddd}, rA7, rC07

         #ifndef BETA0
            VCOP -512(pC), rC00, rC00
            VCOP 64-512(pC), rC01, rC01
            VCOP 2*64-512(pC), rC02, rC02
            VCOP 3*64-512(pC), rC03, rC03
            VCOP 4*64-512(pC), rC04, rC04
            VCOP 5*64-512(pC), rC05, rC05
            VCOP 6*64-512(pC), rC06, rC06
            VCOP 7*64-512(pC), rC07, rC07
         #endif
         MOVC rC00, -512(pC)
         MOVC rC01, 64-512(pC)
         MOVC rC02, 2*64-512(pC)
         MOVC rC03, 3*64-512(pC)
         MOVC rC04, 4*64-512(pC)
         MOVC rC05, 5*64-512(pC)
            sub $1, nnu
         MOVC rC06, 6*64-512(pC)
         MOVC rC07, 7*64-512(pC)
      jnz NLOOP
      add r512, pA
      sub $1, nmu
      mov nnu0, nnu
      mov pB0, pB
   jnz MLOOP
 DONE:
   ret
