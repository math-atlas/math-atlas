#ifndef IFKO_H
#define IFKO_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fko_types.h"
#include "fko_symtab.h"
#include "fko_inst.h"
#include "fko_misc.h"
#include "fko_loop.h"

#ifdef IFKO_DECLARE
   char rout_name[128];
   int rout_flag=0;
#else
   extern char rout_name[128];
   extern int rout_flag;
#endif

#define IRET_BIT 0x1
#define FRET_BIT 0x2
#define DRET_BIT 0x4

/*
 * These are actually arch.c declarations for files that don't want to include
 * all of fko_arch.h
 */
int GetPtrType(void);

#define IFKO_DEBUG_LEVEL 1
#endif
