#include <stdio.h>
/*
 * This tests whether iFKO can do a simple dot product
 */
int iamax_good(int N, double *X)
{
   int i, imax=0;
   double d, max=0.0;
   for (i=0; i < N; i++)
   {
      d = X[i];
      if (d < 0.0) d = -d;
      if (d > max)
      {
         imax = i;
         max = d;
      }
   }
   return(imax);
}
main()
{
   int iamax(int N, double *X);
   double X[5] = {1.0, -2.0, -5.0, 4.0, 5.0};
   double Y[5] = {0.8, 9.2, -0.5, 8.7, 2.2};
   int i, iexp0, iexp1, iret0, iret1;
   double dret, dexp;

   iret0 = iamax(5, X);
   iret1 = iamax(5, Y);
   iexp0 = iamax_good(5, X);
   iexp1 = iamax_good(5, Y);
   if (iret0 == iexp0 && iret1 == iexp1)
      fprintf(stdout, "11d: IAMAX PASSED\n");
   else fprintf(stderr, "11d: IAMAX FAILED: got=(%d,%d), expected=(%d,%d)\n",
                iret0, iret1, iexp0, iexp1);
   exit(!(iret0 == iexp0 && iret1 == iexp1));
}
