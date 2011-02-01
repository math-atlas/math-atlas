#include "atlas_misc.h"
#include "camm_strat1.h"
#include <math.h>
void ATL_UAXPBY(int len, const SCALAR alpha, const TYPE *X, const int incX,
                const SCALAR beta, TYPE *Y, const int incY)
{
  NO_INLINE;
#if defined(SCPLX)
  TYPE w[3]={{0.0,0.0},{-1.0,1.0},{-1.0,1.0}};
#endif
#if defined(DCPLX)
  TYPE w[2]={{0.0,0.0},{-1.0,1.0}};
#endif

#ifndef SREAL
  len+=len;
#endif
#ifdef DCPLX
  len+=len;
#endif

#define VERS 3
#if defined(SCPLX) || defined(DCPLX)
#define N Mjoin(axpby_,Mjoin(VERS,c))
#else
#define N Mjoin(axpby_,VERS)
#endif


#ifndef BITS
#define BITS 5
#endif
#ifndef CL
#define CL 24
#endif

#ifdef SREAL
#undef BITS
#define BITS 4
#endif
#ifdef SREAL
#undef CL
#define CL 24
#endif
#ifdef DREAL
#undef BITS
#define BITS 5
#endif
#ifdef DREAL
#undef CL
#define CL 26
#endif
#ifdef SCPLX
#undef BITS
#define BITS 7
#endif
#ifdef SCPLX
#undef CL
#define CL 26
#endif
#ifdef DCPLX
#undef BITS
#define BITS 6
#endif
#ifdef DCPLX
#undef CL
#define CL 28
#endif

/* #include "out.h" */
/* #include "foo.h" */

#if defined(SCPLX) || defined(DCPLX)
  w[0]=*beta;
#endif

  ASM(

      pls(0,si,5)
      ps(0,5,5)
      pls(0,di,6)
      ps(0,6,6)
#if defined(SCPLX) || defined(DCPLX)
#if defined(SCPLX)
      pls(4,di,7)
      pls(4,si,4)
      pl(8,si,0)
#else
      pls(8,di,7)
      pls(8,si,4)
      pl(16,si,0)
#endif
      ps(0,7,7)
      ps(0,4,4)
      pm(0,7)
      pm(0,4)
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
      , "D" (&alpha), "S" (&beta)
#endif
      : "memory" );

}
