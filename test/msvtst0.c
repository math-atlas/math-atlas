#include <stdio.h>
/*
 * This tests whether iFKO can do aimax but mixed type 
 */
int iamax_good(int N, double *X, float  *Y)
{
   int i, imax=0;
   double  d, max=0.0;
   for (i=0; i < N; i++)
   {
      d = X[i];
      if (d < 0.0) d = -d;
      if (d > max)
      {
         imax = i;
         max = d;
         Y[i] = 1.0;
      }
   }
   return(imax);
}

int comp(int N, float *Y1, float *Y2)
{
   int equal, i;

   equal = 1;
   for ( i = 0; i < N; i++)
      if ( Y1[i] != Y2[i] )
      {  
         equal = 0;
         break;
      }

   return equal;

}

main()
{
   int MSVIAMAX(int N, double  *X, float *Y);
   double  X[5] = {1.0, -2.0, -5.0, 4.0, 5.0};
   float  Y1[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
   float  Y2[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
   int i, iexp0, iret0, cflag;
   double  dret, dexp;
   iret0 = MSVIAMAX(5, X, Y1);
   iexp0 = iamax_good(5, X, Y2);
   
#if 0
   fprintf(stdout,"Y1 = ");
   for (i = 0; i < 5; i++)
      fprintf(stdout, "%f ", Y1[i] );
  
   fprintf(stdout,"\nY2 = ");
   for (i = 0; i < 5; i++)
      fprintf(stdout, "%f ", Y2[i] );

   fprintf(stdout,"\n");

#endif
   cflag = comp(5,Y1,Y2);
   if (iret0 == iexp0 && cflag )
      fprintf(stdout, "0fd: IAMAX LIKE CODE WITH MIXED TYPE PASSED\n");
   else fprintf(stderr, "0fd: IAMAX LIKE CODE WITH MIXED TYPE FAILED: got=%d,\
         expected=%d, cflag = %d\n", iret0, iexp0, cflag);
/*   if ((iret0 == iexp0 && comp(5,Y1, Y2) ))
      return 1;
   else 
      return 0;
*/
   exit(!(iret0 == iexp0 && cflag));
}
