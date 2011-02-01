#
#  Test case 4x4 blocking, assuming A packed in MU-block-major order,
#  B in normal block-major order.  Did not make run as fast as normal
#  A storage, and unrolling K-loop did not make it faster.  Should probably
#  scope viability of 1-d blocking wt this storage.
#
#  Integer register usage shown be these defines
#
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
#define ldc     %r14
#define ldc3    %r15
#define pBW     %rsp


#define rA0     %xmm0
#define rA2     %xmm1
#define rB0     %xmm2
#define rb0     %xmm3
#define rB1     %xmm4
#define rb1     %xmm5
#define rC00    %xmm6
#define rC20    %xmm7
#define rC01    %xmm8
#define rC21    %xmm9
#define rC02    %xmm10
#define rC22    %xmm11
#define rC03    %xmm12
#define rC23    %xmm13
#define BETA    %xmm15

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
#ifdef DCPLX
        #define CMUL(x_) ((x_)*2)
#else
        #define CMUL(x_) (x_)
#endif

#
#  Prefetch defines
#
#if 1
   #define pref2(mem) prefetcht1	mem
   #define prefB(mem) prefetcht0	mem
   #define prefC(mem) prefetchw	        mem
#else
   #define pref2(mem)
   #define prefB(mem)
   #define prefC(mem)
#endif
# offset                rdi           rsi         rdx             xmm0   
# void ATL_USERMM(const int M, const int N, const int K, const TYPE alpha,
# offset                rcx            r8              r9                  8
#                 const TYPE *A, const int lda, const TYPE *B, const int ldb,
# offset                xmm1   40       16             24
#                 const TYPE beta, TYPE *C, const int ldc)
#
	.text
.global ATL_USERMM
ATL_USERMM:
#
#       Save callee-saved iregs
#
        movq    %rbp, -8(%rsp)
        movq    %rbx, -16(%rsp)
        movq    %r12, -32(%rsp)
        movq    %r13, -40(%rsp)
        movq    %r14, -48(%rsp)
        movq    %r15, -56(%rsp)

#ifdef BETAX
        unpcklpd %xmm1, %xmm1
        movapd   %xmm1, BETA
#endif
#
#       pA already comes in right reg
#       Initialize MM=M, NN=N, pB = B; pC = C, incAn = MB*KB*sizeof
#
                        prefB((pB))
                        prefB(64(pB))
        movq    %rdi, MM
        movq    %rsi, NN
        movq    %rdx, KK
        movq    %r9, pB
        movq    16(%rsp), pC
                        prefC((pC))
                        prefC(64(pC))
#
#       Set incCn = (ldc - MB)*sizeof
#
   movslq       24(%rsp), incCn
   movq         incCn, ldc
   shl          $2, incCn
   subq         MM, incCn
   #ifdef DCPLX
	shl	$4, incCn
        shl     $4, ldc
   #else
	shl	$3, incCn
        shl     $3, ldc
   #endif
#
#       Get 4*sizeof*2*KB workspace off the stack
#
        subq    $4*CMUL(8)*2*KB+64+8, %rsp
        
        lea     (ldc,ldc,2), ldc3
#        addq    $120, pA
#        addq    $120, pB
#
#       ldab = KB*sizeof; ldab3 = KB*3*sizeof
#
        movq    KK, ldab
        shl     $3, ldab
        lea     (ldab,ldab,2), ldab3
#if MB == 0 || KB == 0
        movq    ldab, incAn
        imulq   MM, incAn
#else
        movq    $KB*MB*8, incAn
#endif
#
#       KK = -4*sizeof*KB
#
        shl     $5, KK
        neg     KK
        subq    KK, pA
NLOOP:
#
#       Copy 4 KB-length cols of B to 8*KB-length workspace, pBW has form:
#          b00 b00 b01 b01 b02 b02 b03 b03   b10 ...
#       NOTE: rewrite this for efficiency using MMX later
#
        movq    KK, IK
COPYB:
#if 0
        movq    (pB), IM
        movq    IM, (pBW,IK,2)
        movq    IM, 8(pBW,IK,2)
        movq    (pB,ldab), IM
        movq    IM, 16(pBW,IK,2)
        movq    IM, 24(pBW,IK,2)
        movq    (pB,ldab,2), IM
        movq    IM, 32(pBW,IK,2)
        movq    IM, 40(pBW,IK,2)
        movq    (pB,ldab3), IM
        movq    IM, 48(pBW,IK,2)
        movq    IM, 56(pBW,IK,2)
        addq    $8, pB
        addq    $32, IK
        jne     COPYB
        addq    ldab3, pB
#else
        lea     (pB,ldab,4), pB
#endif
        movq    MM, IM
MLOOP:
        movq    KK, IK
        addq    $4*8, IK
#ifdef BETA0
        xorpd   rC00, rC00
        xorpd   rC20, rC20
        xorpd   rC01, rC01
        xorpd   rC21, rC21
        xorpd   rC02, rC02
        xorpd   rC22, rC22
        xorpd   rC03, rC03
        xorpd   rC23, rC23
#else
        movapd  (pC), rC00
        movapd  16(pC), rC20
        movapd  (pC,ldc), rC01
        movapd  16(pC,ldc), rC21
        movapd  (pC,ldc,2),  rC02
        movapd  16(pC,ldc,2), rC22
        movapd  (pC,ldc3),  rC03
        movapd  16(pC,ldc3), rC23
   #ifndef BETA1
        mulpd   BETA, rC00
        mulpd   BETA, rC20
        mulpd   BETA, rC01
        mulpd   BETA, rC21
        mulpd   BETA, rC02
        mulpd   BETA, rC22
        mulpd   BETA, rC03
        mulpd   BETA, rC23
   #endif
#endif
        movapd  (pA,KK), rA0
        movapd  (pBW,KK,2), rB0
        movapd  (pBW,KK,2), rb0
        movapd  16(pA,KK), rA2
        movapd  16(pBW,KK,2), rB1
        movapd  16(pBW,KK,2), rb1
#                addq    $32, pA
KLOOP:
        mulpd   rA0, rB0
        addpd   rB0, rC00
                                movapd  -64+32(pBW,IK,2), rB0
        mulpd   rA2, rb0
        addpd   rb0, rC20
                                movapd  -64+32(pBW,IK,2), rb0
        mulpd   rA0, rB1
        addpd   rB1, rC01
                                movapd  -64+48(pBW,IK,2), rB1

        mulpd   rA2, rb1
        addpd   rb1, rC21
                                movapd  -64+48(pBW,IK,2), rb1
        mulpd   rA0, rB0
        addpd   rB0, rC02
                                movapd  (pBW,IK,2), rB0
        mulpd   rA2, rb0
        addpd   rb0, rC22
                                movapd  (pBW,IK,2), rb0
        mulpd   rA0, rB1
                                movapd  (pA,IK), rA0
        addpd   rB1, rC03
                                movapd  16(pBW,IK,2), rB1
        mulpd   rA2, rb1
                                movapd  16(pA,IK), rA2
        addpd   rb1, rC23
#                                movapd  16(pBW,IK,2), rb1
                                movapd  rB1, rb1

        addq    $32, IK
        jne     KLOOP
#
#       Drain KLOOP
#
        mulpd   rA0, rB0
        addpd   rB0, rC00
                                movapd  -64+32(pBW,IK,2), rB0
        mulpd   rA2, rb0
        addpd   rb0, rC20
                                movapd  -64+32(pBW,IK,2), rb0
        mulpd   rA0, rB1
        addpd   rB1, rC01
                                movapd  -64+48(pBW,IK,2), rB1
#                                addq    $64, pB

        mulpd   rA2, rb1
        addpd   rb1, rC21
                                movapd  -64+48(pBW,IK,2), rb1
#                                movapd  rB1, rb1
        mulpd   rA0, rB0
        addpd   rB0, rC02
        mulpd   rA2, rb0
        addpd   rb0, rC22
        mulpd   rA0, rB1
        addpd   rB1, rC03
        mulpd   rA2, rb1
        addpd   rb1, rC23
#
#       Rest ptrs and store C
#
        movapd  rC00, (pC)
        movapd  rC20, 16(pC)
        movapd  rC01, (pC,ldc)
        movapd  rC21, 16(pC,ldc)
        movapd  rC02, (pC,ldc,2)
        movapd  rC22, 16(pC,ldc,2)
        movapd  rC03, (pC,ldc3)
        movapd  rC23, 16(pC,ldc3)
        addq    $32, pC
        lea     (pA,ldab,4), pA

        subq    $4, IM
        jne     MLOOP

#        lea     (pB, ldab,4), pB
        subq    incAn, pA
        addq    incCn, pC
        subq    $4, NN
        jne     NLOOP
#
#       Restore stack ptr & regs and return
#
        addq    $4*CMUL(8)*2*KB+64+8, %rsp
        movq    -8(%rsp), %rbp
        movq    -16(%rsp), %rbx
        movq    -32(%rsp), %r12
        movq    -40(%rsp), %r13
        movq    -48(%rsp), %r14
        movq    -56(%rsp), %r15
        ret
