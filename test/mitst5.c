#include <stdio.h>
/*
 * This tests whether iFKO can take an integer pointer, and double the value
 * it is pointing to.
 */
main()
{
   void ptr_tst(int*);
   int i1=3, iexp;
   iexp = i1 + i1;
   ptr_tst(&i1);
   if (i1 == iexp) fprintf(stdout, "5i: PTR DOUBLE PASSED\n");
   else fprintf(stderr, "5i: PTR_DOUBLE FAILED: got=%d, expected=%d\n",
                i1, iexp);
   exit(i1-iexp);
}
