#ifndef FKO_OPTREG_H
#define FKO_OPTREG_H

char *Int2Reg(int i);
short Reg2Int(char *regname);
int NumberArchRegs();
void CalcIG(BLIST *blist);

#endif
