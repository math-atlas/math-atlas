#ifndef FKO_BVEC_H
#define FKO_BVEC_H

int NewBitVec(int size);
int *ExtendBitVec(int iv, int nwords);
void SetVecAll(int iv, int val);
void SetVecBit(int iv, int ibit, int val);
int BitVecComb(int ivD, int iv1, int iv2, char op);
int BitVecComp(int iv1, int iv2);
int BitVecCopy(int ivD, int ivS);
int BitVecInvert(int ivD, int ivS);
int BitVecCheck(int iv, int ibit);
int BitVecCheckComb(int iv1, int iv2, char op);
int CountBitsSet(int iv);
char *PrintVecList(int iv, int ioff);
int Array2BitVec(int n, short *sp, short off);
short *BitVec2Array(int iv, int off);
short *BitVec2StaticArray(int iv);
void KillBitVec(int iv);

#endif
