#include <stdio.h>
/*
 * This tests whether iFKO can do a simple dot product
 */
main()
{
   double dot(int N, double *X, double *Y);
   double X[5] = {1.0, -2.0, 3.0, 4.0, -5.0};
   double Y[5] = {0.8, 9.2, -0.5, 8.7, 12.2};
   int i;
   double dret, dexp;

   dret = dot(5, X, Y);
   for (dexp=0.0,i=0; i < 5; i++) dexp += X[i] * Y[i];
   if (dret == dexp) fprintf(stdout, "7d: DDOT PASSED\n");
   else fprintf(stderr, "7d: DDOT FAILED: got=%f, expected=%f\n",
                dret, dexp);
   exit(!(dret == dexp));
}
