#ifndef FKO_OPTREG_H
#define FKO_OPTREG_H

char *Int2Reg0(int i);
char *Int2Reg(int i);
short Reg2Int(char *regname);
int NumberArchRegs();
void CalcIG(BLIST *blist);
int DoScopeRegAsg(BLIST *scope, int thresh, int *tnig);
int DoCopyProp(BLIST *scope);

#endif
