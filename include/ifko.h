#ifndef IFKO_H
#define IFKO_H

#include <stdio.h>
#include <stdlib.h>

#define IFKO_DEBUG_LEVEL 1

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
         while(1); \
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
    INDEADU2D=0;  /* INSTQ's deads is up to date */

    int FKO_UR=0; /* unroll factor */
#else
   extern BBLOCK *bbbase;
   extern char rout_name[128];
   extern int rout_flag, FKO_FLAG;
   extern int CFU2D, CFDOMU2D, CFUSETU2D, INUSETU2D, INDEADU2D;
   extern int FKO_UR;
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

#define DO_ASS(flg_) (!((flg_) & IFF_NOASS))
#define DO_LIL(flg_) ((flg_) & IFF_LIL)
#define DO_KILLCOMMENTS(flg_) ((flg_) & IFF_KILLCOMMENTS)

#define IOPT_GLOB 0x1
/*
 * These are actually arch.c declarations for files that don't want to include
 * all of fko_arch.h
 */
int GetPtrType(void);

#endif
