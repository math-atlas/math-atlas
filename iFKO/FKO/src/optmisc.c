/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#include "fko.h"

int DoStoreNT(BLIST *scope)
/*
 * Changes all stores of selected arrays to non-temporal (cache-through) form
 */
{
   BLIST *bl;
   int i;
   int nchanges=0;
   short st;
   INSTQ *ip;
   enum inst store;
   extern int NWNT;
   extern char **ARRWNT;
   BLIST *killme;
   /*short k;*/

   if (!scope)
      killme = scope = GetGlobalScope();
   for (i=0; i < NWNT; i++)
   {
      st = FindVarFromName(ARRWNT[i]);
      /*k = st + TNREG - 1;*/
      assert(st);
      for (bl=scope; bl; bl = bl->next)
      {
         for (ip=bl->blk->ainst1; ip; ip = ip->next)
         {
            store = 0;
            if (ip->inst[0] == VFST)
               store = VFSTNT;
            else if (ip->inst[0] == VDST)
               store = VDSTNT;
            if (store && STpts2[ip->inst[1]-1] == st)
            {
               ip->inst[0] = store;
               nchanges++;
            }
         }
      }
   }
   if (killme)
      KillBlockList(killme);
   return(nchanges);
}
