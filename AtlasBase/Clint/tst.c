#include <stdio.h>
#include <stdlib.h>
#include "/home/whaley/TEST/ATLAS3.11.38/obj64_8/include/atlas_dgeamm_kern.h"

int main(void)
{
   int i;
   for (i=0; i < ATL_AMM_NCASES; i++)
   {
      printf("kerns=%p, %p, %p\n", 
             ATL_AMM_KERN_b0[i], ATL_AMM_KERN_b1[i], ATL_AMM_KERN_bn[i]);
   }
   return(0);
}
