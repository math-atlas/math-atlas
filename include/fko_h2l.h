#ifndef FKO_H2L_H
#define FKO_H2L_H

void DoConvert(short dest, short src);
void DoMove(short dest, short src);
void DoArrayStore(short ptr, short id);
void DoArrayLoad(short id, short ptr);
void HandlePtrArith(short dest, short src0, char op, short src1);
void DoArith(short dest, short src0, char op, short src1);
void DoReturn(short rret);
void DoLabel(char *name);

#endif
