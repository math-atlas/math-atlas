#ifndef FKO_L2A_H
#define FKO_L2A_H

void KillAllAssln(struct assmln *base);
struct assmln *lil2ass(BBLOCK *bp);
void dump_assembly(FILE *fp, struct assmln *abase);
struct assmln *DumpData(void);

#endif
