#include <stdio.h>
/*
 * This tests whether iFKO can return the float precision constant 3.2
 */
main()
{
   float  exp=3.2, dretp, dretn;
   float  myneg(float );
   dretp = myneg(exp);
   dretn = myneg(-exp);
   if (exp == -dretp && exp == dretn)
      fprintf(stdout, "9s: FPNEG RETURN PASSED\n");
   else fprintf(stderr, 
"9s: FPNEG RET FAILED: got=(%lf,%lf), expected=(%lf,%lf), diff=(%e,%e)\n",
                dretp, dretn, exp, -exp, dretp-exp, dretn-exp);
   exit(!(exp == -dretp && exp == dretn));
}
