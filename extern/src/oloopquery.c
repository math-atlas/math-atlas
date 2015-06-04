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
void PrintCharArray(char *lab, int n, char *sa)
{
   int i;
   if (lab)
      printf("%s", lab);
   printf("%d", (int)*sa);
   for (i=1; i < n; i++)
      printf(", %d",(int)sa[i]);
   printf("\n");
}
void PrintStringArray(char *lab, int n, char **sa)
{
   int i;
   if (lab)
      printf("%s", lab);
   printf("%s", *sa);
   for (i=1; i < n; i++)
      printf(", %s", sa[i]);
   printf("\n");
}
void PrintLoopInfo(fko_olpinfo_t *lp)
{
   int n;
   printf("MAXUNROLL=%d\n", lp->maxunroll);
   printf("LNF=%d\n", lp->LNF);
   printf("NPATHS=%d\n", lp->npaths);
   printf("NIFS=%d\n", lp->nifs);
   if (lp->nifs > 0)
   {
      printf("   MaxElim=%d\n", lp->MaxElimIfs);
      printf("   MinElim=%d\n", lp->MinElimIfs);
      printf("   RedCompElim=%d\n", lp->redcElimIfs);
   }
   printf("VEC=%d\n", lp->vec);
   if (lp->vec > 0)
   {
      printf("   vmaxmin=%d\n", lp->vmaxmin);
      printf("   vredcomp=%d\n", lp->vredcomp);
      printf("   vspec=%d\n", lp->specvec);
      if (lp->specvec)
         PrintCharArray("   pathOK=", lp->npaths, lp->svpath);
   }

   n = lp->nmfptrs;
   printf("NMOVFPPTRS=%d\n", n);
   if (n > 0)
   {
       PrintStringArray("   names: ", n, lp->fptrs);
       PrintShortArray("   sets  : ", n, lp->fsets);
       PrintShortArray("   uses  : ", n, lp->fuses);
       PrintCharArray("   types : ", n, lp->ftyp);
       n = lp->npref;
       printf("   number of prefetchable ptrs: %d\n", n);
       if (n > 0)
          PrintShortArray("      pfindxs: ", n, lp->pffp);
   }
   n = lp->nscal;
   printf("NSCALARS=%d\n", n);
   if (n > 0)
   {
       PrintStringArray("   names: ", n, lp->scnam);
       PrintShortArray("   sets  : ", n, lp->ssets);
       PrintShortArray("   uses  : ", n, lp->suses);
       PrintCharArray("   types : ", n, lp->styp);
       n = lp->nexpand;
       printf("   Number Reducable Expansion: %d\n", n);
       if (n)
          PrintShortArray("      expidxs: ", n, lp->pffp);
   }
}

int main(int nargs, char **args)
{
   char *fnam="stdin";
   if (nargs > 1)
      fnam = args[1];
   FKO_GetOptLoopInfo(fnam);
   PrintLoopInfo(FKO_OLOOPINF);
   FKO_DestroyArchInfo();
   return(0);
}
