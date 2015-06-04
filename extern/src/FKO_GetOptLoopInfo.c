#define DECL_OL_ 1
#include "fko_parseinfo.h"
#include "fko_info.h"

fko_olpinfo_t *FKO_NewOptLoopInfo(void)
{
   fko_olpinfo_t *lp;
   lp = calloc(1, sizeof(fko_olpinfo_t));
   return(lp);
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
      if (il.nwords == 4) /* Moving FP Pointers: X */
      {
         int npf=0;
         assert(!strcmp(wp->word, "Moving"));
         assert(!strcmp(wp->next->word, "FP"));
         assert(!strcmp(wp->next->next->word, "Pointers:"));
         wp = wp->next->next->next;
         assert(sscanf(wp->word, "%d", &n) == 1);

         lp->nmfptrs = n;
         lp->fptrs = malloc(n*sizeof(char*));
         assert(lp->fptrs);
         lp->fsets = malloc(n*sizeof(short));
         assert(lp->fsets);
         lp->fuses = malloc(n*sizeof(short));
         assert(lp->fuses);
         lp->pffp = malloc(n*sizeof(short));
         lp->ftyp = malloc(n);
         assert(lp->ftyp);
/*
 *       Loop over all moving pointers
 */
         for (i=0; i < n; i++)
         {
            FKO_FreeAllWords(il.words);
            assert(FKO_ParseInfoLine(&il, fpin));
            assert(il.nwords == 5);
            wp = il.words;
            lp->fptrs[i] = FKO_Word2Name(wp);

            wp = wp->next;
            sp = wp->word;
            assert(!strncmp(sp, "type=", 5));
            lp->ftyp[i] = FKO_TypeStrToInt(sp+5);

            wp = wp->next;
            sp = wp->word;
            assert(!strncmp(sp, "prefetch=", 9));
            if (sp[9] == '1')
               lp->pffp[npf++] = i;

            wp = wp->next;
            sp = wp->word;
            assert(!strncmp(sp, "sets=", 5));
            assert(sscanf(sp+5, "%hd", lp->fsets+i) == 1);

            wp = wp->next;
            sp = wp->word;
            assert(!strncmp(sp, "uses=", 5));
            assert(sscanf(sp+5, "%hd", lp->fuses+i) == 1);
         }
         lp->npref = npf;
         if (!npf)
         {
            free(lp->pffp);
            lp->pffp = NULL;
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
            assert(!strncmp(sp, "sets=", 5));
            assert(sscanf(sp+5, "%hd", lp->ssets+i) == 1);

            wp = wp->next;
            sp = wp->word;
            assert(!strncmp(sp, "uses=", 5));
            assert(sscanf(sp+5, "%hd", lp->suses+i) == 1);

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
         assert(k > 0);
         assert(sscanf(sp+k+1, "%d", &n) == 1);
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
         }
         else if (k == 6)   /* NUMIFS */
         {
            assert(!strncmp(sp, "NUMIFS", 6));
            lp->nifs = n;
            for (i=0; i < 3; i++)
            {
               FKO_FreeAllWords(il.words);
               assert(FKO_ParseInfoLine(&il, fpin));
            }
         }
         else if (k == 12)  /* VECTORIZABLE */
         {
            assert(!strncmp(sp, "VECTORIZABLE", 12));
            lp->vec = n;
            for (i=0; i < 3; i++)
            {
               FKO_FreeAllWords(il.words);
               assert(FKO_ParseInfoLine(&il, fpin));
               assert(il.nwords == 1);
               wp = il.words;
               sp = wp->word;
               k = wp->eqpos;
               if (k == 13)  /* SpeculationOK */
               {
                  int j;
                  n = lp->npaths;
                  for (j=0; j < n; j++)
                  {
                     FKO_FreeAllWords(il.words);
                     assert(FKO_ParseInfoLine(&il, fpin));
                  }
               }
            }
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
