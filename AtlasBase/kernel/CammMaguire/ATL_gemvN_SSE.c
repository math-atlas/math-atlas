#ifndef ATL_GAS_x8632
   #error "This kernel requires gas x86-32 assembler!"
#endif
#define NO_TRANSPOSE

#ifdef SREAL
   #ifndef ATL_SSE1
      #error This kernel requires ATL_SSE1 to be defined
   #endif
   #define STRIDE 1
   #define LUNROLL 16
   #define NDPM 2
/*    #define ALIGN */
#endif

#ifdef SCPLX
   #ifndef ATL_SSE1
      #error This kernel requires ATL_SSE1 to be defined
   #endif
   #define STRIDE 1
   #define LUNROLL 32
   #define NDPM 2
#endif

#ifdef DREAL
#ifdef ATL_SSE2
   #define STRIDE 4
   #define PREFETCH 1024
   #define LUNROLL 32
   #define NDPM 3
   #define ALIGN
#else
   #define STRIDE 20
   #define PREFETCH 64
   #define LUNROLL 32
   #define NDPM 2
#endif
#endif

#ifdef DCPLX
#ifdef ATL_SSE2
   #define STRIDE 1
   #define PREFETCH 512
   #define LUNROLL 16
   #define NDPM 3
#else
   #define STRIDE 10
   #define LUNROLL 8
   #define NDPM 2
#endif
#endif

#include "ATL_gemv_ger_SSE.h"


