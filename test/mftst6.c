#include <stdio.h>
main()
{
   float arr[5] = {0.9, 1.8, 2.7, 3.6, 4.5};
   int i=2;
   float exp, dret;
   float arr_tst(int,float*);
   float err = 1.0E-15;
   exp = arr[i] + arr[i-1] + arr[i+1];
   dret = arr_tst(i, arr);
   if (exp - dret <= err) fprintf(stdout, "6s: ARRAY INDEX & ADD PASSED\n");
   else fprintf(stderr, "6s: ARRAY INDEX & ADD FAILED: got=%lf, expected=%lf\n",
                dret, exp);
   exit(exp-dret > err);
}
