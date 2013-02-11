#ifndef FKO_BVEC_H
#define FKO_BVEC_H

INT_BVI NewBitVec(int size);
INT32 *ExtendBitVec(INT_BVI iv, INT32 nwords);
void SetVecAll(INT_BVI iv, int val);
void SetVecBit(INT_BVI iv, int ibit, int val);
INT_BVI BitVecComb(INT_BVI ivD, INT_BVI iv1, INT_BVI iv2, char op);
int BitVecComp(INT_BVI iv1, INT_BVI iv2);
INT_BVI BitVecCopy(INT_BVI ivD, INT_BVI ivS);
INT_BVI BitVecInvert(INT_BVI ivD, INT_BVI ivS);
int BitVecCheck(INT_BVI iv, int ibit);
int BitVecCheckComb(INT_BVI iv1, INT_BVI iv2, char op);
int CountBitsSet(INT_BVI iv);
char *PrintVecList(INT_BVI iv, int ioff);
INT_BVI Array2BitVec(int n, short *sp, short off);
short *BitVec2Array(INT_BVI iv, int off);
short *BitVec2StaticArray(INT_BVI iv);
void KillBitVec(INT_BVI iv);

#endif
