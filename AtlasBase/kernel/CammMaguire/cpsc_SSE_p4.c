#include "atlas_misc.h"
#include "camm_strat1.h"
#include <math.h>
void ATL_UCPSC(int len, const SCALAR alpha, const TYPE *X, const int incx,
               TYPE *Y, const int incy)
{
  NO_INLINE;
#if defined(SCPLX)
  const TYPE w[2]={{-1.0,1.0},{-1.0,1.0}};
#endif
#if defined(DCPLX)
  const TYPE w[1]={{-1.0,1.0}};
#endif

#ifndef SREAL
  len+=len;
#endif
#ifdef DCPLX
  len+=len;
#endif

#if defined(SCPLX) || defined(DCPLX)
#define VERSION 3
#define N Mjoin(cpsc_,Mjoin(VERSION,c))
#else
#define VERSION 5
#define N Mjoin(cpsc_,VERSION)
#endif


#ifndef BITS
#define BITS 6
#endif
#ifndef CL
#define CL 20
#endif

#ifdef SREAL
#undef BITS
#define BITS 6
#endif
#ifdef SREAL
#undef CL
#define CL 22
#endif
#ifdef DREAL
#undef BITS
#define BITS 7
#endif
#ifdef DREAL
#undef CL
#define CL 28
#endif
#ifdef SCPLX
#undef BITS
#define BITS 5
#endif
#ifdef SCPLX
#undef CL
#define CL 28
#endif
#ifdef DCPLX
#undef BITS
#define BITS 7
#endif
#ifdef DCPLX
#undef CL
#define CL 28
#endif

/* #include "out.h" */
/* #include "foo.h" */



  ASM(

#if VERSION == 3 || VERSION == 4 || VERSION == 5
      pls(0,di,6)
      ps(0,6,6)
#if defined(SCPLX) || defined(DCPLX)
#if defined(SCPLX)
      pls(4,di,7)
#else
      pls(8,di,7)
#endif
      pl(0,si,0)
      ps(0,7,7)
      pm(0,7)
#endif
#else
      pls(0,di,3)
      ps(0,3,3)
#endif

#define ALIGN
#define INC(a_) a(a_,ax) a(a_,cx)
#define LR dx
#include "camm_tpipe.h"
#undef N

#if VERSION == 4 || VERSION == 5
      ::"c" (X),"a" (Y), "d" (len)
#else
      ::"a" (X),"c" (Y), "d" (len)
#endif
#if defined(SCPLX) || defined(DCPLX)
      , "D" (alpha),"S" (w)
#else
      , "D" (&alpha)
#endif
      : "memory" );

}
