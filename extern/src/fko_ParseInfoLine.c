#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "fko_parseinfo.h"
#include "fko_archinfoC.h"

/*
 * Get a line with at least one non-whitespace character from file fp.
 * RETURNS: 0 if end of file is reached before finding a non-blank line
 *          ELSE: length of read in string (including whitespace!)
 */
static int GetLineWithChars(FILE *fp, int lnlen, char *ln)
{
   int i, HASCHARS=0;
   do
   {
      if (!fgets(ln, lnlen, fp))
         return(0);
      for (i=0; ln[i]; i++)   /* find end of string */
         if (!isspace(ln[i]))
            HASCHARS++;
      if (!HASCHARS)
         i=0;
   }
   while (!i);
   return(i);
}

/*
 * If fp is NULL, free internal storage and return NULL
 * If fp is non-NULL:
 * - RETURNS: line from file fp containing non-whitespace chars, 
 *            or NULL if all lines blank.
 */
static char *FKO_ReadLine(FILE *fp)
{
   int i;
   static int len=0;
   static char *ln=NULL;
/*
 * Free memory?
 */
   if (!fp)
   {
       if (ln)
          free(ln);
       len=0;
       return(NULL);
   }
/*
 * First time call, get space to read lines
 */
   if (!len)
   {
      ln = malloc(128);
      len=128;
      assert(ln);
   }
   i = GetLineWithChars(fp, len, ln);
   if (!i)
      return(NULL);
/*
 * If last character is not '\n', then we need a longer string to read
 * the whole thing!
 */
   while (ln[i-1] != '\n')
   {
      int len2;
      len2 = len+len;
      ln = realloc(ln, len2);
      assert(ln);
      if (!fgets(ln+i, len, fp))
         break;
      len = len2;
      for (; ln[i]; i++);  /* find end of string */
   }
   return(ln);
}

static fko_word_t *FKO_NewWord(char *sp, int len)
{
   fko_word_t *wp;
   wp = malloc(sizeof(fko_word_t));
   assert(wp);

   if (sp && len > 0)
   {
      wp->word = malloc(len+1);
      strcpy(wp->word, sp);
   }
   else
   {
      wp->word = NULL;
      wp->len = 0;
   }
   wp->next = NULL;
   return(wp);
}

static fko_word_t *FKO_FreeWord(fko_word_t *wp)
{
   fko_word_t *next=NULL;
   if (wp)
   {
      next = wp->next;
      if (wp->word)
         free(wp->word);
      free(wp);
   }
   return(next);
}

void FKO_FreeAllWords(fko_word_t *wp)
{
   while(wp)
      wp = FKO_FreeWord(wp);
}

static fko_word_t *FKO_GetNextWord(char *sp)
{
   if (sp)
   {
      int i;

      while (isspace(*sp))  /* skip leading whitespace */
         sp++;
      if (*sp == '\0')
         return(NULL);
      for (i=0; sp[i] && !isspace(sp[i]); i++);
      if (!i)
         return(NULL);
      return(FKO_NewWord(sp, i));
   }
   return(NULL);
}

static fko_word_t *FKO_SplitLineIntoWords(int *NW, char *sp)
{
   int nw=0;
   fko_word_t *wb=NULL;
   if (sp)
   {
      wb = FKO_GetNextWord(sp);
      if (wb)
      {
         fko_word_t *wp=wb;
         do
         {
            nw++;
            sp += wp->len;
            wp->next = FKO_GetNextWord(sp);
            wp = wp->next;
         }
         while(wp);
      }
   }
   *NW = nw;
   return(wb);
}

int FKO_ParseInfoLine(fko_infoline_t *ip, FILE *fp)
{
   char *ln;
   int nw;
   ln = FKO_ReadLine(fp);
   if (!ln)
      return(0);
   ip->words = FKO_SplitLineIntoWords(&nw, ln);
   ip->nwords = nw;
   return(nw);
}
