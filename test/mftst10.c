#include <stdio.h>
main()
{
   float  exp=3.2, dretp, dretn;
   float  myabs(float );
   dretp = myabs(exp);
   dretn = myabs(-exp);
   if (exp == dretp && exp == dretn)
      fprintf(stdout, "10f: FPIF ABS RETURN PASSED\n");
   else fprintf(stderr, 
"10f: FPIF ABS RET FAILED: got=(%lf,%lf), expected=%lf, diff=(%e,%e)\n",
                dretp, dretn, exp, dretp-exp, dretn-exp);
   exit(!(exp == dretp && exp == dretn));
}
