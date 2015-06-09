#include "fko_info.h"
static void FKO_FreeAllStrings(int n, char **sa)
{
   int i;
   for (i=0; i < n; i++)
   {
      if (sa[i])
         free(sa[i]);
   }
}

void FKO_DestroyOptLoopInfoC(fko_olpinfo_t *p)
{
   if (p->vpath)
      free(p->vpath);
   if (p->pnam)
   {
      FKO_FreeAllStrings(p->nmptrs, p->pnam);
      free(p->pnam);
   }
   if (p->ptyp)
      free(p->ptyp);
   if (p->psets)
      free(p->psets);
   if (p->puses)
      free(p->puses);
   if (p->plds)
      free(p->plds);
   if (p->psts)
      free(p->psts);
   if (p->ppf)
      free(p->ppf);

   if (p->p2nam)
   {
      FKO_FreeAllStrings(p->nmptrs, p->p2nam);
      free(p->p2nam);
   }
   if (p->p2typ)
      free(p->p2typ);
   if (p->p2sets)
      free(p->p2sets);
   if (p->p2uses)
      free(p->p2uses);
   if (p->p2lds)
      free(p->p2lds);
   if (p->p2sts)
      free(p->p2sts);
   if (p->p2pf)
      free(p->p2pf);
   if (p->p2cols)
      free(p->p2cols);
   if (p->p2regs)
      free(p->p2regs);
   if (p->p2ptrs)
      free(p->p2ptrs);

   if (p->scnam)
   {
      FKO_FreeAllStrings(p->nscal, p->scnam);
      free(p->scnam);
   }
   if (p->styp)
      free(p->styp);
   if (p->ssets)
      free(p->ssets);
   if (p->suses)
      free(p->suses);
   if (p->rexp)
      free(p->rexp);
   free(p);
}

void FKO_DestroyOptLoopInfo(void)
{
   FKO_DestroyOptLoopInfoC(FKO_OLOOPINF);
}
