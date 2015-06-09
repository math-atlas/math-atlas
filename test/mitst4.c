#include <stdio.h>
main()
{
   int i=2, j;
   int exp = 3*i;
   int simple(int);
   j = simple(i);
   if (j == exp) fprintf(stdout, "4: INTEGER CONSTANT RETURN PASSED\n");
   else
      fprintf(stderr, "4: ICONST RETURN FAILED: iret=%d (expected=%d)\n",
              j, exp);
   exit(j-exp);
}
