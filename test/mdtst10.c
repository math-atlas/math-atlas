#include <stdio.h>
main()
{
   double exp=3.2, dretp, dretn;
   double myabs(double);
   dretp = myabs(exp);
   dretn = myabs(-exp);
   if (exp == dretp && exp == dretn)
      fprintf(stdout, "10d: FPIF ABS RETURN PASSED\n");
   else fprintf(stderr, 
"10d: FPIF ABS RET FAILED: got=(%lf,%lf), expected=%lf, diff=(%e,%e)\n",
                dretp, dretn, exp, dretp-exp, dretn-exp);
   exit(!(exp == dretp && exp == dretn));
}
