#include "math.h"

#define TYPE double
#define ATL_rzero 0.0
#define ATL_rone 1.0
#define Mmax(x, y) ( (x) > (y) ? (x) : (y) ) 
#define fabs(x) ( (x) >= 0.0 ? (x) : -(x) )

#define GetMax8(ptr_, xmax_) \
{ \
   const TYPE *x = (ptr_); \
   register TYPE r0=fabs(*x),r1=fabs(x[1]),r2=fabs(x[2]),r3=fabs(x[3]), \
                 r4=fabs(x[4]),r5=fabs(x[5]),r6=fabs(x[6]),r7=fabs(x[7]); \
   r0 = Mmax(r0,r1); \
   r2 = Mmax(r2,r3); \
   r4 = Mmax(r4,r5); \
   r6 = Mmax(r6,r7); \
   r0 = Mmax(r0,r2); \
   r4 = Mmax(r4,r6); \
   xmax_ = Mmax(r0,r4); \
}

TYPE ATL_UNRM2(int N, const TYPE *X, int incX)
{
   
   void ATL_USSQ(const int N, const TYPE *X, const int incX, const TYPE *scal0, 
                 const TYPE *ssq0);
   
   TYPE ssq=ATL_rzero, scal=ATL_rzero;
/*
 * Find a non-zero initial scale from beginning of vector
 */
   if (N <= 24)
   {
      register int i;
      if (N == 1)
         return(fabs(*X));
      else if (N < 1)
         return(ATL_rzero);
      for (i=0; i < N; i++)
      {
         const register TYPE ax = fabs(X[i]);
         scal = (scal >= ax) ? scal : ax;
      }
      if (scal == ATL_rzero)
         return(ATL_rzero);
      for (i=0; i < N; i++)
      {
         const register TYPE ax = fabs(X[i]);
         register TYPE t0 = ax / scal;
         ssq += t0 * t0;
      }
      return(scal * sqrt(ssq));
   }
   else
   {
      TYPE s2;
      register int i=0;
/*
 *    Find a non-zero scale factor from start of vector
 */
      do
      {
         GetMax8(X+i, scal);
         i += 8;
      }
      while (scal == ATL_rzero && N-i >= 8);
/*
 *    Even with only 0-7 elts left, everything was zero
 */
      if (scal == ATL_rzero)
      {
         const int i0 = i;
         for (; i < N; i++)
         {
            s2 = fabs(X[i]);
            scal = Mmax(scal, s2);
         }
         if (scal == ATL_rzero)
            return(ATL_rzero);
         for (i=i0; i < N; i++)
         {
            const register TYPE ax = fabs(X[i]);
            register TYPE t0 = ax / scal;
            ssq += t0 * t0;
         }
         return(scal * sqrt(ssq));
      }
/*
 *    Scope end of vector to improve scaling bet for structured data
 */
      GetMax8(X+N-8, s2);
      scal = Mmax(scal, s2);
      ATL_USSQ(N, X, incX, &scal, &ssq);
      return(scal * sqrt(ssq));
   }
}
