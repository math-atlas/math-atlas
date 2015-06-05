#include "fko_info.h"

void FKO_DestroyInstInfoC(fko_instinfo_t *dp)
{
   if (dp)
   {
      if (dp->gspills)
         free(dp->gspills);
      if (dp->ospills)
         free(dp->ospills);
      free(dp);
   }
}

void FKO_DestroyInstInfo(void)
{
   FKO_DestroyInstInfoC(FKO_INSTINF);
}
