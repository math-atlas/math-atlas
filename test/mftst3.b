#include <stdio.h>
main()
{
   double p0 = 3.7, p1 = 2.9;
   double exp=2.5+p0+p1, dret;
   double fsimple(double,double);
   dret = fsimple(p0, p1);
   if (exp == dret) fprintf(stdout, "2d: FPPARALOC ADD RETURN PASSED\n");
   else fprintf(stderr, "2d: FPPARALOC ADD FAILED: got=%lf, expected=%lf\n",
                dret, exp);
   exit(exp != dret);
}
