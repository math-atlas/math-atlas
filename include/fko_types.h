#ifndef FKO_TYPES_H
#define FKO_TYPES_H

#define uchar unsigned char
#define ushort unsigned short

union valoff
{
   int i;
   short sa[4];/* used for locals/params: sa[0] indicates para # */
   float f;    /* sa[1] says you were ith para of your data type */
   double d;   /* sa[2] gives entry in deref table               */
   long l;
};

typedef struct instq INSTQ;
struct instq
{
   struct bblock *myblk;
   INSTQ *prev, *next;
   short inst[4];
   ushort use, set, deads;
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

typedef struct bblock BBLOCK;
struct bblock
{
   ushort bnum;              /* block number */
   short ilab;               /* ST entry of label of block (if any) */
   BBLOCK *up, *down;       /* links blocks in code order */
   BBLOCK *usucc, *csucc;   /* (un)conditional successors */
   INSTQ  *inst1, *instN;   /* ptr to this block's 1st and last inst */
   INSTQ  *ainst1, *ainstN; /* ptr to block's 1st and last non-comment inst */
   struct loopq *loop;      /* set for header of loop only */
   struct blist *preds;     /* predecessors to this block */
   ushort dom;              /* dominators of this block */
   ushort uses, defs;       /* uses and defs for this block */
   ushort ins, outs;        /* live vars coming in and leaving block */
   struct iglist *conin, *conout;  /* in/out conflicts list */
   struct iglist *ignodes;         /* all of this block's ignodes */
};

typedef struct blist BLIST;
struct blist
{
   BLIST *next;
   BBLOCK *blk;
};

typedef struct loopq LOOPQ;
struct loopq
{
   int flag;
   short depth;
   short I, beg, end, inc;
   short body_label;
   short loopnum;
   short maxunroll;
   short writedd;     /* write dependence distance */
   short *slivein, *sliveout, *adeadin, *adeadout, *nopf;
   short *aaligned;   /* arrays that have known alignment */
   uchar *abalign;    /* alignments of above arrays */
   BBLOCK *preheader, *header;
   BLIST *blocks;     /* blocks in the loop */
   BLIST *footers, *postfooters;
   ushort blkvec;     /* bitvec equivalent of blocks */
   struct loopq *next;
};

typedef struct ignode IGNODE;
struct ignode
{
   BLIST *myblocks;             /* blocks live range spans */
   struct iglist *conflicts;    /* ignode's conflicting with this one */
   INSTQ *LRbeg, *LRend;        /* First and last inst of live range */
   int freq;                    /* # of uses of this live range */
   ushort myblkreg;             /* blocks live range spans as bitvec */
   ushort regstate;             /* registers being used at this point */
   short var;                   /* ST index of variable */
};

typedef struct iglist IGLIST;
struct iglist
{
   IGNODE *ignode;
   struct iglist *next;
};
#endif
