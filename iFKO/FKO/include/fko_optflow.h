#ifndef FKO_OPTFLOW_H
#define FKO_OPTFLOW_H

int DoUselessJumpElim(void);
int DoUselessLabelElim(int nkeep, short *keeps);
int DoBranchChaining(void);


#endif
