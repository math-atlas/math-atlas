#include <stdio.h>
/*
 * This tests whether iFKO can do a simple dot product
 */
main()
{
   float dot(int N, float *X, float *Y);
   float X[5] = {1.0, -2.0, 3.0, 4.0, -5.0};
   float Y[5] = {0.8, 9.2, -0.5, 8.7, 12.2};
   int i;
   float dret, dexp, diff, tol=5e-6;

   dret = dot(5, X, Y);
   for (dexp=0.0,i=0; i < 5; i++) dexp += X[i] * Y[i];
   diff = dret - dexp;
   if (diff < 0.0) diff = -diff;
   if (diff <= tol) fprintf(stdout, "7s: SDOT PASSED\n");
   else fprintf(stderr, "7s: SDOT FAILED: got=%f, expected=%f, diff=%f\n",
                dret, dexp, diff);
   exit(!(dret <= dexp));
}
