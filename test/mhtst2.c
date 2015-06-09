#include <stdio.h>
/*
 * This tests whether iFKO can do a simple dot product
 */
main()
{
   double parastress(int i0, double d0, int i1, int i2, double d1, int i3,
              double d2, double d3, double d4, int i4, int i5, int i6,
              double d5, int i7, double d6, double d7, int i8, double d8,
              double d9, int i9, int i10, int i11, int i12, int i13, int i14,
              double d10, double d11, double d12, double d13, double d14int,
              int *ires);
   double d[15] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 
                   1.0, 1.1, 1.2, 1.3, 1.4};
   int i[15] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
   int k;
   double dret, dexp, d0, dtol=30e-16;
   int iret, iexp;

   dret = parastress(i[0], d[0], i[1], i[2], d[1], i[3], d[2], d[3], d[4],
                     i[4], i[5], i[6], d[5], i[7], d[6], d[7], i[8], d[8], 
                     d[9], i[9], i[10], i[11], i[12], i[13], i[14],
                     d[10], d[11], d[12], d[13], d[14], &iret);
   for (dexp=0.0,k=0; k < 15; k++) dexp += d[k];
   for (iexp=k=0; k < 15; k++) iexp += i[k];
   d0 = dexp - dret;
   if (d0 < 0.0) 
      d0 = -d0;
   if (d0 <= dtol && iexp == iret)
      fprintf(stdout, "2H: IDPARA STRESS PASSED\n");
   else
      fprintf(stdout, 
         "2H: IDPARA STRESS FAILED: iret=%d, iexp=%d, dret=%f, dexp=%f (%e)\n",
              iret, iexp, dret, dexp, d0);
   exit(!(dret == dexp && iret == iexp));
}
