#ifndef FKO_H
#define FKO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* Majedul: just to avoid the warning by compiler */
#include <ctype.h>

#define IFKO_DEBUG_LEVEL 1 /* to see all the msg */

#if IFKO_DEBUG_LEVEL >= 1
   #define Mstr2(m) # m
   #define Mstr(m) Mstr2(m)
   #define MyAssert(arg_) \
   { \
      if (!(arg_)) \
      { \
         fprintf(stderr, \
                 "\n\nassertion '%s' failed on line %d of %s, hanging:\n\n", \
                 Mstr(arg_), __LINE__, __FILE__);\
         /*while(1);*/ \
         exit(-1); \
      } \
   }
   #define assert MyAssert
#else
   #include <assert.h>
#endif


#include "fko_types.h"
#include "fko_symtab.h"
#include "fko_inst.h"
#include "fko_misc.h"
#include "fko_loop.h"
#include "fko_flow.h"
#include "fko_bvec.h"
#include "fko_vars.h"
#include "fko_arch.h"
#include "fko_optloop.h"
#include "fko_optflow.h"
#include "fko_optmisc.h"
#include "fko_optsimd.h"  
#include "fko_optreg.h"
#include "fko_h2l.h"

#ifdef IFKO_DECLARE
   BBLOCK *bbbase=NULL;
   char rout_name[128];
   int rout_flag=0;
   int FKO_FLAG=0;
   int CFU2D=0,   /* indicates that all BBLOCK fields except */
                  /* dom,uses,defs,ins,out are up to date */
    CFDOMU2D=0,   /* indicates that BBLOCK's dom field is up to date */
    CFUSETU2D=0,  /* BBLOCK's uses,defs,ins,out up to date */
    INUSETU2D=0,  /* INSTQ's use/set are up to date */
    INDEADU2D=0,  /* INSTQ's deads is up to date */
    CFLOOP=0;     /* Loop info up-to-date */

    int FKO_UR=0; /* unroll factor */
    int FKO_SB=0; /* stronger bet unroll factor */
    RTMARKUP rtmu;
#else
   extern BBLOCK *bbbase;
   extern char rout_name[128];
   extern int rout_flag, FKO_FLAG;
   extern int CFU2D, CFDOMU2D, CFUSETU2D, INUSETU2D, INDEADU2D;
   extern int CFLOOP, FKO_UR, FKO_SB;
   extern RTMARKUP rtmu;
#endif

#define IRET_BIT 0x1
#define FRET_BIT 0x2
#define DRET_BIT 0x4

#define IFF_NOASS 0x1   /* don't generate assembler file */
#define IFF_LIL   0x2   /* generate LIL file as <file>.l */
#define IFF_KILLCOMMENTS 0x4  /* don't print comments */
#define IFF_GENINTERM    0x8  /* generate intermediate files and quit */
#define IFF_READINTERM   0x10 /* Start from interm. files rather than HIL */
#define IFF_VECTORIZE    0x20 /* Do SIMD vectorization */
#define IFF_TAR          0x40 /* use use temporal vers for L1 read prefetch */
#define IFF_TAW          0x80 /* use use temporal vers for L1 write prefetch */
#define IFF_3DNOWR       0x100 /* use 3DNow! for L1 read prefetch */
#define IFF_3DNOWW       0x200 /* use 3DNow! for L1 write prefetch */
#define IFF_VERBOSE      0x400 /* verbose output */
#define IFF_OPT2DPTR     0x800 /* optimize 2d array access with min reg & ptr */
#define IFF_NODDE        0x1000 /* optimize 2d array access with min reg & ptr */
#define IFF_BESTVEC      0x2000 /*analyze all vector methods and apply best one*/
#define IFF_SHOWCOMMENTS 0x4000  /* show all comments in code, by default off */
/*
 * Majedul: 
 *    As we will introduce more and more new optimizations, I will keep 
 *    flag according to program states. 
 *    STATE1 has two optimization, hence 2 flags:
 *    STAT1_RC
 *    STAT1_MMC
 */
#define IFF_ST1_RC  0x1
#define IFF_ST1_MMR 0x2
#define IFF_ST3_SE 0x1
#define IFF_ST3_PREF 0x2

#define DO_ASS(flg_) (!((flg_) & IFF_NOASS))
#define DO_LIL(flg_) ((flg_) & IFF_LIL)
#define DO_KILLCOMMENTS(flg_) ((flg_) & IFF_KILLCOMMENTS)
#define DO_VECT(flg_) ((flg_) & IFF_VECTORIZE)

#define IOPT_GLOB 0x1
#define IOPT_SCOP 0x2

#define FKO_abs(x_) (((x_) >= 0) ? (x_) : -(x_))
/*
 * These are actually arch.c declarations for files that don't want to include
 * all of fko_arch.h
 */
int GetPtrType(void);
void UpdatePrefetchInfo();
void RestoreFKOState0();
void SaveFKOState0();
int IsPtrMinPossible(BLIST *scope);
int IsSimpleLoopNestVec(int vflag);
#endif
