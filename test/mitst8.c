#include <stdio.h>
/*
 * This tests whether iFKO can take two integer parameters, and return their
 * sum, plus the integer constant 2
 */
main()
{
   int sloop(int ibeg, int iend);
   int i, ibeg=2, iend=9, iret, iexp;
   iret = sloop(ibeg, iend);
   for (iexp=0, i=ibeg; i < iend; i++) iexp += i;
   if (iret == iexp) fprintf(stdout, "8: ICONST VSUM LOOP PASSED\n");
   else fprintf(stderr, "8: ICONST VSUM LOOP FAILED: got=%d, expected=%d\n",
                iret, iexp);
   exit(iret-iexp);
}
