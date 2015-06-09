#include <stdio.h>
/*
 * This tests whether iFKO can take an integer as a parameter, and return
 * it as the return value of an integer function
 */
main()
{
   int i=5, j;
   int simple(int);
   j = simple(i);
   if (i == j) fprintf(stdout, "1: INTEGER PARAMETER RETURN PASSED\n");
   else
      fprintf(stderr, "1: IPARA RETURN FAILED: iret=%d (expected=%d)\n", j, i);
   exit(i-j);
}
