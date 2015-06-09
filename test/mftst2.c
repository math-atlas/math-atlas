#include <stdio.h>
/*
 * This tests whether iFKO can return the float precision constant 2.5
 * by adding local constants
 */
main()
{
   float exp=2.5, dret;
   float fsimple(void);
   dret = fsimple();
   if (exp == dret) fprintf(stdout, "2s: FPCONST RETURN PASSED\n");
   else fprintf(stderr, "2s: FPCONST RET FAILED: got=%lf, expected=%lf\n",
                dret, exp);
   exit(exp != dret);
}
