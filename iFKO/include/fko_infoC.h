#ifndef FKO_INFOC_H
   #define FKO_INFOC_H 1
   #include <stdarg.h>
/*
 * These constants used to query for information about types
 */
#define FKO_NTYPES 6
#define FKO_TINT  0
#define FKO_TFLT  1
#define FKO_TDBL  2
#define FKO_TVINT 3
#define FKO_TVFLT 4
#define FKO_TVDBL 5

/*
 * These constants used to query for information about special instructions
 * supported by FKO on this architecture
 */
#define FKO_NSINST 3
#define FKO_SICMOV 0     /* cond move, required for redundant computation */
#define FKO_SIMAX  1     /* max instruction */
#define FKO_SIMIN  2     /* min instruction */

typedef struct fko_archinfo fko_archinfo_t;
struct fko_archinfo
{
   int ncaches;                /* # of caches FKO knows about */
      short *clsz;             /* cacheline size for each cache level */
   int npipes;                 /* number of pipelined units FKO knows */
      short *pipelen_add;      /* pipelen for each type */
      short *pipelen_mul;
      short *pipelen_mac;
      short *pipelen_div;
   int regtypes;
      short *numregs;  /* # of regs of each type */
      short *aliased;  /* array of BV, 1 per type */

   int nvtyp;
      short *vlen;
   int nspcinst;       /* # of special instructions */
      short *spcinst;

   int nextinst;
};

fko_archinfo_t *FKO_GetArchInfoC(char *filename);
void FKO_DestroyArchInfoC(fko_archinfo_t *die);

int FKO_NumRegsC(fko_archinfo_t *ap, int typ1);
int FKO_RegtypesAliasedC(fko_archinfo_t *ap, int typ1, int typ2);

int FKO_NumCachesC(fko_archinfo_t *ap);
int FKO_CachelineSizeC(fko_archinfo_t *ap, int cachelvl);

int FKO_PipelenADDC(fko_archinfo_t *ap, int typ);
int FKO_PipelenMULC(fko_archinfo_t *ap, int typ);
int FKO_PipelenMACC(fko_archinfo_t *ap, int typ);
int FKO_PipelenDIVC(fko_archinfo_t *ap, int typ);


int FKO_VeclenC(fko_archinfo_t *ap, int typ);
int FKO_HasSpecialInstC(fko_archinfo_t *ap, int typ, int inst);

typedef struct fko_olpinfo fko_olpinfo_t;
struct fko_olpinfo
{
   int maxunroll;
   int LNF;
   int npaths;           /* for now, assume 64 paths at most */
         char *vpath;    /* npaths-len bool, says whether path can be vec */
         int mmElim;     /* can -MMR eliminate all branches? */
         int rcElim;     /* can -RC eliminate all branches? */
   int nifs;
      int MaxElimIfs;
      int MinElimIfs;
      int rcElimIfs;
   int vec;              /* 0:no, 1:SV, 2:LNHV */
   int nmptrs;     /* # of moving fp ptrs */
      char **pnam;
      char *ptyp;
      short *psets;
      short *puses;
      short *plds;
      short *psts;
      int npref;      /* number of ptrs that are prefetchable */
         short *ppf; /* index of prefetchable ptrs */
   int n2ptrs;     /* # of moving fp ptrs */
      char **p2nam;
      char *p2typ;
      short *p2sets;
      short *p2uses;
      short *p2lds;
      short *p2sts;
      int n2pref;      /* number of ptrs that are prefetchable */
         short *p2pf;  /* index of prefetchable 2-D ptrs */
      short *p2cols;
      short *p2regs;
      short *p2ptrs;
   int nscal;
      char **scnam;
      char *styp;
      short *ssets;
      short *suses;
      int nexpand;
         short *rexp;
};

fko_olpinfo_t *FKO_GetOptLoopInfoC(char *filename);
void FKO_DestroyOptLoopInfoC(fko_olpinfo_t *die);

/*
 * typedef and functions for getting details about the generated instructions
 * Currently, just gives the number of live range spills for optloop & global
 */
typedef struct fko_instinfo fko_instinfo_t;
struct fko_instinfo
{
   int lrspills;  /* number of scopes where we see spills */
      short *ospills;
      short *gspills;
};

fko_instinfo_t *FKO_GetInstInfoC(char *filename);
void FKO_DestroyInstInfoC(fko_instinfo_t *die);

/*
 * Some handy commands for search
 */
int FKO_system(char *outs, int len, const char *frm, ...);
#endif
