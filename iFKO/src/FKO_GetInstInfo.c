/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#define DECL_II_ 1
#include "fko_parseinfo.h"
#include "fko_info.h"

static fko_instinfo_t *FKO_NewInstInfo(void)
{
   fko_instinfo_t *ip;
   ip = calloc(1, sizeof(fko_instinfo_t));
   assert(ip);
   return(ip);
}

fko_instinfo_t *FKO_GetInstInfoC(char *fnin)
{
   fko_instinfo_t *ip;
   FILE *fpin;
   fko_infoline_t il;

   if (!strcmp(fnin, "stdin"))
      fpin = stdin;
   else
   {
      fpin = fopen(fnin, "r");
      assert(fpin);
   }
   ip = FKO_NewInstInfo();

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
      case 8:     /* LRSPILLS= */
         assert(!strncmp(wp->word, "LRSPILLS=", 8));
         ip->lrspills = n = FKO_GetIntFromEqWord(wp);
         for (i=0; i < n; i++)
         {
            FKO_FreeAllWords(il.words);
            assert(FKO_ParseInfoLine(&il, fpin));
            wp = il.words;
            sp = wp->word;
            if (wp->len == 8)  /* OPTLOOP: */
            {
               assert(!strcmp(sp, "OPTLOOP:"));
               ip->ospills = FKO_GetShortArrayFromTypeList(wp->next);
            }
            else if (wp->len == 7) /* GLOBAL: */
            {
               assert(!strcmp(sp, "GLOBAL:"));
               ip->gspills = FKO_GetShortArrayFromTypeList(wp->next);
            }
            else
               assert(0);
         }
         break;
      default:
         fprintf(stderr, "Illegal eqpos=%d in '%s'\n", wp->eqpos,
                 wp->word ? wp->word:"");
         assert(0);
      }
      FKO_FreeAllWords(il.words);
   }
   return(ip);
}

void FKO_GetInstInfo(char *fnin)
{
   FKO_INSTINF = FKO_GetInstInfoC(fnin);
}
