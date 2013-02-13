#include<stdio.h>
#include<stdlib.h>

#define PI 3.14159265358979323846  /* from math.h file */
#define NA (-2271.0) /* got it from private.h */ 


#define dumb_seed(iseed_) srand(iseed_)

#ifndef RAND_MAX  /* rather dangerous non-ansi workaround */
   #define RAND_MAX ((unsigned long)(1<<30))
#endif

/*#define dumb_rand() ( 0.5 - ((double)rand())/((double)RAND_MAX) )*/
#define dumb_rand() ( (float)rand()/((float)RAND_MAX/(2*PI)) )

int main()
{
   int i, n, k;
   float x, y1, y2;
   int inputf(float x, float *y1, float *y2);

   n = 100;
   dumb_seed(n);
   for (i=0; i<n; i++)
   {
      x = dumb_rand();
      k = inputf(x, &y1, &y2);
      if (y1 != NA || y2 != NA)
         fprintf(stdout, "%d: %lf -> %lf, %lf\n", k, x, y1, y2);
   }
   return 0;
}
