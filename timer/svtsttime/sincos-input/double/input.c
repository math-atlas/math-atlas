
#include "private.h"

int inputd(double x, double *y1, double *y2, double *iy)
{
   double y[2], z=0.0;
   int n, ix;
   
   n = 0;
   GET_HIGH_WORD(ix, x);
   ix &= 0x7fffffff;
   
   if (ix <= 0x3fe921fb)
   {
      *y1 = x;
      *y2 = z;
      *iy = 0.0;
   }
   else if (ix >= 0x7ff00000)
   {
      *y1 = NA;
      *y2 = NA;
   }
   else
   {
      n = __ieee754_rem_pio2(x,y);
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
   return n;
}



