#include "atlas_misc.h"
#ifdef BETA1
   #define ADDC(r, p) (r) += (p);
#elif defined(BETA0)
   #define ADDC(r, p)
#elif !defined(BETA0)
   #define ADDC(r, p) (r) -= (p);
#endif
#define SZT size_t
#define CTYPE const TYPE
void ATL_USERMM(SZT nmu, SZT nnu, SZT K, CTYPE *pA, CTYPE *pB, TYPE *pC,
                CTYPE *pAn, CTYPE *pBn, CTYPE *pCn)
{
   register TYPE rA0, rA1, rA2, rA3, rB0, rB1, rb1;
   register TYPE rC00, rC10, rC20, rC30;
   register TYPE rC01, rC11, rC21, rC31;
   CTYPE *pB0 = pB;
   int m, n;
   for (m=0; m < nmu; m++)
   {
      rB0 = *pB;
      rA0 = *pA;
      for (n=0; n < nnu; n++)
      {
/*
 *       K == 1
 */
         rC00 = rA0 * rB0;
         ADDC(rC00, *pC);
         rA1 = pA[1];
         rC10 = rA1 * rB0;
         ADDC(rC10, pC[1]);
         rA2 = pA[2];
         rC20 = rA2 * rB0;
         ADDC(rC20, pC[2]);
         rA3 = pA[3];
         rC30 = rA3 * rB0;
         ADDC(rC30, pC[3]);
         rB1 = pB[1];
         rC01 = rA0 * rB1;
         ADDC(rC01, pC[4]);
            rB0 = pB[2];
         rC11 = rA1 * rB1;
         ADDC(rC11, pC[5]);
            rA0 = pA[4];
         rC21 = rA2 * rB1;
         ADDC(rC21, pC[6]);
            rA1 = pA[5];
         rC31 = rA3 * rB1;
         ADDC(rC31, pC[7]);
            rA2 = pA[6];
/*
 *       K == 2
 */
         rC00 += rA0 * rB0;
            rA3 = pA[7];
         rC10 += rA1 * rB0;
            rB1 = pB[3];
         rC20 += rA2 * rB0;
         rC30 += rA3 * rB0;
            rB0 = pB[4];
         rC01 += rA0 * rB1;
            rA0 = pA[8];
         rC11 += rA1 * rB1;
            rA1 = pA[9];
         rC21 += rA2 * rB1;
            rA2 = pA[10];
         rC31 += rA3 * rB1;
            rA3 = pA[11];
/*
 *       K == 3
 */
         rC00 += rA0 * rB0;
            rB1 = pB[5];
         rC10 += rA1 * rB0;
            rb1 = pB[7];
         rC20 += rA2 * rB0;
         rC30 += rA3 * rB0;
            rB0 = pB[6];
         rC01 += rA0 * rB1;
            rA0 = pA[12];
         rC11 += rA1 * rB1;
            rA1 = pA[13];
         rC21 += rA2 * rB1;
            rA2 = pA[14];
         rC31 += rA3 * rB1;
            rA3 = pA[15];
/*
 *       K == 4
 */
         rC00 += rA0 * rB0;
         *pC = rC00;
         rC10 += rA1 * rB0;
         pC[1] = rC10;
         rC20 += rA2 * rB0;
         pC[2] = rC20;
         rC30 += rA3 * rB0;
         pC[3] = rC30;
            rB0 = pB[8];
         rC01 += rA0 * rb1;
         pC[4] = rC01;
            rA0 = *pA;
         rC11 += rA1 * rb1;
         pC[5] = rC11;
         rC21 += rA2 * rb1;
         pC[6] = rC21;
         rC31 += rA3 * rb1;
         pC[7] = rC31;

         pB += 8;
         pC += 8;
      }
      pA += 16;
      pB = pB0;
   }
}
