#include <stdio.h>
/*
 * This tests whether iFKO can return the double precision constant 2.5
 * by adding local constants
 */
main()
{
   double exp=2.5, dret;
   double fsimple(void);
   dret = fsimple();
   if (exp == dret) fprintf(stdout, "0d: FPCONST RETURN PASSED\n");
   else fprintf(stderr, "0d: FPCONST RET FAILED: got=%lf, expected=%lf\n",
                dret, exp);
   exit(exp != dret);
}
