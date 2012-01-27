/* INTEGER REGISTER SUMMARY:
 * EAX : pA0
 * ECX : pB0
 * EDX : pC0
 * EBX : (ldc-KB)*sizeof() (incCn)
 * ESP : stack pointer
 * EBP : stM
 * ESI : stN
 * EDI : stK   (no longer used)
 */

#ifndef ATL_GAS_x8632
   #error "This kernel requires gas x86 assembler!"
#endif
#ifndef ATL_SSE2
   #error "This routine requires SSE2!"
#endif
#if NB == 60
   #define NBNB4 (NBNB+NBNB+NBNB+NBNB)
   #define NB12  (NB6+NB6)
   #define NB16  (NB8+NB8)
   #if (NB/6)*6 != NB || (NB/2)*2 != NB || (NB/60)*60 != NB
      #error "Unsupported block size!!"
   #endif
#else
   #error "Unsupported block size!!"
#endif
.text
	ALIGN4
.globl	ATL_USERMM
ATL_USERMM:
        finit
	push    %esp
	push    %eax
	push    %ecx
	push    %edx
	push    %ebx
	push    %ebp
	push    %esi
	push    %edi
/*
 *      Put esp in eax, adjusted so the load of BETA is aligned,
 *      store old sp after beta so it can be recovered at end of rout
 */
#ifdef BETAX
	movl	%esp, %eax
        subl    $32, %eax
	shr	$4, %eax
	shl	$4, %eax
	fldl	72(%esp)
	fstl 	(%eax)
	fstpl 	8(%eax)
	movl	%esp, 16(%eax)
#endif
/*
 *      pB0(ecx) = pB0;  stN(esi) = pB0 + NBNB*sizeof()
 */
	movl	64(%esp), %esi
	movl	%esi, %ecx
	addl	$NBNB4+NBNB4, %esi
/*
 *      stM(ebp) = pA0;  ebx = (ldc - NB)*sizeof;  edx = pC0
 */
	movl	56(%esp), %ebp
	movl	84(%esp), %ebx
        subl    $NB, %ebx
	shl	$3, %ebx
	movl	80(%esp), %edx
#ifdef BETAX
	movl	%eax, %esp      /* aligned local BETA/temp access */
#endif
/*
 *      eax = pA0;  stM = pA0 + NBNB*sizeof()
 */
	movl	%ebp, %eax
	addl	$NBNB4+NBNB4, %ebp
LOOPN:
LOOPM:
/*
 *      Zero c0-c5
 */
        xorpd   %xmm0, %xmm0
        xorpd   %xmm1, %xmm1
        xorpd   %xmm2, %xmm2
        xorpd   %xmm3, %xmm3
        xorpd   %xmm4, %xmm4
        xorpd   %xmm5, %xmm5

/*LOOPK: */
        movapd  (%eax), %xmm6
        movapd	(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  16(%eax), %xmm6
        movapd	16(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  16+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  16+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  16+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  32(%eax), %xmm6
        movapd	32(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  32+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  32+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  32+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  32+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  32+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  48(%eax), %xmm6
        movapd	48(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  48+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  48+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  48+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  48+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  48+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  64(%eax), %xmm6
        movapd	64(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  64+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  64+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  64+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  64+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  64+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  80(%eax), %xmm6
        movapd	80(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  80+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  80+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  80+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  80+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  80+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  96(%eax), %xmm6
        movapd	96(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  96+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  96+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  96+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  96+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  96+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  112(%eax), %xmm6
        movapd	112(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  112+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  112+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  112+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  112+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  112+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  128(%eax), %xmm6
        movapd	128(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  128+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  128+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  128+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  128+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  128+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  144(%eax), %xmm6
        movapd	144(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  144+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  144+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  144+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  144+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  144+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  160(%eax), %xmm6
        movapd	160(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  160+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  160+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  160+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  160+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  160+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  176(%eax), %xmm6
        movapd	176(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  176+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  176+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  176+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  176+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  176+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  192(%eax), %xmm6
        movapd	192(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  192+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  192+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  192+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  192+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  192+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  208(%eax), %xmm6
        movapd	208(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  208+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  208+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  208+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  208+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  208+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  224(%eax), %xmm6
        movapd	224(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  224+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  224+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  224+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  224+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  224+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  240(%eax), %xmm6
        movapd	240(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  240+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  240+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  240+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  240+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  240+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  256(%eax), %xmm6
        movapd	256(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  256+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  256+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  256+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  256+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  256+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  272(%eax), %xmm6
        movapd	272(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  272+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  272+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  272+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  272+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  272+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  288(%eax), %xmm6
        movapd	288(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  288+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  288+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  288+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  288+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  288+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  304(%eax), %xmm6
        movapd	304(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  304+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  304+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  304+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  304+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  304+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  320(%eax), %xmm6
        movapd	320(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  320+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  320+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  320+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  320+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  320+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  336(%eax), %xmm6
        movapd	336(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  336+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  336+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  336+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  336+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  336+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  352(%eax), %xmm6
        movapd	352(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  352+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  352+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  352+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  352+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  352+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  368(%eax), %xmm6
        movapd	368(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  368+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  368+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  368+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  368+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  368+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  384(%eax), %xmm6
        movapd	384(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  384+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  384+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  384+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  384+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  384+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  400(%eax), %xmm6
        movapd	400(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  400+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  400+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  400+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  400+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  400+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  416(%eax), %xmm6
        movapd	416(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  416+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  416+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  416+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  416+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  416+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  432(%eax), %xmm6
        movapd	432(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  432+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  432+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  432+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  432+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  432+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  448(%eax), %xmm6
        movapd	448(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  448+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  448+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  448+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  448+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  448+NB16+NB16+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

        movapd  464(%eax), %xmm6
        movapd	464(%ecx), %xmm7
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm0
        movapd  464+NB8(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm1
        movapd  464+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm2
        movapd  464+NB12+NB12(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm3
        movapd  464+NB16+NB16(%eax), %xmm6
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm4
        movapd  464+NB16+NB16+NB8(%eax), %xmm6
	addl	$NB16+NB16+NB16, %eax
	mulpd	%xmm7, %xmm6
	addpd	%xmm6, %xmm5

/*
 *      while (pB0 != stK)
 */
/*
 *      Get these bastard things summed up correctly
 */
                                        /* xmm0 = c0a  c0b */
                                        /* xmm1 = c1a  c1b */
                                        /* xmm2 = c2a  c2b */
                                        /* xmm3 = c3a  c3b */
                                        /* xmm4 = c4a  c4b */
                                        /* xmm5 = c5a  c5b */
        movapd          %xmm0, %xmm7
        unpcklpd        %xmm1, %xmm0    /* xmm0 = c0a  c1a */
        unpckhpd        %xmm1, %xmm7    /* xmm7 = c0b  c1b */
        addpd           %xmm7, %xmm0    /* xmm0 = c0ab c1ab */
        movapd          %xmm2, %xmm7
        unpcklpd        %xmm3, %xmm2    /* xmm2 = c2a  c3a */
        unpckhpd        %xmm3, %xmm7    /* xmm7 = c2b  c3b */
        addpd           %xmm7, %xmm2    /* xmm2 = c2ab c3ab */
        movapd          %xmm4, %xmm7
        unpcklpd        %xmm5, %xmm4    /* xmm4 = c4a  c5a */
        unpckhpd        %xmm5, %xmm7    /* xmm7 = c4b  c5b */
        addpd           %xmm7, %xmm4    /* xmm4 = c4ab c5ab */
#ifndef BETA0
        movupd          (%edx), %xmm5
        movupd          16(%edx), %xmm6
        movupd          32(%edx), %xmm1
   #ifdef BETAX
        movapd          (%esp), %xmm7
        mulpd           %xmm7, %xmm5
        mulpd           %xmm7, %xmm6
        mulpd           %xmm7, %xmm1
   #endif
        addpd           %xmm5, %xmm0
        addpd           %xmm6, %xmm2
        addpd           %xmm1, %xmm4
#endif
/*
 *      *pC0 = rC00; pC0[1] = rC10; pC0[2] = rC20; pC0[3] = rC30;
 */
        movupd  %xmm0, (%edx)
        movupd  %xmm2, 16(%edx)
        movupd  %xmm4, 32(%edx)
/*
 *      pC0 += 6
 */
	addl	$48, %edx
/*
 *      while (pA0 != stM);
 */
	cmp %eax, %ebp
	jne     LOOPM
/*
 *      pC0 += incCn;   pA0 += incAn;  pB0 += incBn;
 */
	addl	%ebx, %edx
	subl	$NBNB4+NBNB4, %eax
	addl	$NB8, %ecx
/*
 *      while (pB0 != stN);
 */
	cmp	%ecx, %esi
	jne	LOOPN
/*
 *      Clean up stack and return
 */
#ifdef BETAX
	movl	16(%esp), %esp
#endif
	pop     %edi
	pop     %esi
	pop     %ebp
	pop     %ebx
	pop     %edx
	pop     %ecx
	pop     %eax
	pop     %esp
	ret
