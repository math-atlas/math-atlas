#include "fko_parseinfo.h"
#include "fko_archinfoC.h"

int NumRegs(fko_archinfo_t *ap, int typ)
{
   return(0);
}
int TypesAliased(fko_archinfo_t *ap, int t1, int t2)
{
   int max=t2, min=t1;
   if (!ap)
      return(0);
   if (!(ap->aliased))
      return(0);
   if (max > min) 
   {
      max = t1;
      min = t2;
   }
   if (min < 0 || max >= ap->regtypes)
      return(0);
   return(ap->aliased[min] & (1 << max));
}
void fko_ParseArch
(
   int *REGTYPES,
   int **NUMREGS
)
{
}
