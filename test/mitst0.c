#include <stdio.h>
/*
 * This tests whether iFKO can return the integer constant 2
 */
main()
{
   int i=3, j;
   int ret_3(void);
   j = ret_3();
   if (i == j) fprintf(stdout, "0: INTEGER CONSTANT RETURN PASSED\n");
   else
      fprintf(stderr, "1: ICONST RETURN FAILED: iret=%d (expected=%d)\n", j, i);
   exit(i-j);
}
