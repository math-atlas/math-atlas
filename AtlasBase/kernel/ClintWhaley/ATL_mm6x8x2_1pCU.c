#include "atlas_misc.h"

static void ATL_dJIK0x0x0TN1x1x1_a1_b1
   (const int M, const int N, const int K, const TYPE alpha, const TYPE *A, const int lda, const TYPE *B, const int ldb, const TYPE beta, TYPE *C, const int ldc)
/*
 * matmul with TA=T, TB=N, MB=0, NB=0, KB=0, 
 * lda=0, ldb=0, ldc=0, mu=1, nu=1, ku=1
 */
{
   #define Mb M
   #define Nb N
   #define Kb K
   const TYPE *stM = A + (lda*Mb);
   const TYPE *stN = B + (ldb*Nb);
   #define incAk 1
   const int incAm = ((lda) - Kb), incAn = -(Mb*lda);
   #define incBk 1
   const int incBm = -(Kb), incBn = (ldb);
   #ifdef TREAL
      #define incCm 1
      const int incCn = (ldc) - (Mb);
   #else
      #define incCm 2
      const int incCn = ((ldc) - (Mb))<<1;
   #endif
   TYPE *pC0=C;
   const TYPE *pA0=A;
   const TYPE *pB0=B;
   register int k;
   register TYPE rA0;
   register TYPE rB0;
   register TYPE rC0_0;
   do /* N-loop */
   {
      do /* M-loop */
      {
         #ifdef BETA0
            rC0_0 = ATL_rzero;
         #elif defined(BETAX)
            rC0_0 = beta * *pC0;
         #else
            rC0_0 = *pC0;
         #endif
         for (k=K; k; k--) /* easy loop to unroll */
         {
            rA0 = *pA0;
            rB0 = *pB0;
            rC0_0 += rA0 * rB0;
            pA0 += incAk;
            pB0 += incBk;
         }
         *pC0 = rC0_0;
         pC0 += incCm;
         pA0 += incAm;
         pB0 += incBm;
      }
      while(pA0 != stM);
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
static void ATL_dJIK0x0x0TN6x1x1_a1_b1
   (const int M, const int N, const int K, const TYPE alpha, const TYPE *A, const int lda, const TYPE *B, const int ldb, const TYPE beta, TYPE *C, const int ldc)
/*
 * matmul with TA=T, TB=N, MB=0, NB=0, KB=0, 
 * lda=0, ldb=0, ldc=0, mu=6, nu=1, ku=1
 */
{
   const int Mb = (M/6)*6;
   #define Nb N
   #define Kb K
   const TYPE *ca=A, *cb=B;
   TYPE *cc=C;
   const TYPE *stM = A + (lda*Mb);
   const TYPE *stN = B + (ldb*Nb);
   #define incAk 1
   const int incAm = ((((lda) << 2)+((lda) << 1)) - Kb), incAn = -(Mb*lda);
   #define incBk 1
   const int incBm = -(Kb), incBn = (ldb);
   #ifdef TREAL
      #define incCm 6
      const int incCn = (ldc) - (Mb);
   #else
      #define incCm 12
      const int incCn = ((ldc) - (Mb))<<1;
   #endif
   TYPE *pC0=C;
   const TYPE *pA0=A, *pA1=pA0+(lda), *pA2=pA1+(lda), *pA3=pA2+(lda), *pA4=pA3+(lda), *pA5=pA4+(lda);
   const TYPE *pB0=B;
   register int k;
   register TYPE rA0, rA1, rA2, rA3, rA4, rA5;
   register TYPE rB0;
   register TYPE rC0_0, rC1_0, rC2_0, rC3_0, rC4_0, rC5_0;
   if (pA0 != stM)
   {
      do /* N-loop */
      {
         do /* M-loop */
         {
            #ifdef BETA0
               rC0_0 = rC1_0 = rC2_0 = rC3_0 = rC4_0 = rC5_0 = ATL_rzero;
            #else
               rC0_0 = *pC0;
               #ifdef TREAL
                  rC1_0 = pC0[1]; rC2_0 = pC0[2]; rC3_0 = pC0[3]; 
                  rC4_0 = pC0[4]; rC5_0 = pC0[5];
               #else
                  rC1_0 = pC0[2]; rC2_0 = pC0[4]; rC3_0 = pC0[6];
                  rC4_0 = pC0[8]; rC5_0 = pC0[10];
               #endif
               #ifdef BETAX
                  rA5 = beta;
                  rC0_0 *= rA5; rC1_0 *= rA5; rC2_0 *= rA5;
                  rC3_0 *= rA5; rC4_0 *= rA5; rC5_0 *= rA5;
               #endif
            #endif
            for (k=K; k; k--) /* easy loop to unroll */
            {
               rA0 = *pA0;
               rB0 = *pB0;
               rA1 = *pA1;
               rA2 = *pA2;
               rA3 = *pA3;
               rA4 = *pA4;
               rA5 = *pA5;
               rC0_0 += rA0 * rB0;
               rC1_0 += rA1 * rB0;
               rC2_0 += rA2 * rB0;
               rC3_0 += rA3 * rB0;
               rC4_0 += rA4 * rB0;
               rC5_0 += rA5 * rB0;
               pA0 += incAk;
               pA1 += incAk;
               pA2 += incAk;
               pA3 += incAk;
               pA4 += incAk;
               pA5 += incAk;
               pB0 += incBk;
            }
            *pC0 = rC0_0;
            #ifdef TREAL
               pC0[1] = rC1_0; pC0[2] = rC2_0; pC0[3] = rC3_0;
               pC0[4] = rC4_0; pC0[5] = rC5_0;
            #else
               pC0[2] = rC1_0; pC0[4] = rC2_0; pC0[6] = rC3_0;
               pC0[8] = rC4_0; pC0[10] = rC5_0;
            #endif
            pC0 += incCm;
            pA0 += incAm;
            pA1 += incAm;
            pA2 += incAm;
            pA3 += incAm;
            pA4 += incAm;
            pA5 += incAm;
            pB0 += incBm;
         }
         while(pA0 != stM);
         pC0 += incCn;
         pA0 += incAn;
         pA1 += incAn;
         pA2 += incAn;
         pA3 += incAn;
         pA4 += incAn;
         pA5 += incAn;
         pB0 += incBn;
      }
      while(pB0 != stN);
   }
   if (k=M-Mb)
      ATL_dJIK0x0x0TN1x1x1_a1_b1(k, N, K, alpha, ca + (Mb*lda), lda, cb, ldb, beta, cc + (Mb SHIFT), ldc);
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
static void ATL_dJIK0x0x0TN1x8x1_a1_b1
   (const int M, const int N, const int K, const TYPE alpha, const TYPE *A, const int lda, const TYPE *B, const int ldb, const TYPE beta, TYPE *C, const int ldc)
/*
 * matmul with TA=T, TB=N, MB=0, NB=0, KB=0, 
 * lda=0, ldb=0, ldc=0, mu=1, nu=8, ku=1
 */
{
   #define Mb M
   const int Nb = (N>>3)<<3;
   #define Kb K
   const TYPE *ca=A, *cb=B;
   TYPE *cc=C;
   const TYPE *stM = A + (lda*Mb);
   const TYPE *stN = B + (ldb*Nb);
   #define incAk 1
   const int incAm = ((lda) - Kb), incAn = -(Mb*lda);
   #define incBk 1
   const int incBm = -(Kb), incBn = (((ldb) << 3));
   #ifdef TREAL
      #define incCm 1
      #define ldc2 ldc
      const int incCn = (((ldc) << 3)) - (Mb);
   #else
      #define incCm 2
      const int incCn = ((((ldc) << 3)) - (Mb))<<1, ldc2 = ldc<<1;
   #endif
   TYPE *pC0=C, *pC1=pC0+(ldc2), *pC2=pC1+(ldc2), *pC3=pC2+(ldc2), 
        *pC4=pC3+(ldc2), *pC5=pC4+(ldc2), *pC6=pC5+(ldc2), *pC7=pC6+(ldc2);
   const TYPE *pA0=A;
   const TYPE *pB0=B, *pB1=pB0+(ldb), *pB2=pB1+(ldb), *pB3=pB2+(ldb), *pB4=pB3+(ldb), *pB5=pB4+(ldb), *pB6=pB5+(ldb), *pB7=pB6+(ldb);
   register int k;
   register TYPE rA0;
   register TYPE rB0, rB1, rB2, rB3, rB4, rB5, rB6, rB7;
   register TYPE rC0_0, rC0_1, rC0_2, rC0_3, rC0_4, rC0_5, rC0_6, rC0_7;
   if (pB0 != stN)
   {
      do /* N-loop */
      {
         do /* M-loop */
         {
            #ifdef BETA0
               rC0_0 = rC0_1 = rC0_2 = rC0_3 = rC0_4 = rC0_5 = 
               rC0_6 = rC0_7 = ATL_rzero;
            #else
               rC0_0 = *pC0; rC0_1 = *pC1; rC0_2 = *pC2;
               rC0_3 = *pC3; rC0_4 = *pC4; rC0_5 = *pC5;
               rC0_6 = *pC6; rC0_7 = *pC7;
               #ifdef BETAX
                  rB7 = beta;
                  rC0_0 *= rB7; rC0_1 *= rB7; rC0_2 *= rB7; rC0_3 *= rB7;
                  rC0_4 *= rB7; rC0_5 *= rB7; rC0_6 *= rB7; rC0_7 *= rB7;
               #endif
            #endif
            for (k=K; k; k--) /* easy loop to unroll */
            {
               rA0 = *pA0;
               rB0 = *pB0;
               rB1 = *pB1;
               rB2 = *pB2;
               rB3 = *pB3;
               rB4 = *pB4;
               rB5 = *pB5;
               rB6 = *pB6;
               rB7 = *pB7;
               rC0_0 += rA0 * rB0;
               rC0_1 += rA0 * rB1;
               rC0_2 += rA0 * rB2;
               rC0_3 += rA0 * rB3;
               rC0_4 += rA0 * rB4;
               rC0_5 += rA0 * rB5;
               rC0_6 += rA0 * rB6;
               rC0_7 += rA0 * rB7;
               pA0 += incAk;
               pB0 += incBk;
               pB1 += incBk;
               pB2 += incBk;
               pB3 += incBk;
               pB4 += incBk;
               pB5 += incBk;
               pB6 += incBk;
               pB7 += incBk;
            }
            *pC0 = rC0_0;
            *pC1 = rC0_1;
            *pC2 = rC0_2;
            *pC3 = rC0_3;
            *pC4 = rC0_4;
            *pC5 = rC0_5;
            *pC6 = rC0_6;
            *pC7 = rC0_7;
            pC0 += incCm;
            pC1 += incCm;
            pC2 += incCm;
            pC3 += incCm;
            pC4 += incCm;
            pC5 += incCm;
            pC6 += incCm;
            pC7 += incCm;
            pA0 += incAm;
            pB0 += incBm;
            pB1 += incBm;
            pB2 += incBm;
            pB3 += incBm;
            pB4 += incBm;
            pB5 += incBm;
            pB6 += incBm;
            pB7 += incBm;
         }
         while(pA0 != stM);
         pC0 += incCn;
         pC1 += incCn;
         pC2 += incCn;
         pC3 += incCn;
         pC4 += incCn;
         pC5 += incCn;
         pC6 += incCn;
         pC7 += incCn;
         pA0 += incAn;
         pB0 += incBn;
         pB1 += incBn;
         pB2 += incBn;
         pB3 += incBn;
         pB4 += incBn;
         pB5 += incBn;
         pB6 += incBn;
         pB7 += incBn;
      }
      while(pB0 != stN);
   }
   if (k=N-Nb)
      ATL_dJIK0x0x0TN1x1x1_a1_b1(M, k, K, alpha, ca, lda, cb + (Nb*ldb), ldb, beta, cc + (Nb*ldc2), ldc);
}
#ifdef ldc2
   #undef ldc2
#endif
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
void ATL_USERMM
   (const int M, const int N, const int K, const TYPE alpha, const TYPE *A, const int lda, const TYPE *B, const int ldb, const TYPE beta, TYPE *C, const int ldc)
/*
 * matmul with TA=T, TB=N, MB=0, NB=0, KB=0, 
 * lda=0, ldb=0, ldc=0, mu=6, nu=8, ku=2
 */
{
   const int Mb = (M/6)*6;
   const int Nb = (N>>3)<<3;
   const int Kb = (K>>1)<<1;
   #if K == 0
      const int Kstart = (K-1)>>1;
   #else
      const int Kstart = (K>>1) - 1;
   #endif
   const TYPE *stM = A + (lda*Mb);
   const TYPE *stN = B + (ldb*Nb), *ca=A, *cb=B;
   TYPE *cc=C;
   #define incAk 2
   const int incAm = ((((lda) << 2)+((lda) << 1)) - Kb), incAn = -(Mb*lda);
   #define incBk 2
   const int incBm = -(Kb), incBn = (((ldb) << 3));
   #ifdef TREAL
      #define incCm 6
      #define ldc2 ldc
      const int incCn = (((ldc) << 3)) - (Mb);
   #else
      #define incCm 12
      const int incCn = ((((ldc) << 3)) - (Mb))<<1, ldc2=ldc<<1;
   #endif
   TYPE *pC0=C, *pC1=pC0+(ldc2), *pC2=pC1+(ldc2), *pC3=pC2+(ldc2), 
        *pC4=pC3+(ldc2), *pC5=pC4+(ldc2), *pC6=pC5+(ldc2), *pC7=pC6+(ldc2);
   const TYPE *pA0=A, *pA1=pA0+(lda), *pA2=pA1+(lda), *pA3=pA2+(lda),
              *pA4=pA3+(lda), *pA5=pA4+(lda);
   const TYPE *pB0=B, *pB1=pB0+(ldb), *pB2=pB1+(ldb), *pB3=pB2+(ldb), 
              *pB4=pB3+(ldb), *pB5=pB4+(ldb), *pB6=pB5+(ldb), *pB7=pB6+(ldb);
   register int k;
   #ifdef BETAX
      TYPE *bp = (TYPE *) &beta;
   #endif
   register TYPE rA0, rA1, rA2, rA3, rA4, rA5;
   register TYPE ra0, ra1, ra2, ra3, ra4, ra5;
   register TYPE rB0, rB1, rB2, rB3, rB4, rB5, rB6, rB7;
   register TYPE rb0, rb1, rb2, rb3, rb4, rb5, rb6, rb7;
   register TYPE rC0_0, rC1_0, rC2_0, rC3_0, rC4_0, rC5_0,
                 rC0_1, rC1_1, rC2_1, rC3_1, rC4_1, rC5_1,
                 rC0_2, rC1_2, rC2_2, rC3_2, rC4_2, rC5_2,
                 rC0_3, rC1_3, rC2_3, rC3_3, rC4_3, rC5_3,
                 rC0_4, rC1_4, rC2_4, rC3_4, rC4_4, rC5_4,
                 rC0_5, rC1_5, rC2_5, rC3_5, rC4_5, rC5_5,
                 rC0_6, rC1_6, rC2_6, rC3_6, rC4_6, rC5_6,
                 rC0_7, rC1_7, rC2_7, rC3_7, rC4_7, rC5_7;
   if (pA0 != stM && pB0 != stN)
   {
   do /* N-loop */
   {
      do /* M-loop */
      {
         #ifdef BETA0
            rC0_0 = rC1_0 = rC2_0 = rC3_0 = rC4_0 = rC5_0 =
            rC0_1 = rC1_1 = rC2_1 = rC3_1 = rC4_1 = rC5_1 =
            rC0_2 = rC1_2 = rC2_2 = rC3_2 = rC4_2 = rC5_2 =
            rC0_3 = rC1_3 = rC2_3 = rC3_3 = rC4_3 = rC5_3 =
            rC0_4 = rC1_4 = rC2_4 = rC3_4 = rC4_4 = rC5_4 =
            rC0_5 = rC1_5 = rC2_5 = rC3_5 = rC4_5 = rC5_5 =
            rC0_6 = rC1_6 = rC2_6 = rC3_6 = rC4_6 = rC5_6 =
            rC0_7 = rC1_7 = rC2_7 = rC3_7 = rC4_7 = rC5_7 = ATL_rzero;
         #else
            #ifdef TREAL
               rC0_0 = *pC0;   rC1_0 = pC0[1]; rC2_0 = pC0[ 2];
               rC3_0 = pC0[3]; rC4_0 = pC0[4]; rC5_0 = pC0[ 5];
               rC0_1 = *pC1;   rC1_1 = pC1[1]; rC2_1 = pC1[ 2];
               rC3_1 = pC1[3]; rC4_1 = pC1[4]; rC5_1 = pC1[ 5];
               rC0_2 = *pC2;   rC1_2 = pC2[1]; rC2_2 = pC2[ 2];
               rC3_2 = pC2[3]; rC4_2 = pC2[4]; rC5_2 = pC2[ 5];
               rC0_3 = *pC3;   rC1_3 = pC3[1]; rC2_3 = pC3[ 2];
               rC3_3 = pC3[3]; rC4_3 = pC3[4]; rC5_3 = pC3[ 5];
               rC0_4 = *pC4;   rC1_4 = pC4[1]; rC2_4 = pC4[ 2];
               rC3_4 = pC4[3]; rC4_4 = pC4[4]; rC5_4 = pC4[ 5];
               rC0_5 = *pC5;   rC1_5 = pC5[1]; rC2_5 = pC5[ 2];
               rC3_5 = pC5[3]; rC4_5 = pC5[4]; rC5_5 = pC5[ 5];
               rC0_6 = *pC6;   rC1_6 = pC6[1]; rC2_6 = pC6[ 2];
               rC3_6 = pC6[3]; rC4_6 = pC6[4]; rC5_6 = pC6[ 5];
               rC0_7 = *pC7;   rC1_7 = pC7[1]; rC2_7 = pC7[ 2];
               rC3_7 = pC7[3]; rC4_7 = pC7[4]; rC5_7 = pC7[ 5];
            #else
               rC0_0 = *pC0;   rC1_0 = pC0[2]; rC2_0 = pC0[ 4];
               rC3_0 = pC0[6]; rC4_0 = pC0[8]; rC5_0 = pC0[10];
               rC0_1 = *pC1;   rC1_1 = pC1[2]; rC2_1 = pC1[ 4];
               rC3_1 = pC1[6]; rC4_1 = pC1[8]; rC5_1 = pC1[10];
               rC0_2 = *pC2;   rC1_2 = pC2[2]; rC2_2 = pC2[ 4];
               rC3_2 = pC2[6]; rC4_2 = pC2[8]; rC5_2 = pC2[10];
               rC0_3 = *pC3;   rC1_3 = pC3[2]; rC2_3 = pC3[ 4];
               rC3_3 = pC3[6]; rC4_3 = pC3[8]; rC5_3 = pC3[10];
               rC0_4 = *pC4;   rC1_4 = pC4[2]; rC2_4 = pC4[ 4];
               rC3_4 = pC4[6]; rC4_4 = pC4[8]; rC5_4 = pC4[10];
               rC0_5 = *pC5;   rC1_5 = pC5[2]; rC2_5 = pC5[ 4];
               rC3_5 = pC5[6]; rC4_5 = pC5[8]; rC5_5 = pC5[10];
               rC0_6 = *pC6;   rC1_6 = pC6[2]; rC2_6 = pC6[ 4];
               rC3_6 = pC6[6]; rC4_6 = pC6[8]; rC5_6 = pC6[10];
               rC0_7 = *pC7;   rC1_7 = pC7[2]; rC2_7 = pC7[ 4];
               rC3_7 = pC7[6]; rC4_7 = pC7[8]; rC5_7 = pC7[10];
            #endif
            #ifdef BETAX
               rb7 = *bp;
               rC0_0 *= rb7; rC1_0 *= rb7; rC2_0 *= rb7;
               rC3_0 *= rb7; rC4_0 *= rb7; rC5_0 *= rb7;
               rC0_1 *= rb7; rC1_1 *= rb7; rC2_1 *= rb7;
               rC3_1 *= rb7; rC4_1 *= rb7; rC5_1 *= rb7;
               rC0_2 *= rb7; rC1_2 *= rb7; rC2_2 *= rb7;
               rC3_2 *= rb7; rC4_2 *= rb7; rC5_2 *= rb7;
               rC0_3 *= rb7; rC1_3 *= rb7; rC2_3 *= rb7;
               rC3_3 *= rb7; rC4_3 *= rb7; rC5_3 *= rb7;
               rC0_4 *= rb7; rC1_4 *= rb7; rC2_4 *= rb7;
               rC3_4 *= rb7; rC4_4 *= rb7; rC5_4 *= rb7;
               rC0_5 *= rb7; rC1_5 *= rb7; rC2_5 *= rb7;
               rC3_5 *= rb7; rC4_5 *= rb7; rC5_5 *= rb7;
               rC0_6 *= rb7; rC1_6 *= rb7; rC2_6 *= rb7;
               rC3_6 *= rb7; rC4_6 *= rb7; rC5_6 *= rb7;
               rC0_7 *= rb7; rC1_7 *= rb7; rC2_7 *= rb7;
               rC3_7 *= rb7; rC4_7 *= rb7; rC5_7 *= rb7;
            #endif
         #endif
         rA0 = *pA0++; rA1 = *pA1++; rA2 = *pA2++; rA3 = *pA3++;
         rA4 = *pA4++; rA5 = *pA5++;
         rB0 = *pB0++; rB1 = *pB1++; rB2 = *pB2++; rB3 = *pB3++;
         rB4 = *pB4++; rB5 = *pB5++; rB6 = *pB6++; rB7 = *pB7++;
         for (k=Kstart; k; k--) /* easy loop to unroll */
         {
            rC0_0 += rA0 * rB0;
            rC1_0 += rA1 * rB0;
            rC2_0 += rA2 * rB0;
            rC3_0 += rA3 * rB0;
            rC4_0 += rA4 * rB0; rb0 = *pB0++;
            rC5_0 += rA5 * rB0;
            rC0_1 += rA0 * rB1;
            rC1_1 += rA1 * rB1; ra0 = *pA0++;
            rC2_1 += rA2 * rB1;
            rC3_1 += rA3 * rB1;
            rC4_1 += rA4 * rB1; ra1 = *pA1++;
            rC5_1 += rA5 * rB1;
            rC0_2 += rA0 * rB2;
            rC1_2 += rA1 * rB2; ra2 = *pA2++;
            rC2_2 += rA2 * rB2;
            rC3_2 += rA3 * rB2;
            rC4_2 += rA4 * rB2; ra3 = *pA3++;
            rC5_2 += rA5 * rB2;
            rC0_3 += rA0 * rB3;
            rC1_3 += rA1 * rB3; ra4 = *pA4++;
            rC2_3 += rA2 * rB3;
            rC3_3 += rA3 * rB3;
            rC4_3 += rA4 * rB3; ra5 = *pA5++;
            rC5_3 += rA5 * rB3;
            rC0_4 += rA0 * rB4;
            rC1_4 += rA1 * rB4; rb1 = *pB1++;
            rC2_4 += rA2 * rB4;
            rC3_4 += rA3 * rB4;
            rC4_4 += rA4 * rB4; rb2 = *pB2++;
            rC5_4 += rA5 * rB4;
            rC0_5 += rA0 * rB5;
            rC1_5 += rA1 * rB5; rb3 = *pB3++;
            rC2_5 += rA2 * rB5;
            rC3_5 += rA3 * rB5;
            rC4_5 += rA4 * rB5; rb4 = *pB4++;
            rC5_5 += rA5 * rB5;
            rC0_6 += rA0 * rB6;
            rC1_6 += rA1 * rB6; rb5 = *pB5++;
            rC2_6 += rA2 * rB6;
            rC3_6 += rA3 * rB6;
            rC4_6 += rA4 * rB6; rb6 = *pB6++;
            rC5_6 += rA5 * rB6;
            rC0_7 += rA0 * rB7;
            rC1_7 += rA1 * rB7; rb7 = *pB7++;
            rC2_7 += rA2 * rB7;
            rC3_7 += rA3 * rB7;
            rC4_7 += rA4 * rB7; rB0 = *pB0++;
            rC5_7 += rA5 * rB7;

            rC0_0 += ra0 * rb0;
            rC1_0 += ra1 * rb0; rA0 = *pA0++;
            rC2_0 += ra2 * rb0;
            rC3_0 += ra3 * rb0;
            rC4_0 += ra4 * rb0; rA1 = *pA1++;
            rC5_0 += ra5 * rb0;
            rC0_1 += ra0 * rb1;
            rC1_1 += ra1 * rb1; rA2 = *pA2++;
            rC2_1 += ra2 * rb1;
            rC3_1 += ra3 * rb1;
            rC4_1 += ra4 * rb1; rA3 = *pA3++;
            rC5_1 += ra5 * rb1;
            rC0_2 += ra0 * rb2;
            rC1_2 += ra1 * rb2; rA4 = *pA4++;
            rC2_2 += ra2 * rb2;
            rC3_2 += ra3 * rb2;
            rC4_2 += ra4 * rb2; rA5 = *pA5++;
            rC5_2 += ra5 * rb2;
            rC0_3 += ra0 * rb3;
            rC1_3 += ra1 * rb3; rB1 = *pB1++;
            rC2_3 += ra2 * rb3;
            rC3_3 += ra3 * rb3;
            rC4_3 += ra4 * rb3; rB2 = *pB2++;
            rC5_3 += ra5 * rb3;
            rC0_4 += ra0 * rb4;
            rC1_4 += ra1 * rb4; rB3 = *pB3++;
            rC2_4 += ra2 * rb4;
            rC3_4 += ra3 * rb4;
            rC4_4 += ra4 * rb4; rB4 = *pB4++;
            rC5_4 += ra5 * rb4;
            rC0_5 += ra0 * rb5;
            rC1_5 += ra1 * rb5; rB5 = *pB5++;
            rC2_5 += ra2 * rb5;
            rC3_5 += ra3 * rb5;
            rC4_5 += ra4 * rb5; rB6 = *pB6++;
            rC5_5 += ra5 * rb5;
            rC0_6 += ra0 * rb6;
            rC1_6 += ra1 * rb6;
            rC2_6 += ra2 * rb6; rB7 = *pB7++;
            rC3_6 += ra3 * rb6;
            rC4_6 += ra4 * rb6;
            rC5_6 += ra5 * rb6;
            rC0_7 += ra0 * rb7;
            rC1_7 += ra1 * rb7;
            rC2_7 += ra2 * rb7;
            rC3_7 += ra3 * rb7;
            rC4_7 += ra4 * rb7;
            rC5_7 += ra5 * rb7;
         }

         #if KB == 0
         if (K != Kb)
         {
            rC0_0 += rA0 * rB0;
            rC1_0 += rA1 * rB0; pA0--;
            rC2_0 += rA2 * rB0;
            rC3_0 += rA3 * rB0; pA1--;
            rC4_0 += rA4 * rB0;
            rC5_0 += rA5 * rB0; pA2--;
            rC0_1 += rA0 * rB1;
            rC1_1 += rA1 * rB1; pA3--;
            rC2_1 += rA2 * rB1;
            rC3_1 += rA3 * rB1; pA4--;
            rC4_1 += rA4 * rB1;
            rC5_1 += rA5 * rB1; pA5--;
            rC0_2 += rA0 * rB2;
            rC1_2 += rA1 * rB2; pB0--;
            rC2_2 += rA2 * rB2;
            rC3_2 += rA3 * rB2; pB1--;
            rC4_2 += rA4 * rB2;
            rC5_2 += rA5 * rB2; pB2--;
            rC0_3 += rA0 * rB3;
            rC1_3 += rA1 * rB3; pB3--;
            rC2_3 += rA2 * rB3;
            rC3_3 += rA3 * rB3; pB4--;
            rC4_3 += rA4 * rB3;
            rC5_3 += rA5 * rB3; pB5--;
            rC0_4 += rA0 * rB4;
            rC1_4 += rA1 * rB4; pB6--;
            rC2_4 += rA2 * rB4;
            rC3_4 += rA3 * rB4; pB7--;
            rC4_4 += rA4 * rB4;
            rC5_4 += rA5 * rB4;
            rC0_5 += rA0 * rB5;
            rC1_5 += rA1 * rB5;
            rC2_5 += rA2 * rB5;
            rC3_5 += rA3 * rB5;
            rC4_5 += rA4 * rB5;
            rC5_5 += rA5 * rB5;
            rC0_6 += rA0 * rB6;
            rC1_6 += rA1 * rB6;
            rC2_6 += rA2 * rB6;
            rC3_6 += rA3 * rB6;
            rC4_6 += rA4 * rB6;
            rC5_6 += rA5 * rB6;
            rC0_7 += rA0 * rB7;
            rC1_7 += rA1 * rB7;
            rC2_7 += rA2 * rB7;
            rC3_7 += rA3 * rB7;
            rC4_7 += rA4 * rB7;
            rC5_7 += rA5 * rB7;
         }
         else
         {
         #endif
         rC0_0 += rA0 * rB0;
         rC1_0 += rA1 * rB0;
         rC2_0 += rA2 * rB0;
         rC3_0 += rA3 * rB0;
         rC4_0 += rA4 * rB0; rb0 = *pB0++;
         rC5_0 += rA5 * rB0;
         rC0_1 += rA0 * rB1;
         rC1_1 += rA1 * rB1; ra0 = *pA0++;
         rC2_1 += rA2 * rB1;
         rC3_1 += rA3 * rB1;
         rC4_1 += rA4 * rB1; ra1 = *pA1++;
         rC5_1 += rA5 * rB1;
         rC0_2 += rA0 * rB2;
         rC1_2 += rA1 * rB2; ra2 = *pA2++;
         rC2_2 += rA2 * rB2;
         rC3_2 += rA3 * rB2;
         rC4_2 += rA4 * rB2; ra3 = *pA3++;
         rC5_2 += rA5 * rB2;
         rC0_3 += rA0 * rB3;
         rC1_3 += rA1 * rB3; ra4 = *pA4++;
         rC2_3 += rA2 * rB3;
         rC3_3 += rA3 * rB3;
         rC4_3 += rA4 * rB3; ra5 = *pA5++;
         rC5_3 += rA5 * rB3;
         rC0_4 += rA0 * rB4;
         rC1_4 += rA1 * rB4; rb1 = *pB1++;
         rC2_4 += rA2 * rB4;
         rC3_4 += rA3 * rB4;
         rC4_4 += rA4 * rB4; rb2 = *pB2++;
         rC5_4 += rA5 * rB4;
         rC0_5 += rA0 * rB5;
         rC1_5 += rA1 * rB5; rb3 = *pB3++;
         rC2_5 += rA2 * rB5;
         rC3_5 += rA3 * rB5;
         rC4_5 += rA4 * rB5; rb4 = *pB4++;
         rC5_5 += rA5 * rB5;
         rC0_6 += rA0 * rB6;
         rC1_6 += rA1 * rB6; rb5 = *pB5++;
         rC2_6 += rA2 * rB6;
         rC3_6 += rA3 * rB6;
         rC4_6 += rA4 * rB6; rb6 = *pB6++;
         rC5_6 += rA5 * rB6;
         rC0_7 += rA0 * rB7;
         rC1_7 += rA1 * rB7; rb7 = *pB7++;
         rC2_7 += rA2 * rB7;
         rC3_7 += rA3 * rB7;
         rC4_7 += rA4 * rB7;
         rC5_7 += rA5 * rB7;

         rC0_0 += ra0 * rb0;
         rC1_0 += ra1 * rb0;
         rC2_0 += ra2 * rb0;
         rC3_0 += ra3 * rb0;
         rC4_0 += ra4 * rb0;
         rC5_0 += ra5 * rb0;
         rC0_1 += ra0 * rb1;
         rC1_1 += ra1 * rb1;
         rC2_1 += ra2 * rb1;
         rC3_1 += ra3 * rb1;
         rC4_1 += ra4 * rb1;
         rC5_1 += ra5 * rb1;
         rC0_2 += ra0 * rb2;
         rC1_2 += ra1 * rb2;
         rC2_2 += ra2 * rb2;
         rC3_2 += ra3 * rb2;
         rC4_2 += ra4 * rb2;
         rC5_2 += ra5 * rb2;
         rC0_3 += ra0 * rb3;
         rC1_3 += ra1 * rb3;
         rC2_3 += ra2 * rb3;
         rC3_3 += ra3 * rb3;
         rC4_3 += ra4 * rb3;
         rC5_3 += ra5 * rb3;
         rC0_4 += ra0 * rb4;
         rC1_4 += ra1 * rb4;
         rC2_4 += ra2 * rb4;
         rC3_4 += ra3 * rb4;
         rC4_4 += ra4 * rb4;
         rC5_4 += ra5 * rb4;
         rC0_5 += ra0 * rb5;
         rC1_5 += ra1 * rb5;
         rC2_5 += ra2 * rb5;
         rC3_5 += ra3 * rb5;
         rC4_5 += ra4 * rb5;
         rC5_5 += ra5 * rb5;
         rC0_6 += ra0 * rb6;
         rC1_6 += ra1 * rb6;
         rC2_6 += ra2 * rb6;
         rC3_6 += ra3 * rb6;
         rC4_6 += ra4 * rb6;
         rC5_6 += ra5 * rb6;
         rC0_7 += ra0 * rb7;
         rC1_7 += ra1 * rb7;
         rC2_7 += ra2 * rb7;
         rC3_7 += ra3 * rb7;
         rC4_7 += ra4 * rb7;
         rC5_7 += ra5 * rb7;
         #if KB == 0
         }
         #endif

         #ifdef TREAL
            *pC0 =   rC0_0; pC0[1] = rC1_0; pC0[ 2] = rC2_0;
            pC0[3] = rC3_0; pC0[4] = rC4_0; pC0[ 5] = rC5_0;
            *pC1 =   rC0_1; pC1[1] = rC1_1; pC1[ 2] = rC2_1;
            pC1[3] = rC3_1; pC1[4] = rC4_1; pC1[ 5] = rC5_1;
            *pC2 =   rC0_2; pC2[1] = rC1_2; pC2[ 2] = rC2_2;
            pC2[3] = rC3_2; pC2[4] = rC4_2; pC2[ 5] = rC5_2;
            *pC3 =   rC0_3; pC3[1] = rC1_3; pC3[ 2] = rC2_3;
            pC3[3] = rC3_3; pC3[4] = rC4_3; pC3[ 5] = rC5_3;
            *pC4 =   rC0_4; pC4[1] = rC1_4; pC4[ 2] = rC2_4;
            pC4[3] = rC3_4; pC4[4] = rC4_4; pC4[ 5] = rC5_4;
            *pC5 =   rC0_5; pC5[1] = rC1_5; pC5[ 2] = rC2_5;
            pC5[3] = rC3_5; pC5[4] = rC4_5; pC5[ 5] = rC5_5;
            *pC6 =   rC0_6; pC6[1] = rC1_6; pC6[ 2] = rC2_6;
            pC6[3] = rC3_6; pC6[4] = rC4_6; pC6[ 5] = rC5_6;
            *pC7 =   rC0_7; pC7[1] = rC1_7; pC7[ 2] = rC2_7;
            pC7[3] = rC3_7; pC7[4] = rC4_7; pC7[ 5] = rC5_7;
         #else
            *pC0 =   rC0_0; pC0[2] = rC1_0; pC0[ 4] = rC2_0;
            pC0[6] = rC3_0; pC0[8] = rC4_0; pC0[10] = rC5_0;
            *pC1 =   rC0_1; pC1[2] = rC1_1; pC1[ 4] = rC2_1;
            pC1[6] = rC3_1; pC1[8] = rC4_1; pC1[10] = rC5_1;
            *pC2 =   rC0_2; pC2[2] = rC1_2; pC2[ 4] = rC2_2;
            pC2[6] = rC3_2; pC2[8] = rC4_2; pC2[10] = rC5_2;
            *pC3 =   rC0_3; pC3[2] = rC1_3; pC3[ 4] = rC2_3;
            pC3[6] = rC3_3; pC3[8] = rC4_3; pC3[10] = rC5_3;
            *pC4 =   rC0_4; pC4[2] = rC1_4; pC4[ 4] = rC2_4;
            pC4[6] = rC3_4; pC4[8] = rC4_4; pC4[10] = rC5_4;
            *pC5 =   rC0_5; pC5[2] = rC1_5; pC5[ 4] = rC2_5;
            pC5[6] = rC3_5; pC5[8] = rC4_5; pC5[10] = rC5_5;
            *pC6 =   rC0_6; pC6[2] = rC1_6; pC6[ 4] = rC2_6;
            pC6[6] = rC3_6; pC6[8] = rC4_6; pC6[10] = rC5_6;
            *pC7 =   rC0_7; pC7[2] = rC1_7; pC7[ 4] = rC2_7;
            pC7[6] = rC3_7; pC7[8] = rC4_7; pC7[10] = rC5_7;
         #endif
         pC0 += incCm; pC1 += incCm; pC2 += incCm; pC3 += incCm;
         pC4 += incCm; pC5 += incCm; pC6 += incCm; pC7 += incCm;
         pA0 += incAm; pA1 += incAm; pA2 += incAm; pA3 += incAm;
         pA4 += incAm; pA5 += incAm;
         pB0 += incBm; pB1 += incBm; pB2 += incBm; pB3 += incBm;
         pB4 += incBm; pB5 += incBm; pB6 += incBm; pB7 += incBm;
      }
      while(pA0 != stM);
      pC0 += incCn;
      pC1 += incCn;
      pC2 += incCn;
      pC3 += incCn;
      pC4 += incCn;
      pC5 += incCn;
      pC6 += incCn;
      pC7 += incCn;
      pA0 += incAn;
      pA1 += incAn;
      pA2 += incAn;
      pA3 += incAn;
      pA4 += incAn;
      pA5 += incAn;
      pB0 += incBn;
      pB1 += incBn;
      pB2 += incBn;
      pB3 += incBn;
      pB4 += incBn;
      pB5 += incBn;
      pB6 += incBn;
      pB7 += incBn;
   }
   while(pB0 != stN);
   }
   if (k=N-Nb)
      ATL_dJIK0x0x0TN6x1x1_a1_b1(M, k, K, alpha, ca, lda, cb + (Nb*ldb), ldb,
                                 beta, cc + (Nb*ldc2), ldc);
   if (Nb && (k=M-Mb))
      ATL_dJIK0x0x0TN1x8x1_a1_b1(k, Nb, K, alpha, ca + (Mb*lda), lda, cb, ldb, beta, cc + (Mb SHIFT), ldc);
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
#ifdef ldc2
   #undef ldc2
#endif
