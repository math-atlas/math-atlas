#include <stdio.h>
/*
 * This tests whether iFKO can return the float  precision constant 3.2
 */
main()
{
   float  exp=3.2, dretp, dretn;
   float  myabs(float );
   dretp = myabs(exp);
   dretn = myabs(-exp);
   if (exp == dretp && exp == dretn)
      fprintf(stdout, "8f: FPABS RETURN PASSED\n");
   else fprintf(stderr, 
"8f: FPABS RET FAILED: got=(%lf,%lf), expected=%lf, diff=(%e,%e)\n",
                dretp, dretn, exp, dretp-exp, dretn-exp);
   exit(!(exp == dretp && exp == dretn));
}
