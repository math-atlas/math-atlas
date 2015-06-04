#ifndef FKO_INFOC_H
   #define FKO_INFOC_H 1
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
   int nifs;
      int MaxElimIfs;
      int MinElimIfs;
      int redcElimIfs;
   int vec;
      int vmaxmin;
      int vredcomp;
      int specvec;
         char *svpath;   /* npaths-len bool, says whether path can be vec */
   int nmfptrs;     /* # of moving fp ptrs */
      char **fptrs;
      short *fsets;
      short *fuses;
      char *ftyp;
      int npref;      /* number of fptrs that are prefetchable */
         short *pffp; /* index of prefetchable ptrs */
   int nscal;
      short *ssets;
      short *suses;
      char **scnam;
      int nexpand;
         short *rexp;
      char *styp;
};

fko_olpinfo_t *FKO_GetOptLoopInfoC(char *filename);
void FKO_DestroyOptLoopInfoC(fko_olpinfo_t *die);
#endif
