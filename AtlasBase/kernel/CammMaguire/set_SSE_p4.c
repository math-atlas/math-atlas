#include "atlas_misc.h"
#include "camm_strat1.h"
void ATL_USET(int len, const SCALAR alpha, TYPE *X, const int incX)
{
  NO_INLINE;

#ifndef SREAL
  len+=len;
#endif
#ifdef DCPLX
  len+=len;
#endif


#define VERS 3
#define N Mjoin(set_,VERS)

#ifndef BITS
#define BITS 4
#endif
#ifndef CL
#define CL 24
#endif

#ifdef SREAL
#undef BITS
#define BITS 3
#endif
#ifdef DREAL
#undef BITS
#define BITS 3
#endif
#ifdef SCPLX
#undef BITS
#define BITS 3
#endif
#ifdef DCPLX
#undef BITS
#define BITS 3
#endif

/* #include "out.h" */
/* #include "foo.h" */

  ASM(


#if defined(SREAL) || defined(DREAL)
      pls(0,cx,0)
      ps(0,0,0)
#elif defined(SCPLX)
      pld(0,cx,0)
      plh(0,0)
#else
      pl(0,cx,0)
#endif
	      
      
#define ALIGN
#define INC(a_) a(a_,ax)
#define LR dx
#include "camm_tpipe.h"
#undef N


      ::"a" (X),
#if defined(SCPLX) || defined(DCPLX)
      "c" (alpha), 
#else
      "c" (&alpha), 
#endif
      "d" (len)
      : "di","memory" );

}
