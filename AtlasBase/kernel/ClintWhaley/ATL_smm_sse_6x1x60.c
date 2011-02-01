/* INTEGER REGISTER SUMMARY:
 *
 * EAX : pA0
 * ECX : pB0
 * EDX : pC0
 * EBX : (ldc-KB)*sizeof() (incCn)
 * ESP : stack pointer
 * EBP : stM
 * ESI : stN
 * EDI : stK
 */

#if NB == 60
   #define NBNB4 14400
   #define NB12  720
   #define NB16  960
#else
   #error "Unsupported block size!!"
#endif

.text
	ALIGN4
.globl	ATL_asmdecor(ATL_USERMM)
ATL_asmdecor(ATL_USERMM):
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
	movl	%esp, %eax
#ifdef BETAX
        subl    $48, %eax
#else
        subl    $32, %eax
#endif
	shr	$4, %eax
	shl	$4, %eax
#ifdef BETAX
	fld	68(%esp)
	fst 	16(%eax)
	fst 	20(%eax)
	fst 	24(%eax)
	fst	28(%eax)
	movl	%esp, 32(%eax)
#else
	movl	%esp, 16(%eax)
#endif
/*
 *      pB0(ecx) = pB0;  stN(esi) = pB0 + NBNB*sizeof()
 */
	movl	60(%esp), %esi
	movl	%esi, %ecx
	addl	$NBNB4, %esi
/*
 *      stM(ebp) = pA0;  ebx = (ldc - NB)*sizeof;  edx = pC0
 */
	movl	52(%esp), %ebp
	movl	76(%esp), %ebx
        subl    $NB, %ebx
	shl	$2, %ebx
	movl	72(%esp), %edx
	movl	%eax, %esp      /* aligned local BETA/temp access */
/*
 *      eax = pA0;  stM = pA0 + NBNB*sizeof()
 */
	movl	%ebp, %eax
	addl	$NBNB4, %ebp

LOOPN:
        prefetchnta (%ecx)
        prefetchnta 32(%ecx)
        prefetchnta 64(%ecx)
        prefetchnta 96(%ecx)
        prefetchnta 128(%ecx)
        prefetchnta 192(%ecx)
        prefetchnta 224(%ecx)
LOOPM:
/*
 *      Zero c0-c5
 */
        xorps   %xmm0, %xmm0
        xorps   %xmm1, %xmm1
        xorps   %xmm2, %xmm2
        xorps   %xmm3, %xmm3
        xorps   %xmm4, %xmm4
        xorps   %xmm5, %xmm5
/*LOOPK: */
        movaps  (%eax), %xmm6
        movaps	(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  16(%eax), %xmm6
        movaps	16(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  16+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  16+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  16+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  16+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  32(%eax), %xmm6
        movaps	32(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  32+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  32+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  32+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  32+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  32+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  48(%eax), %xmm6
        movaps	48(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  48+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  48+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  48+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  48+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  48+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  64(%eax), %xmm6
        movaps	64(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  64+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  64+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  64+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  64+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  64+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  80(%eax), %xmm6
        movaps	80(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  80+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  80+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  80+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  80+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  80+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  96(%eax), %xmm6
        movaps	96(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  96+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  96+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  96+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  96+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  96+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  112(%eax), %xmm6
        movaps	112(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  112+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  112+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  112+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  112+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  112+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  128(%eax), %xmm6
        movaps	128(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  128+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  128+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  128+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  128+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  128+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  144(%eax), %xmm6
        movaps	144(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  144+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  144+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  144+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  144+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  144+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  160(%eax), %xmm6
        movaps	160(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  160+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  160+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  160+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  160+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  160+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  176(%eax), %xmm6
        movaps	176(%ecx), %xmm7
        prefetcht0 (%edx)
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  176+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  176+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  176+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  176+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  176+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  192(%eax), %xmm6
        movaps	192(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  192+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  192+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  192+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  192+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  192+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  208(%eax), %xmm6
        movaps	208(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  208+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  208+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  208+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  208+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  208+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5

        movaps  224(%eax), %xmm6
        movaps	224(%ecx), %xmm7
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm0
        movaps  224+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm1
        movaps  224+NB8(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm2
        movaps  224+NB12(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm3
        movaps  224+NB16(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm4
        movaps  224+NB16+NB4(%eax), %xmm6
	mulps	%xmm7, %xmm6
	addps	%xmm6, %xmm5
/*
 *      Handle last odd 2 elts of C
 */
	movaps		%xmm5, %xmm6	/* xmm6 = c5a  c5b  c5c  c5d */
	unpckhps	%xmm4, %xmm6	/* xmm6 = c4a  c5a  c4b  c5b */
	unpcklps	%xmm4, %xmm5	/* xmm5 = c4c  c5c  c4d  c5d */
	addps		%xmm6, %xmm5	/* xmm5 = c4ac c5ac c4bd c5bd */
        movhlps         %xmm5, %xmm4    /* xmm4 = c4bd    c5bd    X X */
        addps           %xmm5, %xmm4    /* xmm4 = c4abcd  c5abcd  X X */
#ifndef BETA0
        movss           16(%edx), %xmm6 /* xmm6 = c4e       X     X X */
        movss           20(%edx), %xmm7 /* xmm7 = c5e       X     X X */
        unpcklps        %xmm6, %xmm7    /* xmm7 = c4e     c5e     X X */
   #ifdef BETAX
        movaps          16(%esp), %xmm6
        mulps           %xmm6, %xmm7    /* xmm7 = c4e     c5e     X X */
   #endif
        addps           %xmm7, %xmm4    /* xmm4 = c4abcde c5abcde X X */
#endif
        movaps          %xmm4, (%esp)
/*
 *      Get these bastard things summed up correctly
 */
	movaps		%xmm2, %xmm5
	unpckhps	%xmm3, %xmm5
	movaps		%xmm0, %xmm6
	unpckhps	%xmm1, %xmm6
	unpcklps	%xmm3, %xmm2
	unpcklps	%xmm1, %xmm0
	movlhps		%xmm5, %xmm3
	movhlps		%xmm6, %xmm3
	movlhps		%xmm2, %xmm6
	movhlps		%xmm0, %xmm5
	addps		%xmm6, %xmm3
	movlhps		%xmm0, %xmm1
	movhlps		%xmm1, %xmm2
	addps		%xmm5, %xmm2
	addps		%xmm2, %xmm3
#ifndef BETA0
        movups	(%edx), %xmm7
   #ifdef BETAX
        movaps	16(%esp), %xmm5
	mulps	%xmm5, %xmm7
   #endif
        addps   %xmm7, %xmm3
#endif
	movups		%xmm3, (%edx)
        movl	(%esp), %edi
        movl	%edi, 20(%edx)
        movl	4(%esp), %edi
        movl	%edi, 16(%edx)
/*
 *      *pC0 = rC00; pC0[1] = rC10; pC0[2] = rC20; pC0[3] = rC30;
 */
/*
 *      pC0 += 4;   pA0 += 5*NB;   pB0 -= NB;
 */
	addl	$24, %edx
	addl	$NB16+NB8, %eax
/*	addl	$NB16+NB4, %eax */
/*	subl	$NB4, %ecx */
/*
 *      while (pA0 != stM);
 */
	cmp %eax, %ebp
	jne     LOOPM
/*
 *      pC0 += incCn;   pA0 += incAn;  pB0 += incBn;
 */
	addl	%ebx, %edx
	subl	$NBNB4, %eax
	addl	$NB4, %ecx
/*
 *      while (pB0 != stN);
 */
	cmp	%ecx, %esi
	jne	LOOPN
        #ifdef BETAX
        fstp    %st
        #endif
/*
 *      Clean up stack and return
 */
#ifdef BETAX
	movl	32(%esp), %esp
#else
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
