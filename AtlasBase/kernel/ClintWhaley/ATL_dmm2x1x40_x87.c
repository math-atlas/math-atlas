#include "atlas_asm.h"
#ifndef ATL_GAS_x8632
   #error "This kernel requires gas x86-32 assembler!"
#endif
#if !defined(KB) || (KB == 0)
   #error "KB must be a compile-time constant!"
#endif
#if !defined(NB)
   #define NB 0
#endif
#if !defined(MB)
   #define MB 0
#endif
#if (MB/2)*2 != MB
   #error "MB must be multiple of 2!"
#endif

#ifdef DCPLX
   #define OFF 16
   #define CMUL(i_) (2*(i_))
#else
   #define OFF 8
   #define CMUL(i_) i_
#endif
/*
 * Integer register usage shown be these defines
 */
#define pC      %esi
#define pA      %ecx
#define pB      %edi
#define incCn   %eax
#define stM	%bl
#define stN	%bh
#define ldab	%edx
/* #define pA3	%ebp */

#define pA0	pA
#define pB0	pB
#define pfA	incCn

#define NBso	(KB*8)
#define NB2so   (NBso+NBso)
#define NB3so   (NBso+NBso+NBso)
#define NB4so   (NBso+NBso+NBso+NBso)
#define NB5so   (NBso+NBso+NBso+NBso+NBso)
#define NB6so   (NBso+NBso+NBso+NBso+NBso+NBso)
#define NB7so   (NB6so+NBso)
#define NB8so   (NB6so+NB2so)
#define NB9so   (NB6so+NB3so)
#define NB10so   (NB6so+NB4so)
#define NB11so   (NB6so+NB5so)
#if MB != 0
   #define MBKBso  (MB*KB*8)
#endif

/*
 * Prefetch defines
 */
#if 1
   #define pref2(mem) prefetcht1	mem
   #define prefB(mem) prefetcht0	mem
   #define prefC(mem) prefetcht0	mem
#else
   #define pref2(mem)
   #define prefB(mem)
   #define prefC(mem)
#endif
/*offset                    4            8           12                16
 *void ATL_USERMM(const int M, const int N, const int K, const TYPE alpha,
 *offset                     24             28             32            36
 *                const TYPE *A, const int lda, const TYPE *B, const int ldb,
 *offset                       40       48             52
 *                const TYPE beta, TYPE *C, const int ldc)
 */
	.text
.global ATL_asmdecor(ATL_USERMM)
ATL_asmdecor(ATL_USERMM):
/*
 *      Save callee-saved iregs; Save old stack pointer in eax,
 *      so we can adjust for BETA alignment
 */
#define FSIZE 28
#define BETAOFF FSIZE+40
#define COFF 16
	subl	$FSIZE, %esp
	movl	%ebp, 12(%esp)
	movl	%ebx,  8(%esp)
	movl	%esi,  4(%esp)
	movl	%edi,   (%esp)
/*
 *      Initialize pA = A;  pB = B; pC = C;
 */
#if MB == 0
        movl    FSIZE+4(%esp), %ebx
        movl    %ebx, COFF+4(%esp)
        imul    $NBso, %ebx
        subl    $NB2so, %ebx
        movl    %ebx, COFF+8(%esp)
#endif
	movl	FSIZE+24(%esp), pA
	movl	FSIZE+32(%esp), pB
	movl	FSIZE+48(%esp), pC
#if NB == 0
        movb    FSIZE+8(%esp), stN
#else
        movb    $NB, stN
#endif
/*
 *      Set incCn = (ldc - MB)*sizeof
 */
	movl	FSIZE+52(%esp), incCn
   #if MB == 0
        subl    COFF+4(%esp), incCn
        addl    $2, incCn
   #else
	subl	$MB-2, incCn
   #endif
   #ifdef DCPLX
	shl	$4, incCn
   #else
	shl	$3, incCn
   #endif
   	movl	incCn, COFF(%esp)
        movl    pA0, pfA
#if MB == 0
        addl    $NB2so, pfA
        addl    COFF+8(%esp), pfA
#else
        addl    $MBKBso, pfA
#endif
NLOOP:
#if MB == 0
        movb    COFF+4(%esp), stM
        subb     $2, stM
        jz      MLOOPCU
#else
        movb     $MB-2, stM
#endif
#if MB != 6
MLOOP:
	fldl (pB)
	fldl (pA)
	fmul %st(1),%st
	fldl 320(pA)
	fmulp %st,%st(2)
	fldl 8(pB)
	fldl 8(pA)
	fmul %st(1),%st
	fldl 328(pA)
	fmulp %st,%st(2)
	fldl 16(pB)
	fldl 16(pA)
	fmul %st(1),%st
   #if defined(BETA0) || defined (BETAX)
	fldz
   #else
	fldl (pC)
   #endif
	faddp %st,%st(5)
	fldl 336(pA)
	fmulp %st,%st(2)
	fldl 24(pB)
   #if defined(BETA0) || defined (BETAX)
	fldz
   #else
	fldl OFF(pC)
   #endif
	faddp %st,%st(7)
	fldl 24(pA)
	fmul %st(1),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 344(pA)
	fmulp %st,%st(1)
	fldl 32(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 32(pA)
	fmul %st(7),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 352(pA)
	fmulp %st,%st(7)
	fldl 40(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 40(pA)
	fmul %st(5),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 360(pA)
	fmulp %st,%st(5)
	fldl 48(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 48(pA)
	fmul %st(3),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 368(pA)
	fmulp %st,%st(3)
	fldl 56(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 56(pA)
	fmul %st(1),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 376(pA)
	fmulp %st,%st(1)
	fldl 64(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 64(pA)
	fmul %st(7),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 384(pA)
	fmulp %st,%st(7)
                                        pref2((pfA))
                                        addl    $16, pfA
	fldl 72(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 72(pA)
	fmul %st(5),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 392(pA)
	fmulp %st,%st(5)
	fldl 80(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 80(pA)
	fmul %st(3),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 400(pA)
	fmulp %st,%st(3)
	fldl 88(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 88(pA)
	fmul %st(1),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 408(pA)
	fmulp %st,%st(1)
	fldl 96(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 96(pA)
	fmul %st(7),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 416(pA)
	fmulp %st,%st(7)
	fldl 104(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 104(pA)
	fmul %st(5),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 424(pA)
	fmulp %st,%st(5)
	fldl 112(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 112(pA)
	fmul %st(3),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 432(pA)
	fmulp %st,%st(3)
	fldl 120(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 120(pA)
	fmul %st(1),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 440(pA)
	fmulp %st,%st(1)
	fldl 128(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 128(pA)
	fmul %st(7),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 448(pA)
	fmulp %st,%st(7)
	fldl 136(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 136(pA)
	fmul %st(5),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 456(pA)
	fmulp %st,%st(5)
	fldl 144(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 144(pA)
	fmul %st(3),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 464(pA)
	fmulp %st,%st(3)
	fldl 152(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 152(pA)
	fmul %st(1),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 472(pA)
	fmulp %st,%st(1)
	fldl 160(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 160(pA)
	fmul %st(7),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 480(pA)
	fmulp %st,%st(7)
	fldl 168(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 168(pA)
	fmul %st(5),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 488(pA)
	fmulp %st,%st(5)
	fldl 176(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 176(pA)
	fmul %st(3),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 496(pA)
	fmulp %st,%st(3)
	fldl 184(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 184(pA)
	fmul %st(1),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 504(pA)
	fmulp %st,%st(1)
	fldl 192(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 192(pA)
	fmul %st(7),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 512(pA)
	fmulp %st,%st(7)
	fldl 200(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 200(pA)
	fmul %st(5),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 520(pA)
	fmulp %st,%st(5)
	fldl 208(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 208(pA)
	fmul %st(3),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 528(pA)
	fmulp %st,%st(3)
	fldl 216(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 216(pA)
	fmul %st(1),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 536(pA)
	fmulp %st,%st(1)
	fldl 224(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 224(pA)
	fmul %st(7),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 544(pA)
	fmulp %st,%st(7)
	fldl 232(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 232(pA)
	fmul %st(5),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 552(pA)
	fmulp %st,%st(5)
	fldl 240(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 240(pA)
	fmul %st(3),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 560(pA)
	fmulp %st,%st(3)
	fldl 248(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 248(pA)
	fmul %st(1),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 568(pA)
	fmulp %st,%st(1)
	fldl 256(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 256(pA)
	fmul %st(7),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 576(pA)
	fmulp %st,%st(7)
	fldl 264(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 264(pA)
	fmul %st(5),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 584(pA)
	fmulp %st,%st(5)
	fldl 272(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 272(pA)
	fmul %st(3),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 592(pA)
	fmulp %st,%st(3)
	fldl 280(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 280(pA)
	fmul %st(1),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 600(pA)
	fmulp %st,%st(1)
	fldl 288(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 288(pA)
	fmul %st(7),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 608(pA)
	fmulp %st,%st(7)
	fldl 296(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 296(pA)
	fmul %st(5),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 616(pA)
	fmulp %st,%st(5)
	fldl 304(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 304(pA)
	fmul %st(3),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 624(pA)
	fmulp %st,%st(3)
	fldl 312(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 312(pA)
	fmul %st(1),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 632(pA)
	fmulp %st,%st(1)
	fxch %st(6)
	faddp %st,%st(4)
	faddp %st,%st(2)
	faddp %st,%st(2)
	faddp %st,%st(2)
	faddp %st,%st(2)
/*
 *      While (pB != stK);
 */
/*	cmp	pB, stK */
/*	jne	KLOOP */
/*
 *      Write results back to C
 */
   #ifdef BETAX
        fldl    (pC)
        fldl    OFF(pC)
        fldl    BETAOFF(%esp)
        fmul    %st, %st(1)
        fmulp   %st, %st(2)
        faddp   %st, %st(3)
        faddp   %st, %st(1)
   #endif
	fstpl (pC)
	fstpl OFF(pC)

/*       pC += 2;  pA += 2*NB */
/* */
	addl	$CMUL(16), pC
	addl	$NB2so, pA
/*
 *      while (pA != stM);
 */
	subb	$2, stM
	jnz	MLOOP
#endif
/*
 *      Last iteration of MLOOP unrolled for prefetch of next col of B
 */
#if MB == 0
MLOOPCU:
#endif
	fldl (pB)
	fldl (pA)
	fmul %st(1),%st
	fldl 320(pA)
	fmulp %st,%st(2)
	fldl 8(pB)
	fldl 8(pA)
	fmul %st(1),%st
	fldl 328(pA)
	fmulp %st,%st(2)
	fldl 16(pB)
	fldl 16(pA)
	fmul %st(1),%st
   #if defined(BETA0) || defined (BETAX)
	fldz
   #else
	fldl (pC)
   #endif
	faddp %st,%st(5)
	fldl 336(pA)
	fmulp %st,%st(2)
	fldl 24(pB)
   #if defined(BETA0) || defined (BETAX)
	fldz
   #else
	fldl OFF(pC)
   #endif
	faddp %st,%st(7)
	fldl 24(pA)
	fmul %st(1),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 344(pA)
	fmulp %st,%st(1)
	fldl 32(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 32(pA)
	fmul %st(7),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 352(pA)
	fmulp %st,%st(7)
	fldl 40(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 40(pA)
	fmul %st(5),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 360(pA)
	fmulp %st,%st(5)
	fldl 48(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 48(pA)
	fmul %st(3),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 368(pA)
	fmulp %st,%st(3)
	fldl 56(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 56(pA)
	fmul %st(1),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 376(pA)
	fmulp %st,%st(1)
	fldl 64(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 64(pA)
	fmul %st(7),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 384(pA)
	fmulp %st,%st(7)
                                        prefB(NBso(pB0))
                                        prefB(32+NBso(pB0))
	fldl 72(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 72(pA)
	fmul %st(5),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 392(pA)
	fmulp %st,%st(5)
	fldl 80(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 80(pA)
	fmul %st(3),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 400(pA)
	fmulp %st,%st(3)
	fldl 88(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 88(pA)
	fmul %st(1),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 408(pA)
	fmulp %st,%st(1)
                                        prefB(64+NBso(pB0))
                                        prefB(96+NBso(pB0))
	fldl 96(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 96(pA)
	fmul %st(7),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 416(pA)
	fmulp %st,%st(7)
	fldl 104(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 104(pA)
	fmul %st(5),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 424(pA)
	fmulp %st,%st(5)
	fldl 112(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 112(pA)
	fmul %st(3),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 432(pA)
	fmulp %st,%st(3)
	fldl 120(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 120(pA)
	fmul %st(1),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 440(pA)
	fmulp %st,%st(1)
                                        prefB(128+NBso(pB0))
                                        prefB(160+NBso(pB0))
	fldl 128(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 128(pA)
	fmul %st(7),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 448(pA)
	fmulp %st,%st(7)
	fldl 136(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 136(pA)
	fmul %st(5),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 456(pA)
	fmulp %st,%st(5)
	fldl 144(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 144(pA)
	fmul %st(3),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 464(pA)
	fmulp %st,%st(3)
                                        prefB(192+NBso(pB0))
                                        prefB(256+NBso(pB0))
	fldl 152(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 152(pA)
	fmul %st(1),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 472(pA)
	fmulp %st,%st(1)
	fldl 160(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 160(pA)
	fmul %st(7),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 480(pA)
	fmulp %st,%st(7)
	fldl 168(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 168(pA)
	fmul %st(5),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 488(pA)
	fmulp %st,%st(5)
                                        prefB(288+NBso(pB0))
	fldl 176(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 176(pA)
	fmul %st(3),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 496(pA)
	fmulp %st,%st(3)
	fldl 184(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 184(pA)
	fmul %st(1),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 504(pA)
	fmulp %st,%st(1)
	fldl 192(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 192(pA)
	fmul %st(7),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 512(pA)
	fmulp %st,%st(7)
	fldl 200(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 200(pA)
	fmul %st(5),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 520(pA)
	fmulp %st,%st(5)
	fldl 208(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 208(pA)
	fmul %st(3),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 528(pA)
	fmulp %st,%st(3)
	fldl 216(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 216(pA)
	fmul %st(1),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 536(pA)
	fmulp %st,%st(1)
	fldl 224(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 224(pA)
	fmul %st(7),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 544(pA)
	fmulp %st,%st(7)
	fldl 232(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 232(pA)
	fmul %st(5),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 552(pA)
	fmulp %st,%st(5)
	fldl 240(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 240(pA)
	fmul %st(3),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 560(pA)
	fmulp %st,%st(3)
	fldl 248(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 248(pA)
	fmul %st(1),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 568(pA)
	fmulp %st,%st(1)
	fldl 256(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 256(pA)
	fmul %st(7),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 576(pA)
	fmulp %st,%st(7)
	fldl 264(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 264(pA)
	fmul %st(5),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 584(pA)
	fmulp %st,%st(5)
	fldl 272(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 272(pA)
	fmul %st(3),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 592(pA)
	fmulp %st,%st(3)
	fldl 280(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 280(pA)
	fmul %st(1),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 600(pA)
	fmulp %st,%st(1)
	fldl 288(pB)
	fxch %st(7)
	faddp %st,%st(5)
	fldl 288(pA)
	fmul %st(7),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 608(pA)
	fmulp %st,%st(7)
	fldl 296(pB)
	fxch %st(5)
	faddp %st,%st(3)
	fldl 296(pA)
	fmul %st(5),%st
	fxch %st(4)
	faddp %st,%st(2)
	fldl 616(pA)
	fmulp %st,%st(5)
	fldl 304(pB)
	fxch %st(3)
	faddp %st,%st(1)
	fldl 304(pA)
	fmul %st(3),%st
	fxch %st(2)
	faddp %st,%st(6)
	fldl 624(pA)
	fmulp %st,%st(3)
	fldl 312(pB)
	fxch %st(1)
	faddp %st,%st(7)
	fldl 312(pA)
	fmul %st(1),%st
	fxch %st(6)
	faddp %st,%st(4)
	fldl 632(pA)
	fmulp %st,%st(1)
	fxch %st(6)
	faddp %st,%st(4)
	faddp %st,%st(2)
	faddp %st,%st(2)
	faddp %st,%st(2)
	faddp %st,%st(2)
/*
 *      While (pB != stK);
 */
/*	cmp	pB, stK */
/*	jne	KLOOP */
/*
 *      Write results back to C
 */
   #ifdef BETAX
        fldl    (pC)
        fldl    OFF(pC)
        fldl    BETAOFF(%esp)
        fmul    %st, %st(1)
        fmulp   %st, %st(2)
        faddp   %st, %st(3)
        faddp   %st, %st(1)
   #endif
	fstpl (pC)
	fstpl OFF(pC)

/*       pC += 2;  pA += 2*NB */
/* */
/*	addl	$CMUL(16), pC */
/*	addl	$NB2so, pA */
/*
 *      while (pA != stM);
 */
/*	pC += incCn;  pA -= NBNB;  pB += NB; */
 
   	addl	COFF(%esp), pC
   #if MB == 0
        subl    COFF+8(%esp), pA
   #else
        subl    $MBKBso-NB2so, pA
   #endif
        addl    $NBso, pB
/*
 *      while (pB != stN);
 */
	sub	$1, stN
	jnz	NLOOP

/*
 *      Restore callee-saved iregs
 */
	movl	12(%esp), %ebp
	movl	 8(%esp), %ebx
	movl	 4(%esp), %esi
	movl	  (%esp), %edi
	addl	$FSIZE, %esp
	ret
