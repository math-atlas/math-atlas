#include <stdio.h>
main()
{
   float arr[5] = {0.9, 1.8, 2.7, 3.6, 4.5};
   int i=0;
   float exp, dret;
   float arr_tst(int,float*);
   float err = 1.0E-15;
   exp = arr[i];
   dret = arr_tst(i, arr);
   if (exp - dret <= err) fprintf(stdout, "4s: ARRAY INDEX PASSED\n");
   else fprintf(stderr, "4s: ARRAY INDEX FAILED: got=%lf, expected=%lf\n",
                dret, exp);
   exit(exp-dret > err);
}
