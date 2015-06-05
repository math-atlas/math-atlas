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
   if (lp->npaths > 1)
   {
      PrintCharArray("   vecpathOK=", lp->npaths, lp->vpath);
      printf("   mmrElimBR=%d\n", lp->mmElim);
      printf("    rcElimBR=%d\n", lp->rcElim);
      printf("\n   NIFS=%d\n", lp->nifs);
      printf("      MaxElim=%d\n", lp->MaxElimIfs);
      printf("      MinElim=%d\n", lp->MinElimIfs);
      printf("      RedCompElim=%d\n", lp->rcElimIfs);
   }
   printf("VEC=%d\n", lp->vec);

   n = lp->nmptrs;
   printf("NMOVFPPTRS=%d\n", n);
   if (n > 0)
   {
       PrintStringArray("   names: ", n, lp->pnam);
       PrintShortArray("   sets  : ", n, lp->psets);
       PrintShortArray("   uses  : ", n, lp->puses);
       PrintShortArray("   lds   : ", n, lp->plds);
       PrintShortArray("   sts   : ", n, lp->psts);
       PrintCharArray("   types : ", n, lp->ptyp);
       n = lp->npref;
       printf("   number of prefetchable ptrs: %d\n", n);
       if (n > 0)
          PrintShortArray("      pfidx: ", n, lp->ppf);
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
       printf("   Number Reducible Expansion: %d\n", n);
       if (n)
          PrintShortArray("      expidxs: ", n, lp->rexp);
   }
}

int main(int nargs, char **args)
{
   char *fnam="stdin";
   if (nargs > 1)
      fnam = args[1];
   FKO_GetOptLoopInfo(fnam);
   PrintLoopInfo(FKO_OLOOPINF);
   FKO_DestroyOptLoopInfo();
   return(0);
}
