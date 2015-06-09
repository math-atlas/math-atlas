#include <stdio.h>
/*
 * This tests whether iFKO can return a large integral constant
 */
main()
{
   int i=0x8FFFFFFF, j;
   int iconstret(void);
   j = iconstret();
   if (i == j) fprintf(stdout, "10: LARGE INTEGER CONSTANT RETURN PASSED\n");
   else
      fprintf(stderr, "10: ICONST RETURN FAILED: iret=%d (expected=%d)\n", j,i);
   exit(i-j);
}
