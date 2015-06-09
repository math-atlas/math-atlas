#include <stdio.h>
main()
{
   double arr[5] = {0.9, 1.8, 2.7, 3.6, 4.5};
   int i=0;
   double exp, dret;
   double arr_tst(int,double*);
   double err = 1.0E-15;
   exp = arr[i];
   dret = arr_tst(i, arr);
   if (exp - dret <= err) fprintf(stdout, "4d: ARRAY INDEX PASSED\n");
   else fprintf(stderr, "4d: ARRAY INDEX FAILED: got=%lf, expected=%lf\n",
                dret, exp);
   exit(exp-dret > err);
}
