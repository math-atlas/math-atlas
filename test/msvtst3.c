#include <stdio.h>
/*
 * This tests whether iFKO can do simple mixed type assignment
 */
void good_exp(double *X, float  *Y, double dval, float fval)
{
   float fflag;
   double dflag;
   fflag = 1.0;
   dflag = 2.0;

   dflag += dval;
   fflag +=fval;
/*
   *X = dflag;
   *Y = flag;
*/
   X[0] = dflag;
   Y[0] = fflag;
}

main()
{
   void SVTST3(double  *X, float *Y,double dval, float fval);
   double  X1[5] = {1.0, -2.0, -5.0, 4.0, 5.0};
   double  X2[5] = {1.0, -2.0, -5.0, 4.0, 5.0};
   float  Y1[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
   float  Y2[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
   int i, cflag;
   SVTST3(X1, Y1, 2.0, 1.0);
   good_exp(X2, Y2, 2.0, 1.0);
   
#if 0
   fprintf(stdout,"X1 = ");
   for (i = 0; i < 1; i++)
      fprintf(stdout, "%f ", X1[i] );
  
   fprintf(stdout,"\nX2 = ");
   for (i = 0; i < 1; i++)
      fprintf(stdout, "%f ", X2[i] );

   fprintf(stdout,"\n");
   
   fprintf(stdout,"Y1 = ");
   for (i = 0; i < 1; i++)
      fprintf(stdout, "%f ", Y1[i] );
  
   fprintf(stdout,"\nY2 = ");
   for (i = 0; i < 1; i++)
      fprintf(stdout, "%f ", Y2[i] );

   fprintf(stdout,"\n");

#endif
   if ( (X1[0] == X2[0]) && (Y1[0] == Y2[0]) )
      fprintf(stdout, "3fd: SIMPLE ASSIGNMENT WITH MIXED TYPE PASSED\n");
   else fprintf(stderr, "3fd: SIMPLE ASSIGNMENT WITH MIXED TYPE FAILED\n");
   exit(!((X1[0] == X2[0]) && (Y1[0] == Y2[0])));
}
