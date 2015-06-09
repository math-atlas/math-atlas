#include <stdio.h>
/*
 * This tests whether iFKO can multiply two ints, using a loop
 */
main()
{
   int multiply(int, int);
   const int ip0 = 3, ip1 = 7;
   int i, iret, iexp=ip0*ip1;
   iret = multiply(ip0, ip1);
   if (iret == iexp) fprintf(stdout, "14i: IMULTIPLY LOOP PASSED\n");
   else fprintf(stderr, "14i: IMULTIPLY LOOP FAILED: got=%d, expected=%d\n",
                iret, iexp);
   exit(iret-iexp);
}
