#include <stdio.h>
/*
 * This tests whether iFKO can do a simple dot product
 */
main()
{
   int parastress(int i0, int i1, int i2, int i3, int i4, int i5, int i6,
                  int i7, int i8, int i9, int i10, int i11, int i12, int i13,
                  int i14);
              
   int i[15] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
   int k, iret, iexp;

   iret = parastress(i[0], i[1], i[2], i[3], i[4], i[5], i[6], i[7], i[8], 
                     i[9], i[10], i[11], i[12], i[13], i[14]);
                     
   for (iexp=k=0; k < 15; k++) iexp += i[k];
   if (iexp == iret)
      fprintf(stdout, "0H: IPARA STRESS PASSED\n");
   else
      fprintf(stdout, 
              "0H: IPARA STRESS FAILED: iret=%d, iexp=%d\n", iret, iexp);
   exit(!(iret == iexp));
}
