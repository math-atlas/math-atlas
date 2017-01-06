#ifndef FKO_TYPES_H
#define FKO_TYPES_H

#define uchar unsigned char


#define ushort unsigned short

/*
 * FIXED: After implementing SV and UNROLLING of SV, short are no longer 
 * enough to index bit vector 
 */
/*
 * custom types for bvec
 */
#define INT_BVI unsigned int
#define INT32  int

#define INT_DTC int

union valoff
{
   int i;
   short sa[4];/* used for locals/params: sa[0] indicates para # */
   float f;    /* sa[1] says you were ith para of your data type */
   double d;   /* sa[2] gives entry in deref table               */
   long l;
};

typedef struct arrayinfo ARRAYINFO;
struct arrayinfo
{
   short ptr;     /* ST index of the array ptr */
   short ndim;    /* dimension of the array */
   short *ldas;   /* list of ST index of lda (higher to lower dim), size dim-1*/
   short *urlist; /* st of unroll factor for array (high to low dim), size dim*/
/* special list to manage 2D array only; for higher dimension need to expand */
   short *colptrs; /* column pointers only supported for 2D array now */
   short *cldas;  /* custom ldas like: lda, -lda, 3*lda to optimize in X86 */
};

typedef struct instq INSTQ;
struct instq
{
   struct bblock *myblk;
   INSTQ *prev, *next;
   short inst[4];
#if 0
   ushort use, set, deads;
#else
   INT_BVI use, set, deads;
#endif
};

typedef struct ilist ILIST;
struct ilist
{
   INSTQ *inst;
   ILIST *next;
};
/*
 * NOTE: upst is added to keep track of the value of a ptr updated by
 * by keep tracking this, we can unroll any PTRF_CONSTINC
 */
struct ptrinfo
{
   ILIST *ilist;          /* ptrs to nupdate store inst */
   struct ptrinfo *next;
   short ptr;
   short flag;            /* see PTRF_* defined in fko_optloop.h */
   short nupdate;         /* # of times updated */
   short upst;            /* ST index of the const  val: ptr = ptr + val; */
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

struct slist /* general list of short data, can be used as ST list */
{
   short id;
   struct slist *next;
};

struct sdata  /* Structure for static data */
{
   char *name;           /* global variable name */
   short len;            /* length in chars of data */
   short align;          /* required memory alignment */
   unsigned char *vals;  /* values of data */
   struct sdata *next;
};

#if 0
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
#else
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
   INT_BVI dom;              /* dominators of this block */
   INT_BVI uses, defs;       /* uses and defs for this block */
   INT_BVI ins, outs;        /* live vars coming in and leaving block */
   INT_BVI RDins, RDouts;    /* reaching defs coming in and leaving block */
   INT_BVI conin, conout;    /* IGnum conflicts in/out */
   INT_BVI ignodes;          /* all of this block's ignodes */
};
#endif

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
#define VS_SELECT 512  /* updated as select operation */
#define VS_SHADOW 1024 /* for shadowing with index */
#define VS_VLEN   2048 /* for extra vlen value in shadowing  */
/*
 * flag for Loop markups
 */
#define LMU_NO_CLEANUP  0x1
#define LMU_UNSAFE_RC  0x2
/*
 * NOTE: when even we add/update any element of this data structure, we need to
 * check following functions whether it affects or not:
 * InvalidateLoopInfo, KillFullLoop, 
 * WriteState0MiscToFile, ReadState0MiscFromFile
 */
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
   /*short writedd;*/    /* write dependence distance -- deleted since not used*/
   short itermul; 
   int LMU_flag;     /* to keep track of the loop mark up */
/* In vectorization, arrays that are not incremented are still called scalars*/
   short *varrs;     /* vectorized arrays */
   short *vscal;     /* vectorized scalars */
   short *vsflag;    /* info array for vscal */
   short *vsoflag;   /* info array for output vscal */
   short *vvscal;    /* vect locals used for vscal */
   short *bvvscal;   /* backup var for vvscal, needed in SV */
/*
 * NOTE: vvinit keeps special const/var STindex which may be needed to adjust 
 * initialization of a vector. 0 means vector is initialized normally with 
 * only the associated scalar;
 * meaning of other value depends on the special vectors. 
 * In shadow VRC, we need to initialize shadow vector, for example vimax on 
 * iamax, specially according to the whole instruction. Example:
 * imax = i + 1;  initialization of Vimax would be [i+1, i+1+1, i+1+2...]
 * vvinit will keep the const part.
 * NOTE: I keep it here to have this facility. There may be other case in future
 * where we would need this adjustment.
 */
   short *vvinit;    /* sometime special adjustment is needed to init a vector*/
/*
 * alignment related.
 * Note: mbalign indicate mutual alignmnet byte. Same value of the arrays in 
 * this list indicate mutual alignment among themselves
 * NOTE NOTE NOTE:
 * we support '*' for all ptrs/arrays in markup and there is no way to find out
 * moving ptr without analyis at that stage. So, we keep ptr as NULL and byte as
 * the aligned value to indicate that case. e.g., 
 * aaligned == NULL and abalign !=NULL (abalign[0] == 1) indicate that all 
 * arrays are aligned to the spicific byte found in abaligned[1]. 
 */
   short *aaligned;   /* arrays that have known alignment */
   short *abalign;    /* alignments of above arrays */
   short *maaligned;    /* arrays which are mutually aligned with other arrays */
   short *mbalign;    /* mutual alignment byte of the above array */
   short *faalign;    /* arrays need to be force aligned */
   short *fbalign;    /* alignments of the above arrays */
/*
 * depending on the markup, loop peeling select a ptr to forced align to veclen.
 * it is used in loop speicalization later. 
 */
   short fa2vlen;     /* forced align to vector length in loop peeling */ 

   short *nopf;      /* arrays which should not be prefetched */
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
#if 0   
   ushort blkvec;     /* bitvec equivalent of blocks */
   ushort outs;       /* reg/var live on loop exit */
   ushort sets;       /* reg/var set in loop; not kept up to date (scratch)! */
#else
   INT_BVI blkvec;     /* bitvec equivalent of blocks */
   INT_BVI outs;       /* reg/var live on loop exit */
   INT_BVI sets;       /* reg/var set in loop; not kept up to date (scratch)! */
#endif
   struct loopq *next;
};

typedef struct ignode IGNODE;
struct ignode
{
   int ignum;                 /* IG index of this node */
   BLIST *blkbeg;               /* block(s) live range starts in */
   BLIST *blkend;               /* block(s) live range ends in */
   BLIST *blkspan;              /* block(s) live range completely spans */
   BLIST *ldhoist;              /* list places to hoist the load to */
   BLIST *stpush;               /* list places to push any store to */
   int nread;                   /* # of reads of var in this live range */
   int nwrite;                  /* # of writes of var in this live range */
#if 0
   ushort myblkvec;             /* blocks live range includes as bitvec */
   ushort liveregs;             /* registers being used at this point */
   ushort conflicts;            /* IG conflicting with this one */
#else
   INT_BVI myblkvec;             /* blocks live range includes as bitvec */
   INT_BVI liveregs;             /* registers being used at this point */
   INT_BVI conflicts;            /* IG conflicting with this one */
#endif
   short var;                   /* ST index of variable */
   short deref;                 /* deref entry of var */
   short reg;                   /* register assigned to this live range */
};
/*
 * flag for path analysis
 */
#define LP_VEC 1                /* Loop Path vectorizable */
#define LP_OPT_LCONTROL 2       /* Loop Path loop control optimizable */
#define LP_OPT_MOVPTR 4         /* Loop Path Moving Ptr optimizable */
/*
 * flag for scalar variable analysis
 */
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
#define SC_SELECT 4096

typedef struct looppath LOOPPATH;
struct looppath    /* data structure for paths in loop */
{
   short pnum;                  /* path index of this path*/
   BLIST *blocks;               /* blocks in the path */
   BBLOCK *head;                /* head block of the path */
   BBLOCK *tail;                /* tail block of the path */
#if 0
   ushort uses;                 /* var uses in the path */
   ushort defs;                 /* var defs in the path */
#else
   INT_BVI uses;                 /* var uses in the path */
   INT_BVI defs;                 /* var defs in the path */
#endif
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

#define PK_INIT_MEM 1
#define PK_INIT_MEM_ACTIVE  2
#define PK_INIT_VLIST 4
#define PK_MEM_LOAD 8
#define PK_MEM_BROADCAST 16
#define PK_MEM_STORE 32
#define PK_ARITH_OP 64

typedef struct pack PACK;
struct pack     /* data structure to manage pack in SLP vectorization */
{
   int pnum;         /* pack id */
   short vflag;      /* type? */
   int vlen;         /* elements in vector */
   int isVec;        /* pack vectorizable? */
   int pktype;       /* pack type based on operation */
   ILIST *sil;       /* scalar/original instruction list */
   ILIST *vil;       /* vector instruction list */
   ILIST *depil;     /* point to the inst which this pack depends on */     
   INT_BVI uses;     /* uses in pack */
   INT_BVI defs;     /* defs in pack */
   short *scdef;     /* scalar vars which defs in sil */
   short *scuse;     /* scalar vars which in use in sil */
   short *vsc;       /* created vectors: dest, src1, src2 */
};
/*
 * NOTE: TYPE uses 0x 0~F. See fko_symtab.h
 */
#define NSLP_ACC 0x10
#define NSLP_SCAL 0x20
#define SLP_VBROADCAST 0x40

typedef struct slpvector SLP_VECTOR;
struct slpvector
{
   int flag;          /* type, livein, liveout */
/*
 * NOTE: since we have many statuses, we should use them in flag!!
 */
   int islivein;      /* need to create it in pre-header */
   int islive;        /* reached lived or dead by scalar update: schedule */
   int isused;     /* whether is used in blk slp */
   short vec;
   short redvar;      /* reduction var for svars, needed for vvrsums */
   int vlen;
   short *svars;      /* consecutive svars which this vector represented*/
   struct slpvector *next;
};

typedef struct lplist LPLIST;
struct lplist
{
   LOOPQ *loop;         /* loopq */
   struct lplist *next; 
};

#if 0
typedef struct iglist IGLIST;
struct iglist
{
   IGNODE *ignode;
   struct iglist *next;
};
#endif
/*
 * NOTE: Following definitions are for applying repeatable optimizations.
 */
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
   ushort flag;          /* IOPT_GLOB, IOPT_SCOP */
   int *nspill;   /* number of spilling in each type after this optblk 
                             is done, more precisely, number of live-range 
                             which don't get register calculated in RegAsg; 
                            -1 if not related to RegAsg */
};
/*
 * type for rout markups
 * NOTE: supported markups for alignment only now
 */
typedef struct rtmarkup RTMARKUP;
struct rtmarkup
{
   short *aaligned;   /* arrays that have known alignment */
   short *abalign;    /* alignments of above arrays */
   short *maaligned;    /* arrays which are mutually aligned with other arrays */
   short *mbalign;    /* mutual alignment byte of the above array */
   short *faalign;    /* arrays need to be force aligned */
   short *fbalign;    /* alignments of the above arrays */
};
#endif
