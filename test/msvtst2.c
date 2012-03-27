#include <stdio.h>
/*
 * This tests whether iFKO can do simple loop-if with mixed type
 */
double good_exp(float *X, float fval, double dval, int N)
{
   int i;
   double  dlocal = 2.0;
   for (i=0; i < N; i++)
   {
      if ( X[i] > fval)
         dlocal += dval;
      else
         dlocal += 1.0;
   }
   return dlocal;
}


main()
{
   double SVTST2(float *X, float fval, double dval, int N);
   float  X[5] = {1.0, -2.0, 0.0, -3, 4};
   int i, cflag;
   double dret, exp, err;
   err = 1.0E-15;
   dret = SVTST2(X, 0.0, 0.0, 5);
   exp = good_exp(X, 0.0, 0.0, 5);
/* why abs value is not used? */   
   if (exp - dret <= err )
      fprintf(stdout, "2fd: LOOP-IF WITH MIXED TYPE PASSED\n");
   else
   {
      fprintf(stderr, "2fd: LOOP-IF WITH MIXED TYPE FAILED!"); 
      fprintf(stdout," expected =%lf, got = %lf \n", exp, dret);
   }
   exit(!(exp - dret <= err));
}
