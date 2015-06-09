#include <stdio.h>
/*
 * This tests whether iFKO can do a simple dot product
 */
main()
{
   double parastress(double d0, double d1, double d2, double d3, double d4,
                     double d5, double d6, double d7, double d8, double d9, 
                     double d10, double d11, double d12, double d13,
                     double d14);
   double d[15] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 
                   1.0, 1.1, 1.2, 1.3, 1.4};
   int k;
   double dret, dexp, diff;
   double tol = (7.0*2.0+1.0)*1.0e-15;

   dret = parastress(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], 
                     d[8], d[9], d[10], d[11], d[12], d[13], d[14]);
   for (dexp=0.0,k=0; k < 15; k++) dexp += d[k];
   diff = dexp - dret;
   if (diff < 0.0) diff = -diff;
   if (diff <= tol)
      fprintf(stdout, "1H: DPARA STRESS PASSED\n");
   else
      fprintf(stdout, "1H: DPARA STRESS FAILED: dret=%f, dexp=%f, diff=%e\n",
              dret, dexp, diff);
   exit(!(diff <= tol));
}
