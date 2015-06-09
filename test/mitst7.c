#include <stdio.h>
/*
 * This tests whether iFKO can take two integer parameters, and return their
 * sum, plus the integer constant 2
 */
main()
{
   int sloop(void);
   int i, iret, iexp;
   iret = sloop();
   for (iexp=i=0; i < 10; i++) iexp += i;
   if (iret == iexp) fprintf(stdout, "7i: ICONST SUM LOOP PASSED\n");
   else fprintf(stderr, "7i: ICONST SUM LOOP FAILED: got=%d, expected=%d\n",
                iret, iexp);
   exit(iret-iexp);
}
