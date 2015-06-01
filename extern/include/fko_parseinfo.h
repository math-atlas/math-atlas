#ifndef FKO_PARSEINFO_H
   #define FKO_PARSEINFO_H 1

typedef struct fko_word fko_word_t;
struct fko_word
{
   char *word;
   unsigned short len;
   fko_word_t *next;
};

typedef struct fko_infoline fko_infoline_t;
struct fko_infoline
{
   int nwords;
   fko_word_t *words;
};

#endif
