#ifndef FKO_VARS_H
   #define FKO_VARS_H

void CalcInsOuts(BBLOCK *base);
char *BV2VarNames(int iv);
void AddSetUseComments(BBLOCK *base);
void AddDeadComments(BBLOCK *base);
void CalcBlocksDeadVariables(BBLOCK *bp);
void CalcAllDeadVariables();

#endif
