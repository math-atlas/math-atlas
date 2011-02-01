#include "atlas_misc.h"
#include "camm_strat1.h"
void 
ATL_USCAL(int len, const SCALAR alpha, TYPE *X, const int incX) {

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
#define VERS 4
#define N Mjoin(scal_,Mjoin(VERS,c))
#else
#define VERS 4
#define N Mjoin(scal_,VERS)
#endif

#ifndef BITS
#define BITS 4
#endif
#ifndef CL
#define CL 40
#endif

#ifdef SREAL
#undef BITS
#define BITS 8
#endif
#ifdef SREAL
#undef CL
#define CL 20
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
#define BITS 8
#endif
#ifdef DCPLX
#undef CL
#define CL 20
#endif


/* #include "out.h" */
/* #include "foo.h" */


  ASM(

#if VERS >= 3
#if defined(SCPLX) || defined(DCPLX)
      pl(0,si,0)
#endif
      pls(0,cx,6)
#if defined(SCPLX) || defined(DCPLX)
#if defined(SCPLX)
      pls(4,cx,7)
#else
      pls(8,cx,7)
#endif
#endif
      ps(0,6,6)
#if defined(SCPLX) || defined(DCPLX)
      ps(0,7,7)
      pm(0,7)
#endif
#else
      pls(0,cx,0)
      ps(0,0,0)
#endif     
 
#define ALIGN
#define INC(a_) a(a_,ax)
#define LR dx
#include "camm_tpipe.h"
#undef N

      ::"a" (X), "d" (len)
#if defined(SCPLX) || defined(DCPLX)
      , "c" (alpha),"S" (w)
#else
      , "c" (&alpha)
#endif
      : "di","memory" );

}
