/*
 * This file prototypes & documents ATLAS's info querying functions which
 * are written to be called from any language that can interoperate with C.
 * They are not thread safe, because they allocate/deallocate structures
 * stored unsafely in a global data structure (this avoid returning pointers
 * and structures, which aren't supported by all languages).  
 * If you need thread safety, you'll have to directly use the C native 
 * parsing functions, documented in fko_infoC.h.
 */
#ifndef FKO_INFO_H
   #define FKO_INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fko_infoC.h"
#ifdef DECL_
   fko_archinfo_t *FKO_ARCHINF=NULL;
#else
   extern fko_archinfo_t *FKO_ARCHINF;
#endif
#ifdef DECL_OL_
   fko_olpinfo_t *FKO_OLOOPINF=NULL;
#else
   extern fko_olpinfo_t *FKO_OLOOPINF;
#endif
/*
 * Translates a string describing the FKO type into an integer for use
 * in the querying functions that that take type arguments.  str should
 * be a valid FKO type specifier.  FKO presently supports:
 *    "INT", "DOUBLE", "FLOAT" "VINT", "VDOUBLE", "VFLOAT"
 */
int FKO_TypestrToInt(char *str);

/*
 * Translate special instruction string to an integer that can be passed
 * to HasSpecialInst.  Legal inst: "cmov", "max", "min".
 */
int FKO_InststrToInt(char *inst);

/*
 * This function returns a handle to a FKO archinfo struct.  If it is called,
 * the memory isn't freed until the paired destroy func is called.
 * These functions must be called prior to any other funcs specified in this
 * file.  They are not thead safe.
 */
void FKO_GetArchInfo(char *fname);  /* "stdin" reads from standard in */
void FKO_DestroyArchInfo(void);

/* 
 * Functions describing registers
 */
int FKO_NumRegs(int typ);     /* # of registers for type typ */
int FKO_RegtypesAliased(int typ1, int typ2);  /* 1 if types aliased, else 0 */

/*
 * Functions describing cache info iFKO was compiled with.
 */
int FKO_NumCaches(void);  /* how many cache lvls iFKO knows about */
int FKO_CachelineSize(int lvl);  /* linesize for lvl [0,NC-1] */

/*
 * Functions concerned primarily with vectorization
 */
int FKO_Veclen(int typ); /* vector length (1: can't vectorize) type */

/*
 * Querying length of pipeline for add/multiply/multiply&accumulate.
 * 0: pipelen not specified.  -1: iFKO does not support inst on this arch
 */
int FKO_PipelenADD(int typ);
int FKO_PipelenMUL(int typ);
int FKO_PipelenMAC(int typ);
/* 
 * Misc functions
 */
int FKO_HasSpecialInst(int typ, int inst);  /* arch has max/min/cmov? */
/*
 *****************************************************************************
 * Below this line are optloop functions                                     *
 *****************************************************************************
 */
void FKO_GetOptLoopInfo(char *fname);  /* "stdin" reads from standard in */
void FKO_DestroyOptLoopInfo(void);
#endif
