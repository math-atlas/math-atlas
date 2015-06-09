/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#define DECL_OL_ 1
#include "fko_parseinfo.h"
#include "fko_info.h"

fko_olpinfo_t *FKO_NewOptLoopInfo(void)
{
   fko_olpinfo_t *lp;
   lp = calloc(1, sizeof(fko_olpinfo_t));
   return(lp);
}

static int FKO_GetPtrInfo(FILE *fpin, fko_infoline_t *ip, int TWOD, int i,
   char **nam, char *typ, short *sets, short *uses, short *lds, short *sts, 
   short *ppf, short *ncol, short *nreg, short *nptr)
{
   fko_word_t *wp;
   char *sp;
   int npf=0;

   FKO_FreeAllWords(ip->words);
   assert(FKO_ParseInfoLine(ip, fpin));
   if (TWOD)
      assert(ip->nwords == 10);
   else
      assert(ip->nwords == 7);
   wp = ip->words;
   nam[i] = FKO_Word2Name(wp);

   wp = wp->next;
   sp = wp->word;
   assert(!strncmp(sp, "type=", 5));
   typ[i] = FKO_TypeStrToInt(sp+5);

   wp = wp->next;
   sp = wp->word;
   assert(!strncmp(sp, "uses=", 5));
   assert(sscanf(sp+5, "%hd", uses+i) == 1);

   wp = wp->next;
   sp = wp->word;
   assert(!strncmp(sp, "sets=", 5));
   assert(sscanf(sp+5, "%hd", sets+i) == 1);

   wp = wp->next;
   sp = wp->word;
   assert(!strncmp(sp, "lds=", 4));
   assert(sscanf(sp+4, "%hd", lds+i) == 1);

   wp = wp->next;
   sp = wp->word;
   assert(!strncmp(sp, "sts=", 4));
   assert(sscanf(sp+4, "%hd", sts+i) == 1);

   wp = wp->next;
   sp = wp->word;
   assert(!strncmp(sp, "prefetch=", 9));
   if (sp[9] == '1')
   {
      *ppf = i;
      npf = 1;
   }

   if (TWOD)
   {
      wp = wp->next;
      sp = wp->word;
      assert(!strncmp(sp, "ncol=", 5));
      assert(sscanf(sp+5, "%hd", ncol+i) == 1);

      wp = wp->next;
      sp = wp->word;
      assert(!strncmp(sp, "nreg=", 5));
      assert(sscanf(sp+5, "%hd", nreg+i) == 1);

      wp = wp->next;
      sp = wp->word;
      assert(!strncmp(sp, "nptr=", 5));
      assert(sscanf(sp+5, "%hd", nptr+i) == 1);

   }
   return(npf);
}

fko_olpinfo_t *FKO_GetOptLoopInfoC(char *fnin)
{
   fko_olpinfo_t *lp;
   FILE *fpin;
   fko_infoline_t il;
   fko_word_t *wp;
   char *sp;

   if (!strcmp(fnin, "stdin"))
      fpin = stdin;
   else
   {
      fpin = fopen(fnin, "r");
      assert(fpin);
   }
/*
 * First line is OPTLOOP=[0,1].  Quick return if no optloop
 */
   assert(FKO_ParseInfoLine(&il, fpin));  /* first line is OPTLOOP=[0,1] */
   sp = il.words->word;
   assert(il.nwords == 1);
   assert(!strncmp(sp, "OPTLOOP=", 8));
   if (sp[8] == '0')
      return(NULL);
   FKO_FreeAllWords(il.words);

   lp = FKO_NewOptLoopInfo();
   while(FKO_ParseInfoLine(&il, fpin))
   {
      int n, i;

      wp = il.words;
      if (il.nwords == 4) /* Moving [1,2]D Pointers: X */
      {
         int npf=0;
         int TWOD=0;
         char **nam, *typ;
         short *sets, *uses, *lds, *sts, *ppf;
         short *cols=NULL, *regs=NULL, *ptrs=NULL;
         assert(!strcmp(wp->word, "Moving"));
         sp = wp->next->word;
         if (sp[0] == '2')
            TWOD = 1;
         else
           assert(sp[0] == '1');
         assert(sp[1] == 'D' && sp[2] == '\0');
         assert(!strcmp(wp->next->next->word, "Pointers:"));
         wp = wp->next->next->next;
         assert(sscanf(wp->word, "%d", &n) == 1);

         nam = malloc(n*sizeof(char*));
         assert(nam);
         sets = malloc(n*sizeof(short));
         assert(sets);
         uses = malloc(n*sizeof(short));
         assert(uses);
         lds = malloc(n*sizeof(short));
         assert(lds);
         sts = malloc(n*sizeof(short));
         assert(sts);
         ppf = malloc(n*sizeof(short));
         assert(ppf);
         typ = malloc(n);
         assert(typ);
         if (TWOD)
         {
            lp->n2ptrs = n;
            lp->p2cols = cols = malloc(n*sizeof(short));
            assert(cols);
            lp->p2regs = regs = malloc(n*sizeof(short));
            assert(regs);
            lp->p2ptrs = ptrs = malloc(n*sizeof(short));
            assert(ptrs);
            lp->p2nam = nam;
            lp->p2typ = typ;
            lp->p2sets = sets;
            lp->p2uses = uses;
            lp->p2lds = lds;
            lp->p2sts = sts;
            lp->p2pf = ppf;
         }
         else
         {
            lp->nmptrs = n;
            lp->pnam = nam;
            lp->ptyp = typ;
            lp->psets = sets;
            lp->puses = uses;
            lp->plds = lds;
            lp->psts = sts;
            lp->ppf = ppf;
         }
/*
 *       Loop over all moving pointers
 */
         for (i=0; i < n; i++)
         {
            npf += FKO_GetPtrInfo(fpin, &il, TWOD, i, nam, typ, sets, uses, 
                                  lds, sts, ppf+npf, cols, regs, ptrs);
         }
         lp->npref = npf;
         if (!npf)
         {
            free(lp->ppf);
            lp->ppf = NULL;
         }
      }
      else if (il.nwords == 5) /* Scalars Used in Loop: X */
      {
         int nex=0;
         assert(!strcmp(wp->word, "Scalars"));
         assert(!strcmp(wp->next->word, "Used"));
         assert(!strcmp(wp->next->next->word, "in"));
         assert(!strcmp(wp->next->next->next->word, "Loop:"));
         wp = wp->next->next->next->next;
         assert(sscanf(wp->word, "%d", &n) == 1);

         lp->nscal = n;
         lp->scnam = malloc(n*sizeof(char*));
         lp->ssets = malloc(n*sizeof(short));
         assert(lp->ssets);
         lp->suses = malloc(n*sizeof(short));
         assert(lp->suses);
         lp->rexp = malloc(n*sizeof(short));
         assert(lp->rexp);
         lp->styp = malloc(n);
         assert(lp->styp);
/*
 *       Loop over all moving pointers
 */
         for (i=0; i < n; i++)
         {
            FKO_FreeAllWords(il.words);
            assert(FKO_ParseInfoLine(&il, fpin));
            assert(il.nwords == 5);
            wp = il.words;
            lp->scnam[i] = FKO_Word2Name(wp);

            wp = wp->next;
            sp = wp->word;
            assert(!strncmp(sp, "type=", 5));
            lp->styp[i] = FKO_TypeStrToInt(sp+5);

            wp = wp->next;
            sp = wp->word;
            assert(!strncmp(sp, "uses=", 5));
            assert(sscanf(sp+5, "%hd", lp->suses+i) == 1);

            wp = wp->next;
            sp = wp->word;
            assert(!strncmp(sp, "sets=", 5));
            assert(sscanf(sp+5, "%hd", lp->ssets+i) == 1);

            wp = wp->next;
            sp = wp->word;
            assert(!strncmp(sp, "ReduceExpandable=", 17));
            if (sp[17] == '1')
               lp->rexp[nex++] = i;
         }
         lp->nexpand = nex;
         if (!nex)
         {
            free(lp->rexp);
            lp->rexp = NULL;
         }
      }
      else if (il.nwords == 1) /* common case of VAR=X */
      {
         int k;
         wp = il.words;
         sp = wp->word;
         k = wp->eqpos;
         n = FKO_GetIntFromEqWord(wp);
         if (k == 9)  /* MaxUnroll */
         {
            assert(!strncmp(sp, "MaxUnroll", 9));
            lp->maxunroll = n;
         }
         else if (k == 14)  /* LoopNormalForm */
         {
            assert(!strncmp(sp, "LoopNormalForm", 14));
            lp->LNF = n;
         }
         else if (k == 8)   /* NUMPATHS */
         {
            assert(!strncmp(sp, "NUMPATHS", 8));
            lp->npaths = n;
            if (n > 1)
            {
               for (k=0; k < 3; k++)
               {
                  FKO_FreeAllWords(il.words);
                  assert(FKO_ParseInfoLine(&il, fpin));
                  wp = il.words;
                  sp = wp->word;
                  if (wp->len == 13)  /* VECTORIZABLE: */
                  {
                     assert(!strcmp(sp, "VECTORIZABLE:"));
                     lp->vpath = FKO_GetBoolValsFromWords(lp->npaths, wp->next);
                  }
                  else if (wp->len == 21) /* EliminateAllBranches: */
                  {  /* this info redundant, so just ignore it */
                     assert(!strcmp(sp, "EliminateAllBranches:"));
                  }
                  else if (wp->eqpos == 6)  /* NUMIFS= */
                  {
                     int j;
                     assert(!strncmp(sp, "NUMIFS=", 6));
                     lp->nifs = FKO_GetIntFromEqWord(wp);
                     for (j=0; j < 3; j++)
                     {
                        FKO_FreeAllWords(il.words);
                        assert(FKO_ParseInfoLine(&il, fpin));
                        wp = il.words;
                        sp = wp->word;
                        assert(wp->eqpos > 15);
                        assert(!strncmp(sp+wp->eqpos-13, "EliminatedIfs=", 14));
                        if (wp->eqpos == 16) /* MaxEliminatedIfs */
                        {
                           assert(*sp == 'M');
                           if (sp[1] == 'a')
                           {
                              assert(sp[2] == 'x');
                              lp->MaxElimIfs = FKO_GetIntFromEqWord(wp);
                           }
                           else
                           {
                              assert(sp[2] == 'n');
                              lp->MinElimIfs = FKO_GetIntFromEqWord(wp);
                           }
                        }
                        else if (wp->eqpos == 20) /* RedCompEliminatedIfs= */
                        {
                           assert(!strncmp(sp, "RedComp", 7));
                           lp->rcElimIfs = FKO_GetIntFromEqWord(wp);
                        }
                        else
                           assert(0);
                     }
                  }
               }
            }
         }
      }
      else if (il.nwords == 2 || il.nwords == 3) /* VECTORIZATION: op1 [op2] */
      {
         assert(!strcmp(wp->word, "VECTORIZATION:"));
         sp = wp->next->word;
         lp->vec = 0;
         if (!strcmp(sp, "NONE"))
            lp->vec = 0;
         else if (strcmp(sp, "SpecVec"))
            lp->vec += 1;
         else if (strcmp(sp, "LoopLvl"))
            lp->vec += 2;
         else 
            assert(0);
         if (il.nwords == 3)
         {
            sp = wp->next->next->word;
            if (strcmp(sp, "SpecVec"))
               lp->vec += 1;
            else if (strcmp(sp, "LoopLvl"))
               lp->vec += 2;
            else 
               assert(0);
         }
      }
      else
      {
         fprintf(stderr, "Illegal number of words = %d ('%s', '%s')\n",
                 il.nwords, il.nwords > 0 ? il.words->word : "",
                 il.nwords > 1 ? il.words->next->word : "");
         assert(0);
      }
      FKO_FreeAllWords(il.words);
   }

   if (fpin != stdin)
      fclose(fpin);
   return(lp);
}

void FKO_GetOptLoopInfo(char *fname)
{
   FKO_OLOOPINF = FKO_GetOptLoopInfoC(fname);
}
