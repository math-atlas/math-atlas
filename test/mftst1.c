#include <stdio.h>
/*
 * This tests whether iFKO can return the float precision constant 3.2
 */
main()
{
   float p0 = 3.7, p1 = 2.9;
   float exp=p0+p1, dret;
   float fsimple(float,float);
   dret = fsimple(p0, p1);
   if (exp == dret) fprintf(stdout, "1s: FPPARA ADD RETURN PASSED\n");
   else
      fprintf(stderr, "1s: FPPARA ADD FAILED: got=%lf, expected=%lf, diff=%e\n",
              dret, exp, dret-exp);
   exit(exp != dret);
}
