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
   printf("PIPELINES=%d\n", ap->nfpupipes);
   if (ap->pipelen_add)
      PrintShortArray("   pipelen_add: ", FKO_NTYPES, ap->pipelen_add);
   if (ap->pipelen_mul)
      PrintShortArray("   pipelen_mul: ", FKO_NTYPES, ap->pipelen_mul);
   if (ap->pipelen_mac)
      PrintShortArray("   pipelen_mac: ", FKO_NTYPES, ap->pipelen_mac);
   printf("regtypes=%d\n", ap->regtypes);
   if (ap->numregs)
      PrintShortArray("   numregs: ", FKO_NTYPES, ap->numregs);
   if (ap->aliased)
      PrintShortArray("   aliased: ", FKO_NTYPES, ap->aliased);
   printf("ncaches=%d\n", ap->ncaches);
   if (ap->clsz)
      PrintShortArray("   clsz: ", ap->ncaches, ap->clsz);
   printf("vectypes=%d\n", ap->vectypes);
   if (ap->vlen)
      PrintShortArray("   veclen: ", ap->vectypes, ap->vlen);
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
