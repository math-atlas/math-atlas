#include <stdio.h>
/*
 * This tests whether iFKO can do integer multiplication
 */
main()
{
   int m0=(-7), m1=3;
   int iexp=m0*m1, iret;
   int mul_tst(int top, int bot);
   iret = mul_tst(m0, m1);
   if (iret == iexp) fprintf(stdout, "12: IMUL TEST PASSED\n");
   else
      fprintf(stderr, "12: IMUL TEST FAILED: iexp=%d, iret=%d\n", iexp, iret);
   exit(iret-iexp);
}
