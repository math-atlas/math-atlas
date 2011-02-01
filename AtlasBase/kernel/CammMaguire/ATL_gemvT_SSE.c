#ifndef ATL_GAS_x8632
   #error "This kernel requires gas x86-32 assembler!"
#endif
#ifdef SREAL
   #define STRIDE 1
   #define LUNROLL 32
   #define NDPM 2
/*    #define ALIGN */
#endif

#ifdef SCPLX
   #define STRIDE 1
   #define LUNROLL 32
   #define NDPM 2
#endif

#ifdef DREAL
#ifdef ATL_SSE2
   #define STRIDE 2
   #define ALIGN
   #define NDPM 2
   #define PREFETCH 256
   #define LUNROLL 32
#else
   #define STRIDE 20
/*    #define PREFETCH 48 */
   #define LUNROLL 32
   #define NDPM 2
#endif
#endif

#ifdef DCPLX
#ifdef ATL_SSE2
   #define STRIDE 4
   #define PREFETCH 128
   #define LUNROLL 8
   #define NDPM 3
#else
   #define STRIDE 10
   #define LUNROLL 16
   #define NDPM 2
#endif
#endif

#include "ATL_gemv_ger_SSE.h"

