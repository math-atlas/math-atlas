#include "ifko.h"
#include "fko_loop.h"
#include <stdarg.h>

int const2shift(int c)
{
   int i;
   for (i=0; i < 32; i++)
      if ((c ^ (1<<i)) == 0) return(i);
   return(-1);
}

void PrintComment(INSTQ *prev, INSTQ *next, ...)
{
   va_list argptr;
   char *form;
   char ln[256];

   va_start(argptr, next);
   form = va_arg(argptr, char*);
   vsprintf(ln, form, argptr);
   va_end(argptr);
   InsNewInst(prev, next, COMMENT, STstrconstlookup(ln), 0, 0);
}

void fko_error(int errno, ...)
{
   va_list argptr;
   char *form;
   va_start(argptr, errno);
   form = va_arg(argptr, char*);
   vfprintf(stderr, form, argptr);
   va_end(argptr);
   fprintf(stderr, "\nExiting iFKO with error number %d\n", errno);
/*   exit(errno); */
}

void fko_warn(int errno, ...)
{
   va_list argptr;
   char *form;
   va_start(argptr, errno);
   form = va_arg(argptr, char*);
   fprintf(stderr, "WARNING: ");
   vfprintf(stderr, form, argptr);
   fprintf(stderr, "\n");
   va_end(argptr);
}


struct loopq *NewLoop(int flag)
{
   struct loopq *lp, *l;
   short lnum=0;

   lp = malloc(sizeof(struct loopq));
   assert(lp);
   lp->flag = flag;
   lp->slivein = lp->sliveout = lp->adeadin = lp->adeadout = lp->nopf =
                 lp->aaligned = NULL;
   lp->abalign = NULL;
   lp->maxunroll = 0;
   lp->next = NULL;
   if (loopq)
   {
      lnum++;
      for (l=loopq; l->next; lnum++, l = l->next);
      l->next = lp;
   }
   else loopq = lp;
   lp->loopnum = lnum;
   return(lp);
}
