#include "ifko.h"
#include "fko_loop.h"
#include <stdarg.h>

int Size2Shift(int n)
/*
 * If N is a power of two, i of 2^i, otherwise, returns 0
 */
{
   int i, j;
   for (i=0,j=1; j < n; i++)
      j = 1<<i;
   return(j==n ? i : 0);
}

short *NewShortTable(int *n, short *old, int chunk)
{
   short *new;
   int i, nn = *n, newN = *n + chunk;
   new = malloc(sizeof(short)*newN);
   assert(new);
   for (i=0; i < nn; i++)
      new[i] = old[i];
   for (; i < newN; i++)
      new[i] = -1;
   if (old) free(old);
   *n = newN;
   return(new);
}

void *NewPtrTable(int *n, void *old0, int chunk)
{
   int **new;
   int **old = old0;
   int i, nn = *n, newN = *n + chunk;

   new = malloc(sizeof(void*)*newN);
   assert(new);
   for (i=0; i < nn; i++)
      new[i] = old[i];
   for (; i < newN; i++)
      new[i] = NULL;
   if (old) free(old);
   *n = newN;
   return(new);
}

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
ILIST *NewIlist(INSTQ *inst, ILIST *next)
{
   ILIST *ip;
   ip = malloc(sizeof(ILIST));
   assert(ip);
   ip->inst = inst;
   ip->next = next;
   return(ip);
}

ILIST *KillIlist(ILIST *ip)
{
   ILIST *in;
   if (ip)
   {
      in = ip->next;
      free(ip);
   }
   else 
      in = NULL;
   return(in);
}

void KillAllIlist(ILIST *ip)
{
   while(ip)
      ip = KillIlist(ip);
}
struct ptrinfo *NewPtrinfo(short ptr, short flag, struct ptrinfo *next)
{
   struct ptrinfo *p;

   p = malloc(sizeof(struct ptrinfo *));
   assert(p);
   p->ilist = NULL;
   p->next = next;
   p->ptr = ptr;
   p->flag = flag;
   p->nupdate = 0;
   return(p);
}

void KillAllPtrinfo(struct ptrinfo *base)
{
   struct ptrinfo *pn;

   while(base)
   {
      pn = base->next;
      free(base);
      base = pn;
   }
}

struct ptrinfo *FindPtrinfo(struct ptrinfo *base, short ptr)
{
   for (; base; base = base->next)
      if (base->ptr == ptr)
         break;
   return(base);
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
   char ln[2048];

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
   char ln[2048];

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
