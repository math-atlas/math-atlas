#include <stdio.h>
/*
 * This tests whether iFKO can do integer multiplication
 */
main()
{
   int m0=(7), m1=120;
   int iexp=m0*m1, iret;
   int mul_tst(int top, int bot);
   iret = mul_tst(m0, m1);
   if (iret == iexp) fprintf(stdout, "15: IMUL LOOP TEST PASSED\n");
   else
      fprintf(stderr, "15: IMUL LOOP TEST FAILED: iexp=%d, iret=%d\n", iexp, iret);
   exit(iret-iexp);
}
