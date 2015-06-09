#ifndef FKO_SETTINGS_H
   #define FKO_SETTINGS_H 1
/*
 * Choose one of the supported assemblies/ABIs, default will be Linux/AMD64.
 * If you manually set one of these, set ARCH_ as well so default won't be used.
 */
#if 0
   #define LINUX_X86_64 1
   #define OSX_PPC
   #define LINUX_X86_64 1
   #define SOLARIS_SPARC
   #define LINUX_X86_64 1
   #define LINUX_X86_32
   #define ARCH_ 1  /* uncomment this line, if you uncomm earlier line */
#endif
/*
 * If arch isn't defined, scope if ATLAS's flags are being used, else default
 */
#ifndef ARCH_
   #if !defined(LINUX_X86_64) && !defined(LINUX_X86_32) && \
       !defined(SOLARIS_SPARC) && !defined(OSX_PPC)
      #ifdef ATL_OS_Linux
         #ifdef ATL_GAS_x8632
            #define LINUX_X86_32 1
         #else
            #define LINUX_X86_64 1
         #endif
      #else
         #define LINUX_X86_64 1
      #endif
   #endif
   #define ARCH_ 1
#endif
#if defined(LINUX_X86_64) || defined(LINUX_X86_32)
   #define X86 1
#else 
   #define X86 0
#endif

/*
 * For x86, define what SIMD vector support you've got.  Presently must have
 * at least SSE3, though past fko worked fine with SSE2.
 */
#if X86
      #define SSE3 1
   #if 0
      #define SSE41 1
      #define AVX 1
      #define AVX2 1
   #endif

   #ifdef SSE3
      #define VECDEF 1
   #endif
   #ifdef SSE41
      #define VECDEF 1
   #endif
   #ifdef AVX
      /*#define ARCH_HAS_MAC 1 */ /*In some AMD processors, like: FX-4100 */
      /*#define FMA4 1 */ /* In some AMD processors, like: FX-4100 */
      #define VECDEF 1
   #endif
   #ifdef AVX2
      #define ARCH_HAS_MAC 1
      #define VECDEF 1
   #endif
/* 
 * For default, see if we've got ATLAS-style defs, else default to AVX2
 * This is newest that iFKO supports, and will run fastest.  It will error
 * on old machine, and the installer should be alerted to fix!
 */
   #ifndef VECDEF
      #if defined(ATL_AVXMAC) || defined(ATL_AVXFMA4)
         #define AVX2 1
         #define ARCH_HAS_MAC 1
         #if defined(ATL_AVXFMA4)
            #define FMA4 1
         #endif
      #elif defined(ATL_AVX)
         #define AVX 1
      #elif defined(ATL_SSE3)
         #define SSE3
      #else
         #define AVX2 1
      #endif
      #define VECDEF 1
   #endif
#endif
/*
 * If you know it, can set the pipeline length of various FPU operations
 * to limit the need to search.  This can also be auto-detected by FKO,
 * UPDATE WITH HOW.
 */
#ifndef PIPELINE_VDMAC
   #define PIPELINE_VDMAC 0
#endif
#ifndef PIPELINE_VFMAC
   #define PIPELINE_VFMAC 0
#endif
#ifndef PIPELINE_DMAC
   #define PIPELINE_DMAC 0
#endif
#ifndef PIPELINE_FMAC
   #define PIPELINE_FMAC 0
#endif
#ifndef PIPELINE_VDMUL
   #define PIPELINE_VDMUL 0
#endif
#ifndef PIPELINE_VFMUL
   #define PIPELINE_VFMUL 0
#endif
#ifndef PIPELINE_DMUL
   #define PIPELINE_DMUL 0
#endif
#ifndef PIPELINE_FMUL
   #define PIPELINE_FMUL 0
#endif
#ifndef PIPELINE_VDADD
   #define PIPELINE_VDADD 0
#endif
#ifndef PIPELINE_VFADD
   #define PIPELINE_VFADD 0
#endif
#ifndef PIPELINE_DADD
   #define PIPELINE_DADD 0
#endif
#ifndef PIPELINE_FADD
   #define PIPELINE_FADD 0
#endif
/*
 * Present searches don't care at all about these settings
 */
#ifndef  PIPELINE_VIMAC
   #define PIPELINE_VIMAC 0
#endif
#ifndef  PIPELINE_VIMAC
   #define PIPELINE_VIMAC 0
#endif
#ifndef  PIPELINE_VIMUL
   #define PIPELINE_VIMUL 0
#endif
#ifndef  PIPELINE_VIMUL
   #define PIPELINE_VIMUL 0
#endif

#endif  /* end multiple inclusion guard */