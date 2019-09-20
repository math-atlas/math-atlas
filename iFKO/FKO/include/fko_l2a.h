#ifndef FKO_L2A_H
#define FKO_L2A_H

/*
 * NOTE: AVX512 doesn't support YMM16~YMM31 register for some instructions 
 * which it doesn't extend from AVX...
 */
#ifdef IFKO_DECLARE
   int VREGSAVE=0;
#else
   extern int VREGSAVE;
#endif

void KillAllAssln(struct assmln *base);
struct assmln *lil2ass(BBLOCK *bp);
void dump_assembly(FILE *fp, struct assmln *abase);
struct assmln *DumpData(void);

#endif
