#if !defined(MB) || (MB == 0)
   #error "MB must be a compile-time constant!"
#endif
#if !defined(NB) || (NB == 0)
   #error "NB must be a compile-time constant!"
#endif
#if !defined(KB) || (KB == 0)
   #error "KB must be a compile-time constant!"
#endif
#if (MB/14)*14 != NB
   #error "MB must be multiple of 14!"
#endif
#if (KB/2)*2 != KB
   #define KBR ((KB/2)*16)
   #define movapd movupd
#endif
/*
 * Integer register usage shown be these defines
 */
#define pC      %rsi
#define pA      %rcx
#define pB      %rdi
#define incCn   %rax
#define stM     %rdx
#define stN     %rbp
#define pfA	%r8
/*       rax     used in 32/64 conversion */

#define NBso	(KB*8)
#define NBNBso  (KB*KB*8)
#define NB2so   (NBso+NBso)
#define NB3so   (NBso+NBso+NBso)
#define NB4so   (NBso+NBso+NBso+NBso)
#define NB5so   (NBso+NBso+NBso+NBso+NBso)
#define NB6so   (NB3so+NB3so)
#define NB7so   (NB3so+NB4so)
#define NB8so   (NB4so+NB4so)
#define NB9so   (NB4so+NB5so)
#define NB10so   (NB5so+NB5so)
#define NB11so   (NB6so+NB5so)
#define NB12so   (NB7so+NB5so)
#define NB13so   (NB8so+NB5so)
#define NB14so   (NB9so+NB5so)

/*
 * SSE2 register usage shown be these defines
 */
#define rA0	%xmm0
#define rB0     %xmm1
#define rC0	%xmm2
#define rC1	%xmm3
#define rC2	%xmm4
#define rC3	%xmm5
#define rC4	%xmm6
#define rC5	%xmm7
#define rC6	%xmm8
#define rC7	%xmm9
#define rC8	%xmm10
#define rC9	%xmm11
#define rC10	%xmm12
#define rC11	%xmm13
#define rC12	%xmm14
#define rC13	%xmm15
/*
 * Prefetch defines
 */
#define pref2(mem) prefetcht1   mem
#define prefB(mem) prefetcht0   mem
#define prefC(mem) prefetchw    mem

	.text
.global ATL_asmdecor(ATL_USERMM)
ATL_asmdecor(ATL_USERMM):
/*
 *      Save callee-saved iregs
 */
	movq	%rbp, -8(%rsp)
/*	movq	%rbx, -16(%rsp) */
#ifdef BETAX
	movsd	%xmm1, -24(%rsp)
#endif
/*
 *      pA already comes in right reg
 *      Initialize pB = B; pC = C; NBso = NB * sizeof;
 */
	movq	%r9, pB
	movq	16(%rsp), pC
/*
 *      stM = pA + MBKBso;  stN = pB + NBKBso;
 */
	movq	pA, stM
	addq	$(MB*NBso-NB14so), stM
	movq	stM, pfA
	addq	$NB14so, pfA
	movq	$(NB*NBso), stN
	addq	pB, stN
/*
 *      convert ldc to 64 bits, and then set incCn = (ldc - NB)*sizeof
 */
	movl	24(%rsp), %eax
	cltq
	subq	$(NB-14), incCn
#ifdef DREAL
	shl	$3, incCn
#else
	shl	$4, incCn
#endif
UNLOOP:
/*
 *      stK = pB + NBso
 */
/*	movq	pB, stK */
/*	addq	$NBso, stK */
UMLOOP:
/*
 *      rC[0-13] = pC[0-13] * beta
 */
#ifdef BETA0
	xorpd	rC0, rC0
	xorpd	rC1, rC1
	xorpd	rC2, rC2
	xorpd	rC3, rC3
	xorpd	rC4, rC4
	xorpd	rC5, rC5
	xorpd	rC6, rC6
	xorpd	rC7, rC7
	xorpd	rC8, rC8
	xorpd	rC9, rC9
	xorpd	rC10, rC10
	xorpd	rC11, rC11
	xorpd	rC12, rC12
	xorpd	rC13, rC13
#else
   #ifdef DREAL
	movsd	(pC), rC0
	movsd	8(pC), rC1
	movsd	16(pC), rC2
	movsd	24(pC), rC3
	movsd	32(pC), rC4
	movsd	40(pC), rC5
	movsd	48(pC), rC6
	movsd	56(pC), rC7
	movsd	64(pC), rC8
	movsd	72(pC), rC9
	movsd	80(pC), rC10
	movsd	88(pC), rC11
	movsd	96(pC), rC12
	movsd	104(pC), rC13
   #else
	movsd	(pC), rC0
	movsd	16(pC), rC1
	movsd	32(pC), rC2
	movsd	48(pC), rC3
	movsd	64(pC), rC4
	movsd	80(pC), rC5
	movsd	96(pC), rC6
	movsd	112(pC), rC7
	movsd	128(pC), rC8
	movsd	144(pC), rC9
	movsd	160(pC), rC10
	movsd	176(pC), rC11
	movsd	192(pC), rC12
	movsd	208(pC), rC13
   #endif
   #ifdef BETAX
	mulsd	-24(%rsp), rC0
	mulsd	-24(%rsp), rC1
	mulsd	-24(%rsp), rC2
	mulsd	-24(%rsp), rC3
	mulsd	-24(%rsp), rC4
	mulsd	-24(%rsp), rC5
	mulsd	-24(%rsp), rC6
	mulsd	-24(%rsp), rC7
	mulsd	-24(%rsp), rC8
	mulsd	-24(%rsp), rC9
	mulsd	-24(%rsp), rC10
	mulsd	-24(%rsp), rC11
	mulsd	-24(%rsp), rC12
	mulsd	-24(%rsp), rC13
   #endif
#endif
/*KLOOP: */
#if KB > 1
	movapd	(pB), rB0
	movapd	(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 3
	movapd	16(pB), rB0
	movapd	16(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	16+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	16+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	16+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	16+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	16+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	16+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	16+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	16+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	16+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	16+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	16+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	16+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	16+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 5
	movapd	32(pB), rB0
	movapd	32(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	32+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	32+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	32+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	32+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	32+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	32+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	32+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	32+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	32+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	32+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	32+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	32+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	32+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 7
	movapd	48(pB), rB0
	movapd	48(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	48+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	48+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	48+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	48+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	48+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	48+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	48+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	48+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	48+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	48+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	48+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	48+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	48+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 9
	movapd	64(pB), rB0
	movapd	64(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	64+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	64+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	64+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	64+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	64+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	64+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	64+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	64+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	64+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	64+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	64+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	64+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	64+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 11
	movapd	80(pB), rB0
	movapd	80(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	80+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	80+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	80+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	80+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	80+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	80+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	80+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	80+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	80+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	80+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	80+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	80+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	80+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 13
	movapd	96(pB), rB0
	movapd	96(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	96+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	96+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	96+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	96+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	96+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	96+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	96+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	96+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	96+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	96+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	96+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	96+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	96+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 15
	movapd	112(pB), rB0
	movapd	112(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	112+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	112+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	112+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	112+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	112+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	112+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	112+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	112+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	112+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	112+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	112+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	112+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	112+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 17
	movapd	128(pB), rB0
	movapd	128(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	128+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	128+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	128+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	128+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	128+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	128+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	128+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	128+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	128+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	128+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	128+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	128+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	128+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 19
	movapd	144(pB), rB0
	movapd	144(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	144+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	144+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	144+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	144+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	144+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	144+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	144+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	144+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	144+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	144+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	144+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	144+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	144+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 21
	movapd	160(pB), rB0
	movapd	160(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	160+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	160+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	160+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	160+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	160+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	160+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	160+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	160+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	160+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	160+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	160+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	160+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	160+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 23
	movapd	167(pB), rB0
	movapd	176(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	176+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	176+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	176+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	176+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	176+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	176+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	176+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	176+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	176+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	176+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	176+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	176+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	176+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 25
	movapd	192(pB), rB0
	movapd	192(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	192+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	192+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	192+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	192+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	192+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	192+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	192+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	192+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	192+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	192+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	192+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	192+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	192+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 27
	movapd	208(pB), rB0
	movapd	208(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	208+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	208+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	208+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	208+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	208+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	208+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	208+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	208+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	208+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	208+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	208+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	208+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	208+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 29
	movapd	224(pB), rB0
	movapd	224(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	224+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	224+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	224+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	224+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	224+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	224+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	224+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	224+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	224+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	224+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	224+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	224+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	224+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 31
	movapd	240(pB), rB0
	movapd	240(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	240+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	240+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	240+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	240+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	240+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	240+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	240+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	240+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	240+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	240+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	240+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	240+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	240+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 33
	movapd	256(pB), rB0
	movapd	256(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	256+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	256+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	256+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	256+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	256+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	256+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	256+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	256+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	256+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	256+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	256+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	256+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	256+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 35
	movapd	272(pB), rB0
	movapd	272(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	272+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	272+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	272+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	272+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	272+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	272+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	272+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	272+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	272+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	272+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	272+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	272+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	272+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 37
	movapd	288(pB), rB0
	movapd	288(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	288+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	288+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	288+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	288+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	288+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	288+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	288+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	288+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	288+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	288+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	288+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	288+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	288+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 39
	movapd	304(pB), rB0
	movapd	304(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	304+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	304+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	304+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	304+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	304+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	304+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	304+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	304+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	304+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	304+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	304+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	304+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	304+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 41
	movapd	320(pB), rB0
	movapd	320(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	320+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	320+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	320+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	320+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	320+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	320+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	320+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	320+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	320+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	320+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	320+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	320+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	320+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 43
	movapd	336(pB), rB0
	movapd	336(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	336+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	336+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	336+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	336+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	336+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	336+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	336+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	336+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	336+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	336+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	336+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	336+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	336+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 45
	movapd	352(pB), rB0
	movapd	352(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	352+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	352+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	352+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	352+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	352+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	352+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	352+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	352+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	352+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	352+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	352+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	352+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	352+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 47
	movapd	368(pB), rB0
	movapd	368(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	368+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	368+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	368+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	368+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	368+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	368+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	368+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	368+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	368+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	368+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	368+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	368+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	368+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 49
	movapd	384(pB), rB0
	movapd	384(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	384+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	384+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	384+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	384+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	384+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	384+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	384+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	384+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	384+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	384+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	384+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	384+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	384+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 51
	movapd	400(pB), rB0
	movapd	400(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	400+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	400+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	400+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	400+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	400+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	400+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	400+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	400+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	400+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	400+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	400+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	400+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	400+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 53
	movapd	416(pB), rB0
	movapd	416(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	416+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	416+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	416+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	416+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	416+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	416+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	416+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	416+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	416+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	416+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	416+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	416+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	416+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 55
	movapd	432(pB), rB0
	movapd	432(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	432+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	432+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	432+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	432+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	432+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	432+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	432+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	432+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	432+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	432+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	432+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	432+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	432+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

/*	addq    $448, pB */
/*	addq    $448, pA */

/*
 *      While (pB != stK);
 */
/*	cmp	pB, stK */
/*	jne	UKLOOP */
#ifdef KBR
	movlpd	KBR(pB), rB0
	movlpd	KBR(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC0
	movlpd	KBR+NBso(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC1
	movlpd	KBR+NB2so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC2
	movlpd	KBR+NB3so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC3
	movlpd	KBR+NB4so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC4
	movlpd	KBR+NB5so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC5
	movlpd	KBR+NB6so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC6
	movlpd	KBR+NB7so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC7
	movlpd	KBR+NB8so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC8
	movlpd	KBR+NB9so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC9
	movlpd	KBR+NB10so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC10
	movlpd	KBR+NB11so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC11
	movlpd	KBR+NB12so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC12
	movlpd	KBR+NB13so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC13
#endif

/*
 *      Get these bastard things summed up correctly
 */
                                        /* rC0 = c0a  c0b */
                                        /* rC1 = c1a  c1b */
                                        /* rC2 = c2a  c2b */
                                        /* rC3 = c3a  c3b */
/* */
                                        /* rC4 = c4a  c4b */
                                        /* rC5 = c5a  c5b */
                                        /* rC6 = c6a  c6b */
                                        /* rC7 = c7a  c7b */
	pref2((pfA))
	pref2(64(pfA))
        movapd          rC0, rA0
        movapd          rC4, rB0
        unpcklpd        rC1, rC0        /* rC0 = c0a  c1a */
        unpcklpd        rC5, rC4        /* rC4 = c4a  c5a */
	prefC(112(pC))
	prefC(176(pC))
        unpckhpd        rC1, rA0        /* rA0 = c0b  c1b */
        unpckhpd        rC5, rB0        /* rB0 = c4b  c5b */
        addpd           rA0, rC0        /* rC0 = c0ab c1ab */
        addpd           rB0, rC4        /* rC4 = c4ab c5ab */
        movapd          rC2, rA0
        movapd          rC6, rB0
        unpcklpd        rC3, rC2        /* rC2 = c2a  c3a */
        unpcklpd        rC7, rC6        /* rC6 = c6a  c7a */
        unpckhpd        rC3, rA0        /* rA0 = c2b  c3b */
        unpckhpd        rC7, rB0        /* rB0 = c6b  c7b */
        addpd           rA0, rC2        /* rC2 = c2ab c3ab */
        addpd           rB0, rC6        /* rC6 = c6ab c7ab */
/* */
                                        /* rC8 = c08a  c08b */
                                        /* rC9 = c09a  c09b */
                                        /* rC10 = c10a  c10b */
                                        /* rC11 = c11a  c11b */
/* */
					/* rC12 = c12a c12b */
					/* rC13 = c13a c13b */
	pref2(128(pfA))
	addq	$150, pfA
        movapd          rC8, rA0
        movapd          rC10, rC1
	movapd		rC12, rC3	
        unpcklpd        rC9, rC8        /* rC8 = c08a  c09a */
        unpcklpd        rC11, rC10      /* rC10 = c10a  c11a */
	unpcklpd	rC13, rC12	/* rC12 = c12a c13a */
        unpckhpd        rC11, rC1       /* rC1 = c10b  c11b */
        unpckhpd        rC9, rA0        /* rA0 = c08b  c09b */
	unpckhpd	rC13, rC3	/* rC3  = c12b c13b */
        addpd           rA0, rC8        /* rC8 = c08ab c09ab */
        addpd           rC1, rC10       /* rC10 = c10ab c11ab */
	addpd		rC3, rC12	/* rc12 = c12ab c13ab */
/*
 *      Write results back to C
 */
#ifdef DREAL
	movupd	rC0, (pC)
	movupd	rC2, 16(pC)
	movupd	rC4, 32(pC)
	movupd	rC6, 48(pC)
	movupd	rC8, 64(pC)
	movupd	rC10, 80(pC)
	movupd	rC12, 96(pC)
#else
	movlpd	rC0, (pC)
	movhpd	rC0, 16(pC)
	movlpd	rC2, 32(pC)
	movhpd	rC2, 48(pC)
	movlpd	rC4, 64(pC)
	movhpd	rC4, 80(pC)
	movlpd	rC6, 96(pC)
	movhpd	rC6, 112(pC)
	movlpd	rC8, 128(pC)
	movhpd	rC8, 144(pC)
	movlpd	rC10, 160(pC)
	movhpd	rC10, 176(pC)
	movlpd	rC12, 192(pC)
	movhpd	rC12, 208(pC)
#endif
/*
 *      pC += 14;  pA += 14*NB; pB -= NB;
 */
#ifdef DREAL
	addq	$112, pC
#else
	addq	$224, pC
#endif
	addq	$NB14so, pA
/*
 *      while (pA != stM);
 */
	cmp	pA, stM
	jne	UMLOOP

/*
 *      Last iteration of M-loop unrolled to prefetch next col of B
 */
#ifdef BETA0
	xorpd	rC0, rC0
	xorpd	rC1, rC1
	xorpd	rC2, rC2
	xorpd	rC3, rC3
	xorpd	rC4, rC4
	xorpd	rC5, rC5
	xorpd	rC6, rC6
	xorpd	rC7, rC7
	xorpd	rC8, rC8
	xorpd	rC9, rC9
	xorpd	rC10, rC10
	xorpd	rC11, rC11
	xorpd	rC12, rC12
	xorpd	rC13, rC13
#else
   #ifdef DREAL
	movsd	(pC), rC0
	movsd	8(pC), rC1
	movsd	16(pC), rC2
	movsd	24(pC), rC3
	movsd	32(pC), rC4
	movsd	40(pC), rC5
	movsd	48(pC), rC6
	movsd	56(pC), rC7
	movsd	64(pC), rC8
	movsd	72(pC), rC9
	movsd	80(pC), rC10
	movsd	88(pC), rC11
	movsd	96(pC), rC12
	movsd	104(pC), rC13
   #else
	movsd	(pC), rC0
	movsd	16(pC), rC1
	movsd	32(pC), rC2
	movsd	48(pC), rC3
	movsd	64(pC), rC4
	movsd	80(pC), rC5
	movsd	96(pC), rC6
	movsd	112(pC), rC7
	movsd	128(pC), rC8
	movsd	144(pC), rC9
	movsd	160(pC), rC10
	movsd	176(pC), rC11
	movsd	192(pC), rC12
	movsd	208(pC), rC13
   #endif
   #ifdef BETAX
	mulsd	-24(%rsp), rC0
	mulsd	-24(%rsp), rC1
	mulsd	-24(%rsp), rC2
	mulsd	-24(%rsp), rC3
	mulsd	-24(%rsp), rC4
	mulsd	-24(%rsp), rC5
	mulsd	-24(%rsp), rC6
	mulsd	-24(%rsp), rC7
	mulsd	-24(%rsp), rC8
	mulsd	-24(%rsp), rC9
	mulsd	-24(%rsp), rC10
	mulsd	-24(%rsp), rC11
	mulsd	-24(%rsp), rC12
	mulsd	-24(%rsp), rC13
   #endif
#endif
/*KLOOP_UR: */
#if KB > 1
	movapd	(pB), rB0
	movapd	(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif
	prefB(NBso(pB))
	prefB(64+NBso(pB))

#if KB > 3
	movapd	16(pB), rB0
	movapd	16(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	16+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	16+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	16+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	16+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	16+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	16+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	16+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	16+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	16+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	16+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	16+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	16+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	16+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 5
	movapd	32(pB), rB0
	movapd	32(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	32+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	32+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	32+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	32+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	32+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	32+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	32+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	32+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	32+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	32+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	32+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	32+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	32+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 7
	movapd	48(pB), rB0
	movapd	48(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	48+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	48+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	48+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	48+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	48+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	48+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	48+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	48+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	48+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	48+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	48+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	48+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	48+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 9
	movapd	64(pB), rB0
	movapd	64(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	64+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	64+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	64+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	64+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	64+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	64+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	64+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	64+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	64+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	64+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	64+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	64+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	64+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 11
	movapd	80(pB), rB0
	movapd	80(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	80+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	80+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	80+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	80+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	80+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	80+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	80+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	80+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	80+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	80+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	80+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	80+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	80+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 13
	movapd	96(pB), rB0
	movapd	96(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	96+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	96+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	96+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	96+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	96+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	96+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	96+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	96+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	96+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	96+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	96+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	96+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	96+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 15
	movapd	112(pB), rB0
	movapd	112(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	112+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	112+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	112+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	112+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	112+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	112+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	112+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	112+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	112+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	112+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	112+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	112+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	112+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 17
	movapd	128(pB), rB0
	movapd	128(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	128+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	128+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	128+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	128+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	128+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	128+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	128+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	128+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	128+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	128+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	128+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	128+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	128+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif
	prefB(128+NBso(pB))
	prefB(192+NBso(pB))

#if KB > 19
	movapd	144(pB), rB0
	movapd	144(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	144+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	144+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	144+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	144+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	144+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	144+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	144+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	144+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	144+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	144+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	144+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	144+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	144+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 21
	movapd	160(pB), rB0
	movapd	160(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	160+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	160+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	160+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	160+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	160+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	160+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	160+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	160+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	160+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	160+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	160+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	160+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	160+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 23
	movapd	176(pB), rB0
	movapd	176(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	176+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	176+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	176+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	176+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	176+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	176+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	176+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	176+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	176+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	176+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	176+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	176+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	176+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 25
	movapd	192(pB), rB0
	movapd	192(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	192+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	192+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	192+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	192+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	192+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	192+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	192+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	192+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	192+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	192+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	192+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	192+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	192+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 27
	movapd	208(pB), rB0
	movapd	208(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	208+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	208+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	208+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	208+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	208+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	208+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	208+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	208+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	208+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	208+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	208+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	208+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	208+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 29
	movapd	224(pB), rB0
	movapd	224(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	224+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	224+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	224+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	224+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	224+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	224+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	224+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	224+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	224+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	224+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	224+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	224+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	224+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 31
	movapd	240(pB), rB0
	movapd	240(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	240+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	240+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	240+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	240+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	240+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	240+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	240+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	240+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	240+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	240+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	240+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	240+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	240+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif
	prefB(256+NBso(pB))
	prefB(320+NBso(pB))

#if KB > 33
	movapd	256(pB), rB0
	movapd	256(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	256+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	256+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	256+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	256+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	256+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	256+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	256+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	256+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	256+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	256+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	256+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	256+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	256+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 35
	movapd	272(pB), rB0
	movapd	272(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	272+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	272+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	272+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	272+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	272+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	272+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	272+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	272+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	272+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	272+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	272+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	272+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	272+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 37
	movapd	288(pB), rB0
	movapd	288(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	288+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	288+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	288+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	288+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	288+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	288+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	288+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	288+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	288+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	288+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	288+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	288+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	288+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 39
	movapd	304(pB), rB0
	movapd	304(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	304+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	304+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	304+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	304+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	304+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	304+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	304+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	304+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	304+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	304+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	304+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	304+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	304+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 41
	movapd	320(pB), rB0
	movapd	320(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	320+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	320+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	320+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	320+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	320+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	320+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	320+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	320+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	320+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	320+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	320+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	320+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	320+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 43
	movapd	336(pB), rB0
	movapd	336(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	336+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	336+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	336+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	336+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	336+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	336+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	336+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	336+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	336+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	336+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	336+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	336+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	336+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 45
	movapd	352(pB), rB0
	movapd	352(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	352+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	352+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	352+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	352+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	352+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	352+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	352+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	352+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	352+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	352+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	352+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	352+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	352+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 47
	movapd	368(pB), rB0
	movapd	368(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	368+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	368+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	368+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	368+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	368+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	368+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	368+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	368+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	368+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	368+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	368+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	368+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	368+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 49
	movapd	384(pB), rB0
	movapd	384(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	384+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	384+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	384+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	384+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	384+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	384+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	384+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	384+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	384+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	384+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	384+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	384+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	384+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 51
	movapd	400(pB), rB0
	movapd	400(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	400+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	400+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	400+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	400+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	400+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	400+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	400+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	400+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	400+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	400+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	400+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	400+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	400+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 53
	movapd	416(pB), rB0
	movapd	416(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	416+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	416+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	416+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	416+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	416+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	416+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	416+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	416+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	416+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	416+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	416+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	416+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	416+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

#if KB > 55
	movapd	432(pB), rB0
	movapd	432(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC0
	movapd	432+NBso(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC1
	movapd	432+NB2so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC2
	movapd	432+NB3so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC3
	movapd	432+NB4so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC4
	movapd	432+NB5so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC5
	movapd	432+NB6so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC6
	movapd	432+NB7so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC7
	movapd	432+NB8so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC8
	movapd	432+NB9so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC9
	movapd	432+NB10so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC10
	movapd	432+NB11so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC11
	movapd	432+NB12so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC12
	movapd	432+NB13so(pA), rA0
	mulpd	rB0, rA0
	addpd	rA0, rC13
#endif

/*	addq    $448, pB */
/*	addq    $448, pA */

/*
 *      While (pB != stK);
 */
/*	cmp	pB, stK */
/*	jne	KLOOP_UR */
#ifdef KBR
	movlpd	KBR(pB), rB0
	movlpd	KBR(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC0
	movlpd	KBR+NBso(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC1
	movlpd	KBR+NB2so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC2
	movlpd	KBR+NB3so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC3
	movlpd	KBR+NB4so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC4
	movlpd	KBR+NB5so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC5
	movlpd	KBR+NB6so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC6
	movlpd	KBR+NB7so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC7
	movlpd	KBR+NB8so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC8
	movlpd	KBR+NB9so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC9
	movlpd	KBR+NB10so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC10
	movlpd	KBR+NB11so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC11
	movlpd	KBR+NB12so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC12
	movlpd	KBR+NB13so(pA), rA0
	mulsd	rB0, rA0
	addsd	rA0, rC13
#endif

/*
 *      Get these bastard things summed up correctly
 */
                                        /* rC0 = c0a  c0b */
                                        /* rC1 = c1a  c1b */
                                        /* rC2 = c2a  c2b */
                                        /* rC3 = c3a  c3b */
/* */
                                        /* rC4 = c4a  c4b */
                                        /* rC5 = c5a  c5b */
                                        /* rC6 = c6a  c6b */
                                        /* rC7 = c7a  c7b */
        movapd          rC0, rA0
        movapd          rC4, rB0
	addq	incCn, pC
	prefC((pC))
	prefC(64(pC))
        unpcklpd        rC1, rC0        /* rC0 = c0a  c1a */
        unpcklpd        rC5, rC4        /* rC4 = c4a  c5a */
        unpckhpd        rC1, rA0        /* rA0 = c0b  c1b */
        unpckhpd        rC5, rB0        /* rB0 = c4b  c5b */
        addpd           rA0, rC0        /* rC0 = c0ab c1ab */
	subq	incCn, pC
        addpd           rB0, rC4        /* rC4 = c4ab c5ab */
        movapd          rC2, rA0
        movapd          rC6, rB0
        unpcklpd        rC3, rC2        /* rC2 = c2a  c3a */
        unpcklpd        rC7, rC6        /* rC6 = c6a  c7a */
	prefB(384+NBso(pB))
        unpckhpd        rC3, rA0        /* rA0 = c2b  c3b */
        unpckhpd        rC7, rB0        /* rB0 = c6b  c7b */
        addpd           rA0, rC2        /* rC2 = c2ab c3ab */
        addpd           rB0, rC6        /* rC6 = c6ab c7ab */
/* */
                                        /* rC8 = c08a  c08b */
                                        /* rC9 = c09a  c09b */
                                        /* rC10 = c10a  c10b */
                                        /* rC11 = c11a  c11b */
/* */
					/* rC12 = c12a c12b */
					/* rC13 = c13a c13b */
        movapd          rC8, rA0
        movapd          rC10, rC1
	movapd		rC12, rC3	
        unpcklpd        rC9, rC8        /* rC8 = c08a  c09a */
        unpcklpd        rC11, rC10      /* rC10 = c10a  c11a */
	unpcklpd	rC13, rC12	/* rC12 = c12a c13a */
        unpckhpd        rC11, rC1       /* rC1 = c10b  c11b */
        unpckhpd        rC9, rA0        /* rA0 = c08b  c09b */
	unpckhpd	rC13, rC3	/* rC3  = c12b c13b */
        addpd           rA0, rC8        /* rC8 = c08ab c09ab */
        addpd           rC1, rC10       /* rC10 = c10ab c11ab */
	addpd		rC3, rC12	/* rc12 = c12ab c13ab */
/*
 *      Write results back to C
 */
#ifdef DREAL
	movupd	rC0, (pC)
	movupd	rC2, 16(pC)
	movupd	rC4, 32(pC)
	movupd	rC6, 48(pC)
	movupd	rC8, 64(pC)
	movupd	rC10, 80(pC)
	movupd	rC12, 96(pC)
#else
	movlpd	rC0, (pC)
	movhpd	rC0, 16(pC)
	movlpd	rC2, 32(pC)
	movhpd	rC2, 48(pC)
	movlpd	rC4, 64(pC)
	movhpd	rC4, 80(pC)
	movlpd	rC6, 96(pC)
	movhpd	rC6, 112(pC)
	movlpd	rC8, 128(pC)
	movhpd	rC8, 144(pC)
	movlpd	rC10, 160(pC)
	movhpd	rC10, 176(pC)
	movlpd	rC12, 192(pC)
	movhpd	rC12, 208(pC)
#endif
/*
 *      pC += 14;  pA += 14*NB; pB -= NB;
 */
/*	addq	$112, pC */
/*	addq	$NB14so, pA */
/*
 *      pC += incCn;  pA -= NBNB;  pB += NB;
 */
	addq	incCn, pC
	subq	$(MB*NBso)-NB14so, pA
	addq	$NBso, pB
/*
 *      while (pB != stN);
 */
	cmp	pB, stN
	jne	UNLOOP

/*
 *	Restore callee-saved iregs
 */
	movq	-8(%rsp), %rbp
/*	movq	-16(%rsp), %rbx */
	ret
