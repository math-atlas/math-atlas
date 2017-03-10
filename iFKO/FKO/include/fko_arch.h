#ifndef FKO_ARCH_H
#define FKO_ARCH_H

#include "fko_settings.h"

/*============================================================================= 
 * Define architecure here:
 *============================================================================*/
#if !defined(LINUX_PPC) && !defined(OSX_PPC) && !defined(LINUX_X86_32) && \
    !defined(LINUX_X86_64) && !defined(SOLARIS_SPARC)
/*   #define FKO_ANSIC32 */
/*   #define LINUX_X86_64  */
     #define LINUX_X86_64    
/*   #define LINUX_X86_32 */
/*   #define SOLARIS_SPARC  */
/*   #define OSX_PPC  */
#endif
/*
 * Provide detailed system-level information here.  Will be set using guesswork
 * later if not defined here
 */
#if 0
/*
 * SSE1 or SSE2 not sufficient.  Need SSE3 for vertical add.
 * If overridng vector type here, (eg, AVX2), also define VECDEF
 */
   /*#define AVX2/AVX/SSE41/SSE3*/
   #define AVX2
   #define VECDEF 1
      
   #define FADDPIPELEN 4
   #define DADDPIPELEN 4
   #define FMULPIPELEN 4
   #define DMULPIPELEN 4
   #define FMACPIPELEN 6
   #define DMACPIPELEN 6
   #define FPUPIPEDEF 1
   
   #ifndef NCACHE
      #define NCACHE 2
/*
 *    linesize can also be set in command line
 */
      #ifdef ARCH_DECLARE
         short LINESIZE[NCACHE] = {32,32};
      #else
         extern short LINESIZE[NCACHE];
      #endif
   #endif
#endif

#if defined(FKO_ANSIC32) || defined(FKO_ANSIC64)
   #define FKO_ANSIC
#endif
#ifdef X86_64
   #define ADDR64
#endif
#if defined(LINUX_X86_32)
   #define X86_32
#endif
#if defined(LINUX_PPC) || defined(OSX_PPC)
   #define PPC
#endif
#if defined(LINUX_X86_64)
   #define X86_64
#endif
#if defined(LINUX_X86_32) || defined(LINUX_X86_64)
   #ifndef X86
      #define X86 1
   #endif
#endif
#ifdef SOLARIS_SPARC
   #define SPARC
#endif
#if X86 == 0
   #undef X86
#endif

/*============================================================================
 * Addressing Mode (load/store memory):
 * Multiplying factor in addressing mode of different architecturses
 *============================================================================*/
#ifdef PPC
   #define ArchHasLoadMul(mul_) ((mul_) == 0 || (mul_) == 1 || (mul_) == -1)
#elif defined(SPARC)
   #define ArchHasLoadMul(mul_) ((mul_) == 0 || (mul_) == 1 || (mul_) == -1)
#elif defined (X86)
   #define ArchHasLoadMul(mul_) \
      ( ((mul_) == 1) || ((mul_) == 2) || ((mul_) == 4) || ((mul_) == 8) || \
        ((mul_) == 0) || ((mul_) == -1) )
   #if defined(X86_64)
      #define ArchPtrIsLong 1
   #endif
   #define ArchConstAndIndex
#endif

/*=============================================================================
 * vector unit:
 * 1. supported types
 * 2. vector length
 * 
 * NOTE: FKO supports vector integer (both for 32bit and 64 bit integer) 
 * operations only for internal transformation purpose so far (like: shadow RC). 
 * NOTE: Only AVX2 supports vector integer operation, since it adds instructions 
 * like: vpaddd, vpaddq.
 * AVX2 has vpminsd/vpminsd but doesn't have any for 64bit. We ignore them here.
 *
 *============================================================================*/
#if defined(PPC) || defined(SPARC)
   #ifdef ArchHasVec
      #undef ArchHasVec  /* vec not supported in fko*/
   #endif
#elif defined(X86)
   #if !defined(ArchHasVec)
      #define ArchHasVec
   #endif
   #ifndef VECDEF        /* if not defined by user at top */
      #define AVX2
      /*#define AVX*/
      /*#define SSE41*/
   #endif
   #ifdef AVX2
      #define AVX /* AVX2 includes AVX plus some extra inst */
      #define VINT_CMOV
      #define INT_VEC
      #define FKO_IVLEN 8
   #endif
   #ifdef AVX
      #define ArchHasMemBroadcast
      #define ArchHasFPthreeOps
      #define ArchHasVFPthreeOps
      #define ArchHasVINTthreeOps
      #define FP_VEC
      #define FKO_SVLEN 8
      #define DP_VEC
      #define FKO_DVLEN 4
      /*#define INT_VEC*/
      /*#define FKO_IVLEN 8*/
   /*#elif defined(SSE41)*/ 
   #else /* by default SSE4.1*/
      /*#define INT_VEC*/
      /*#define FKO_IVLEN 4*/
/*
 *    added synthetic inst in SSE to support mem broadcast
 */
      #define ArchHasMemBroadcast
      #define SSE3   /* sse4.1 includes sse3 */
      #define FP_VEC
      #define FKO_SVLEN 4
      #define DP_VEC
      #define FKO_DVLEN 2
   #endif
#endif

/*=============================================================================
 * FMAC inst
 * 1. supported types
 *============================================================================*/
#ifdef PPC
   #ifdef ARCH_HAS_MAC
      #define ArchHasMAC
      #define FP_MAC
      #define DP_MAC
   #endif

#elif defined(SPARC)
   #ifdef ARCH_HAS_MAC
      #undef ArchHasMAC   /* not supported in fko yet */
   #endif

#elif defined(X86)
   #ifdef ARCH_HAS_MAC   /* defiend in fko_settings.h */    
      #if ARCH_HAS_MAC == 1
         #define ArchHasMAC   /* defined internally */
         /*#define FMA4*/     /* get retarded */
         #ifndef FMA4
            #define FMA3
         #endif
         #define FP_MAC
         #define DP_MAC
         #define VFP_MAC
         #define VDP_MAC
      #endif
   #endif
#endif

/*=============================================================================
 * SELECT/CMOV inst
 * 1. supported types
 *============================================================================*/

#ifdef PPC
   #ifdef ArchHasSelect
      #undef ArchHasSelect  /* select not supported in fko for this arch */
   #endif

#elif defined(SPARC)
   #ifdef ArchHasSelect
      #undef ArchHasSelect /* select not supported in fko for this arch */
   #endif

#elif defined(X86)
   #if !defined(ArchHasSelect)
      #define ArchHasSelect
   #endif

   #define INT_CMOV

   #ifdef AVX2 /* includes AVX too*/
      #define VINT_CMOV
   #endif

   #ifdef AVX
      #define FP_CMOV
      #define DP_CMOV
      #define VFP_CMOV
      #define VDP_CMOV
   #endif
#endif

/*=============================================================================
 * Max/Min inst
 * 1. supported types
 *============================================================================*/
#ifdef PPC
   #ifdef ArchHasMaxMin
      #undef ArchHasMaxMin  /* select not supported in fko for this arch */
   #endif

#elif defined(SPARC)
   #ifdef ArchHasMaxMin
      #undef ArchHasMaxMin /* select not supported in fko for this arch */
   #endif

#elif defined(X86)
   #if !defined(ArchHasMaxMin)
      #define ArchHasMaxMin
   #endif
   
   #ifdef AVX2 /* includes AVX too*/
      #define VINT_MAX /* only for 32bit int, not for 64bit int */
      #define VINT_MIN /* only for 32bit int, not for 64bit int */
   #endif
/*
 * max/min supported in both AVX and SSE4.1
 */
   #define FP_MAX
   #define FP_MIN
   #define DP_MAX
   #define DP_MIN
   #define VFP_MAX
   #define VFP_MIN
   #define VDP_MAX
   #define VDP_MIN
#endif

#if 0         
/*=============================================================================
 * FPU PIPE LINE
 * NOTE: 
 *============================================================================*/
#ifdef PPC
#elif defined(SPARC)
#elif defined(X86)
   #if !defined(FPUPIPELINED)
      #define FPUPIPELINED
   #endif
   //#define FPPIPE 4    /* just an example, not actual value*/
   //#define DPPIPE 6
   #ifndef FPUPIPEDEF
      #define FADDPIPELEN 4
      #define DADDPIPELEN 4
      #define FMULPIPELEN 4
      #define DMULPIPELEN 4
      #define FMACPIPELEN 6
      #define DMACPIPELEN 6
   #endif
#endif
#endif

/*=============================================================================
 * REGISTER INFO
 *
 *============================================================================*/
/*
 * 1st ireg is always stack pointer.  The next NSIR iregs are registers that
 * can be used by the caller without saving, and the first such register is
 * the register return values of that type are moved to.  The remaining regs
 * may be used, but must be saved in the prologue and restored in epilogue
 * The arch.regs arrays store the actual names of the registers;  In our
 * instructions, register i's name is stored in archiregs[i-1].
 */
#define IREGBEG   1
#ifdef SPARC
   #define NIR  31                      /* # of integer regs */
   #define TNIR 32
   #define NSIR 15                      /* # of scratch iregs */
   #define NFR  32                      /* # of float regs */
   #define NSFR 32
   #define NDR  32                      /* # of double regs */
   #define NSDR 32
   #define FREGBEG  (IREGBEG+TNIR)
   #define DREGBEG  (FREGBEG+NFR)
   #define ZEROREG 32
   #define IRETREG 10
   #define FRETREG FREGBEG
   #define DRETREG DREGBEG
   #ifdef ARCH_DECLARE
      int  icallersave[TNIR] = 
       {0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0, 1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 0,0,0};
/*
 *    Note that we are using the SAVE statement, so the callee explicitly
 *    saves nothing
 */
      int  icalleesave[TNIR] = 
#if 1
     {0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 1,1,0}; 
#else
     {0, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0}; 
#endif
      int iparareg[TNIR] = {0, 0,0,0,0,0,0, 1,1,1,1,1,1,1,1,
                            0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,1,1};
      char *archiregs[TNIR] = 
         {"@sp", "@l0", "@l1", "@l2", "@l3", "@l4", "@l5", "@l6", "@l7",
          "@i0", "@i1", "@i2", "@i3", "@i4", "@i5", 
          "@o0", "@o1", "@o2", "@o3", "@o4", "@o5", "@o7",
          "@g1", "@g2", "@g3", "@g4", "g5", "g6", "g7", 
          "@i6", "@i7", "@g0"};
      int  fcalleesave[NFR] = 
       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      int  fcallersave[NFR] = 
       {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
      char *archfregs[NFR] = 
         {"@f0", "@f1", "@f2", "@f3", "@f4", "@f5", "@f6", "@f7", 
          "@f8", "@f9", "@f10", "@f11", "@f12", "@f13", "@f14", "@f15", 
          "@f16", "@f17", "@f18", "@f19", "@f20", "@f21", "@f22", "@f23", 
          "@f24", "@f25", "@f26", "@f27", "@f28", "@f29", "@f30", "@f31"
         };
      char *archdregs[NDR] = 
         {"@f0", "@f2", "@f4", "@f6", "@f8", "@f10", "@f12", "@f14", 
          "@f16", "@f18", "@f20", "@f22", "@f24", "@f26", "@f28", "@f30", 
          "@f32", "@f34", "@f36", "@f38", "@f40", "@f42", "@f44", "@f46", 
          "@f48", "@f50", "@f52", "@f54", "@f56", "@f58", "@f60", "@f62"
         };
   #else
      extern char *archiregs[TNIR], *archfregs[NFR], *archdregs[NDR];
      extern int iparareg[NIR], icalleesave[NIR], icallersave[NIR], 
                 fcalleesave[NFR], fcallersave[NFR];
   #endif
   #define dcallersave fcallersave
   #define dcalleesave fcalleesave
#endif

#ifdef X86_32
   #define IMPLICITICC 1
   #define NIR 8
   #define NSIR 3
   #define NFR   8                      /* # of float regs */
   #define TNFR  9
   #define NSFR  8
   #define TNDR  9
   #define NDR   8                      /* # of double regs */
   #define NSDR  8
   #define NVFR  8
   #define NVDR  8
   #define FREGBEG  (IREGBEG+NIR)
   #define DREGBEG  (FREGBEG+TNFR)
   #define VFREGBEG (DREGBEG+TNDR)
   #define VDREGBEG (VFREGBEG+NVFR)
/*
 * Majedul: adding VIR (vector Int reg bank)
 */
   #define NVIR 8
   #define VIREGBEG (VDREGBEG+NVDR)
   #define IRETREG 4
   #define FRETREG (TNFR+FREGBEG-1)
   #define DRETREG (TNDR+DREGBEG-1)
   #ifdef ARCH_DECLARE
      int icallersave[NIR] = {0, 1, 1, 1, 0, 0, 0, 0};
      int  icalleesave[NIR] = {0, 0, 0, 0, 1, 1, 1, 1};
      int iparareg[NIR] = {0,0,0,0,0,0,0,0};
      char *archiregs[NIR] = 
      {"@esp", "@edx", "@ecx", "@eax", "@ebp", "@ebx", "@esi", "@edi"};   
      int  fcallersave[TNFR] = {1,1,1,1,1,1,1,1, 1};
      int  fcalleesave[TNFR] = {0,0,0,0,0,0,0,0, 0};
      #ifdef AVX
         char *archfregs[TNFR] =
            {"@ymm0", "@ymm1", "@ymm2", "@ymm3", "@ymm4", "@ymm5", "@ymm6",
            "@ymm7", "@st"};
         char *archxmmregs[TNFR] =
            {"@xmm0", "@xmm1", "@xmm2", "@xmm3", "@xmm4", "@xmm5", "@xmm6",
            "@xmm7", "@st"};
      #else
         char *archfregs[TNFR] =
            {"@xmm0", "@xmm1", "@xmm2", "@xmm3", "@xmm4", "@xmm5", "@xmm6",
            "@xmm7", "@st"};
      #endif
   #else
      extern char *archiregs[NIR], *archfregs[TNFR];
      extern int iparareg[NIR], icalleesave[NIR], icallersave[NIR], 
                 fcalleesave[TNFR], fcallersave[TNFR];
   #endif
   #define archvfregs archfregs
   #define archvdregs archfregs
   #define archdregs archfregs
   #define archviregs archfregs
   #define dcallersave fcallersave
   #define dcalleesave fcalleesave
#endif

#ifdef X86_64
   #define IMPLICITICC 1
   #define IMPLICITFCC 1
   #define IMPLICITDCC 1
   #define NSR  8
   #define NIR 16
   #define NSIR 3
   #define NFR   16                      /* # of float regs */
   #define NSFR  16 
   #define NDR   16                      /* # of double regs */
   #define NSDR  16 
   #define NVDR  16
   #define NVFR  16
   #define FREGBEG  (IREGBEG+NIR)
   #define DREGBEG  (FREGBEG+NFR)
   #define VFREGBEG (DREGBEG+NDR)
   #define VDREGBEG (VFREGBEG+NVFR)
/*
 * Majedul: adding VIR (vector Int reg bank)
 */
   #define NVIR 16
   #define VIREGBEG (VDREGBEG+NVDR)
   #define IRETREG 4
   #define FRETREG FREGBEG
   #define DRETREG DREGBEG
   #ifdef ARCH_DECLARE
      char *archsregs[NSR] = 
      {"@esp", "@edx", "@ecx", "@eax", "@esi", "@edi", "@ebp", "@ebx"};
      int  icallersave[NIR] = {0, 1,1,1,1,1,0,0, 1,1,1,1, 0,0,0,0};
      int  icalleesave[NIR] = {0, 0,0,0,0,0,1,1, 0,0,0,0, 1,1,1,1};
/*
 *    Majedul: rdx and rcx can also be used for parameters
 */
      /*int iparareg[NIR] = {0,0,0,0,1,1,0,0, 1,1,0,0,0,0,0,0};*/
      int iparareg[NIR] = {0,1,1,0,1,1,0,0, 1,1,0,0,0,0,0,0};
      char *archiregs[NIR] = 
      {"@rsp", "@rdx", "@rcx", "@rax", "@rsi", "@rdi", "@rbp", "@rbx",
       "@r8", "@r9", "@r10", "@r11", "@r12", "@r13", "@r14", "@r15"};
      int  fcallersave[NFR] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
      int  fcalleesave[NFR] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      #ifdef AVX
         char *archfregs[NFR] = 
         {"@ymm0", "@ymm1", "@ymm2", "@ymm3", "@ymm4", "@ymm5", "@ymm6", "@ymm7",
         "@ymm8", "@ymm9", "@ymm10", "@ymm11", "@ymm12", "@ymm13", "@ymm14", 
         "@ymm15"};
         char *archxmmregs[NFR] = 
         {"@xmm0", "@xmm1", "@xmm2", "@xmm3", "@xmm4", "@xmm5", "@xmm6", "@xmm7",
         "@xmm8", "@xmm9", "@xmm10", "@xmm11", "@xmm12", "@xmm13", "@xmm14", 
         "@xmm15"};
      #else
         char *archfregs[NFR] = 
         {"@xmm0", "@xmm1", "@xmm2", "@xmm3", "@xmm4", "@xmm5", "@xmm6", "@xmm7",
         "@xmm8", "@xmm9", "@xmm10", "@xmm11", "@xmm12", "@xmm13", "@xmm14", 
         "@xmm15"};
      #endif
   #else
      extern char *archiregs[NIR], *archfregs[NFR], *archsregs[NSR];
      extern int iparareg[NIR], icalleesave[NIR], icallersave[NIR], 
                 fcalleesave[NFR], fcallersave[NFR];
      /*extern char *archxmmregs[NFR];*/
   #endif
   #define archvdregs archfregs
   #define archvfregs archfregs
   #define archdregs archfregs
   #define archviregs archfregs
   #define dcallersave fcallersave
   #define dcalleesave fcalleesave
   
#endif

#ifdef PPC
   #define NICC 4
   #define NFCC 4
   #define NIR  29
   #define TNIR 30
   #define NFR  32
   #define NSFR 18
   #define NDR  32
   #define NSDR 18
   #define NVFR 32
   #define NDFR 32
   #define FREGBEG  (IREGBEG+TNIR)
   #define DREGBEG  (FREGBEG+NFR)
   #define IRETREG 3
   #define FRETREG (1+FREGBEG)
   #define DRETREG (1+DREGBEG)
   #define VFREGBEG (DREGBEG+NDR)
   #ifdef LINUX_PPC
      #define NSIR  11
      #ifdef ARCH_DECLARE
         char *archiregs[TNIR] = 
            {"1", "11", "3", "4", "5", "6", "7", "8", "9", "10", "12",
             "13", "14", "15", "16", "17", "18", "19", "20", "21",
             "22", "23", "24", "25", "26", "27", "28", "29", "30", 
             "31" /*, "0" */};
         char *archfregs[NFR] = 
            {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", 
             "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", 
             "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};
         char *archvfregs[NVFR] = 
            {"v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", 
             "v9", "v10", "v11", "v12", "v13", "v14", "v14", "v15",
             "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
             "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"};
      #else
         extern char *archiregs[TNIR], *archfregs[NFR];
      #endif
   #else
      #define ICCDONE
      #define FCCDONE
      #define NSIR  10
      #ifdef ARCH_DECLARE
         char *ICCREGS[NICC] = {"cr0", "cr2", "cr3", "cr4"};
         char *FCCREGS[NFCC] = {"cr1", "cr5", "cr6", "cr7"};
         int  icallersave[TNIR] = 
          {0, 1,1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
         int  icalleesave[TNIR] = 
          {0, 0,0,0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
         int iparareg[TNIR] = {0,0,1,1,1,1,1,1,1,1,0, 0,0,0,0,0,0,0,0,0,
                               0,0,0,0,0,0,0,0,0, 0};
         char *archiregs[TNIR] = 
            {"r1", "r11", "r3", "r4", "r5", "r6", "r7", "r8", "r9","r10","r12",
             "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21",
             "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", 
             "r31"/*, "r0" */};
         int  fcallersave[NFR] =
          {1,1,1,1,1,1,1,1,1,1,1,1,1,1 ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
         int  fcalleesave[NFR] =
          {0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
         char *archfregs[NFR] = 
            {"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", 
             "f11", "f12", "f13", "f14", "f15", "f16", "f17", "f18", "f19", 
             "f20", "f21", "f22", "f23", "f24", "f25", "f26", "f27", "f28", 
             "f29", "f30", "f31"};
      #else
         extern char *archiregs[TNIR], *archfregs[NFR], 
                     *ICCREGS[NICC], *FCCREGS[NFCC];
         extern int iparareg[TNIR], icalleesave[TNIR], icallersave[TNIR], 
                    fcalleesave[NFR], fcallersave[NFR];
      #endif
   #endif
   #define archvfregs archfregs
   #define archvdregs archfregs
   #define archdregs archfregs
   #define dcallersave fcallersave
   #define dcalleesave fcalleesave
#endif
#ifdef FKO_ANSIC
   #define NIR  32
   #define NFR  32
   #define NDR  32
   #define IRETREG 3
   #define FRETREG (1+FREGBEG)
   #define DRETREG (1+DREGBEG)
   #ifdef ARCH_DECLARE
      int  icallersave[NIR] = 
       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      int  icalleesave[TNIR] = 
       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      char *archiregs[TNIR] = 
         {"ri0", "ri1", "ri2", "ri3", "ri4", "ri5", "ri6", "ri7", "ri8", 
          "ri9","ri10", "ri11", "ri12", "ri13", "ri14", "ri15", "ri16", 
          "ri17", "ri18", "ri19", "ri20", "ri21", "ri22", "ri23", "ri24", 
          "ri25", "ri26", "ri27", "ri28", "ri29", "ri30", "ri31"};
      char *archfregs[NFR] = 
         {"rf0", "rf1", "rf2", "rf3", "rf4", "rf5", "rf6", "rf7", "rf8", 
          "rf9", "rf10", "rf11", "rf12", "rf13", "rf14", "rf15", "rf16", 
          "rf17", "rf18", "rf19", "rf20", "rf21", "rf22", "rf23", "rf24",
          "rf25", "rf26", "rf27", "rf28", "rf29", "rf30", "rf31"};
      char *archdregs[NDR] = 
         {"rd0", "rd1", "rd2", "rd3", "rd4", "rd5", "rd6", "rd7", 
          "rd8", "rd9", "rd10", "rd11", "rd12", "rd13", "rd14", "rd15",
          "rd16", "rd17", "rd18", "rd19", "rd20", "rd21", "rd22", "rd23",
          "rd24", "rd25", "rd26", "rd27", "rd28", "rd29", "rd30", "rd31"};
   #else
      extern char *archiregs[TNIR], *archfregs[NFR], *archdregs[NDR];
   #endif
   #define fcallersave icallersave
   #define dcallersave icallersave
   #define archdregs archfregs
   #define dcallersave fcallersave
   #define dcalleesave fcalleesave
#endif
#ifndef TNIR
   #define TNIR NIR
#endif
#ifndef TNFR
   #define TNFR NFR
#endif
#ifndef TNDR
   #define TNDR NDR
#endif
#ifndef NICC
   #define NICC 1
#endif
#ifndef NFCC
   #define NFCC 1
#endif
#ifndef LASTREG
   #ifdef VIREGBEG   /* added at last to support T_VINT */
      #define LASTREG (VIREGBEG+NVIR)
   #elif defined(VDREGBEG)
      #define LASTREG (VDREGBEG+NVDR)
   #elif defined(VFREGBEG)
      #define LASTREG (VFREGBEG+NVFR)
   #else
      #define LASTREG (DREGBEG+TNDR)
   #endif
#endif
#ifndef ICC0
   #define ICC0 LASTREG
#endif
#ifndef FCC0
   #define FCC0 (ICC0+NICC+1)
#endif
#ifndef PCREG
   #define PCREG (FCC0 + NFCC)
#endif
#ifndef TNREG
   #define TNREG (PCREG+1)
#endif
#ifndef IREGEND
   #define IREGEND (IREGBEG + TNIR)
#endif
#ifndef FREGEND
   #define FREGEND (FREGBEG + TNFR)
#endif
#ifndef DREGEND
   #define DREGEND (DREGBEG + TNDR)
#endif
#if defined(VFREGBEG) && !defined(VFREGEND)
   #define VFREGEND (VFREGBEG + NVFR)
#endif
#if defined(VDREGBEG) && !defined(VDREGEND)
   #define VDREGEND (VDREGBEG + NVDR)
#endif
/* added to support VINT */      
#if defined(VIREGBEG) && !defined(VIREGEND)
   #define VIREGEND (VIREGBEG + NVIR)
#endif
#ifndef ICCBEG
   #define ICCBEG ICC0
#endif
#ifndef ICCEND
   #define ICCEND (ICCBEG + NICC)
#endif
#ifndef FCCBEG
   #define FCCBEG FCC0
#endif
#ifndef FCCEND
   #define FCCEND (FCCBEG + NFCC)
#endif

#ifndef ICCDONE
   #if NICC != 1
      #error "if ICCREGS is undefined, NICC must be 1!!"
   #endif
   #ifdef ARCH_DECLARE
      char *ICCREGS[1] = {"icc"};
   #else
      extern char *ICCREGS[1];
   #endif
#endif
#ifndef FCCDONE
   #if NFCC != 1
      #error "if FCCREGS is undefined, NFCC must be 1!!"
   #endif
   #ifdef ARCH_DECLARE
      char *FCCREGS[1] = {"fcc"};
   #else
      extern char *FCCREGS[1];
   #endif
#endif

/* 
 * Setup cache info
 */
#ifdef NCACHE     /* defined in fko_settings.h */
   #if NCACHE == 1
      #ifdef ARCH_DECLARE 
         short LINESIZE[NCACHE] = {CLSZ1};
      #else
         extern short LINESIZE[NCACHE];
      #endif 
   #elif NCACHE == 2
      #ifdef ARCH_DECLARE 
         short LINESIZE[NCACHE] = {CLSZ1,CLSZ2};
      #else
         extern short LINESIZE[NCACHE];
      #endif 
   #elif NCACHE == 3
      #ifdef ARCH_DECLARE 
         short LINESIZE[NCACHE] = {CLSZ1,CLSZ2, CLSZ3};
      #else
         extern short LINESIZE[NCACHE];
      #endif
   #elif NCACHE == 4
      #ifdef ARCH_DECLARE 
         short LINESIZE[NCACHE] = {CLSZ1,CLSZ2, CLSZ3, CLSZ4};
      #else
         extern short LINESIZE[NCACHE];
      #endif
   #endif
#else
   #ifdef X86_64
      #define COREI
   #endif
   #ifdef PIII
      #define NCACHE 2
      #ifdef ARCH_DECLARE
         short LINESIZE[NCACHE] = {32,32};
      #else
         extern short LINESIZE[NCACHE];
      #endif
   #elif defined(EFF)
      #define NCACHE 2
      #ifdef ARCH_DECLARE
         short LINESIZE[NCACHE] = {256,256};
      #else
         extern short LINESIZE[NCACHE];
      #endif
   #elif defined(HAMMER)
      #define NCACHE 1
      #ifdef ARCH_DECLARE
         short LINESIZE[NCACHE] = {64};
      #else
         extern short LINESIZE[NCACHE];
      #endif
   #elif defined(p4) /* P4/P4E */
      #define NCACHE 1
      #ifdef ARCH_DECLARE
         /*short LINESIZE[NCACHE] = {128};*/
         short LINESIZE[NCACHE] = {64};
      #else
         extern short LINESIZE[NCACHE];
      #endif
   #elif defined(COREI) /* corei, AMD defaults */
      #define NCACHE 3
      #ifdef ARCH_DECLARE
         short LINESIZE[NCACHE] = {64,64,64};
      #else
         extern short LINESIZE[NCACHE];
      #endif
   #else /* defaults */
      #define NCACHE 2
      #ifdef ARCH_DECLARE
         short LINESIZE[NCACHE] = {64,64};
      #else
         extern short LINESIZE[NCACHE];
      #endif
   #endif
#endif
/*
 * NOTES:
 * For P4 & P4E, all inst fetch to L2 cache, linesize = 128
 * For PIII : prefetchnta : fetch 32 bytes into L1 only
 *            prefetcht0  : fetch 32 bytes into L1 & L2
 *            prefetcht[1,2]: fetch 32 bytes into L2 only
 * For Hammer, prefetch, prefetcht[0,1,2] all same, fetch to L1
 *             prefetchw fetches to L1, but marks as modified
 *             prefetchnta is brought to L1, but if evicted, not put in L2
 *             linesize = 64.
 */

short GetReg(short type);
void GenPrologueEpilogueStubs(BBLOCK *bbase, int rsav);
int FinalizePrologueEpilogue(BBLOCK *bbase, int rsav);
short type2len(int type);
short type2shift(int type);
void FeedbackArchInfo(FILE *fpout);
int RevealArchMemUses(void);
short vtype2elem(int type);
short iName2Reg(char *rname);
short GetVecAlignByte();
#endif
