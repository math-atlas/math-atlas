#ifndef FKO_LOOP_H
#define FKO_LOOP_H

#include "fko_types.h"

#ifdef IFKO_DECLARE
   struct loopq *loopq=NULL, *optloop=NULL;
#else
   extern struct loopq *loopq, *optloop;
#endif
#define L_PINC_BIT      0x1   /* Positive incremement */
#define L_PSTART_BIT    0x2   /* Positive start */
#define L_PEND_BIT      0x4   /* Positive end   */
#define L_ZSTART_BIT    0x8   /* start may be 0 */
#define L_ZEND_BIT      0x10  /* end may be 0 */
#define L_NINC_BIT      0x20  /* Negative incremement */
#define L_NSTART_BIT    0x40  /* Negative start */
#define L_NEND_BIT      0x80  /* Negative end   */
#define L_IREF_BIT      0x100 /* index is refed in body */
#define L_MINC_BIT      0x200 /* END = BEG + INC*I for some non-zero I */
#define L_FORWARDLC_BIT 0x400 /* use forward loop control */
#define L_SIMPLELC_BIT  0x800 /* use simple loop control */
#define L_NSIMPLELC_BIT 0x1000 /* loop already in SimpleLC format */

#define PTRF_INC        0x1
#define PTRF_CONTIG     0x2
#define PTRF_CONSTINC   0x4
#define PTRF_MIXED      0x8


#endif
