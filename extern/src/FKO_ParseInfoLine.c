#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "fko_parseinfo.h"
#include "fko_infoC.h"

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

static fko_word_t *FKO_NewWord(char *sp, int len, short eqpos)
{
   fko_word_t *wp;
   wp = malloc(sizeof(fko_word_t));
   assert(wp);

   if (sp && len > 0)
   {
      wp->word = malloc(len+1);
      wp->len = len;
      wp->eqpos = eqpos;
      strncpy(wp->word, sp, len);
      wp->word[len] = '\0';

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

static fko_word_t *FKO_GetNextWord(char **SP)
{
   char *sp = *SP;
   if (sp)
   {
      int i, eq=0, q=0;

      while (isspace(*sp))  /* skip leading whitespace */
         sp++;
      if (*sp == '\0')
         return(NULL);
      for (i=0; sp[i] && !isspace(sp[i]); i++)
      {
         if (sp[i] == '=')
            eq = i;
/*
 *       If we find a ', look for no other chars until we find end '
 */
         else if (sp[i] == '\'')
         {
             for (i++; sp[i] != '\0' && sp[i] != '\''; i++);
             assert(sp[i] == '\'');
             q = i;
         }
      }
      if (!i)
         return(NULL);
      *SP = sp + i;
      return(FKO_NewWord(sp, i, q?q-i:eq));
   }
   return(NULL);
}

static fko_word_t *FKO_SplitLineIntoWords(int *NW, char *sp)
{
   int nw=0;
   fko_word_t *wb=NULL;
   if (sp)
   {
      wb = FKO_GetNextWord(&sp);
      if (wb)
      {
         fko_word_t *wp=wb;
         do
         {
            nw++;
            wp->next = FKO_GetNextWord(&sp);
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

int FKO_GetIntFromEqWord(fko_word_t *wp)
{
   int i;
   assert(wp->eqpos > 0);
   assert(sscanf(wp->word+wp->eqpos+1, "%d", &i) == 1);
   return(i);
}

/*
 * RETURNS: a short with a bit set for each type that appears in word list
 */
short FKO_GetBVFromTypeList(fko_word_t *wp)
{
   short bv=0;
   while(wp)
   {
      char *sp=wp->word;
      char ch=(*sp);
      if (ch == 'v')
      {
         assert (sp[2] == '\0');
         ch = sp[1];
         if (ch == 'i')
            bv |= 1 << FKO_TVINT;
         else if (ch == 'f')
            bv |= 1 << FKO_TVFLT;
         else if (ch == 'd')
            bv |= 1 << FKO_TVDBL;
         else
            assert(0);
      }
      else
      {
         assert (sp[1] == '\0');
         if (ch == 'i')
            bv |= 1 << FKO_TINT;
         else if (ch == 'f')
            bv |= 1 << FKO_TFLT;
         else if (ch == 'd')
            bv |= 1 << FKO_TDBL;
         else
            assert(0);
      }
      wp = wp->next;
   }
   return(bv);
}

/*
 * RETURNS: FKO_NTYPES len short array, with 0 (type not mentioned in list)
 *    or value provided by word.
 */
short *FKO_GetShortArrayFromTypeList(fko_word_t *wp)
{
   short *va;

   va = calloc(FKO_NTYPES, sizeof(short));
   assert(va);

   while (wp)
   {
      int k;
      char *sp;

      sp = wp->word;
      if (sp[0] == 'v') /* vector type */
      {
         assert(sp[2] == '=');
         if (sp[1] == 'f')
            k = FKO_TVFLT;
         else if (sp[1] == 'd')
            k = FKO_TVDBL;
         else 
         {
            assert(sp[1] == 'i');
            k = FKO_TVINT;
         }
         assert(sscanf(sp+3, "%hi", va+k) == 1);
      }
      else              /* scalar type */
      {
         assert(sp[1] == '=');
         if (*sp == 'f')
            k = FKO_TFLT;
         else if (*sp == 'd')
            k = FKO_TDBL;
         else 
         {
            k = FKO_TINT;
            assert(*sp == 'i');
         }
         assert(sscanf(sp+2, "%hi", va+k) == 1);
      }
      wp = wp->next;
   }

   return(va);
}

/*
 * Gets nw-len short array from wp (must be at least nw long!)
 */
short *FKO_GetShortArrayFromWords(int nw, fko_word_t *wp)
{
   short *va;
   int i;

   va = malloc(nw*sizeof(short));
   assert(va);
   for (i=0; i < nw; i++, wp = wp->next)
   {
      assert(wp);
      assert(sscanf(wp->word, "%hi", va+i) == 1);
   }
   return(va);
}

char FKO_GetBoolFromWord(fko_word_t *wp)
{
   int len=wp->len;
   char ch=1;
   assert(len > 2);
   assert(wp->word[len-2] == '=');
   if (wp->word[len-1] == '0')
      ch = 0;
   else
      assert(wp->word[len-1] == '1');
   return(ch);
}
/*
 * Gets nw-len from wp words; each word must be just '1' or '0'
 */
char *FKO_GetBoolValsFromWords(int nw, fko_word_t *wp)
{
   char *bp;
   int i;
   bp = malloc(nw);
   assert(bp);
   for (i=0; i < nw; i++, wp=wp->next);
   {
      assert(wp);
      assert(wp->len == 1);
      if (wp->word[0] == '0')
         bp[i] = 0;
      else if (wp->word[0] == '1')
         bp[i] = 1;
      else
         assert(0);
   }
}

int FKO_TypeStrToInt(char *st)
{
   int ty;
   char ch = *st;
   
   if (ch == 'v')
   {
      ch = st[1];
      if (ch == 'i')
         ty = FKO_TVINT;
      else if (ch == 'd')
         ty = FKO_TVDBL;
      else if (ch == 'f')
         ty = FKO_TVFLT;
   }
   else if (ch == 'i')
      ty = FKO_TINT;
   else if (ch == 'd')
      ty = FKO_TDBL;
   else if (ch == 'f')
      ty = FKO_TFLT;
   return(ty);
}

char *FKO_Word2Name(fko_word_t *wp)
{
   char *sin, *sout;
   int i, len = wp->len;

   sin = wp->word;
   assert(*sin == '\'' && sin[len-2] == '\'' && sin[len-1] == ':');
   sout = malloc(len-2);
   assert(sout);

   len -= 3;
   sin++;
   for (i=0; i < len; i++)
      sout[i] = sin[i];
   sout[len] = '\0';

   return(sout);
}
