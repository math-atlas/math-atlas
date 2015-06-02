#ifndef FKO_ARCHINFOC_H
   #define FKO_ARCHINFOC_H 1
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
   int nfpupipes;
      short *pipelen_add;
      short *pipelen_mul;
      short *pipelen_mac;
   int regtypes;
      short *numregs;  /* # of regs of each type */
      short *aliased;  /* array of BV, 1 per type */

   int ncaches;
      short *clsz;
   int vectypes;
   int nextinst;
};

fko_archinfo_t *FKO_GetArchInfoC(char *filename);
void FKO_DestroyArchInfoC(fko_archinfo_t *die);

int FKO_NumRegsC(fko_archinfo_t *ap, int typ1);
int FKO_RegtypesAliasedC(fko_archinfo_t *ap, int typ1, int typ2);

int FKO_NumCachesC(fko_archinfo_t *ap);
int FKO_CacheLinesizeC(fko_archinfo_t *ap, int cachelvl);

int FKO_PipelenADDC(fko_archinfo_t *ap, int typ);
int FKO_PipelenMULC(fko_archinfo_t *ap, int typ);
int FKO_PipelenMACC(fko_archinfo_t *ap, int typ);

int FKO_VeclenC(fko_archinfo_t *ap, int typ);
int FKO_HasSpecialInstC(fko_archinfo_t *ap, int typ, int inst);

#endif
