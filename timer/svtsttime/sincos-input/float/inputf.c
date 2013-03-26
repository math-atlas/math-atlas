#include "private.h"
/*
 * it works for for sin and cos 
 */

int inputf(float x, float *y1, float *y2, float *iy)
{
   float y[2], z=0.0;
   int n, ix;
   
   n = 0;
   GET_FLOAT_WORD(ix, x);
   ix &= 0x7fffffff;
   
   if (ix <= 0x3f490fd8)
   {
      *y1 = x;
      *y2 = z;
      *iy = 0.0;
   }
   else if (ix >= 0x7f800000)
   {
      *y1 = NA;
      *y2 = NA;
   }
   else
   {
      n = __ieee754_rem_pio2f(x,y);
      switch(n&3)
      {
         case 0:
            *y1 = y[0];
            *y2 = y[1];
            *iy = 1.0;
            break;
         case 1:
            *y1 = NA;
            *y2 = NA;
            break;
         case 2:
            *y1 = y[0];
            *y2 = y[1];
            *iy = 1.0;
            break;
         default:
            *y1 = NA;
            *y2 = NA;
      }
   }
   return (n&3);
}
