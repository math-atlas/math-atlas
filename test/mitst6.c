#include <stdio.h>
/*
 * This tests whether iFKO can take an integer array, and return a particular
 * element.
 */
main()
{
   int arr[5] = {1, 2, 3, 4, 5};
   int arr_tst(int, int*);
   int i=3, iexp, i1;
   iexp = arr[i];
   i1 = arr_tst(i, arr);
   if (i1 == iexp) fprintf(stdout, "6i: PTR INDEX PASSED\n");
   else fprintf(stderr, "6i: PTR INDEX FAILED: got=%d, expected=%d\n",
                i1, iexp);
   exit(i1-iexp);
}
