#include <stdio.h>
/*
 * This tests whether iFKO can do a simple dot product
 */
main()
{
   double parastress(int i0, double d0, int i1, int i2, double d1, int i3,
              double d2, double d3, double d4, int i4, int i5, int i6,
              double d5, int i7, double d6, double d7, int i8, double d8,
              double d9, int i9, int *ires);
   double d[10] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
   double i[10] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9};
   int k;
   double dret, dexp;
   int iret, iexp;

   dret = parastress(i[0], d[0], i[1], i[2], d[1], i[3], d[2], d[3], d[4],
                     i[4], i[5], i[6], d[5], i[7], d[6], d[7], i[8], d[8], d[9],
                     i[9], &iret);
   for (dexp=0.0,k=0; k < 10; k++) dexp += d[k];
   for (iexp=k=0; k < 10; k++) iexp += i[k];
   if (dret == dexp && iexp == iret)
      fprintf(stdout, "7d: SDPARA STRESS PASSED\n");
   else
      fprintf(stdout, 
              "1H: SDPARA STRESS FAILED: iret=%d, iexp=%d, dret=%f, dexp=%f\n",
              iret, iexp, dret, dexp);
   exit(!(dret == dexp && iret == iexp));
}
