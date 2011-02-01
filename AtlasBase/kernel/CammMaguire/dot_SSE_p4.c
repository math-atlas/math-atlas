#include "atlas_misc.h"
#include "camm_strat1.h"
#include <math.h>
#if defined(SCPLX) || defined(DCPLX)
void
#else
TYPE
#endif
ATL_UDOT(int len, const TYPE *X, const int incx,
	 const TYPE *Y, const int incy
#if defined(SCPLX) || defined(DCPLX)
	 ,SCALAR dot
#endif
	 )
{
#if defined(SREAL) || defined(DREAL)
  TYPE dot;
#endif
#if defined(SCPLX)
#ifdef Conj_
  const TYPE w[2]={{-1.0,1.0},{-1.0,1.0}};
#else
  const TYPE w[2]={{1.0,-1.0},{1.0,-1.0}};
#endif
#endif
#if defined(DCPLX)
#ifdef Conj_
  const TYPE w[1]={{-1.0,1.0}};
#else
  const TYPE w[1]={{1.0,-1.0}};
#endif
#endif
  NO_INLINE;

#ifndef SREAL
  len+=len;
#endif
#ifdef DCPLX
  len+=len;
#endif

#undef SHUFEND
#if defined(SCPLX) || defined(DCPLX)
#define VERS 2c
#define SHUFEND
#else
#define VERS 1
#endif
#define N Mjoin(dot_,VERS)


#ifndef BITS
#define BITS 5
#endif
#ifndef CL
#define CL 40
#endif

#ifdef SREAL
#undef BITS
#define BITS 6
#endif
#ifdef SREAL
#undef CL
#define CL 40
#endif
#ifdef DREAL
#undef BITS
#define BITS 6
#endif
#ifdef DREAL
#undef CL
#define CL 44
#endif
#ifdef SCPLX
#undef BITS
#define BITS 3
#endif
#ifdef SCPLX
#undef CL
#define CL 48
#endif
#ifdef DCPLX
#undef BITS
#define BITS 8
#endif
#ifdef DCPLX
#undef CL
#define CL 40
#endif

/* #include "out.h" */
/* #include "foo.h" */

  ASM(

      px(0)
#if defined(SCPLX) || defined(DCPLX)
      px(6)
      pl(0,di,7)
#endif

#define ALIGN
#define INC(a_) a(a_,ax) a(a_,cx)
#define LR dx
#include "camm_tpipe.h"
#undef N

#if defined(SCPLX) || defined(DCPLX)

#if defined(SHUFEND)

#ifdef Conj_
      pm(7,6)
#else
      pm(7,0)
#endif

#undef ISHUF
#undef RSHUF
#if defined(SCPLX)
#define ISHUF 13*17
#define RSHUF 8*17

      pc(0,5)
      plh(6,0)
      phl(5,6)

#else

#define ISHUF 3
#define RSHUF 0

#endif

      pc(0,5)
      ps(ISHUF,6,5)
      ps(RSHUF,6,0)
      pa(5,0)

#else

#if defined(SCPLX) || defined(DCPLX)
      ps(CSHUF,6,6)
      pa(6,0)
#endif

#endif
#endif

#if defined(SREAL) || defined(SCPLX)
      px(1)
      phl(0,1)
      pa(1,0)
#endif
#if defined(SREAL) || defined(DREAL)
      pc(0,2)
      ps(1,2,2)
      pasr(2,0)
      pus(0,0,si)
#elif defined(SCPLX)
      pud(0,0,si)
#else
      pu(0,0,si)
#endif

      ::"a" (X),"c" (Y), "d" (len)
#if defined(SCPLX) || defined(DCPLX)
      , "S" (dot), "D" (w)
      : "memory" );
#else
      , "S" (&dot)
      : "di","memory" );
#endif
#if defined(SREAL) || defined(DREAL)
  return dot;
#endif

}
