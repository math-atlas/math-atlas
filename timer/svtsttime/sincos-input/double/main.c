#include<stdio.h>
#include<stdlib.h>

#define PI 3.14159265358979323846  /* from math.h file */
#define NA (-2271.0) /* private */ 


#define dumb_seed(iseed_) srand(iseed_)
#ifndef RAND_MAX  /* rather dangerous non-ansi workaround */
   #define RAND_MAX ((unsigned long)(1<<30))
#endif

/*#define dumb_rand() ( 0.5 - ((double)rand())/((double)RAND_MAX) )*/
#define dumb_rand() ( (double)rand()/((double)RAND_MAX/(2*PI)) )


int main()
{
   int i, n, k;
   double x, y1, y2, iy;
   double inputd(double x, double *y1, double *y2, double *iy);
   n = 100;
   dumb_seed(n);
   for (i=0; i<n; i++)
   {
      x = dumb_rand();
      k = inputd(x, &y1, &y2, &iy);
      if (y1 != NA || y2 != NA)
         fprintf(stdout, "%d: %lf -> %lf, %lf, %lf\n", k, x, y1, y2, iy);
   }
   return 0;
}
