#include <stdio.h>
/*
 * This tests whether iFKO can return the double precision constant 3.2
 */
main()
{
   double p0 = 3.7, p1 = 2.9;
   double exp=p0+p1, dret;
   double fsimple(double,double);
   dret = fsimple(p0, p1);
   if (exp == dret) fprintf(stdout, "1d: FPPARA ADD RETURN PASSED\n");
   else
      fprintf(stderr, "1d: FPPARA ADD FAILED: got=%lf, expected=%lf, diff=%e\n",
              dret, exp, dret-exp);
   exit(exp != dret);
}
