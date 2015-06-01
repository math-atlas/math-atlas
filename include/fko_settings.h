#ifndef FKO_SETTINGS_H
   #define FKO_SETTINGS_H 1
/*
 * Describe the cache information if you know it below.  Fill in a linesize
 * macro for each level given in NCACHE.  
 * On intel machines, guess 64 for any level you don't know.
 */
#define NCACHE 2   /* number of lvls of cache iFKO knows about */
#define CLSZ1 64   /* linesize in bytes of first lvl of cache */
#define CLSZ2 64   /* linesize in bytes of second lvl of cache */
/* #define CLSZ3 128 */
/*
 * Uncomment next line if the FPU can perform an floating point and
 * accumulate (FMAC) without penalty
 */
/* #define ARCH_HAS_MAC 1 */

/*
 * Replace the 0's below with the length of your FPU pipeline.
 * If the FPU is unpipelined for that operation, make it -1.
 * If you are unsure, leave it zero. 
 */
#define PIPELEN_FADD
#define PIPELEN_FADD 0
#define PIPELEN_FMUL 0
#define PIPELEN_FDIV 0
#ifdef ARCH_HAS_MAC
   #define PIPELEN_FMAC 0
#endif

#endif
