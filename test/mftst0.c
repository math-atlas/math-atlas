#include <stdio.h>
/*
 * This tests whether iFKO can return the double precision constant 3.2
 */
main()
{
   double exp=3.2, dret;
   double fsimple(void);
   dret = fsimple();
   if (exp == dret) fprintf(stdout, "0d: FPCONST RETURN PASSED\n");
   else fprintf(stderr, 
                "0d: FPCONST RET FAILED: got=%lf, expected=%lf, diff=%e\n",
                dret, exp, dret-exp);
   exit(exp != dret);
}
