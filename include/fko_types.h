#ifndef FKO_TYPES_H
#define FKO_TYPES_H

#define uchar unsigned char

/*
 * FIXME: After implementing SV and UNROLLING of SV, short are no longer 
 * enough. bitvec exceeds 65535 .
 */
#if 1
   #define ushort unsigned short
#else /* doesn't work ! */
   #define ushort unsigned int
   typedef int short;
#endif

union valoff
{
   int i;
   short sa[4];/* used for locals/params: sa[0] indicates para # */
   float f;    /* sa[1] says you were ith para of your data type */
   double d;   /* sa[2] gives entry in deref table               */
   long l;
};

#ifdef IFKO_DECLARE
   char *optmnem[] = {"Do Nothing", "IG Reg Asg", "Copy Prop", "LP Reg Asg",
                      "Useless Jump Elim", "Useless Label Elim", 
                      "Branch Chaining", "Enforce Load Store", 
                      "Remove One Use Loads", "Last Use Load Removal",
                      "ReverseCopyProp", "NONE"};
   char *optabbr[] = {"DN", "ra", "cp", "gr", "uj", "ul", "bc", "ls", "u1", 
                      "lu", "rc", "00"};
#else
   extern char *optmnem[], *optabbr[];
#endif
enum FKOOPT {DoNothing, RegAsg, CopyProp, GlobRegAsg, UselessJmpElim, 
             UselessLabElim, BranchChain, EnforceLoadStore, 
             RemoveOneUseLoads, LastUseLoadRemoval, ReverseCopyProp, MaxOpt};
struct optblkq
{
   enum FKOOPT *opts;    /* list of ordered opts to perform                  */
   struct optblkq *down; /* opts included in while(change)                   */
   struct optblkq *next; /* succeeding opts not in while(change)             */
   struct optblkq *ifblk;/* if non-null, this is conditional block, and      *
                          * down is applied if ifblk produces changes, and   *
                          * next is applied if not                           */
   ushort nopt;          /* # of opt in opts                                 */
   ushort maxN;          /* if zero, do not do while(change)                 *
                          * if nonzero, do at most that many applications    */
   ushort bnum;          /* number of this block                             */
   ushort flag;
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
   short nupdate;         /* # of times updated */
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
/*
 * flag for vectors
 */
#define VS_LIVEIN  1   /* live on loop entry */
#define VS_LIVEOUT 2   /* live on loop exit */
#define VS_SET     4   /* set inside loop */
#define VS_ACC     8   /* updated with add */
#define VS_MUL     16  /* updated with mul */
#define VS_EQ      32  /* updated by assignment */
#define VS_ABS     64  /* updated by absolute value */
#define VS_MAX    128  /* updated as max var */
#define VS_MIN    256  /* updated as min var */
/*
 * flag for Loop markups
 */
/*#define LMU_MUTUALLY_ALIGNED 1*/ /*all ptr are mutually aligned inside loop */

typedef struct loopq LOOPQ;
struct loopq
{
   int flag, vflag;
   short ndup;        /* # of times body has been dupd (unroll & cleanup) */
   short depth;
   short I, beg, end, inc;
   short body_label;
   short end_label;   /* target for break */
   short CU_label;    /* -1: cleanup not required; 0: not gened yet; */
   short PTCU_label;  /* post-tail cleanup jmpback label */
   short NE_label;    /* no loop entry label */
   short loopnum;
   short maxunroll;
   short writedd;    /* write dependence distance */
   int LMU_flag;     /* to keep track of the loop mark up */
/* In vectorization, arrays that are not incremented are still called scalars*/
   short *varrs;     /* vectorized arrays */
   short *vscal;     /* vectorized scalars */
   short *vsflag;    /* info array for vscal */
   short *vsoflag;   /* info array for output vscal */
   short *vvscal;    /* vect locals used for vscal */
   short *bvvscal;   /* backup var for vvscal, needed in SV */
   short *nopf;      /* arrays which should not be prefetched */
   short *aaligned;  /* arrays that have known alignment */
   uchar *abalign;   /* alignments of above arrays */
   short malign;     /* if mutually aligned,what it aligned for, 0 not malign */  
   short *pfarrs;     /* arrays which should be prefetched */
   short *pfdist;     /* dist in bytes to prefetch pfarrs */
   short *pfflag;     /* flag for prefetch */
   short *ae;         /* accumulators to expand */
   short *ne;         /* # of acc to use for each original acc */
   short **aes;       /* shadow acc for each ae */
/*
 * Majedul: max/min vars can also be candidate for variable expansion
 * (scalar/vector expansion). So, keep data structure for that
 * NOTE: Right now, consider only max vars
 * NOTE: after adding any new data structure, must update InvalidateLoopInfo 
 * in flow.c
 */
   short *se;         /* scalar vars to expand */
   short *nse;         /* # of expanded vars to use for each original var */
   short **ses;       /* shadow scalar/vectors for each var to expand */
   int *seflag;       /* flag to indicate type of scalar: SC_ACC/SC_MAX/SC_MIN*/
/*
 * to keep track for -rc, -mmr 
 */
   short *maxvars;      /* to track max vars, as redundant xform changes it*/
   short *minvars;      /* to track min vars, as redundant xform changes it*/
   
   BBLOCK *preheader, *header;
   BLIST *tails, *posttails;
   BLIST *blocks;     /* blocks in the loop */
   ushort blkvec;     /* bitvec equivalent of blocks */
   ushort outs;       /* reg/var live on loop exit */
   ushort sets;       /* reg/var set in loop; not kept up to date (scratch)! */
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

#define LP_VEC 1                /* Loop Path vectorizable */
#define LP_OPT_LCONTROL 2       /* Loop Path loop control optimizable */
#define LP_OPT_MOVPTR 4         /* Loop Path Moving Ptr optimizable */


#define SC_SET 1
#define SC_USE 2
#define SC_PRIVATE 4
#define SC_LIVEIN 8
#define SC_LIVEOUT 16
#define SC_ACC 32
#define SC_MUL 64
#define SC_EQ 128
#define SC_ABS 256
#define SC_MIXED 512
#define SC_MAX 1024
#define SC_MIN 2048

typedef struct looppath LOOPPATH;
struct looppath    /* data structure for paths in loop */
{
   short pnum;                  /* path index of this path*/
   BLIST *blocks;               /* blocks in the path */
   BBLOCK *head;                /* head block of the path */
   BBLOCK *tail;                /* tail block of the path */
   ushort uses;                 /* var uses in the path */
   ushort defs;                 /* var defs in the path */
   struct ptrinfo *ptrs;        /* ptr info in the path */
   short *scal;                 /* scalar vars in path, skipped integer */
   short *sflag;                /* sc flag for scalar variable */
   short lpflag;                /* Vector/LoopControl opt flag */
/* data for vector paths. NOTE: vect local will be decided at final stage */
   short vflag;                /* vector type */
   short *varrs;                /* vectorized arrays */
   short *vscal;                /* vectorized scalars */
   short *vsflag;               /* info array for vscal */
   short *vsoflag;              /* info array for output vscal */
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
