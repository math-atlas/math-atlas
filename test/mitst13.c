#include <stdio.h>
main()
{
   int sloop(void);
   int i, iret, iexp;
   iret = sloop();
   for (iexp=i=0; i < 10; i++) iexp += i;
   if (iret == iexp) fprintf(stdout, "13: ICONST SUM IFLOOP PASSED\n");
   else fprintf(stderr, "13: ICONST SUM IFLOOP FAILED: got=%d, expected=%d\n",
                iret, iexp);
   exit(iret-iexp);
}
