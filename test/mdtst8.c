#include <stdio.h>
/*
 * This tests whether iFKO can return the double precision constant 3.2
 */
main()
{
   double exp=3.2, dretp, dretn;
   double myabs(double);
   dretp = myabs(exp);
   dretn = myabs(-exp);
   if (exp == dretp && exp == dretn)
      fprintf(stdout, "8d: FPABS RETURN PASSED\n");
   else fprintf(stderr, 
"8d: FPABS RET FAILED: got=(%lf,%lf), expected=%lf, diff=(%e,%e)\n",
                dretp, dretn, exp, dretp-exp, dretn-exp);
   exit(!(exp == dretp && exp == dretn));
}
