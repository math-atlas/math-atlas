#include <stdio.h>
main()
{
   double arr[5] = {0.9, 1.8, 2.7, 3.6, 4.5};
   int i=2;
   double exp, dret;
   double arr_tst(int,double*);
   double err = 1.0E-15;
   exp = arr[i+1];
   dret = arr_tst(i, arr);
   if (exp - dret <= err) fprintf(stdout, "4d: ARRAY INDEX+CON PASSED\n");
   else fprintf(stderr, "4d: ARRAY INDEX+CON FAILED: got=%lf, expected=%lf\n",
                dret, exp);
   exit(exp-dret > err);
}
