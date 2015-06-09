#include <stdio.h>
/*
 * This tests whether iFKO can do a simple loop with mixed type
 */
void good_exp(int N, double *X, float  *Y)
{
   int i, imax=0;
   double  d, max=0.0;
   for (i=0; i < N; i++)
   {
      d = X[i];
      if (d < 0.0) d = -d;
      X[i] = d;
      Y[i] = 1.0;
   }
}

int comp(int N, double *X1, double *X2, float *Y1, float *Y2)
{
   int equal, i;

   equal = 1;
   for ( i = 0; i < N; i++)
   {
/*    
 *    float can not be compared like this. but as we don't have 
 *    any actual computation (just using copy), I use that 
 */      
      if ( Y1[i] != Y2[i] || X1[i] != X2[i])
      {  
         equal = 0;
         break;
      }
   }

   return equal;

}

main()
{
   void SVTST1(int N, double  *X, float *Y);
   double  X1[5] = {1.0, -2.0, -5.0, 4.0, 5.0};
   double  X2[5] = {1.0, -2.0, -5.0, 4.0, 5.0};
   float  Y1[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
   float  Y2[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
   int i, cflag;
   SVTST1(5, X1, Y1);
   good_exp(5, X2, Y2);
   
#if 0
   fprintf(stdout,"X1 = ");
   for (i = 0; i < 5; i++)
      fprintf(stdout, "%f ", X1[i] );
  
   fprintf(stdout,"\nX2 = ");
   for (i = 0; i < 5; i++)
      fprintf(stdout, "%f ", X2[i] );

   fprintf(stdout,"\n");
   
   fprintf(stdout,"Y1 = ");
   for (i = 0; i < 5; i++)
      fprintf(stdout, "%f ", Y1[i] );
  
   fprintf(stdout,"\nY2 = ");
   for (i = 0; i < 5; i++)
      fprintf(stdout, "%f ", Y2[i] );

   fprintf(stdout,"\n");

#endif
   cflag = comp(5,X1, X2, Y1,Y2);
   if (cflag )
      fprintf(stdout, "1fd: SIMPLE LOOP WITH MIXED TYPE PASSED\n");
   else fprintf(stderr, "1fd: SIMPLE LOOP WITH MIXED TYPE FAILED\n");
   exit(!(cflag));
}
