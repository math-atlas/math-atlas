/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

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
void PrintInstInfo(fko_instinfo_t *ip)
{
   printf("LIVE RANGE SPILLS=%d\n", ip->lrspills);
   if (ip->ospills)
      PrintShortArray("   optloop: ", FKO_NTYPES, ip->ospills);
   else
      printf("   optloop: NONE\n");
   if (ip->gspills)
      PrintShortArray("    global: ", FKO_NTYPES, ip->gspills);
   else
      printf("    global: NONE\n");
}

int main(int nargs, char **args)
{
   char *fnam="stdin";
   if (nargs > 1)
      fnam = args[1];
   FKO_GetInstInfo(fnam);
   PrintInstInfo(FKO_INSTINF);
   FKO_DestroyInstInfo();
   return(0);
}
