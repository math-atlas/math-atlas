#ifndef FKO_LOOP_H
#define FKO_LOOP_H

#include "fko_types.h"

#ifdef IFKO_DECLARE
   struct loopq *loopq=NULL;
#else
   extern struct loopq *loopq;
#endif
#define L_PINC_BIT      0x1   /* Positive incremement */
#define L_PSTART_BIT    0x2   /* Positive start */
#define L_PEND_BIT      0x4   /* Positive end   */
#define L_ZSTART_BIT    0x8   /* start may be 0 */
#define L_ZEND_BIT      0x10  /* end may be 0 */
#define L_NINC_BIT      0x20  /* Negative incremement */
#define L_NSTART_BIT    0x40  /* Negative start */
#define L_NEND_BIT      0x80  /* Negative end   */

#endif
