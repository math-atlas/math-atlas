#include <stdio.h>
/*
 * This tests whether iFKO can return the double precision constant 3.2
 */
main()
{
   double exp=3.2, dretp, dretn;
   double myneg(double);
   dretp = myneg(exp);
   dretn = myneg(-exp);
   if (exp == -dretp && exp == dretn)
      fprintf(stdout, "9d: FPNEG RETURN PASSED\n");
   else fprintf(stderr, 
"9d: FPNEG RET FAILED: got=(%lf,%lf), expected=(%lf,%lf), diff=(%e,%e)\n",
                dretp, dretn, exp, -exp, dretp-exp, dretn-exp);
   exit(!(exp == -dretp && exp == dretn));
}
