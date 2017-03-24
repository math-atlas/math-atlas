/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
/*
 * This file contains the routines necessary to manipulate the basic block
 * bit vectors
 */
/*
 * TODO: We need to use macro for the type of nvalloc and nvused. 
 * here int is considered 32 bit. We will change it with custom type int32 
 * to make it sure. So, the declaration will look like it:
 *    
 * static INT_BVI nvalloc = 0, 
 *               nvused = 0;
 * static INT32 **bvecs=NULL,
 *              *ni=NULL;
 * static INT_BVI FKO_BVTMP = 0;
 *
 */
#include "fko.h"

/*#define CHUNKSIZE 512 */ 
static INT_BVI CHUNKSIZE=512; /* should be large as the LIL increased */

static INT_BVI nvalloc = 0,   /* # of slots allocated for bit vectors*/
               nvused = 0;    /* # of slots presently in use */
static INT32 **bvecs = NULL,  /* ptr to individual bit vectors */
             *ni = NULL;      /* # of integers required by this bit vector */

INT_BVI FKO_BVTMP = 0;

static void NewVecChunk(int increase)
{
   INT_BVI i, n;
   INT32 **newv, *newni;
   n = nvalloc + increase;
   newv = malloc(sizeof(INT32*)*n);
   newni = malloc(sizeof(INT32)*n);
   assert(newv && newni); /* Majedul: this check is added */
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

static INT_BVI GetUnusedBVI()
/*
 * NOTE: for extreme large bit vector, this is very expensive
 * FIXME: We can keep a table to point unused index to reduce time of this
 * function. We can use fixed-size table(or, queue) for this and update the 
 * table for each kill of bvec. When there are no unused bvec, we can search
 * the bvec table to update this table. It should reduce the computation time.
 */
{
   INT_BVI i;
   for (i=0; i < nvused; i++)
      if (!ni[i]) return(i);
#if 0
   if (nvused == nvalloc) NewVecChunk(CHUNKSIZE);
#else
   if (nvused == nvalloc)
   {
      CHUNKSIZE <<= 1;
      assert(CHUNKSIZE > 0); /* limit the value of CHUNKSIZE*/ 
      NewVecChunk(CHUNKSIZE);
   }
#endif
   return(nvused++);
}

INT_BVI NewBitVec(int size)
/*
 * Allocates a new bit vector with space for size elements
 */
{
   /*int nv, *v, i;*/
   INT_BVI i;
   INT32 nv, *v;

   i = GetUnusedBVI();
   nv = (size+32) >> 5;
   v = calloc(nv, sizeof(INT32));
   assert(v);
   bvecs[i] = v;
   ni[i] = nv;
   return(i+1);
}

void KillBitVec(INT_BVI iv)
{
   iv--;
   if(bvecs[iv]) free(bvecs[iv]);
   bvecs[iv] = NULL; /* it is necessary for KillAllBitVec()  */
   ni[iv] = 0;
}

void KillAllBitVec()
/*
 * Majedul: this function will kill all bit vectors. 
 * Needed when we restore the FKO state 0
 */
{
   INT_BVI i;
   for (i=0; i < nvused; i++) /* mem alloc upto nvused! */
   {
      if (bvecs[i])  /* already freed one is asigned NULL, may check ni[i]! */ 
         free(bvecs[i]);
      bvecs[i] = NULL;
      ni[i] = 0;
   }

   if (bvecs) free(bvecs);
   if (ni) free(ni);
/*
 * intialize all the global var with initial value
 */
   nvalloc=0;  
   nvused=0;  
   bvecs=NULL;
   ni=NULL;  
   FKO_BVTMP=0; /* need to init with 0 so that new bitvec will be allocated */
   CHUNKSIZE=512;
}

INT32 *ExtendBitVec(INT_BVI iv, INT32 nwords)
/*
 * To increase the size of specific bitvector
 * This function increases the size by allocating new memory keeping the index
 * same.
 * Majedul: This can be implemented simply by allocating a new memory without
 * using the NewBitVec function at all.
 */
{
   INT32 j, k;
   INT_BVI iv0;
   INT32 *v;
   
/*
 * FIXED: [Majedul] There was a segmentation fault assoicated with the follwoing
 * statement:  v = bvecs[NewBitVec(nwords*32)-1]; 
 * Reason: NewBitVec function changes the global pointer bvecs itselt. But the
 * compiler can't track this up and trying to store value using the old bvecs 
 * pointer. 
 */
   iv0 = NewBitVec(nwords*32)-1;
   v = bvecs[iv0];        
   for (k=ni[--iv], j=0; j < k; j++) v[j] = bvecs[iv][j];
   for (ni[iv] = nwords; j < nwords; j++) v[j] = 0;
/*
 * Majedul:
 * iv0 and iv never be same if ExtendedBitVec is called for already created
 * bit vector which should be the case. 
 */
   assert(iv0!=iv);
/*
 * Majedul:
 * Free the old allocated memory.
 * HERE HERE: both bvecs[iv0] and bvecs[iv] points the same memory!!!
 * make bvecs[iv0] NULL
 */
   free(bvecs[iv]);
   bvecs[iv0] = NULL;
   ni[iv0] = 0; 
   
   bvecs[iv] = v;
   return(v);
}

void SetVecAll(INT_BVI iv, int val)
/*
 * sets all bits in vector depending on val
 */
{
   INT32 n, i;
   INT32 *v;

   assert(iv); /* it should be applied on existing bitvec */

   if (val) val = -1;
   v = bvecs[--iv];
   assert(v);

   n = ni[iv];
   for (i=0; i < n; i++) v[i] = val;
}

void SetVecBit(INT_BVI iv, int ibit, int val)
/*
 * Sets bit ibit in vector iv as indicated by val
 */
{
   INT32 *v;
   INT32 i;
   assert(iv); /* should applied on existing one */
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

int BitVecCheck(INT_BVI iv, int ibit)
/*
 * Returns value nonzero if ibit in bit vector iv is set, zero otherwise
 */
{
   INT32 n, k;
/*
 * Majedul: added additional checking! iv != 0 and iv !=nvused
 */
   assert(iv>0 && iv<=nvused);
/*
 * FIXED: as bnum gets 0 when a new block is created, bnum-1 becomes -1
 * this is a major bug which creates the invalid read! 
 */
   assert(ibit!=-1); 

   n = ni[--iv];
   k = ibit >> 5; 
   ibit -= k << 5;
/*
 * FIXED: k should be < n. 
 *        (k >= n) means we want to set a bit but it is not allocated yet!
 * keep in mind: argument iv starts from 1 but ibit starts from 0.
 * NOTE: One way to solve this issue is to extend the bit vector and returns 
 * false. The logic behind this is that this position may be set afterward as a 
 * bit vector keep increasing from its default size of 32 anyway.
 */
   if (k >= n) 
   {
/*
 *    NOTE: For BitVecCheck, extending the bvec doesn't make sense. We will
 *    return 0 without Extending the bvec. SetBitVec will extend if neccessary.
 */
      /*ExtendBitVec(iv+1, k+1);*/
      return 0;
   }
   return(bvecs[iv][k] & (1<<ibit));
}

INT_BVI BitVecComb(INT_BVI ivD, INT_BVI iv1, INT_BVI iv2, char op)
/*
 * ivD = iv1 | iv2; if ivD == 0, allocate new vector
 * RETURNS ivD
 */
{
   int i, n;

   assert(iv1 && iv2); /* iv1 && iv2 should not be 0 */
   
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
   if (n < ni[ivD])
      for (n=ni[ivD]; i < n; i++) bvecs[ivD][i] = 0;
   return(ivD+1);
}

int BitVecCheckComb(INT_BVI iv1, INT_BVI iv2, char op)
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


int BitVecComp(INT_BVI iv1, INT_BVI iv2)
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

INT_BVI BitVecDup(INT_BVI ivD, INT_BVI ivS, char op)
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

INT_BVI BitVecCopy(INT_BVI ivD, INT_BVI ivS)
{
   return(BitVecDup(ivD, ivS, '='));
}
INT_BVI BitVecInvert(INT_BVI ivD, INT_BVI ivS)
{
   return(BitVecDup(ivD, ivS, '~'));
}

int GetSetBitX(INT_BVI iv, int I)
/*
 * RETURNS ith bit that is 1
 * NOTE: this function expends huge time for extreme large bvec table
 */
{
   int i, j, n, v, k=0;

   assert(I > 0);
   n = ni[--iv];
   for (j=0; j < n; j++)
   {
      v = bvecs[iv][j];
      if (!v) continue;       /*majedul: no need to check bit if v is 0 */
      for (i=0; i < 32; i++)
      {
         if (v & (1<<i))
         {
            if (++k == I) return(j*32+i+1);
         }
      }
   }
   return(0);
}

int CountBitsSet(INT_BVI iv)
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

int AnyBitsSet(INT_BVI iv)
/*
 * Returns : 1st bit set, 0 if none set
 */
{
   INT32 i, j, k, n;
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

char *PrintVecList(INT_BVI iv, int ioff)
/*
 * RETURNS: ptr to string containing # of all set bits
 */
{
/*
 * Majedul: bit vector may be beyond 4096 when unrolling factor is large. We
 * only print bitvec in debug code. Release code should not have this problem.
 * 
 * */
   static char ln[4096];
   char *sptr;
   int i, j, k, n;

   if (!iv)
      return("NOT SET");
   sptr = ln;
   iv--;
   n = ni[iv];
   for (j=0; j < n; j++)
   {
      k = bvecs[iv][j];
      for (i=0; i < 32; i++)
      {
         if (k & (1<<i))
            sptr += sprintf(sptr, "%d, ", j*32+i+ioff);
         assert((size_t)sptr < (size_t) ln + 4096);
      }
   }
   if (sptr != ln) sptr[-2] = '\0';
   return(ln);
}
#if 0
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
#else
INT_BVI Array2BitVecFlagged(int n, short *sp, short off, int init)
{
   static int iv=0;
   int i;
   if (!init)
   {
      if (!iv)
         iv = NewBitVec(32);
      else
         SetVecAll(iv, 0);
      for (i=0; i < n; i++)
         SetVecBit(iv, sp[i]+off, 1);
   }
   else
   {
      if (iv) KillBitVec(iv);
      iv = 0;
   }
   return(iv);
}
INT_BVI Array2BitVec(int n, short *sp, short off)
{
   return(Array2BitVecFlagged(n, sp, off, 0));
}
#endif
short *BitVec2StaticArray(INT_BVI iv)
   /*
    * Translates a bitvector to an array of shorts, where each element holds the
    * position of the set bit.  Position 0 is N, the length of the array
    * (the number of bits set).
    * RETURNS: (N+1) length short static array
    */
{
   int i, n;
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
         vals[i] = GetSetBitX(iv, i)-1;
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

short *BitVec2Array(INT_BVI iv, int off)
/*
 * Translates a bitvector to an array of shorts, where each element holds the
 * (position of the set bit + off)
 * Position 0 is N, the length of the array (the number of bits set).
 * RETURNS: (N+1) length integer array
 */
{
   int i, n;
   short *vals;

   n = CountBitsSet(iv);
   vals = malloc((n+1)*sizeof(short));
   assert(vals);
   vals[0] = n;
   for (i=1; i <= n; i++)
      vals[i] = GetSetBitX(iv, i)-1 + off;
   #if IFKO_DEBUG_LEVEL > 0  && 0
      fprintf(stderr, "ivals = ");
      for (i=1; i <= n; i++)
         fprintf(stderr, "%d,", vals[i]);
      fprintf(stderr, "\n");
   #endif
   return(vals);
}

/*
 * to print the static member of bvec
 */
void PrintBVecInfo(FILE *fout)
{
   fprintf(fout, "CHUNK_SIZE = %d\n", CHUNKSIZE);
   fprintf(fout, "nvused = %d\n", nvused);
   fprintf(fout, "nvalloc = %d\n", nvalloc);
}
