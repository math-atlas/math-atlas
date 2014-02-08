#include "atlas_asm.h"

#ifdef SREAL
   #define SZ 4
   #define movapd movaps
   #define vmulpd vmulps
   #define vsubpd vsubps
   #define vaddpd vaddps
   #define vfmadd231pd vfmadd231ps
   #define vbroadcastsd vbroadcastss
#else
   #define vmovapd vmovaps
   #define SZ 8
#endif

#define rB0     %zmm0
#define rB1     %zmm1
#define rA0     %zmm2
#define rA1     %zmm3
#define rA2     %zmm4
#define rA3     %zmm5
#define rA4     %zmm6
#define rA5     %zmm7
#define rC00    %zmm8
#define rC10    %zmm9
#define rC20    %zmm10
#define rC30    %zmm11
#define rC40    %zmm12
#define rC50    %zmm13
#define rC01    %zmm14
#define rC11    %zmm15
#define rC21    %zmm16
#define rC31    %zmm17
#define rC41    %zmm18
#define rC51    %zmm19
#define rC02    %zmm20
#define rC12    %zmm21
#define rC22    %zmm22
#define rC32    %zmm23
#define rC42    %zmm24
#define rC52    %zmm25
#define rC03    %zmm26
#define rC13    %zmm27
#define rC23    %zmm28
#define rC33    %zmm29
#define rC43    %zmm30
#define rC53    %zmm31

/*
 * Prioritize original registers for inner-loop operations, but inc regs
 * can be anything w/o changing opcode size, so use new regs for those
 */
#define KK      %rdx  /* API reg */
#define pA      %rcx  /* API reg */
#define pB      %rax  /* comes in as r9 */
#define r256    %r9   /* set after mov r9 to pC () */
/*
 * Then N-loop variables much less important, so use any orig regs left
 */
#define pA0     %r8   /* set after mov r8 to pB (rax) */
#define pC      %rsi  /* set after mov rsi to nnu () */
#define nnu     %r10  /* comes in as rsi */
#define pfA     %rbx
#define incB    %rbp
#define r192    %r12
#define KK0     %rdi
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
#define PFDISTB 256
#define PFDISTA0 768
#define PFDISTA 1920 // 1536
#define prefA(m_) vprefetch0 m_
#define prefB(m_) vprefetch0 m_
#define prefC(m_) vprefetche0 m_
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
/*
 * Save callee-saved iregs
 */
   movq %rbp, -8(%rsp)
      vprefetch0 (pA)
   movq %rbx, -16(%rsp)
      vprefetch0 64(pA)
   movq %r12, -24(%rsp)
      vprefetch0 128(pA)
   movq %r13, -32(%rsp)
      vprefetch0 192(pA)
   movq %r14, -40(%rsp)
      vprefetch0 256(pA)
   movq %r15, -48(%rsp)
      vprefetch0 320(pA)
/*
 * Load paramaters
 */
   mov %rdi, nmu
      vprefetch0 384(pA)
   mov %rsi, nnu
      vprefetch0 448(pA)
   mov %r8, pB
      vprefetch0 512(pA)
   mov %r9, pC
      vprefetch0 (pB)
   mov nnu, nnu0
      vprefetch0 576(pA)
   mov KK, incB
      vprefetch0 640(pA)
   sub $-128, pC
      vprefetch0 704(pA)
   sub $-128, pA
      vprefetch0 64(pB)
   mov $256, r256
      vprefetch0 768(pA)
   mov $192, r192
      vprefetch0 832(pA)
   mov pA, pA0
      vprefetch0 896(pA)
/*
 * incAm = MU*sizeof*K = 48*8*K = 128*3*K
 * incB = NU*sizeof*K = 4*8*K = 32*K
 */
   lea (KK,KK,2), incAm       /* incAm = 3*K */
      vprefetch0 960(pA)
   shl $7, incAm              /* incAm = 128*3*K */
   #ifdef SREAL
      shl $4, incB            /* incB = 16*K */
   #else
      shl $5, incB            /* incB = 32*K */
   #endif
   mov incB, KK
   add incB, pB
   neg KK
   mov pB, pB0
   mov KK, KK0

            vprefetche0 -128(pC)
            vprefetche0 -64(pC)
            vprefetche0 (pC)
            vprefetche0 64(pC)
            vprefetche0 128(pC)
            vprefetche0 192(pC)
            vprefetche0 256(pC)
            vprefetche0 320(pC)
            vprefetche0 384(pC)
            vprefetche0 448(pC)
            vprefetche0 512(pC)
            vprefetche0 576(pC)
            vprefetche0 640(pC)
            vprefetche0 704(pC)
            vprefetche0 768(pC)
            vprefetche0 832(pC)
            vprefetche0 896(pC)
            vprefetche0 960(pC)
            vprefetche0 1024(pC)
            vprefetche0 1088(pC)
            vprefetche0 1152(pC)
            vprefetche0 1216(pC)
   ALIGN16
   MLOOP:
      vbroadcastsd (pB,KK), rB0
      vbroadcastsd SZ(pB,KK), rB1
      NLOOP:
/*
 *       1st iteration peeled to zero rCxx
 */
         vmovapd -128(pA), rA0
         vmulpd rA0, rB0, rC00
         vmovapd -64(pA), rA1
         vmulpd rA1, rB0, rC10
         vmovapd (pA), rA2
         vmulpd rA2, rB0, rC20
         vmovapd 64(pA), rA3
         vmulpd rA3, rB0, rC30
         vmovapd 128(pA), rA4
         vmulpd rA4, rB0, rC40
         vmovapd 192(pA), rA5
         vmulpd rA5, rB0, rC50
            vbroadcastsd 2*SZ(pB,KK), rB0

         vmulpd rA0, rB1, rC01
            vprefetch0 64(pB,KK)
         vmulpd rA1, rB1, rC11
               vprefetch2 512(pB,KK)
         vmulpd rA2, rB1, rC21
               vprefetch0 1024(pA)
         vmulpd rA3, rB1, rC31
               vprefetch0 1084(pA)
         vmulpd rA4, rB1, rC41
               vprefetch0 1152(pA)
         vmulpd rA5, rB1, rC51
            vbroadcastsd 3*SZ(pB,KK), rB1

         vmulpd rA0, rB0, rC02
               vprefetch0 1216(pA)
         vmulpd rA1, rB0, rC12
               vprefetch0 1280(pA)
         vmulpd rA2, rB0, rC22
               vprefetch0 1344(pA)
         vmulpd rA3, rB0, rC32
               vprefetch0 1408(pA)
         vmulpd rA4, rB0, rC42
               vprefetch1 1472(pA)
         vmulpd rA5, rB0, rC52
            vbroadcastsd 4*SZ(pB,KK), rB0

         vmulpd rA0, rB1, rC03
            vmovapd 256(pA), rA0
         vmulpd rA1, rB1, rC13
            vmovapd 320(pA), rA1
         vmulpd rA2, rB1, rC23
            vmovapd 384(pA), rA2
         vmulpd rA3, rB1, rC33
            vmovapd 448(pA), rA3
         vmulpd rA4, rB1, rC43
            vmovapd 512(pA), rA4
         vmulpd rA5, rB1, rC53
            vmovapd 576(pA), rA5
/*
 *       2nd iteration
 */
         FMAC rA0, rB0, rC00
            vbroadcastsd 5*SZ(pB,KK), rB1
         FMAC rA1, rB0, rC10
               vprefetch1 1536(pA)
         FMAC rA2, rB0, rC20
               vprefetch1 1600(pA)
         FMAC rA3, rB0, rC30
               vprefetch1 1664(pA)
         FMAC rA4, rB0, rC40
               vprefetch2 1728(pA)
         FMAC rA5, rB0, rC50
            vbroadcastsd 6*SZ(pB,KK), rB0

         FMAC rA0, rB1, rC01
               vprefetch2 1792(pA)
         FMAC rA1, rB1, rC11
               vprefetch2 1856(pA)
         FMAC rA2, rB1, rC21
         FMAC rA3, rB1, rC31
         FMAC rA4, rB1, rC41
         FMAC rA5, rB1, rC51
            vbroadcastsd 7*SZ(pB,KK), rB1

         FMAC rA0, rB0, rC02
            add $1152, pA
         FMAC rA1, rB0, rC12
            add $8*SZ, KK
         FMAC rA2, rB0, rC22
         FMAC rA3, rB0, rC32
         FMAC rA4, rB0, rC42
         FMAC rA5, rB0, rC52
            vbroadcastsd (pB,KK), rB0

         FMAC rA0, rB1, rC03
            vmovapd 640-1152(pA), rA0
         FMAC rA1, rB1, rC13
            vmovapd 704-1152(pA), rA1
         FMAC rA2, rB1, rC23
            vmovapd 768-1152(pA), rA2
         FMAC rA3, rB1, rC33
            vmovapd 832-1152(pA), rA3
         FMAC rA4, rB1, rC43
            vmovapd 896-1152(pA), rA4
         FMAC rA5, rB1, rC53
            vmovapd 960-1152(pA), rA5
//            vprefetche0 1280(pC)
//            vprefetche0 1344(pC)
         jz KDONE

         KLOOP:
/*
 *          1st iteration
 */
            FMAC rA0, rB0, rC00
               vbroadcastsd SZ(pB,KK), rB1
            FMAC rA1, rB0, rC10
               vprefetch0 64(pB,KK)
            FMAC rA2, rB0, rC20
               vprefetch2 512(pB,KK)
            FMAC rA3, rB0, rC30
               vprefetch2 PFDISTA(pA)
            FMAC rA4, rB0, rC40
               vprefetch2 64+PFDISTA(pA)
            FMAC rA5, rB0, rC50
               vbroadcastsd 2*SZ(pB,KK), rB0

            FMAC rA0, rB1, rC01
               vprefetch2 128+PFDISTA(pA)
            FMAC rA1, rB1, rC11
               vprefetch2 192+PFDISTA(pA)
            FMAC rA2, rB1, rC21
               vprefetch2 256+PFDISTA(pA)
            FMAC rA3, rB1, rC31
               vprefetch2 320+PFDISTA(pA)
            FMAC rA4, rB1, rC41
               vprefetch2 384+PFDISTA(pA)
            FMAC rA5, rB1, rC51
               vbroadcastsd 3*SZ(pB,KK), rB1

            FMAC rA0, rB0, rC02
               vprefetch2 448+PFDISTA(pA)
            FMAC rA1, rB0, rC12
               vprefetch2 512+PFDISTA(pA)
            FMAC rA2, rB0, rC22
               vprefetch2 576+PFDISTA(pA)
            FMAC rA3, rB0, rC32
               vprefetch2 640+PFDISTA(pA)
            FMAC rA4, rB0, rC42
               vprefetch2 704+PFDISTA(pA)
            FMAC rA5, rB0, rC52
               vbroadcastsd 4*SZ(pB,KK), rB0

            FMAC rA0, rB1, rC03
               vmovapd -128(pA), rA0
            FMAC rA1, rB1, rC13
               vmovapd -64(pA), rA1
            FMAC rA2, rB1, rC23
               vmovapd (pA), rA2
            FMAC rA3, rB1, rC33
               vmovapd 64(pA), rA3
            FMAC rA4, rB1, rC43
               vmovapd 128(pA), rA4
            FMAC rA5, rB1, rC53
               vmovapd (pA,r192), rA5
/*
 *          2nd iteration
 */
            FMAC rA0, rB0, rC00
               vbroadcastsd 5*SZ(pB,KK), rB1
            FMAC rA1, rB0, rC10
               vprefetch0 PFDISTA0(pA)
            FMAC rA2, rB0, rC20
               vprefetch0 64+PFDISTA0(pA)
            FMAC rA3, rB0, rC30
               vprefetch0 128+PFDISTA0(pA)
            FMAC rA4, rB0, rC40
               vprefetch0 192+PFDISTA0(pA)
            FMAC rA5, rB0, rC50
               vbroadcastsd 6*SZ(pB,KK), rB0

            FMAC rA0, rB1, rC01
               vprefetch0 256+PFDISTA0(pA)
            FMAC rA1, rB1, rC11
               vprefetch0 320+PFDISTA0(pA)
            FMAC rA2, rB1, rC21
               vprefetch0 384+PFDISTA0(pA)
            FMAC rA3, rB1, rC31
               vprefetch0 448+PFDISTA0(pA)
            FMAC rA4, rB1, rC41
               vprefetch0 512+PFDISTA0(pA)
            FMAC rA5, rB1, rC51
               vbroadcastsd 7*SZ(pB,KK), rB1

            FMAC rA0, rB0, rC02
               vprefetch0 576+PFDISTA0(pA)
            FMAC rA1, rB0, rC12
               vprefetch0 640+PFDISTA0(pA)
            FMAC rA2, rB0, rC22
               lea (pA, r192,4), pA
            FMAC rA3, rB0, rC32
               add $64, KK
            FMAC rA4, rB0, rC42
               vprefetch0 704+PFDISTA0(pA)
            FMAC rA5, rB0, rC52
               vbroadcastsd (pB,KK), rB0

            FMAC rA0, rB1, rC03
               vmovapd 256-768(pA), rA0
            FMAC rA1, rB1, rC13
               vmovapd 320-768(pA), rA1
            FMAC rA2, rB1, rC23
               vmovapd 384-768(pA), rA2
            FMAC rA3, rB1, rC33
               vmovapd 448-768(pA), rA3
            FMAC rA4, rB1, rC43
               vmovapd 512-768(pA), rA4
            FMAC rA5, rB1, rC53
               vmovapd 576-768(pA), rA5

         jnz KLOOP
KDONE:
         #ifndef BETA0
            VCOP -128(pC), rC00, rC00
         mov KK0, KK
            VCOP -64(pC), rC10, rC10
         add incB, pB
            VCOP (pC), rC20, rC20
            VCOP 64(pC), rC30, rC30
            VCOP 128(pC), rC40, rC40
            VCOP 192(pC), rC50, rC50
            VCOP (pC,r256), rC01, rC01
            VCOP 320(pC), rC11, rC11
         #endif
         vmovapd rC00, -128(pC)
            vprefetche0 1536-128(pC)
         vmovapd rC10, -64(pC)
            vprefetche0 1536-64(pC)
         vmovapd rC20, (pC)
            vprefetche0 1536(pC)
         vmovapd rC30, 64(pC)
            vprefetche0 1536+64(pC)
         vmovapd rC40, 128(pC)
            vprefetche0 1536+128(pC)
         vmovapd rC50, (pC, r192)
            vprefetche0 1536+192(pC)
         vmovapd rC01, (pC,r256)
            vprefetche0 1536+256(pC)
         vmovapd rC11, 320(pC)
            vprefetche0 1536+320(pC)

         #ifndef BETA0
            VCOP 384(pC), rC21, rC21
            VCOP 448(pC), rC31, rC31
            VCOP (pC,r256,2), rC41, rC41
            VCOP 576(pC), rC51, rC51
            VCOP 640(pC), rC02, rC02
            VCOP 704(pC), rC12, rC12
            VCOP 768(pC), rC22, rC22
            VCOP 832(pC), rC32, rC32
         #endif
         vmovapd rC21, 384(pC)
            vprefetche0 1536+384(pC)
         vmovapd rC31, 448(pC)
            vprefetche0 1536+448(pC)
         vmovapd rC41, (pC,r256,2)
            vprefetche0 1536+512(pC)
         vmovapd rC51, 576(pC)
            vprefetche0 1536+576(pC)
         vmovapd rC02, 640(pC)
            vprefetche0 1536+640(pC)
         vmovapd rC12, 704(pC)
            vprefetche0 1536+704(pC)
         vmovapd rC22, 768(pC)
            vprefetche0 1536+768(pC)
         vmovapd rC32, 832(pC)
            vprefetche0 1536+832(pC)
         #ifndef BETA0
            VCOP 896(pC), rC42, rC42
            VCOP 960(pC), rC52, rC52
            VCOP (pC,r256,4), rC03, rC03
            VCOP 1088(pC), rC13, rC13
            VCOP 1152(pC), rC23, rC23
            VCOP 1216(pC), rC33, rC33
            VCOP 1280(pC), rC43, rC43
            VCOP 1344(pC), rC53, rC53
         #endif
         vmovapd rC42, 896(pC)
            vprefetche0 1536+896(pC)
         vmovapd rC52, 960(pC)
            vprefetche0 1536+960(pC)
         vmovapd rC03, (pC,r256,4)
            vprefetche0 1536+1024(pC)
         vmovapd rC13, 1088(pC)
            vprefetche0 1536+1088(pC)
         vmovapd rC23, 1152(pC)
            vprefetche0 1536+1152(pC)
         vmovapd rC33, 1216(pC)
            vprefetche0 1536+1216(pC)
         vmovapd rC43, 1280(pC)
            vprefetche0 1536+1280(pC)
         vmovapd rC53, 1344(pC)
            vprefetche0 1536+1344(pC)
         #ifndef BETA0
            add $1536, pC  /* pC += MU*NU*sizeof = 32*6*8 = 1536 */
            sub $1, nnu
         #else
            mov KK0, KK
            add incB, pB
            add $1536, pC  /* pC += MU*NU*sizeof = 32*6*8 = 1536 */
            sub $1, nnu
         #endif
         vbroadcastsd SZ(pB,KK), rB1
         mov pA0, pA
      jnz NLOOP
      mov nnu0, nnu
      add incAm, pA0
      mov pB0, pB
      mov pA0, pA
      sub $1, nmu
      mov KK0, KK
   jnz MLOOP
 DONE:
   movq -8(%rsp), %rbp
   movq -16(%rsp), %rbx
   movq -24(%rsp), %r12
   movq -32(%rsp), %r13
   movq -40(%rsp), %r14
   movq -48(%rsp), %r15
   ret

L1:
L2:
.text
.global findSize
findSize:
    mov     $L2-L1, %rax
    ret

