#include <stdio.h>
main()
{
   double p0 = 3.7, p1 = 2.9;
   double exp=2.5+p0+p1, dret;
   double fsimple(double,double);
   double err = 1.0E-15;
   dret = fsimple(p0, p1);
   if (exp - dret <= err) fprintf(stdout, "3d: FPPARALOC ADD RETURN PASSED\n");
   else fprintf(stderr, "3d: FPPARALOC ADD FAILED: got=%lf, expected=%lf\n",
                dret, exp);
   exit(exp-dret > err);
}
