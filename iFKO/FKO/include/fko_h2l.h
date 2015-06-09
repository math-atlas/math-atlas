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
void DoComment(char *str);
void DoEmptyReturn();
void DoGoto(char *name);
void DoVecInit(short vid, struct slist *elem);
void DoReduce(short sid, short vid, char op, short iconst);
void DoArrayBroadcast(short id, short ptr);
void DoArrayPrefetch(short lvl, short ptrderef, int wpf);
short AddArrayDeref(short array, short index, int offset);
struct loopq *DoLoop(short I, short start, short end, short inc,
                     short sst, short send, short sinc);
void DoIf(char op, short id, short avar, char *labnam);
short AddOpt2dArrayDeref(short base, short hdm, short ldm, int unroll);
void FinishLoop(LOOPQ *lp);
void HandlePtrArithNoSizeofUpate(short dest, short src0, char op, short src1);

#endif
