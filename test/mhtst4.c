#include <stdio.h>
/*
 * This tests whether iFKO can do a simple dot product
 */
main()
{
   double parastress(int i0, double d0, float f0, int i1, float f1, int i2,
                     double d1, int i3, double d2, float f2, double d3,
                     double d4, int i4, int i5, int i6, float f3, float f4,
                     float f5, double d5, int i7, double d6, double d7, int i8,
                     double d8, double d9, int i9, int i10, float f6, float f7,
                     int i11, int i12, int i13, int i14, double d10,
                     double d11, double d12, double d13, double d14, 
                     float f8, float f9, float *fret, int *ires);
   double d[15] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 
                   1.0, 1.1, 1.2, 1.3, 1.4};
   float  f[15] = {2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 
                   3.0, 3.1, 3.2, 3.3, 3.4};
   int i[15] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
   float fret, fexp, fdiff, ftol = 10e-6;
   double diff, tol=15e-16;
   int k;
   double dret, dexp;
   int iret, iexp;

   dret = parastress(i[0], d[0], f[0], i[1], f[1],i[2], d[1], i[3], d[2], f[2],
                     d[3], d[4], i[4], i[5], i[6], f[3], f[4], f[5], d[5],
                     i[7], d[6], d[7], i[8], d[8], d[9], i[9], i[10], 
                     f[6], f[7], i[11], i[12], i[13], i[14], d[10], d[11],
                     d[12], d[13], d[14], f[8], f[9], &fret, &iret);
   for (fexp=0.0,k=0; k < 10; k++) fexp += f[k];
   for (dexp=0.0,k=0; k < 15; k++) dexp += d[k];
   for (iexp=k=0; k < 15; k++) iexp += i[k];
   diff = dret - dexp;
   fdiff = fret - fexp;
   if (diff < 0.0) diff = -diff;
   if (fdiff < 0.0) fdiff = -fdiff;
   if (diff <= tol && fdiff <= ftol && iexp == iret)
      fprintf(stdout, "4H: IFDPARA STRESS PASSED\n");
   else
   {
      fprintf(stdout, 
"4H: IFDPARA STRESS FAILED: iret=%d, iexp=%d, dret=%f, dexp=%f, diff=%e\n",
              iret, iexp, dret, dexp);
      fprintf(stdout, 
              "                           fret=%f, fexp=%f, fdiff=%e\n",
              fret, fexp, fdiff);
   }
   exit(!(diff <= tol && fdiff <= ftol && iret == iexp));
}
