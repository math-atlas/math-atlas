#include <stdio.h>
/*
 * This tests whether iFKO can handle a boatload of float params
 */
main()
{
   float parastress(float d0, float d1, float d2, float d3, float d4,
                     float d5, float d6, float d7, float d8, float d9, 
                     float d10, float d11, float d12, float d13,
                     float d14);
   float d[15] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 
                   1.0, 1.1, 1.2, 1.3, 1.4};
   int k;
   float dret, dexp, diff;
   float tol = (7.0*2.0+1.0)*1.0e-6;

   dret = parastress(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], 
                     d[8], d[9], d[10], d[11], d[12], d[13], d[14]);
   for (dexp=0.0,k=0; k < 15; k++) dexp += d[k];
   diff = dexp - dret;
   if (diff < 0.0) diff = -diff;
   if (diff <= tol)
      fprintf(stdout, "3H: SPARA STRESS PASSED\n");
   else
      fprintf(stdout, "3H: SPARA STRESS FAILED: dret=%f, dexp=%f, diff=%e\n",
              dret, dexp, diff);
   exit(!(diff <= tol));
}
