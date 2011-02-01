#include "atlas_misc.h"
#include "camm_strat1.h"
#include <math.h>
TYPE ATL_UASUM(int len, const TYPE *X, const int incX)
/*
 * Only machines like x86 with extended precision (both arithmetic and sqrt)
 * will be able to use this kernel.  On machines with standard 64 bit
 * precision, this will fail the overflow/underflow tests.
 */
{
  TYPE res,j=-0.0;
#if defined (SREAL) || defined(SCPLX)
  int i=( *(int *)&j ^ -1);
#endif
#if defined (DREAL)
  long long i=( *(long long *)&j ^ -1);
#endif
  NO_INLINE;

#ifndef SREAL
  len+=len;
#endif
#ifdef DCPLX
  len+=len;
#endif

#define VERS 1
#define N Mjoin(0x1_asum_,VERS)

#ifndef BITS
#define BITS 6
#endif
#ifndef CL
#define CL 60
#endif

#ifdef DREAL
#undef BITS
#define BITS 6
#endif
#ifdef DREAL
#undef CL
#define CL 56
#endif
#ifdef SREAL
#undef BITS
#define BITS 6
#endif
#ifdef SREAL
#undef CL
#define CL 40
#endif


/* #include "out.h" */
/* #include "foo.h" */

  ASM(


      px(0)
      pls(0,di,4)
      ps(0,4,4)

#define ALIGN
#define INC(a_) a(a_,ax)
#define LR dx
#include "camm_tpipe.h"
#undef N

#ifdef SREAL
      px(1)
      phl(0,1)
      pa(1,0)
#endif
      pc(0,2)
      ps(1,2,2)
      pasr(2,0)
      pus(0,0,cx)


      ::"a" (X),"c" (&res), "d" (len), "D" (&i)
      : "memory" );
  return res;

}
