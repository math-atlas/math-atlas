#include <stdio.h>
/*
 * This tests whether iFKO can do integer division
 */
main()
{
   int top=(-8), bot=3;
   int iexp=top/bot, iret;
   int div_tst(int top, int bot);
   iret = div_tst(top, bot);
   if (iret == iexp) fprintf(stdout, "11: IDIV TEST PASSED\n");
   else
      fprintf(stderr, "11: IDIV TEST FAILED: iexp=%d, iret=%d\n", iexp, iret);
   exit(iret-iexp);
}
