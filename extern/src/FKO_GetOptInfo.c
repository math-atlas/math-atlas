#define DECL_ 1
#include "fko_parseinfo.h"
#include "fko_info.h"

fko_archinfo_t *FKO_NewArchInfo(void)
{
   fko_archinfo_t *ap;
   ap = calloc(1, sizeof(fko_archinfo_t));
   return(ap);
}

fko_archinfo_t *FKO_GetArchInfoC(char *fnin)
{
   fko_archinfo_t *ap;
   FILE *fpin;
   fko_infoline_t il;

   if (!strcmp(fnin, "stdin"))
      fpin = stdin;
   else
   {
      fpin = fopen(fnin, "r");
      assert(fpin);
   }
   ap = FKO_NewArchInfo();

   while(FKO_ParseInfoLine(&il, fpin))
   {
      fko_word_t *wp;
      char *sp;
      int n, i;

      assert(il.nwords == 1);  /* all top-level arch are single words */
      wp = il.words;
      assert(wp->eqpos > 0);   /* all top-lvl arch end in =%d */
      switch(wp->eqpos)        /* avoid many strcmp */
      {
               /* 012345678901 */
      case 9:  /* PIPELINES= */
         assert(!strncmp(wp->word, "PIPELINES=", 9));
         n = FKO_GetIntFromEqWord(wp);
         ap->npipes = n;
         for (i=0; i < n; i++)      /* PIPELEN_xxx: */
         {                          /* 012345678901 */
            short *nr;
            FKO_FreeAllWords(il.words);
            assert(FKO_ParseInfoLine(&il, fpin));
            wp = il.words;
            sp = wp->word;
            assert(wp->len == 12);
            assert(!strncmp(sp, "PIPELEN_", 8));
            wp = wp->next;
            assert(wp);
            nr = FKO_GetShortArrayFromTypeList(wp);
            sp += 8;
            if (*sp == 'A')
            {
               assert(sp[1] == 'D' && sp[2] == 'D');
               ap->pipelen_add = nr;
            }
            else if (*sp == 'D')
            {
               assert(sp[1] == 'I' && sp[2] == 'V');
               ap->pipelen_div = nr;
            }
            else
            {
               assert(*sp == 'M');
               if (sp[1] == 'U')
               {
                  assert(sp[2] == 'L');
                  ap->pipelen_mul = nr;
               }
               else
               {
                  assert(sp[1] == 'A');
                  assert(sp[2] == 'C');
                  ap->pipelen_mac = nr;
               }
            }
         }
         break;
      case 7:  /* NCACHES= */
         assert(!strncmp(wp->word, "NCACHES=", 7));
         n = FKO_GetIntFromEqWord(wp);
         ap->ncaches = n;
         if (n)
         {
            FKO_FreeAllWords(il.words);
            assert(FKO_ParseInfoLine(&il, fpin));
            assert(il.nwords == n+1);
            wp = il.words;
            assert(!strcmp(wp->word, "LINESIZES:"));
            ap->clsz = FKO_GetShortArrayFromWords(n, wp->next);
         }
         break;
      case 8:  /* REGTYPES, VECTYPES */
         sp = wp->word;
         if (*sp == 'R') 
         {
            int k;
            assert(!strncmp(sp, "REGTYPES=", 9));
            n = FKO_GetIntFromEqWord(wp);
            ap->regtypes = n;
            for (k=0; k < 2; k++)  /* need ALIASGROUPS & NUMREGS! */
            {
               FKO_FreeAllWords(il.words);
               assert(FKO_ParseInfoLine(&il, fpin));
               wp = il.words;
               sp = wp->word;
               if (*sp == 'N') /* NUMREGS: */
               {
                  assert(!strcmp(sp, "NUMREGS:"));
                  assert(n+1 == il.nwords);
                  ap->numregs = FKO_GetShortArrayFromTypeList(wp->next);
               }
               else            /* ALIASGROUPS= */
               {
                  int ng;        /* number of alias groups */
                  short *agBV, *typBV;
                  assert(!strncmp(sp, "ALIASGROUPS=", 12));
                  ng = FKO_GetIntFromEqWord(wp);
                  agBV = calloc(ng, sizeof(short));
                  assert(agBV);
/*
 *                Read in a bitvec for each aliasgroup
 */
                  for (i=0; i < ng; i++)
                  {
                     FKO_FreeAllWords(il.words);
                     assert(FKO_ParseInfoLine(&il, fpin));
                     wp = il.words;
                     assert(!strcmp(wp->word, "ALIASED:"));
                     agBV[i] = FKO_GetBVFromTypeList(wp->next);
                  }
/*
 *                Now create an array of NTYPE-len bitvecs which shows 
 *                all aliased registers for that type
 */
                  typBV = calloc(FKO_NTYPES, sizeof(short));
                  ap->aliased = typBV;
                  for (i=0; i < FKO_NTYPES; i++)
                  {
                     const int tbit = 1<<i;
                     int k;
                     typBV[i] = tbit;  /* always aliased with itself */
                     for (k=0; k < ng; k++)
                     {
                        const int bv=agBV[k];
                        if (bv & tbit)     /* this type in alias list: */
                           typBV[i] |= bv; /* all ali types ali wt this one */
                     }
                  }
                  free(agBV);
               }
            }
         }
         else /* VECTYPES */
         {
            assert(!strncmp(sp, "VECTYPES=", 9));
            n = FKO_GetIntFromEqWord(wp);
            ap->nvtyp = n;
            if (n)
            {
               FKO_FreeAllWords(wp);
               assert(FKO_ParseInfoLine(&il, fpin));
               assert (n+1 == il.nwords);
               wp = il.words;
               sp = wp->word;
               assert(!strcmp(sp, "VECLEN:"));
               wp = wp->next;
               assert(wp);
               ap->vlen = FKO_GetShortArrayFromTypeList(wp);
            }
         }
         break;
      case 12: /* EXTENDEDINST */
         assert(!strncmp(wp->word, "EXTENDEDINST=", 12));
         n = FKO_GetIntFromEqWord(wp);
         if (n)
         {
            short *bvs;
            bvs = ap->spcinst = calloc(FKO_NTYPES, sizeof(short));
            assert(bvs);
            for (i=0; i < n; i++)
            {
               short ibv, k;
               FKO_FreeAllWords(il.words);
               assert(FKO_ParseInfoLine(&il, fpin));
               wp = il.words;
               assert(wp);
               sp = wp->word;
               assert(wp->next);
               ibv = FKO_GetBVFromTypeList(wp->next);
               if (*sp == 'M')
               {
                  if (!strcmp(sp, "MININST:"))
                     k = FKO_SIMIN;
                  else
                  {
                     assert(!strcmp(sp, "MAXINST:"));
                     k = FKO_SIMAX;
                  }
               }
               else
               {
                  assert(!strcmp(sp, "CONDMOV:"));
                  k = FKO_SICMOV;
               }
               k = 1 << k;
               if (ibv & (1<<FKO_TINT))
                  bvs[FKO_TINT] |= k;
               if (ibv & (1<<FKO_TFLT))
                  bvs[FKO_TFLT] |= k;
               if (ibv & (1<<FKO_TDBL))
                  bvs[FKO_TDBL] |= k;

               if (ibv & (1<<FKO_TVINT))
                  bvs[FKO_TVINT] |= k;
               if (ibv & (1<<FKO_TVFLT))
                  bvs[FKO_TVFLT] |= k;
               if (ibv & (1<<FKO_TVDBL))
                  bvs[FKO_TVDBL] |= k;
            }
         }
         break;
      default:
         fprintf(stderr, "No = found in top-level archinfo string: '%s'!\n",
                 wp->word ? wp->word:"NULL");
      }
      FKO_FreeAllWords(il.words);
   }

   if (fpin != stdin)
      fclose(fpin);
   return(ap);
}

void FKO_GetArchInfo(char *fname)
{
   FKO_ARCHINF = FKO_GetArchInfoC(fname);
}
