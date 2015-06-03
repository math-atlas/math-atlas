#include "fko_archinfo.h"

void FKO_DestroyArchInfoC(fko_archinfo_t *dp)
{
   if (dp)
   {
      if (dp->pipelen_add)
         free(dp->pipelen_add);
      if (dp->pipelen_mul)
         free(dp->pipelen_mul);
      if (dp->pipelen_mac)
         free(dp->pipelen_mac);
      if (dp->pipelen_div)
         free(dp->pipelen_div);
      if (dp->numregs)
         free(dp->numregs);
      if (dp->aliased)
         free(dp->aliased);
      if (dp->clsz)
         free(dp->clsz);
      if (dp->vlen)
         free(dp->vlen);
      if (dp->spcinst)
         free(dp->spcinst);
      free(dp);
   }
}

void FKO_DestroyArchInfo(void)
{
   FKO_DestroyArchInfoC(FKO_ARCHINF);
}
