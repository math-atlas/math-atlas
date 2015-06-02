#include "fko_archinfo.h"

void PrintShortArray(char *lab, int n, short *sa)
{
   int i;
   if (lab)
      printf("%s", lab);
   printf("%hi", *sa);
   for (i=1; i < n; i++)
      printf(", %hi", sa[i]);
   printf("\n");
}
void PrintArchInfo(fko_archinfo_t *ap)
{
   printf("NFPUPIPES=%d\n", ap->nfpupipes);
   if (ap->pipelen_add)
      PrintShortArray("   pipelenADD: ", FKO_NTYPES, ap->pipelen_add);
   if (ap->pipelen_mul)
      PrintShortArray("   pipelenMUL: ", FKO_NTYPES, ap->pipelen_mul);
   if (ap->pipelen_mac)
      PrintShortArray("   pipelenMAC: ", FKO_NTYPES, ap->pipelen_mac);
}

int main(int nargs, char **args)
{
   char *fnam="stdin";
   if (nargs > 1)
      fnam = args[1];
   FKO_GetArchInfo(fnam);
   PrintArchInfo(FKO_ARCHINF);
   return(0);
}
