#ifndef FKO_SETTINGS_H
   #define FKO_SETTINGS_H 1
/*
 * set machine architecture
 */
#define LINUX_X86_64    
/* #define LINUX_X86_32 */
/* #define SOLARIS_SPARC  */
/* #define OSX_PPC  */
/*
 * Describe the cache information if you know it below.  Fill in a linesize
 * macro for each level given in NCACHE.  
 * On intel machines, guess 64 for any level you don't know.
 */
#define NCACHE 2   /* number of lvls of cache iFKO knows about */
#define CLSZ1 64   /* linesize in bytes of first lvl of cache */
#define CLSZ2 64   /* linesize in bytes of second lvl of cache */
/* #define CLSZ3 128 */
/*
 * Uncomment next line if the FPU can perform an floating point and
 * accumulate (FMAC) without penalty
 */
#define ARCH_HAS_MAC 1 
/*
 * Uncomment any definition where you know the pipelen for your architecture
 * for that type (including vector types), and set the 0 to the number of
 * latency stages.  The FPU is unpipelined, make it -1.  
 * If you are unsure, leave it undefined.
 */
#define FPU 1
#if 0
   #define PIPELEN_FADD  0
   #define PIPELEN_FMUL  0
   #define PIPELEN_FDIV  0
   #define PIPELEN_DADD  0
   #define PIPELEN_DMUL  0
   #define PIPELEN_DDIV  0
   #define PIPELEN_IDIV  0
   #ifdef ARCH_HAS_MAC
      #define PIPELEN_FMAC  0
      #define PIPELEN_DMAC  0
      #define PIPELEN_IMAC  0
   #endif
/*
 * Piplength defines for vector types
 */
   #define PIPELEN_VIDIV 0
   #define PIPELEN_VFADD 0
   #define PIPELEN_VFMUL 0
   #define PIPELEN_VFDIV 0
   #define PIPELEN_VDADD 0
   #define PIPELEN_VDMUL 0
   #define PIPELEN_VDDIV 0
   #ifdef ARCH_HAS_MAC
      #define PIPELEN_VFMAC 0
      #define PIPELEN_VDMAC 0
      #define PIPELEN_VIMAC 0
   #endif
#endif
/*
 * SSE1 or SSE2 not sufficient.  Need SSE3 for vertical add.
 * If overridng vector type here, (eg, AVX2), also define VECDEF
 */
#define VECDEF 1
   /*#define AVX2/AVX/SSE41/SSE3*/
#define AVX2


#endif
