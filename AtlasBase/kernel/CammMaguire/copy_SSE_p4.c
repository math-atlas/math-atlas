#include "atlas_misc.h"
#include "camm_strat1.h"
#include <math.h>
void ATL_UCOPY(int len, const TYPE *X, const int incX,
               TYPE *Y, const int incY)
{
  NO_INLINE;

#ifndef SREAL
  len+=len;
#endif
#ifdef DCPLX
  len+=len;
#endif

#define VERS 3
#define N Mjoin(copy_,VERS)


#ifndef BITS
#define BITS 5
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
#define CL 24
#endif
#ifdef DREAL
#undef BITS
#define BITS 5
#endif
#ifdef DREAL
#undef CL
#define CL 28
#endif



/* #include "out.h" */
/* #include "foo.h" */

  ASM(

#define ALIGN
#define INC(a_) a(a_,ax) a(a_,cx)
#define LR dx
#include "camm_tpipe.h"
#undef N

#if VERS == 3
      ::"c" (X),"a" (Y), "d" (len)
#else
      ::"a" (X),"c" (Y), "d" (len)
#endif
      : "di","memory" );

}
