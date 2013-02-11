#include "atlas_misc.h"

int ATL_IAMAX(const int N, const TYPE *X, const int incX)
{
/*
 * call the amax inside 
 */
   TYPE ATL_UAMAX(const int N, const TYPE *X, const int incX);
   int i;
   TYPE amax, x;
   amax = ATL_UAMAX(N, X, incX);
   for (i=0; i < N; i++)
   {
      x = X[i];
      if (x < ATL_rzero) x = -x;
      if (x == amax) break;
   }
   return(i);
}
