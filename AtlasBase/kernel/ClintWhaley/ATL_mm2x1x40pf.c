/* #define ATL_NOL2PREFETCH */
#include "atlas_misc.h"
#include "atlas_prefetch.h"

#ifndef KB
   #error This kernel requires KB=40!!
#elif KB != 40
   #error This kernel requires KB=40!!
#endif
#ifdef MB
   #if (MB/2)*2 != MB
      #error This kernel requires MB be a multiple of 2!!
   #endif
#endif
void ATL_USERMM
   (const int M, const int N, const int K, const TYPE alpha, const TYPE *A, const int lda, const TYPE *B, const int ldb, const TYPE beta, TYPE *C, const int ldc)
/*
 * matmul with TA=T, TB=N, MB=40, NB=40, KB=40,
 * lda=40, ldb=40, ldc=0, mu=2, nu=1, ku=40
 */
{
   const TYPE *stM = A + (M > 2 ? M*KB-KB2 : M*KB);
   const TYPE *stN = B + N*KB;
   #define incAk 40
   const int incAm = 40, incAn = -M*KB;
   #define incBk 40
   const int incBm = -40, incBn = 40;
   #ifdef TREAL
      #define incCm 2
   #else
      #define incCm 4
   #endif
   const int incCn = (ldc - M)SHIFT;
   TYPE *pC0=C;
   const TYPE *pA0=A;
   const TYPE *pB0=B;
   const TYPE *pfA=A+NBNB, *pfB=B+KB;
   register int k;
   register TYPE rA0, rA1;
   register TYPE rB0;
   register TYPE m0, m1, m2, m3, m4;
   register TYPE rC0_0, rC1_0;

   do /* N-loop */
   {
      do /* M-loop */
      {
         #ifdef BETA0
            rC0_0 = rC1_0 = 0.0; /* ATL_pfl1W(pC0); */
         #else
            rC0_0 = *pC0;
            #ifdef TREAL
               rC1_0 = pC0[1];
            #else
               rC1_0 = pC0[2];
            #endif
            #ifdef BETAX
               m4 = beta;
               rC0_0 *= m4;
               rC1_0 *= m4;
            #endif
         #endif
/*
 *       Start pipeline
 */
         rA0 = *pA0;
         rB0 = *pB0;
         rA1 = pA0[40];
         m0 = rA0 * rB0;
         m1 = rA1 * rB0;
         rA0 = pA0[1];
         rB0 = pB0[1];
         rA1 = pA0[41];
         m2 = rA0 * rB0;
         m3 = rA1 * rB0;
         rA0 = pA0[2];
         rB0 = pB0[2];
         rA1 = pA0[42];
         m4 = rA0 * rB0;

/*
 *       Completely unrolled K-loop
 */
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[3];
         rB0 = pB0[3];
         rA1 = pA0[43];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[4];
         rB0 = pB0[4];
         rA1 = pA0[44];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[5];
         rB0 = pB0[5];
         rA1 = pA0[45];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[6];
         rB0 = pB0[6];
         rA1 = pA0[46];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[7];
         rB0 = pB0[7];
         rA1 = pA0[47];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[8];
         rB0 = pB0[8];
         rA1 = pA0[48];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[9];
         rB0 = pB0[9];
         rA1 = pA0[49];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[10];
         rB0 = pB0[10];
         rA1 = pA0[50];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[11];
         rB0 = pB0[11];
         rA1 = pA0[51];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[12];
         rB0 = pB0[12];
         rA1 = pA0[52];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[13];
         rB0 = pB0[13];
         rA1 = pA0[53];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[14];
         rB0 = pB0[14];
         rA1 = pA0[54];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[15];
         rB0 = pB0[15];
         rA1 = pA0[55];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[16];
         rB0 = pB0[16];
         rA1 = pA0[56];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[17];
         rB0 = pB0[17];
         rA1 = pA0[57];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[18];
         rB0 = pB0[18];
         rA1 = pA0[58];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[19];
         rB0 = pB0[19];
         rA1 = pA0[59];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[20];
         rB0 = pB0[20];
         rA1 = pA0[60];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[21];
         rB0 = pB0[21];
         rA1 = pA0[61];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[22];
         rB0 = pB0[22];
         rA1 = pA0[62];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[23];
         rB0 = pB0[23];
         rA1 = pA0[63];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[24];
         rB0 = pB0[24];
         rA1 = pA0[64];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[25];
         rB0 = pB0[25];
         rA1 = pA0[65];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[26];
         rB0 = pB0[26];
         rA1 = pA0[66];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[27];
         rB0 = pB0[27];
         rA1 = pA0[67];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[28];
         rB0 = pB0[28];
         rA1 = pA0[68];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[29];
         rB0 = pB0[29];
         rA1 = pA0[69];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[30];
         rB0 = pB0[30];
         rA1 = pA0[70];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[31];
         rB0 = pB0[31];
         rA1 = pA0[71];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[32];
         rB0 = pB0[32];
         rA1 = pA0[72];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[33];
         rB0 = pB0[33];
         rA1 = pA0[73];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[34];
         rB0 = pB0[34];
         rA1 = pA0[74];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[35];
         rB0 = pB0[35];
         rA1 = pA0[75];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[36];
         rB0 = pB0[36];
         rA1 = pA0[76];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[37];
         rB0 = pB0[37];
         rA1 = pA0[77];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[38];
         rB0 = pB0[38];
         rA1 = pA0[78];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[39];
         rB0 = pB0[39];
         rA1 = pA0[79];
         rC1_0 += m3;
         m3 = rA0 * rB0;
/*
 *       Drain pipe on last iteration of K-loop
 */
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rC1_0 += m0;
         rC0_0 += m1;
         rC1_0 += m2;
         rC0_0 += m3;
         rC1_0 += m4;
         pA0 += incAk;
         pB0 += incBk;
         *pC0 = rC0_0;
         #ifdef TREAL
            pC0[1] = rC1_0;
         #else
            pC0[2] = rC1_0;
         #endif
         pC0 += incCm;
         pA0 += incAm;
         pB0 += incBm;
      }
      while(pA0 != stM);
#if !defined(MB) || MB == 0
      if (M > 2)
      {
#endif
         #ifdef BETA0
            rC0_0 = rC1_0 = 0.0;
         #else
            rC0_0 = *pC0;
            #ifdef TREAL
               rC1_0 = pC0[1];
            #else
               rC1_0 = pC0[2];
            #endif
            #ifdef BETAX
               rA0 = beta;
               rC0_0 *= rA0;
               rC1_0 *= rA0;
            #endif
         #endif
/*
 *       Start pipeline
 */
         rA0 = *pA0;
         rB0 = *pB0;
         rA1 = pA0[40];
         m0 = rA0 * rB0; ATL_pfl1R(pfB); ATL_pfl1R(pfB+4); ATL_pfl1R(pfB+8);   ATL_pfl1R(pfB+12);
         m1 = rA1 * rB0; 
         rA0 = pA0[1];
         rB0 = pB0[1];
         rA1 = pA0[41];
         m2 = rA0 * rB0;
         m3 = rA1 * rB0;
         rA0 = pA0[2];
         rB0 = pB0[2];
         rA1 = pA0[42];
         m4 = rA0 * rB0;

/*
 *       Completely unrolled K-loop
 */
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[3];
         rB0 = pB0[3];
         rA1 = pA0[43];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[4];
         rB0 = pB0[4];
         rA1 = pA0[44];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[5];
         rB0 = pB0[5];
         rA1 = pA0[45];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[6];
         rB0 = pB0[6];
         rA1 = pA0[46];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[7];
         rB0 = pB0[7];
         rA1 = pA0[47];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;  ATL_pfl1R(pfB+16); ATL_pfl1R(pfB+20); ATL_pfl1R(pfB+24);
         rA0 = pA0[8];
         rB0 = pB0[8];
         rA1 = pA0[48];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[9];
         rB0 = pB0[9];
         rA1 = pA0[49];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;   
         m4 = rA1 * rB0;
         rA0 = pA0[10];
         rB0 = pB0[10];
         rA1 = pA0[50];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;   
         m1 = rA1 * rB0;
         rA0 = pA0[11];
         rB0 = pB0[11];
         rA1 = pA0[51];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;   
         m3 = rA1 * rB0;
         rA0 = pA0[12];
         rB0 = pB0[12];
         rA1 = pA0[52];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[13];
         rB0 = pB0[13];
         rA1 = pA0[53];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[14];
         rB0 = pB0[14];
         rA1 = pA0[54];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[15];
         rB0 = pB0[15];
         rA1 = pA0[55];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[16];
         rB0 = pB0[16];
         rA1 = pA0[56];
         rC1_0 += m2;
         m2 = rA0 * rB0;
                        
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[17];
         rB0 = pB0[17];
         rA1 = pA0[57];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[18];
         rB0 = pB0[18];
         rA1 = pA0[58];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[19];
         rB0 = pB0[19];
         rA1 = pA0[59];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[20];
         rB0 = pB0[20];
         rA1 = pA0[60];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[21];
         rB0 = pB0[21];
         rA1 = pA0[61];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0; ATL_pfl1R(pfB+28);  ATL_pfl1R(pfB+32); ATL_pfl1R(pfB+36);
         rA0 = pA0[22];
         rB0 = pB0[22];
         rA1 = pA0[62];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[23];
         rB0 = pB0[23];
         rA1 = pA0[63];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[24];
         rB0 = pB0[24];
         rA1 = pA0[64];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[25];
         rB0 = pB0[25];
         rA1 = pA0[65];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[26];
         rB0 = pB0[26];
         rA1 = pA0[66];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[27];
         rB0 = pB0[27];
         rA1 = pA0[67];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[28];
         rB0 = pB0[28];
         rA1 = pA0[68];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[29];
         rB0 = pB0[29];
         rA1 = pA0[69];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[30];
         rB0 = pB0[30];
         rA1 = pA0[70];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[31];
         rB0 = pB0[31];
         rA1 = pA0[71];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[32];
         rB0 = pB0[32];
         rA1 = pA0[72];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[33];
         rB0 = pB0[33];
         rA1 = pA0[73];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[34];
         rB0 = pB0[34];
         rA1 = pA0[74];
         rC1_0 += m3;
         m3 = rA0 * rB0;
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rA0 = pA0[35];
         rB0 = pB0[35];
         rA1 = pA0[75];
         rC1_0 += m0;
         m0 = rA0 * rB0;
         rC0_0 += m1;
         m1 = rA1 * rB0;
         rA0 = pA0[36];
         rB0 = pB0[36];
         rA1 = pA0[76];
         rC1_0 += m2;
         m2 = rA0 * rB0;
         rC0_0 += m3;
         m3 = rA1 * rB0;
         rA0 = pA0[37];
         rB0 = pB0[37];
         rA1 = pA0[77];
         rC1_0 += m4;
         m4 = rA0 * rB0;
         rC0_0 += m0;
         m0 = rA1 * rB0;
         rA0 = pA0[38];
         rB0 = pB0[38];
         rA1 = pA0[78];
         rC1_0 += m1;
         m1 = rA0 * rB0;
         rC0_0 += m2;
         m2 = rA1 * rB0;
         rA0 = pA0[39];
         rB0 = pB0[39];
         rA1 = pA0[79];
         rC1_0 += m3;
         m3 = rA0 * rB0;
/*
 *       Drain pipe on last iteration of K-loop
 */
         rC0_0 += m4;
         m4 = rA1 * rB0;
         rC1_0 += m0;
         rC0_0 += m1;
         rC1_0 += m2;      pfB += 40;
         rC0_0 += m3;      pfA += 8;
         rC1_0 += m4;
         pA0 += incAk;
         pB0 += incBk;
         *pC0 = rC0_0;
         #ifdef TREAL
            pC0[1] = rC1_0;
         #else
            pC0[2] = rC1_0;
         #endif
         pC0 += incCm;
         pA0 += incAm;
         pB0 += incBm;
#if !defined(MB) || MB == 0
      }
#endif
      pC0 += incCn;
      pA0 += incAn;
      pB0 += incBn;
   }
   while(pB0 != stN);
}
#ifdef incAm
   #undef incAm
#endif
#ifdef incAn
   #undef incAn
#endif
#ifdef incAk
   #undef incAk
#endif
#ifdef incBm
   #undef incBm
#endif
#ifdef incBn
   #undef incBn
#endif
#ifdef incBk
   #undef incBk
#endif
#ifdef incCm
   #undef incCm
#endif
#ifdef incCn
   #undef incCn
#endif
#ifdef incCk
   #undef incCk
#endif
#ifdef Mb
   #undef Mb
#endif
#ifdef Nb
   #undef Nb
#endif
#ifdef Kb
   #undef Kb
#endif
