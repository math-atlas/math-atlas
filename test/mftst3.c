#include <stdio.h>
main()
{
   float p0 = 3.7, p1 = 2.9;
   float exp=2.5+p0+p1, dret;
   float fsimple(float,float);
   float err = 1.0E-15;
   dret = fsimple(p0, p1);
   if (exp - dret <= err) fprintf(stdout, "3s: FPPARALOC ADD RETURN PASSED\n");
   else fprintf(stderr, "3s: FPPARALOC ADD FAILED: got=%lf, expected=%lf\n",
                dret, exp);
   exit(exp-dret > err);
}
