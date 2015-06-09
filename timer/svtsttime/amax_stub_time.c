#include "atlas_misc.h"

int ATL_IAMAX(const int N, const TYPE *X, const int incX)
{
/*
 * for timing result doesn't matter... for 
 */
   TYPE ATL_UAMAX(const int N, const TYPE *X, const int incX);
   TYPE amax;
   amax = ATL_UAMAX(N,X,incX);
   return((int)amax);
}
