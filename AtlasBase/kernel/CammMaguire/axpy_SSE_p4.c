#include "atlas_misc.h"
#include "camm_strat1.h"
#include <math.h>
void ATL_UAXPY(int len, const SCALAR alpha, const TYPE *X, const int incX,
               TYPE *Y, const int incY)
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
#define VERS 2c
#else
#define VERS 2
#endif
#define N Mjoin(axpy_,VERS)


#ifndef BITS
#define BITS 5
#endif
#ifndef CL
#define CL 24
#endif

#ifdef SREAL
#undef BITS
#define BITS 6
#endif
#ifdef SREAL
#undef CL
#define CL 24
#endif
#ifdef DREAL
#undef BITS
#define BITS 8
#endif
#ifdef DREAL
#undef CL
#define CL 24
#endif
#ifdef SCPLX
#undef BITS
#define BITS 8
#endif
#ifdef SCPLX
#undef CL
#define CL 20
#endif
#ifdef DCPLX
#undef BITS
#define BITS 4
#endif
#ifdef DCPLX
#undef CL
#define CL 20
#endif

/* #include "out.h" */
/* #include "foo.h" */

  ASM(

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

#define ALIGN
#define INC(a_) a(a_,ax) a(a_,cx)
#define LR dx
#include "camm_tpipe.h"
#undef N

      ::"a" (Y),"c" (X), "d" (len)
#if defined(SCPLX) || defined(DCPLX)
      , "D" (alpha),"S" (w)
#else
      , "D" (&alpha)
#endif
      : "memory" );

}
