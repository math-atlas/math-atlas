#ifndef FKO_PARSEINFO_H
   #define FKO_PARSEINFO_H 1

#include <stdio.h>

typedef struct fko_word fko_word_t;
struct fko_word
{
   char *word;
   unsigned short len;
   short eqpos;     /* >0: pos of =; <0 add to len to find last '; 0: neither */
   fko_word_t *next;
};

typedef struct fko_infoline fko_infoline_t;
struct fko_infoline
{
   int nwords;
   fko_word_t *words;
};

int FKO_ParseInfoLine(fko_infoline_t *ip, FILE *fp);
void FKO_FreeAllWords(fko_word_t *wp);
int FKO_GetIntFromEqWord(fko_word_t *wp);
short *FKO_GetShortArrayFromTypeList(fko_word_t *wp);
short FKO_GetBVFromTypeList(fko_word_t *wp);
short *FKO_GetShortArrayFromWords(int nw, fko_word_t *wp);
char FKO_GetBoolFromWord(fko_word_t *wp);
int FKO_TypeStrToInt(char *st);
char *FKO_Word2Name(fko_word_t *wp);

#endif
