#include <stdio.h>
/*
 * This tests whether iFKO can return the float precision constant 3.2
 */
main()
{
   float exp=3.2, dret;
   float fsimple(void);
   dret = fsimple();
   if (exp == dret) fprintf(stdout, "0s: FPCONST RETURN PASSED\n");
   else fprintf(stderr, 
                "0s: FPCONST RET FAILED: got=%lf, expected=%lf, diff=%e\n",
                dret, exp, dret-exp);
   exit(exp != dret);
}
