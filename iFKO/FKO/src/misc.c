/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#include "fko.h"
#include "fko_loop.h"
#include <stdarg.h>

int FindInShortList(int n, short *sp, short val)
/*
 * Finds val in list sp
 * RETURNS: index (starting from 1) of sp entry, or 0 if not found
 */
{
   int i;

   for (i=0; i < n; i++)
      if (sp[i] == val)
         return(i+1);
   return(0);
}

int AddToShortList(int n, short *sp, short val)
/*
 * Adds val to list sp, if it is not already there.
 * RETURNS: (possibly new) n
 */
{
   int i;
   for (i=0; i != n && sp[i] != val; i++);
   if (i == n)
      sp[n++] = val;
   return(n);
}
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
ILIST *NewIlistAtEnd(INSTQ *inst, ILIST *top)
{
   ILIST *ip, *il;
   ip = malloc(sizeof(ILIST));
   assert(ip);
   ip->inst = inst;
   ip->next = NULL;
   il = top;
   if (!il)
   {
      return(ip);
   }
   while(il->next)
      il = il->next;
   il->next = ip;
   return(top);
}
ILIST *NewIlistInBetween(INSTQ *inst, ILIST *prev, ILIST *next)
/*
 * add new ilist (with inst) in between prev and next and return ptr of new
 * ilist node
 */
{
   ILIST *il;
   il = malloc(sizeof(ILIST));
   assert(il);
   il->inst = inst;
   il->next = next;
   if (prev)
      prev->next = il;
   return(il);
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

ILIST *KillThisIlist(ILIST *ibase, ILIST *killme)
{
   ILIST *il;
   if (killme == ibase)
      ibase = KillIlist(killme);
   else
   {
      for (il=ibase; il && il->next != killme; il = il->next);
      assert(il);
      il->next = KillIlist(il);
   }
   return(ibase);
}
void KillAllIlist(ILIST *ip)
{
   while(ip)
      ip = KillIlist(ip);
}
struct ptrinfo *NewPtrinfo(short ptr, short flag, struct ptrinfo *next)
{
   struct ptrinfo *p;

   p = malloc(sizeof(struct ptrinfo));
   assert(p);
   p->ilist = NULL;
   p->next = next;
   p->ptr = ptr;
   p->flag = flag;
   p->nupdate = 0;
   p->upst = 0;
   return(p);
}

void KillAllPtrinfo(struct ptrinfo *base)
{
   struct ptrinfo *pn;

   while(base)
   {
      pn = base->next;
      if (base->ilist) KillAllIlist(base->ilist);
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

struct locinit *NewLI(short id, short con, struct locinit *next)
{
   struct locinit *lp;
   lp = malloc(sizeof(struct locinit));
   assert(lp);
   lp->id = id;
   lp->con = con;
   lp->next = next;
   return(lp);
}

void KillAllLI(struct locinit *die)
{
   struct locinit *next;
   while (die)
   {
      next = die->next;
      free(die);
      die = next;
   }
}

LPLIST *NewLPlist(LPLIST *next, LOOPQ *loop)
{
   LPLIST *lp;
   lp = malloc(sizeof(LPLIST));
   assert(lp);
   lp->loop = loop;
   lp->next = next;
   return(lp);
}

LPLIST *KillLPlist(LPLIST *del)
{
   LPLIST *lp;
   if (del)
   {
      lp = del->next;
      free(del);
   }
   else
      lp = NULL;
   
   return(lp);
}

void KillAllLPlist(LPLIST *lp)
{
   while(lp)
      lp = KillLPlist(lp);
}

int IsUpperLoop(LPLIST *l0, LOOPQ *loop)
{
   LPLIST *ll;
   BLIST *bl;

   for (ll=l0; ll; ll=ll->next)
   {
      for (bl=ll->loop->blocks; bl; bl=bl->next)
         if (!FindBlockInList(loop->blocks, bl->blk))
            return(0);
   }
   return(1);
}

LPLIST *FindLoopNest(LOOPQ *lp0)
{
   LPLIST *ll, *l;
   LOOPQ *lp;
   extern LOOPQ *loopq; /* already sorted by decreasing depth */
   
   ll = NULL;
   ll = NewLPlist(ll, optloop);
   for (lp=loopq->next; lp; lp=lp->next) /* 1st loop is optloop, skip it*/
   {
      if (IsUpperLoop(ll, lp))
         ll = NewLPlist(ll, lp);
   }
   return(ll);
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
/*
 * Majedul: 2048 is not enough now (unrolling after SV)
 */
   char ln[4096];

   va_start(argptr, next);
   form = va_arg(argptr, char*);
   vsprintf(ln, form, argptr);
   assert(strlen(ln) < 4096);
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
#if 1
   exit(errno); 
#else
    while(1);
#endif
}

void fko_warn(int errno, ...)
{
   va_list argptr;
   char *form;
   if (FKO_FLAG & IFF_VERBOSE)
   {
      va_start(argptr, errno);
      form = va_arg(argptr, char*);
      fprintf(stderr, "WARNING: ");
      vfprintf(stderr, form, argptr);
      fprintf(stderr, "\n");
      va_end(argptr);
   }
}
