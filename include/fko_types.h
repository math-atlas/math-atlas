#ifndef FKO_TYPES_H
#define FKO_TYPES_H

#define uchar unsigned char

union valoff
{
   int i;
   short sa[4];/* used for locals/params: sa[0] indicates para # */
   float f;    /* sa[1] says you were ith para of your data type */
   double d;   /* sa[2] gives entry in deref table               */
};

typedef struct instq INSTQ;
struct instq
{
   INSTQ *prev, *next;
   short inst[4];
};

struct idlist
{
   char *name;
   struct idlist *next;
};

struct assmln
{
   char *ln;
   struct assmln *next;
};

struct locinit
{
   short id, con;
   struct locinit *next;
};

struct sdata  /* Structure for static data */
{
   char *name;           /* global variable name */
   short len;            /* length in chars of data */
   short align;          /* required memory alignment */
   unsigned char *vals;  /* values of data */
   struct sdata *next;
};

struct loopq
{
   int flag;
   short I, beg, end, inc;
   short body_label;
   short loopnum;
   short maxunroll;
   short *slivein, *sliveout, *adeadin, *adeadout, *nopf;
   short *aaligned;   /* arrays that have known alignment */
   uchar *abalign;    /* alignments of above arrays */
   INSTQ *ibeg, *iend;
   struct loopq *next;
};

#endif
