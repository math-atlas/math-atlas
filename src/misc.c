#include "ifko.h"
#include "fko_loop.h"
#include <stdarg.h>

struct locinit *NewLocinit(short id, short con, struct locinit *next)
{
   struct locinit *lp;
   lp = malloc(sizeof(struct locinit));
   assert(lp);
   lp->id = id;
   lp->con = con;
   lp->next = next;
   return(lp);
}

void KillAllLocinit(struct locinit *libase)
{
   struct locinit *lp;
   while(libase)
   {
      lp = libase->next;
      free(libase);
      libase = lp;
   }
}

int const2shift(int c)
{
   int i;
   for (i=0; i < 32; i++)
      if ((c ^ (1<<i)) == 0) return(i);
   return(-1);
}

INSTQ *PrintMajorComment(BBLOCK *blk, INSTQ *prev, INSTQ *next, ...)
{
   va_list argptr;
   char *form;
   char ln[256];

   va_start(argptr, next);
   form = va_arg(argptr, char*);
   vsprintf(ln, form, argptr);
   va_end(argptr);
   InsNewInst(blk, prev, next, COMMENT, 0, 0, 0);
   InsNewInst(blk, prev, next, COMMENT, STstrconstlookup(ln), 0, 0);
   return(InsNewInst(blk, prev, next, COMMENT, 0, 0, 0));
}

INSTQ *PrintComment(BBLOCK *blk, INSTQ *prev, INSTQ *next, ...)
{
   va_list argptr;
   char *form;
   char ln[256];

   va_start(argptr, next);
   form = va_arg(argptr, char*);
   vsprintf(ln, form, argptr);
   va_end(argptr);
   return(InsNewInst(blk, prev, next, COMMENT, STstrconstlookup(ln), 0, 0));
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
#if 0
   exit(errno); 
#else
    while(1);
#endif
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
