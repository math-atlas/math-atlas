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

typedef struct ilist ILIST;
struct ilist
{
   INSTQ *inst;
   ILIST *next;
};

struct ptrinfo
{
   ILIST *ilist;          /* ptrs to nupdate store inst */
   struct ptrinfo *next;
   short ptr;
   short flag;            /* see PTRF_* defined in fko_optloop.h */
   ushort nupdate;        /* # of times updated */
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
   ushort conin, conout;    /* IGnum conflicts in/out */
   ushort ignodes;          /* all of this block's ignodes */
};

typedef struct blist BLIST;
struct blist
{
   BLIST *next;
   BBLOCK *blk;
   void *ptr;   /* used only for ignodes */
};

typedef struct loopq LOOPQ;
struct loopq
{
   int flag;
   short ndup;        /* # of times body has been dupd (unroll & cleanup) */
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
   BLIST *tails, *posttails;
   BLIST *blocks;     /* blocks in the loop */
   ushort blkvec;     /* bitvec equivalent of blocks */
   ushort outs;       /* reg/var live on loop exit */
   ushort sets;       /* reg/var set in loop */
   struct iglist *iglist;
   struct loopq *next;
};

typedef struct ignode IGNODE;
struct ignode
{
   BLIST *blkbeg;               /* block(s) live range starts in */
   BLIST *blkend;               /* block(s) live range ends in */
   BLIST *blkspan;              /* block(s) live range completely spans */
   BLIST *ldhoist;              /* list places to hoist the load to */
   BLIST *stpush;               /* list places to push any store to */
   int nread;                   /* # of reads of var in this live range */
   int nwrite;                  /* # of writes of var in this live range */
   ushort myblkvec;             /* blocks live range includes as bitvec */
   ushort liveregs;             /* registers being used at this point */
   ushort conflicts;            /* IG conflicting with this one */
   short var;                   /* ST index of variable */
   short deref;                 /* deref entry of var */
   short ignum;                 /* IG index of this node */
   short reg;                   /* register assigned to this live range */
};

#if 0
typedef struct iglist IGLIST;
struct iglist
{
   IGNODE *ignode;
   struct iglist *next;
};
#endif

#endif
