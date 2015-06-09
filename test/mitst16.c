#include <stdio.h>
/*
 * This tests whether iFKO can take two integer parameters, and return their
 * sum, plus the integer constant 2
 */
main()
{
   int sloop(int ibeg, int iend, int *ia);
   int i, ibeg=1, iend=10, iret, iexp;
   int ia[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
   iret = sloop(ibeg, iend, ia);
   for (iexp=0, i=ibeg; i < iend; i++) iexp += ia[i];
   if (iret == iexp) fprintf(stdout, "16: IARRAY REVERSE SUM LOOP PASSED\n");
   else
    fprintf(stderr, "16: IARRAY REVERSE SUM LOOP FAILED: got=%d, expected=%d\n",
            iret, iexp);
   exit(iret-iexp);
}
