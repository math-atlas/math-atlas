#ifndef IFKO_H
#define IFKO_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define IFKO_DEBUG_LEVEL 1

#include "fko_types.h"
#include "fko_symtab.h"
#include "fko_inst.h"
#include "fko_misc.h"
#include "fko_loop.h"
#include "fko_flow.h"
#include "fko_bvec.h"
#include "fko_vars.h"

#ifdef IFKO_DECLARE
   char rout_name[128];
   int rout_flag=0;
int CFU2D=0,      /* indicates that all BBLOCK fields except */
                  /* dom,uses,defs,ins,out are up to date */
    CFDOMU2D=0,   /* indicates that BBLOCK's dom field is up to date */
    CFUSETU2D=0,  /* BBLOCK's uses,defs,ins,out up to date */
    INUSETU2D=0,  /* INSTQ's use/set are up to date */
    INDEADU2D=0;  /* INSTQ's deads is up to date */
#else
   extern char rout_name[128];
   extern int rout_flag;
   extern int CFU2D, CFDOMU2D, CFUSETU2D, INUSETU2D, INDEADU2D;
#endif

#define IRET_BIT 0x1
#define FRET_BIT 0x2
#define DRET_BIT 0x4

/*
 * These are actually arch.c declarations for files that don't want to include
 * all of fko_arch.h
 */
int GetPtrType(void);

#endif
