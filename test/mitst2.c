#include <stdio.h>
/*
 * This tests whether iFKO can take two integer parameters, and return their
 * sum, plus the integer constant 2
 */
main()
{
   int simple(int, int);
   int i1=3, i2=5, iret, iexp;
   iexp = i1 + i2 + 2;
   iret = simple(i1, i2);
   if (iret == iexp) fprintf(stdout, "2: IPARAS/CONST ADD PASSED\n");
   else fprintf(stderr, "2: IPARAS/CONST ADD FAILED: got=%d, expected=%d\n",
                iret, iexp);
   exit(iret-iexp);
}
