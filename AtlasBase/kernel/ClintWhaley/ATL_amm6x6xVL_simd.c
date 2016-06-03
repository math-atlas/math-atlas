/*
 * Automatically Tuned Linear Algebra Software v3.10.3
 * Copyright (C) 2016 R. Clint Whaley
 */
#include "atlas_simd.h"
#include "atlas_prefetch.h"
#if defined(SCPLX) || defined(DCPLX)
   #include "atlas_cplxsimd.h"
   #define TCPLX 1
   #define SHIFT <<1
#else
   #define SHIFT
   #define TREAL 1
#endif
#if !defined(SREAL) && !defined(DREAL) && !defined(SCPLX) && !defined(DCPLX)
   #define DREAL 1
#endif
#ifndef TYPE
   #if defined(SREAL) || defined(SCPLX)
      #define TYPE float
   #else
      #define TYPE double
   #endif
#endif
#ifndef ATL_MM_KB 
   #ifdef KB
      #if KB > 0
         #define ATL_KBCONST 1
         #define ATL_MM_KB KB
      #else
         #define ATL_KBCONST 0
         #define ATL_MM_KB K
      #endif
   #else
      #define ATL_KBCONST 0
      #define ATL_MM_KB K
   #endif
#else
   #if ATL_MM_KB > 0
      #define ATL_KBCONST 1
   #else
      #undef ATL_MM_KB
      #define ATL_MM_KB K
      #define ATL_KBCONST 0
   #endif
#endif
#ifdef BETA1
   #define ATL_vbeta(p_, d_) \
   { \
      ATL_vuld(rA0, p_); \
      ATL_vadd(d_, d_, rA0); \
      ATL_vust(p_, d_); \
   }
#elif defined(BETA0)
   #define ATL_vbeta(p_, d_) ATL_vust(p_, d_)
#else
   #define ATL_vbeta(p_, d_) \
   { \
      ATL_vuld(rA0, p_); \
      ATL_vmac(d_, rA0, vBE); \
      ATL_vust(p_, d_); \
   }
#endif

#ifndef ATL_CSZT
   #include <stddef.h>
   #define ATL_CSZT const size_t
#endif
#if 0
void ATL_USERMM
(
   ATL_CSZT nmus,
   ATL_CSZT nnus,
   ATL_CSZT K,
   const double *pA, /* 4*KB*nmus-length access-major array of A */
   const double *pB, /* 2*KB*nnus-length access-major array of B */
   double *pC,   /* 4*2*nnus*nmus-length access-major array of C */
   const double *pAn, /* next block of A */
   const double *pBn, /* next block of B */
   const double *pCn  /* next block of C */
)
#endif
#ifndef ATL_RESTRICT
   #if defined(__STDC_VERSION__) && (__STDC_VERSION__/100 >= 1999)
      #define ATL_RESTRICT restrict
   #else
      #define ATL_RESTRICT
   #endif
#endif
void ATL_USERMM
   (const int M, const int N, const int K, const TYPE alpha,
    const TYPE * ATL_RESTRICT A, const int lda,
    const TYPE * ATL_RESTRICT B, const int ldb, const TYPE beta,
    TYPE * ATL_RESTRICT C, const int ldc)
/*
 * Performs a GEMM with M,N,K unrolling (& jam) of (4,2,4).
 * Vectorization of VLEN=[4,8] (d,s) along K dim, vec unroll=(4,2,1).
 * You may set compile-time constant K dim by defining ATL_MM_KB.
 */
{
   const TYPE *pB0=B, *pB1=B+ldb, *pB2=pB1+ldb, *pB3=pB2+ldb, 
              *pB4=pB3+ldb, *pB5=pB4+ldb;
   const TYPE *pA0=A, *pA1=A+lda, *pA2=pA1+lda, *pA3=pA2+lda, 
              *pA4=pA3+lda, *pA5=pA4+lda, *aa=A;
   const TYPE *pfA = A + lda*M, *pfB = B + ldb*N;
   const size_t ldc2 = ldc SHIFT;
   TYPE *pC0=C;
   int i, j, k;
   #if !defined(BETA0) && !defined(BETA1) && !defined(TCPLX)
      ATL_VTYPE vBE;
   #endif
   #if ATL_KBCONST == 0
      const size_t incAm = (lda<<2)+(lda<<1), incBn = (ldb<<2)+(ldb<<1);
   #else
      #define incAm (6*ATL_MM_KB)
      #define incBn (6*ATL_MM_KB)
   #endif
   const size_t incC=((ldc2<<2)+(ldc2<<1));
   #if !defined(BETA0) && !defined(BETA1) && !defined(TCPLX)
      ATL_vbcast(vBE, &beta);
   #endif

   for (j=0; j < N; j += 6)
   {
      for (i=0; i < M; i += 6)
      {
         register ATL_VTYPE rC00, rC10, rC20, rC30, rC40, rC50,
                            rC01, rC11, rC21, rC31, rC41, rC51,
                            rC02, rC12, rC22, rC32, rC42, rC52,
                            rC03, rC13, rC23, rC33, rC43, rC53,
                            rC04, rC14, rC24, rC34, rC44, rC54,
                            rC05, rC15, rC25, rC35, rC45, rC55,
                            rB0, rB1, rB2, rB3, rB4, rB5,
                            rA0, rA1, rA2, rA3, rA4, rA5;
         /* Peel K=0 iteration to avoid zero of rCxx and extra add  */
         ATL_vld(rB0, pB0); pB0 += ATL_VLEN;
         ATL_vld(rA0, pA0); pA0 += ATL_VLEN;
         ATL_vmul(rC00, rA0, rB0);
         ATL_vld(rA1, pA1); pA1 += ATL_VLEN;
         ATL_vmul(rC10, rA1, rB0);
         ATL_vld(rA2, pA2); pA2 += ATL_VLEN;
         ATL_vmul(rC20, rA2, rB0);
         ATL_vld(rA3, pA3); pA3 += ATL_VLEN;
         ATL_vmul(rC30, rA3, rB0);
         ATL_vld(rA4, pA4); pA4 += ATL_VLEN;
         ATL_vmul(rC40, rA4, rB0);
         ATL_vld(rA5, pA5); pA5 += ATL_VLEN;
         ATL_vmul(rC50, rA5, rB0);

         ATL_vld(rB1, pB1); pB1 += ATL_VLEN;
         ATL_vmul(rC01, rA0, rB1);
         ATL_vld(rB2, pB2); pB2 += ATL_VLEN;
         ATL_vmul(rC11, rA1, rB1);
         ATL_vld(rB3, pB3); pB3 += ATL_VLEN;
         ATL_vmul(rC21, rA2, rB1);
         ATL_vld(rB4, pB4); pB4 += ATL_VLEN;
         ATL_vmul(rC31, rA3, rB1);
         ATL_vld(rB5, pB5); pB5 += ATL_VLEN;
         ATL_vmul(rC41, rA4, rB1);
         ATL_vmul(rC51, rA5, rB1);

         ATL_vmul(rC02, rA0, rB2);
            ATL_pfl1R(pfB); pfB += 36;
         ATL_vmul(rC12, rA1, rB2);
         ATL_vmul(rC22, rA2, rB2);
         ATL_vmul(rC32, rA3, rB2);
         ATL_vmul(rC42, rA4, rB2);
         ATL_vmul(rC52, rA5, rB2);

         ATL_vmul(rC03, rA0, rB3);
            ATL_pfl1R(pfA); pfA += 36;
         ATL_vmul(rC13, rA1, rB3);
         ATL_vmul(rC23, rA2, rB3);
         ATL_vmul(rC33, rA3, rB3);
         ATL_vmul(rC43, rA4, rB3);
         ATL_vmul(rC53, rA5, rB3);

         ATL_vmul(rC04, rA0, rB4);
         ATL_vmul(rC14, rA1, rB4);
         ATL_vmul(rC24, rA2, rB4);
         ATL_vmul(rC34, rA3, rB4);
         ATL_vmul(rC44, rA4, rB4);
         ATL_vmul(rC54, rA5, rB4);

         ATL_vmul(rC05, rA0, rB5);
            ATL_vld(rA0, pA0); pA0 += ATL_VLEN;
         ATL_vmul(rC15, rA1, rB5);
            ATL_vld(rA1, pA1); pA1 += ATL_VLEN;
         ATL_vmul(rC25, rA2, rB5);
            ATL_vld(rA2, pA2); pA2 += ATL_VLEN;
         ATL_vmul(rC35, rA3, rB5);
            ATL_vld(rA3, pA3); pA3 += ATL_VLEN;
         ATL_vmul(rC45, rA4, rB5);
            ATL_vld(rA4, pA4); pA4 += ATL_VLEN;
         ATL_vmul(rC55, rA5, rB5);
            ATL_vld(rA5, pA5); pA5 += ATL_VLEN;
         for (k=ATL_VLEN; k < ATL_MM_KB; k += ATL_VLEN)
         {
               ATL_vld(rB0, pB0);
               pB0 += ATL_VLEN;
            ATL_vmac(rC00, rA0, rB0);
               ATL_vld(rB1, pB1);
               pB1 += ATL_VLEN;
            ATL_vmac(rC10, rA1, rB0);
               ATL_vld(rB2, pB2);
               pB2 += ATL_VLEN;
            ATL_vmac(rC20, rA2, rB0);
               ATL_vld(rB3, pB3);
               pB3 += ATL_VLEN;
            ATL_vmac(rC30, rA3, rB0);
               ATL_vld(rB4, pB4);
               pB4 += ATL_VLEN;
            ATL_vmac(rC40, rA4, rB0);
               ATL_vld(rB5, pB5);
               pB5 += ATL_VLEN;
            ATL_vmac(rC50, rA5, rB0);

            ATL_vmac(rC01, rA0, rB1);
            ATL_vmac(rC11, rA1, rB1);
            ATL_vmac(rC21, rA2, rB1);
            ATL_vmac(rC31, rA3, rB1);
            ATL_vmac(rC41, rA4, rB1);
            ATL_vmac(rC51, rA5, rB1);

            ATL_vmac(rC02, rA0, rB2);
            ATL_vmac(rC12, rA1, rB2);
            ATL_vmac(rC22, rA2, rB2);
            ATL_vmac(rC32, rA3, rB2);
            ATL_vmac(rC42, rA4, rB2);
            ATL_vmac(rC52, rA5, rB2);

            ATL_vmac(rC03, rA0, rB3);
            ATL_vmac(rC13, rA1, rB3);
            ATL_vmac(rC23, rA2, rB3);
            ATL_vmac(rC33, rA3, rB3);
            ATL_vmac(rC43, rA4, rB3);
            ATL_vmac(rC53, rA5, rB3);

            ATL_vmac(rC04, rA0, rB4);
            ATL_vmac(rC14, rA1, rB4);
            ATL_vmac(rC24, rA2, rB4);
            ATL_vmac(rC34, rA3, rB4);
            ATL_vmac(rC44, rA4, rB4);
            ATL_vmac(rC54, rA5, rB4);

            ATL_vmac(rC05, rA0, rB5);
               ATL_vld(rA0, pA0);
               pA0 += ATL_VLEN;
            ATL_vmac(rC15, rA1, rB5);
               ATL_vld(rA1, pA1);
               pA1 += ATL_VLEN;
            ATL_vmac(rC25, rA2, rB5);
               ATL_vld(rA2, pA2);
               pA2 += ATL_VLEN;
            ATL_vmac(rC35, rA3, rB5);
               ATL_vld(rA3, pA3);
               pA3 += ATL_VLEN;
            ATL_vmac(rC45, rA4, rB5);
               ATL_vld(rA4, pA4);
               pA4 += ATL_VLEN;
            ATL_vmac(rC55, rA5, rB5);
               ATL_vld(rA5, pA5);
               pA5 += ATL_VLEN;
         }
         #if ATL_VLEN == 2
            ATL_vvrsum2(rC00, rC10);
            ATL_vvrsum2(rC20, rC30);
            ATL_vvrsum2(rC40, rC50);
            ATL_vvrsum2(rC01, rC11);
            ATL_vvrsum2(rC21, rC31);
            ATL_vvrsum2(rC41, rC51);
            ATL_vvrsum2(rC02, rC12);
            ATL_vvrsum2(rC22, rC32);
            ATL_vvrsum2(rC42, rC52);
            ATL_vvrsum2(rC03, rC13);
            ATL_vvrsum2(rC23, rC33);
            ATL_vvrsum2(rC43, rC53);
            ATL_vvrsum2(rC04, rC14);
            ATL_vvrsum2(rC24, rC34);
            ATL_vvrsum2(rC44, rC54);
            ATL_vvrsum2(rC05, rC15);
            ATL_vvrsum2(rC25, rC35);
            ATL_vvrsum2(rC45, rC55);
            #ifdef TCPLX
               #ifdef BETA0
                  *pC0   = rC00[0];
                  pC0[2] = rC00[1];
                  pC0[4] = rC20[0];
                  pC0[6] = rC20[1];
                  pC0[8] = rC40[0];
                  pC0[10] = rC40[1];
                  pC0 += ldc2;
                  *pC0   = rC01[0];
                  pC0[2] = rC01[1];
                  pC0[4] = rC21[0];
                  pC0[6] = rC21[1];
                  pC0[8] = rC41[0];
                  pC0[10]= rC41[1];
                  pC0 += ldc2;
                  *pC0   = rC02[0];
                  pC0[2] = rC02[1];
                  pC0[4] = rC22[0];
                  pC0[6] = rC22[1];
                  pC0[8] = rC42[0];
                  pC0[10]= rC42[1];
                  pC0 += ldc2;
                  *pC0   = rC03[0];
                  pC0[2] = rC03[1];
                  pC0[4] = rC23[0];
                  pC0[6] = rC23[1];
                  pC0[8] = rC43[0];
                  pC0[10]= rC43[1];
                  pC0 += ldc2;
                  *pC0   = rC04[0];
                  pC0[2] = rC04[1];
                  pC0[4] = rC24[0];
                  pC0[6] = rC24[1];
                  pC0[8] = rC44[0];
                  pC0[10]= rC44[1];
                  pC0 += ldc2;
                  *pC0   = rC05[0];
                  pC0[2] = rC05[1];
                  pC0[4] = rC25[0];
                  pC0[6] = rC25[1];
                  pC0[8] = rC45[0];
                  pC0[10]= rC45[1];
               #elif defined(BETA1)
                  *pC0   += rC00[0];
                  pC0[2] += rC00[1];
                  pC0[4] += rC20[0];
                  pC0[6] += rC20[1];
                  pC0[8] += rC40[0];
                  pC0[10]+= rC40[1];
                  pC0 += ldc2;
                  *pC0   += rC01[0];
                  pC0[2] += rC01[1];
                  pC0[4] += rC21[0];
                  pC0[6] += rC21[1];
                  pC0[8] += rC41[0];
                  pC0[10]+= rC41[1];
                  pC0 += ldc2;
                  *pC0   += rC02[0];
                  pC0[2] += rC02[1];
                  pC0[4] += rC22[0];
                  pC0[6] += rC22[1];
                  pC0[8] += rC42[0];
                  pC0[10]+= rC42[1];
                  pC0 += ldc2;
                  *pC0   += rC03[0];
                  pC0[2] += rC03[1];
                  pC0[4] += rC23[0];
                  pC0[6] += rC23[1];
                  pC0[8] += rC43[0];
                  pC0[10]+= rC43[1];
                  pC0 += ldc2;
                  *pC0   += rC04[0];
                  pC0[2] += rC04[1];
                  pC0[4] += rC24[0];
                  pC0[6] += rC24[1];
                  pC0[8] += rC44[0];
                  pC0[10]+= rC44[1];
                  pC0 += ldc2;
                  *pC0   += rC05[0];
                  pC0[2] += rC05[1];
                  pC0[4] += rC25[0];
                  pC0[6] += rC25[1];
                  pC0[8] += rC45[0];
                  pC0[10]+= rC45[1];
               #else
                  *pC0   = rC00[0] + beta * *pC0;
                  pC0[2] = rC00[1] + beta * pC0[2];
                  pC0[4] = rC20[0] + beta * pC0[4];
                  pC0[6] = rC20[1] + beta * pC0[6];
                  pC0[8] = rC40[0] + beta * pC0[8];
                  pC0[10]= rC40[1] + beta * pC0[10];
                  pC0 += ldc2;
                  *pC0   = rC01[0] + beta * *pC0;
                  pC0[2] = rC01[1] + beta * pC0[2];
                  pC0[4] = rC21[0] + beta * pC0[4];
                  pC0[6] = rC21[1] + beta * pC0[6];
                  pC0[8] = rC41[0] + beta * pC0[8];
                  pC0[10]= rC41[1] + beta * pC0[10];
                  pC0 += ldc2;
                  *pC0   = rC02[0] + beta * *pC0;
                  pC0[2] = rC02[1] + beta * pC0[2];
                  pC0[4] = rC22[0] + beta * pC0[4];
                  pC0[6] = rC22[1] + beta * pC0[6];
                  pC0[8] = rC42[0] + beta * pC0[8];
                  pC0[10]= rC42[1] + beta * pC0[10];
                  pC0 += ldc2;
                  *pC0   = rC03[0] + beta * *pC0;
                  pC0[2] = rC03[1] + beta * pC0[2];
                  pC0[4] = rC23[0] + beta * pC0[4];
                  pC0[6] = rC23[1] + beta * pC0[6];
                  pC0[8] = rC43[0] + beta * pC0[8];
                  pC0[10]= rC43[1] + beta * pC0[10];
                  pC0 += ldc2;
                  *pC0   = rC04[0] + beta * *pC0;
                  pC0[2] = rC04[1] + beta * pC0[2];
                  pC0[4] = rC24[0] + beta * pC0[4];
                  pC0[6] = rC24[1] + beta * pC0[6];
                  pC0[8] = rC44[0] + beta * pC0[8];
                  pC0[10]= rC44[1] + beta * pC0[10];
                  pC0 += ldc2;
                  *pC0   = rC05[0] + beta * *pC0;
                  pC0[2] = rC05[1] + beta * pC0[2];
                  pC0[4] = rC25[0] + beta * pC0[4];
                  pC0[6] = rC25[1] + beta * pC0[6];
                  pC0[8] = rC45[0] + beta * pC0[8];
                  pC0[10]= rC45[1] + beta * pC0[10];
               #endif
            #else
               ATL_vbeta(pC0, rC00);
               ATL_vbeta(pC0+2, rC20);
               ATL_vbeta(pC0+4, rC40);
               pC0 += ldc2;
               ATL_vbeta(pC0, rC01);
               ATL_vbeta(pC0+2, rC21);
               ATL_vbeta(pC0+4, rC41);
               pC0 += ldc2;
               ATL_vbeta(pC0, rC02);
               ATL_vbeta(pC0+2, rC22);
               ATL_vbeta(pC0+4, rC42);
               pC0 += ldc2;
               ATL_vbeta(pC0, rC03);
               ATL_vbeta(pC0+2, rC23);
               ATL_vbeta(pC0+4, rC43);
               pC0 += ldc2;
               ATL_vbeta(pC0, rC04);
               ATL_vbeta(pC0+2, rC24);
               ATL_vbeta(pC0+4, rC44);
               pC0 += ldc2;
               ATL_vbeta(pC0, rC05);
               ATL_vbeta(pC0+2, rC25);
               ATL_vbeta(pC0+4, rC45);
            #endif
         #elif ATL_VLEN == 4
            ATL_vvrsum4(rC00, rC10, rC20, rC30);
            ATL_vvrsum2(rC40, rC50);
            ATL_vvrsum4(rC01, rC11, rC21, rC31);
            ATL_vvrsum2(rC41, rC51);
            ATL_vvrsum4(rC02, rC12, rC22, rC32);
            ATL_vvrsum2(rC42, rC52);
            ATL_vvrsum4(rC03, rC13, rC23, rC33);
            ATL_vvrsum2(rC43, rC53);
            ATL_vvrsum4(rC04, rC14, rC24, rC34);
            ATL_vvrsum2(rC44, rC54);
            ATL_vvrsum4(rC05, rC15, rC25, rC35);
            ATL_vvrsum2(rC45, rC55);
            #ifdef TCPLX
               #ifdef BETA0
               #elif defined(BETA1)
               #else
               #endif
            #else
               ATL_vbeta(pC0, rC00);
               #ifdef BETA0
                  pC0[4] = rC40[0];
                  pC0[5] = rC40[1];
               #elif defined(BETA1)
                  pC0[4] += rC40[0];
                  pC0[5] += rC40[1];
               #else
                  pC0[4] = rC40[0] + beta*pC0[4];
                  pC0[5] = rC40[1] + beta*pC0[5];
               #endif
               pC0 += ldc;
               ATL_vbeta(pC0, rC01);
               #ifdef BETA0
                  pC0[4] = rC41[0];
                  pC0[5] = rC41[1];
               #elif defined(BETA1)
                  pC0[4] += rC41[0];
                  pC0[5] += rC41[1];
               #else
                  pC0[4] = rC41[0] + beta*pC0[4];
                  pC0[5] = rC41[1] + beta*pC0[5];
               #endif
               pC0 += ldc;
               ATL_vbeta(pC0, rC02);
               #ifdef BETA0
                  pC0[4] = rC42[0];
                  pC0[5] = rC42[1];
               #elif defined(BETA1)
                  pC0[4] += rC42[0];
                  pC0[5] += rC42[1];
               #else
                  pC0[4] = rC42[0] + beta*pC0[4];
                  pC0[5] = rC42[1] + beta*pC0[5];
               #endif
               pC0 += ldc;
               ATL_vbeta(pC0, rC03);
               #ifdef BETA0
                  pC0[4] = rC43[0];
                  pC0[5] = rC43[1];
               #elif defined(BETA1)
                  pC0[4] += rC43[0];
                  pC0[5] += rC43[1];
               #else
                  pC0[4] = rC43[0] + beta*pC0[4];
                  pC0[5] = rC43[1] + beta*pC0[5];
               #endif
               pC0 += ldc;
               ATL_vbeta(pC0, rC04);
               #ifdef BETA0
                  pC0[4] = rC44[0];
                  pC0[5] = rC44[1];
               #elif defined(BETA1)
                  pC0[4] += rC44[0];
                  pC0[5] += rC44[1];
               #else
                  pC0[4] = rC44[0] + beta*pC0[4];
                  pC0[5] = rC44[1] + beta*pC0[5];
               #endif
               pC0 += ldc;
               ATL_vbeta(pC0, rC05);
               #ifdef BETA0
                  pC0[4] = rC45[0];
                  pC0[5] = rC45[1];
               #elif defined(BETA1)
                  pC0[4] += rC45[0];
                  pC0[5] += rC45[1];
               #else
                  pC0[4] = rC45[0] + beta*pC0[4];
                  pC0[5] = rC45[1] + beta*pC0[5];
               #endif
            #endif
         #else
            #error "VLEN NOT SUPPORTED!"
         #endif
         pC0 -= (ldc2<<2)+ldc2 - (6 SHIFT);
         pB0 = B;
         pB1 = B + ldb;
         pB2 = pB1 + ldb;
         pB3 = pB2 + ldb;
         pB4 = pB3 + ldb;
         pB5 = pB4 + ldb;
         A += incAm;
         pA0 = A;
         pA1 = A + lda;
         pA2 = pA1 + lda;
         pA3 = pA2 + lda;
         pA4 = pA3 + lda;
         pA5 = pA4 + lda;
      }  /* end of loop over M */
      A = aa;
      pA0 = A;
      pA1 = A + lda;
      pA2 = pA1 + lda;
      pA3 = pA2 + lda;
      pA4 = pA3 + lda;
      pA5 = pA4 + lda;
      C += incC;
      pC0 = C;
      B += incBn;
      pB0 = B;
      pB1 = B + ldb;
      pB2 = pB1 + ldb;
      pB3 = pB2 + ldb;
      pB4 = pB3 + ldb;
      pB5 = pB4 + ldb;
   }  /* end of loop over N */
}
