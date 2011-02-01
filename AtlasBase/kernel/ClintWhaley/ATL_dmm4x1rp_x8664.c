/*
 * Integer register usage shown be these defines
 */
#define pA      %rcx
#define ldab    %rax
#define ldab3   %rdx
#define pB      %rdi
#define pC      %rsi
#define incCn   %r10
#define MM      %r8
#define NN      %r11
#define pfA     %r9
#define KK      %rbx
#define IK      %rbp
#define IM      %r12
#define incAn   %r13


#define rA0     %xmm0
#define rA1     %xmm1
#define rA2     %xmm2
#define rA3     %xmm3
#define rB0     %xmm4
#define ra0     %xmm5
#define ra1     %xmm6
#define ra2     %xmm7
#define ra3     %xmm8
#define rb0     %xmm9
#define rC0     %xmm10
#define rC1     %xmm11
#define rC2     %xmm12
#define rC3     %xmm13
#define BETA    %xmm14

#define NB0so   0
#define NBso	(KB*8)
#define NB1so	(KB*8)
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
   #define prefC(mem) prefetchw	        mem
#else
   #define pref2(mem)
   #define prefB(mem)
   #define prefC(mem)
#endif
/*offset                rdi           rsi         rdx             xmm0   
 *void ATL_USERMM(const int M, const int N, const int K, const TYPE alpha,
 *offset                rcx            r8              r9                  8
 *                const TYPE *A, const int lda, const TYPE *B, const int ldb,
 *offset                xmm1   40       16             24
 *                const TYPE beta, TYPE *C, const int ldc)
 */
	.text
.global ATL_USERMM
ATL_USERMM:
/*
 *      Save callee-saved iregs
 */
        movq    %rbp, -8(%rsp)
        movq    %rbx, -16(%rsp)
        movq    %r12, -32(%rsp)
        movq    %r13, -40(%rsp)

#ifdef BETAX
        unpcklpd %xmm1, %xmm1
        movapd   %xmm1, BETA
#endif
/*
 *      pA already comes in right reg
 *      Initialize MM=M, NN=N, pB = B; pC = C, incAn = MB*KB*sizeof
 */
                        prefB((pB))
                        prefB(64(pB))
        movq    %rdi, MM
        movq    %rsi, NN
        movq    %rdx, KK
        movq    %r9, pB
        movq    16(%rsp), pC
                        prefC((pC))
                        prefC(64(pC))
/*
 *      Set incCn = (ldc - MB)*sizeof
 */
   movslq       24(%rsp), incCn
   subq         MM, incCn
   #ifdef DCPLX
	shl	$4, incCn
   #else
	shl	$3, incCn
   #endif
        addq    $120, pA
        addq    $120, pB
/*
 *      ldab = KB*sizeof; ldab3 = KB*3*sizeof
 */
        movq    KK, ldab
        shl     $3, ldab
        lea     (ldab,ldab,2), ldab3
#if MB == 0 || KB == 0
        movq    ldab, incAn
        imulq   MM, incAn
#else
        movq    $KB*MB*8, incAn
#endif
NLOOP:
        movq    MM, IM
MLOOP:
        movq    KK, IK
        subq    $4, IK
        movapd  -120(pB), rB0
        movapd  -120(pA), rA0
        movapd  -120(pA,ldab), rA1
        movapd  -120(pA,ldab,2), rA2
        movapd  -120(pA,ldab3), rA3
        movapd  16-120(pB), rb0
        movapd  16-120(pA), ra0
        movapd  16-120(pA,ldab), ra1
        movapd  16-120(pA,ldab,2), ra2
        movapd  16-120(pA,ldab3), ra3
/*        addq    $32, pA */
/*        addq    $32, pB */
#ifdef BETA0
        xorpd   rC0, rC0
        xorpd   rC1, rC1
        xorpd   rC2, rC2
        xorpd   rC3, rC3
#else
        movsd   (pC), rC0
        movsd   8(pC), rC1
        movsd   16(pC), rC2
        movsd   24(pC), rC3
   #ifndef BETA1
        mulpd   BETA, rC0
        mulpd   BETA, rC1
        mulpd   BETA, rC2
        mulpd   BETA, rC3
   #endif
#endif
/*KLOOP: */

#if KB > 4
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	32-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	32-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	32-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	32-120(pB),rB0
	addpd	rA3,rC3
			movapd	32-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	48-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	48-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	48-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	48-120(pB),rb0
	addpd	ra3,rC3
			movapd	48-120(pA,ldab3),ra3
#endif

#if KB > 8
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	64-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	64-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	64-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	64-120(pB),rB0
	addpd	rA3,rC3
			movapd	64-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	80-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	80-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	80-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	80-120(pB),rb0
	addpd	ra3,rC3
			movapd	80-120(pA,ldab3),ra3
#endif

#if KB > 12
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	96-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	96-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	96-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	96-120(pB),rB0
	addpd	rA3,rC3
			movapd	96-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	112-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	112-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	112-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	112-120(pB),rb0
	addpd	ra3,rC3
			movapd	112-120(pA,ldab3),ra3
#endif

#if KB > 16
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	128-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	128-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	128-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	128-120(pB),rB0
	addpd	rA3,rC3
			movapd	128-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	144-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	144-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	144-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	144-120(pB),rb0
	addpd	ra3,rC3
			movapd	144-120(pA,ldab3),ra3
#endif

#if KB > 20
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	160-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	160-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	160-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	160-120(pB),rB0
	addpd	rA3,rC3
			movapd	160-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	176-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	176-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	176-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	176-120(pB),rb0
	addpd	ra3,rC3
			movapd	176-120(pA,ldab3),ra3
#endif

#if KB > 24
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	192-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	192-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	192-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	192-120(pB),rB0
	addpd	rA3,rC3
			movapd	192-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	208-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	208-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	208-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	208-120(pB),rb0
	addpd	ra3,rC3
			movapd	208-120(pA,ldab3),ra3
#endif

#if KB > 28
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	224-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	224-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	224-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	224-120(pB),rB0
	addpd	rA3,rC3
			movapd	224-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	240-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	240-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	240-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	240-120(pB),rb0
	addpd	ra3,rC3
			movapd	240-120(pA,ldab3),ra3
#endif

#if KB > 32
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	256-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	256-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	256-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	256-120(pB),rB0
	addpd	rA3,rC3
			movapd	256-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	272-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	272-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	272-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	272-120(pB),rb0
	addpd	ra3,rC3
			movapd	272-120(pA,ldab3),ra3
#endif

#if KB > 36
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	288-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	288-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	288-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	288-120(pB),rB0
	addpd	rA3,rC3
			movapd	288-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	304-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	304-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	304-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	304-120(pB),rb0
	addpd	ra3,rC3
			movapd	304-120(pA,ldab3),ra3
#endif

#if KB > 40
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	320-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	320-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	320-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	320-120(pB),rB0
	addpd	rA3,rC3
			movapd	320-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	336-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	336-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	336-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	336-120(pB),rb0
	addpd	ra3,rC3
			movapd	336-120(pA,ldab3),ra3
#endif

#if KB > 44
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	352-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	352-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	352-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	352-120(pB),rB0
	addpd	rA3,rC3
			movapd	352-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	368-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	368-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	368-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	368-120(pB),rb0
	addpd	ra3,rC3
			movapd	368-120(pA,ldab3),ra3
#endif

#if KB > 48
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	384-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	384-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	384-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	384-120(pB),rB0
	addpd	rA3,rC3
			movapd	384-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	400-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	400-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	400-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	400-120(pB),rb0
	addpd	ra3,rC3
			movapd	400-120(pA,ldab3),ra3
#endif

#if KB > 52
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	416-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	416-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	416-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	416-120(pB),rB0
	addpd	rA3,rC3
			movapd	416-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	432-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	432-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	432-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	432-120(pB),rb0
	addpd	ra3,rC3
			movapd	432-120(pA,ldab3),ra3
#endif

#if KB > 56
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	448-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	448-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	448-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	448-120(pB),rB0
	addpd	rA3,rC3
			movapd	448-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	464-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	464-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	464-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	464-120(pB),rb0
	addpd	ra3,rC3
			movapd	464-120(pA,ldab3),ra3
#endif

#if KB > 60
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	480-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	480-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	480-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	480-120(pB),rB0
	addpd	rA3,rC3
			movapd	480-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	496-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	496-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	496-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	496-120(pB),rb0
	addpd	ra3,rC3
			movapd	496-120(pA,ldab3),ra3
#endif

#if KB > 64
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	512-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	512-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	512-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	512-120(pB),rB0
	addpd	rA3,rC3
			movapd	512-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	528-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	528-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	528-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	528-120(pB),rb0
	addpd	ra3,rC3
			movapd	528-120(pA,ldab3),ra3
#endif

#if KB > 68
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	544-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	544-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	544-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	544-120(pB),rB0
	addpd	rA3,rC3
			movapd	544-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	560-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	560-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	560-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	560-120(pB),rb0
	addpd	ra3,rC3
			movapd	560-120(pA,ldab3),ra3
#endif

#if KB > 72
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	576-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	576-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	576-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	576-120(pB),rB0
	addpd	rA3,rC3
			movapd	576-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	592-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	592-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	592-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	592-120(pB),rb0
	addpd	ra3,rC3
			movapd	592-120(pA,ldab3),ra3
#endif

#if KB > 76
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	608-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	608-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	608-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	608-120(pB),rB0
	addpd	rA3,rC3
			movapd	608-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	624-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	624-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	624-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	624-120(pB),rb0
	addpd	ra3,rC3
			movapd	624-120(pA,ldab3),ra3
#endif

#if KB > 80
	mulpd	rB0,rA0
	addpd	rA0,rC0
			movapd	640-120(pA),rA0
	mulpd	rB0,rA1
	addpd	rA1,rC1
			movapd	640-120(pA,ldab),rA1
	mulpd	rB0,rA2
	addpd	rA2,rC2
			movapd	640-120(pA,ldab,2),rA2
	mulpd	rB0,rA3
			movapd	640-120(pB),rB0
	addpd	rA3,rC3
			movapd	640-120(pA,ldab3),rA3

	mulpd	rb0,ra0
	addpd	ra0,rC0
			movapd	656-120(pA),ra0
	mulpd	rb0,ra1
	addpd	ra1,rC1
			movapd	656-120(pA,ldab),ra1
	mulpd	rb0,ra2
	addpd	ra2,rC2
			movapd	656-120(pA,ldab,2),ra2
	mulpd	rb0,ra3
			movapd	656-120(pB),rb0
	addpd	ra3,rC3
			movapd	656-120(pA,ldab3),ra3
#endif


/*        addq    $32, pA */
/*        addq    $32, pB */
/*        subq    $4, IK */
/*        jne     KLOOP */
/*
 *      Drain K-pipe
 */
        mulpd   rB0, rA0
        addpd   rA0, rC0
        mulpd   rB0, rA1
        addpd   rA1, rC1
        mulpd   rB0, rA2
        addpd   rA2, rC2
        mulpd   rB0, rA3
        addpd   rA3, rC3

        mulpd   rb0, ra0
        addpd   ra0, rC0
        mulpd   rb0, ra1
        addpd   ra1, rC1
        mulpd   rb0, ra2
        addpd   ra2, rC2
        mulpd   rb0, ra3
        addpd   ra3, rC3
/*
 *      Sum up vectors, and write results to C
 */
   #if 1
        haddpd  rC1, rC0
        haddpd  rC3, rC2
   #else
        movapd          rC0, rA0
        movapd          rC2, rB0
        unpcklpd        rC1, rC0
        unpcklpd        rC3, rC2
        unpckhpd        rC1, rA0
        unpckhpd        rC3, rB0
        addpd           rA0, rC0
        addpd           rB0, rC2
   #endif
        movupd  rC0, (pC)
        movupd  rC2, 16(pC)
        addq    $32, pC
        lea     (pA,ldab,4), pA
/*        subq    ldab, pB */
/*        addq    ldab3, pA */

        subq    $4, IM
        jne     MLOOP

        addq    ldab, pB
        subq    incAn, pA
        addq    incCn, pC
        subq    $1, NN
        jne     NLOOP
/*
 *      Restore regs and return
 */
        movq    -8(%rsp), %rbp
        movq    -16(%rsp), %rbx
        movq    -32(%rsp), %r12
        movq    -40(%rsp), %r13
        ret
