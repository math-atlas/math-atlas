#ifndef FKO_OPTREG_H
#define FKO_OPTREG_H

char *Int2Reg(int i);
char *Int2Reg0(int i);
short Reg2Int(char *regname);
int ireg2type(int ireg);
int Reg2Regstate(int k);
int FindLiveregs(INSTQ *here);
INT_BVI AllRegVec(int k);
int Scope2BV(BLIST *scope);
int CalcScopeIG(BLIST *scope);
int NumberArchRegs();
void CalcIG(BLIST *blist);
int DoScopeRegAsg(BLIST *scope, int thresh, int *tnig, int *nspill);
int DoCopyProp(BLIST *scope);
int DoLoopGlobalRegAssignment(LOOPQ *loop);
int DoRemoveOneUseLoads(BLIST *scope);
int DoLastUseLoadRemoval(BLIST *scope);
int DoReverseCopyProp(BLIST *scope);
int DoEnforceLoadStore(BLIST *scope);
int DoOptArchSpecInst(BLIST *scope);
#endif
