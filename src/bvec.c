/*
 * This file contains the routines necessary to manipulate the basic block
 * bit vectors
 */
#include "fko.h"
#define CHUNKSIZE 64
static int nvalloc=0,      /* # of slots allocated for bit vectors*/
           nvused=0,       /* # of slots presently in use */
           **bvecs=NULL,   /* ptr to individual bit vectors */
           *ni=NULL;       /* # of integers required by this bit vector */
int FKO_BVTMP=0;

static void NewVecChunk(int increase)
{
   int i, n;
   int **newv, *newni;
   n = nvalloc + increase;
   newv = malloc(sizeof(int*)*n);
   newni = malloc(sizeof(int)*n);
   for (i=0; i < nvused; i++)
   {
      newv[i] = bvecs[i];
      newni[i] = ni[i];
   }
   if (bvecs) free(bvecs);
   if (ni) free(ni);
   bvecs = newv;
   ni = newni;
   nvalloc = n;
}

static int GetUnusedBVI()
{
   int i, n;
   for (i=0; i < nvused; i++)
      if (!ni[i]) return(i);
   if (nvused == nvalloc) NewVecChunk(CHUNKSIZE);
   return(nvused++);
}

int NewBitVec(int size)
/*
 * Allocates a new bit vector with space for size elements
 */
{
   int nv, *v, i;

   i = GetUnusedBVI();
   nv = (size+32) >> 5;
   v = calloc(nv, sizeof(int));
   assert(v);
   bvecs[i] = v;
   ni[i] = nv;
   return(i+1);
}

void KillBitVec(int iv)
{
   free(bvecs[--iv]);
   bvecs[iv] = NULL;
   ni[iv] = 0;
}

int *ExtendBitVec(int iv, int nwords)
{
   int j, k;
   int *v;

   v = bvecs[NewBitVec(nwords*32)-1];
   for (k=ni[--iv], j=0; j < k; j++) v[j] = bvecs[iv][j];
   for (ni[iv] = nwords; j < nwords; j++) v[j] = 0;
   free(bvecs[iv]);
   bvecs[iv] = v;
   return(v);
}

void SetVecAll(int iv, int val)
/*
 * sets all bits in vector depending on val
 */
{
   int n, i;
   int *v;

   if (val) val = -1;
   v = bvecs[--iv];
   n = ni[iv];
   for (i=0; i < n; i++) v[i] = val;
}

void SetVecBit(int iv, int ibit, int val)
/*
 * Sets bit ibit in vector iv as indicated by val
 */
{
   int *v;
   int i, j, k;
   iv--;
   i = ibit >> 5;
/*
 * If we are out of space, get new bit vec and trash the old
 */
   if (i >= ni[iv]) ExtendBitVec(iv+1, i+1);
   v = &bvecs[iv][i];
   ibit -= i << 5;
   if (!val) *v &= ~(1<<ibit);
   else *v |= (1<<ibit);
}

int BitVecComb(int ivD, int iv1, int iv2, char op)
/*
 * ivD = iv1 | iv2; if ivD == 0, allocate new vector
 * RETURNS ivD
 */
{
   int i, n;
   iv1--; iv2--;
   n = ni[iv1];
   if (n > ni[iv2]) ExtendBitVec(iv2+1, n);
   else if (n < ni[iv2])
   {
      n = ni[iv2];
      ExtendBitVec(iv1+1, n);
   }
   if (!ivD) ivD = NewBitVec(n<<5) - 1;
   else ivD--;
   if (n > ni[ivD]) ExtendBitVec(ivD+1, n);
   if (op == '|')
      for (i=0; i < n; i++) bvecs[ivD][i] = bvecs[iv1][i] | bvecs[iv2][i];
   else if (op == '-')
      for (i=0; i < n; i++) bvecs[ivD][i] = bvecs[iv1][i] & ~bvecs[iv2][i];
   else /* op == '&' */
      for (i=0; i < n; i++) bvecs[ivD][i] = bvecs[iv1][i] & bvecs[iv2][i];
   return(ivD+1);
}

int BitVecCheckComb(int iv1, int iv2, char op)
/*
 * RETURNS : 0 if iv1 op iv2 is zero, nonzero otherwise
 */
{
   int i, n;
   iv1--; iv2--;
   n = ni[iv1];
   if (n > ni[iv2]) ExtendBitVec(iv2+1, n);
   else if (n < ni[iv2])
   {
      n = ni[iv2];
      ExtendBitVec(iv1+1, n);
   }
   if (op == '|')
   {
      for (i=0; i < n; i++)
         if (bvecs[iv1][i] | bvecs[iv2][i])
            return(1);
   }
   else if (op == '-')
   {
      for (i=0; i < n; i++) 
         if(bvecs[iv1][i] & ~bvecs[iv2][i])
            return(1);
   }
   else /* op == '&' */
   {
      for (i=0; i < n; i++) 
         if (bvecs[iv1][i] & bvecs[iv2][i])
            return(1);
   }
   return(0);
}

int BitVecCheck(int iv, int ibit)
/*
 * Returns value nonzero if ibit in bit vector iv is set, zero otherwise
 */
{
   int n, k;

   n = ni[--iv];
   k = ibit >> 5;
   ibit -= k << 5;
   return(bvecs[iv][k] & (1<<ibit));
}

int BitVecComp(int iv1, int iv2)
/*
 * RETURNS : 0 if vectors are the same, 1 otherwise
 */
{
   int i, n;
   iv1--; iv2--;
   n = ni[iv1];
   if (n > ni[iv2]) ExtendBitVec(iv2+1, n);
   else if (n < ni[iv2])
   {
      n = ni[iv2];
      ExtendBitVec(iv1+1, n);
   }
   for (i=0; i < n; i++) 
      if (bvecs[iv1][i] != bvecs[iv2][i]) return(1);
   return(0);
}

int BitVecDup(int ivD, int ivS, char op)
/*
 * Copies ivS to ivD, allocating ivD if necessary
 */
{
   int i, n;

   n = ni[--ivS];
   if (!ivD) ivD = NewBitVec(n<<5) - 1;
   else ivD--;
   if (n > ni[ivD]) ExtendBitVec(ivD+1, n);
   else if (n < ni[ivD])
   {
      n = ni[ivD];
      ExtendBitVec(ivS+1, n);
   }
   if (op == '~') for (i=0; i < n; i++) bvecs[ivD][i] = ~bvecs[ivS][i];
   else for (i=0; i < n; i++) bvecs[ivD][i] = bvecs[ivS][i];
   return(ivD+1);
}

int BitVecCopy(int ivD, int ivS)
{
   return(BitVecDup(ivD, ivS, '='));
}
int BitVecInvert(int ivD, int ivS)
{
   return(BitVecDup(ivD, ivS, '~'));
}

int GetSetBitX(int iv, int I)
/*
 * RETURNS ith bit that is 1
 */
{
   int i, j, n, v, k=0;

   assert(I > 0);
   n = ni[--iv];
   for (j=0; j < n; j++)
   {
      v = bvecs[iv][j];
      for (i=0; i < 32; i++)
      {
         if (v & (1<<i))
         {
            if (++k == I) return(j*32+i);
         }
      }
   }
   return(0);
}

int CountBitsSet(int iv)
/*
 * RETURNS: number of bits set in bitvec iv
 */
{
   int i, j, k, m, n=0;
   m = ni[--iv];
   for (j=0; j < m; j++)
   {
      k = bvecs[iv][j];
      for (i=0; i < 32; i++)
         if (k & (1<<i)) n++;
   }
   return(n);
}

int AnyBitsSet(int iv)
/*
 * Returns : 1st bit set, 0 if none set
 */
{
   int i, j, k, n;
   n = ni[--iv];
   for (j=0; j < n; j++)
   {
      k = bvecs[iv][j];
      for (i=0; i < 32; i++)
         if (k & (1<<i))
            return(j*32+i+1);
   }
   return(0);
}

char *PrintVecList(int iv, int ioff)
/*
 * RETURNS: ptr to string containing # of all set bits
 */
{
   static char ln[2048];
   char *sptr;
   int i, j, k, n;

   sptr = ln;
   iv--;
   n = ni[iv];
   for (j=0; j < n; j++)
   {
      k = bvecs[iv][j];
      for (i=0; i < 32; i++)
      {
         if (k & (1<<i))
            sptr += sprintf(sptr, "%d, ", i+ioff);
      }
   }
   if (sptr != ln) sptr[-2] = '\0';
   return(ln);
}

int Array2BitVec(int n, short *sp, short off)
/*
 * Given array of shorts, creates a bitvec with bits set at pos sp[]+off
 */
{
   static int iv=0;
   int i;

   if (!iv)
      iv = NewBitVec(32);
   else
      SetVecAll(iv, 0);
   for (i=0; i < n; i++)
      SetVecBit(iv, sp[i]+off, 1);
   return(iv);
}
short *BitVec2StaticArray(int iv)
/*
 * Translates a bitvector to an array of shorts, where each element holds the
 * position of the set bit.  Position 0 is N, the length of the array
 * (the number of bits set).
 * RETURNS: (N+1) length short static array
 */
{
   int i, j, n;
   static int N=0;
   static short *vals=NULL;

   if (iv > 0)
   {
      n = CountBitsSet(iv);
      if (n >= N)
      {
         if (vals) free(vals);
         N = n+1;
         vals = malloc(N*sizeof(short));
         assert(vals);
      }
      vals[0] = n;
      for (i=1; i <= n; i++)
         vals[i] = GetSetBitX(iv, i);
      #if IFKO_DEBUG_LEVEL > 0  && 0
         fprintf(stderr, "ivals = ");
         for (i=1; i <= n; i++)
            fprintf(stderr, "%d,", vals[i]);
         fprintf(stderr, "\n");
      #endif
   }
   else
   {
      N = 0;
      if (vals) free(vals);
      vals = NULL;
   }
   return(vals);
}

short *BitVec2Array(int iv, int off)
/*
 * Translates a bitvector to an array of shorts, where each element holds the
 * (position of the set bit + off)
 * Position 0 is N, the length of the array (the number of bits set).
 * RETURNS: (N+1) length integer array
 */
{
   int i, j, n;
   short *vals;

   n = CountBitsSet(iv);
   vals = malloc((n+1)*sizeof(short));
   assert(vals);
   vals[0] = n;
   for (i=1; i <= n; i++)
      vals[i] = GetSetBitX(iv, i) + off;
   #if IFKO_DEBUG_LEVEL > 0  && 0
      fprintf(stderr, "ivals = ");
      for (i=1; i <= n; i++)
         fprintf(stderr, "%d,", vals[i]);
      fprintf(stderr, "\n");
   #endif
   return(vals);
}
