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
   register ATL_VTYPE rB0, rC0_0, rC1_0, rC2_0, rC3_0, rB1, rC0_1, rC1_1, rC2_1,
                      rC3_1, rA0, rA1, rA2, rA3;
   const TYPE *pB0=B, *pB1=B+ldb; 
   const TYPE *pA0=A, *pA1=A+lda, *pA2=pA1+lda, *pA3=pA2+lda, *aa=A;
   const TYPE *pfA = A + lda*M;
   TYPE *pC0=C, *pC1=C+(ldc SHIFT);
   int i, j, k;
   const int nmus = M>>2, nnus=N>>1;
   #if !defined(BETA0) && !defined(BETA1)
      #ifdef TCPLX
         #if ATL_VLEN == 4
            const ATL_VTYPE vBE={beta, 1.0, beta, 1.0};
         #elif ATL_VLEN == 16
            const ATL_VTYPE vBE={1.0, beta, 1.0, beta,
                                 1.0, beta, 1.0, beta,
                                 1.0, beta, 1.0, beta,
                                 1.0, beta, 1.0, beta}
         #elif ATL_VLEN == 8
            const ATL_VTYPE vBE={1.0, beta, 1.0, beta,
                                 1.0, beta, 1.0, beta};
         #elif ATL_VLEN == 2
            const ATL_VTYPE vBE={1.0, beta};
         #else
           #error "Unsupported VLEN!"
         #endif
      #else
         ATL_VTYPE vBE;
      #endif
   #endif
   #if ATL_KBCONST == 0
      const size_t incAm = lda<<2, incBn = ldb+ldb;
   #else
      #define incAm (4*ATL_MM_KB)
      #define incBn (2*ATL_MM_KB)
   #endif
   const size_t incC=(ldc+ldc)SHIFT;
   #if !defined(BETA0) && !defined(BETA1) && !defined(TCPLX)
      ATL_vbcast(vBE, &beta);
   #endif

   for (j=0; j < nnus; j++)
   {
      for (i=0; i < nmus; i++)
      {
         /* Peel K=0 iteration to avoid zero of rCxx and extra add  */
         ATL_vld(rB0, pB0);
         pB0 += ATL_VLEN;
         ATL_vld(rA0, pA0);
         pA0 += ATL_VLEN;
         ATL_vld(rA1, pA1);
         pA1 += ATL_VLEN;
         ATL_vld(rA2, pA2);
         pA2 += ATL_VLEN;
         ATL_vld(rA3, pA3);
         pA3 += ATL_VLEN;
         ATL_vld(rB1, pB1);
         pB1 += ATL_VLEN;
         ATL_vmul(rC0_0, rA0, rB0);
            ATL_pfl2R(pfA);
         ATL_vmul(rC1_0, rA1, rB0);
         ATL_vmul(rC2_0, rA2, rB0);
            pfA += 8;
         ATL_vmul(rC3_0, rA3, rB0);
         ATL_vmul(rC0_1, rA0, rB1);
            ATL_vld(rA0, pA0);
            pA0 += ATL_VLEN;
         ATL_vmul(rC1_1, rA1, rB1);
            ATL_vld(rA1, pA1);
            pA1 += ATL_VLEN;
         ATL_vmul(rC2_1, rA2, rB1);
            ATL_vld(rA2, pA2);
            pA2 += ATL_VLEN;
         ATL_vmul(rC3_1, rA3, rB1);
            ATL_vld(rA3, pA3);
            pA3 += ATL_VLEN;
         for (k=ATL_VLEN; k < ATL_MM_KB; k += ATL_VLEN)
         {
               ATL_vld(rB0, pB0);
               pB0 += ATL_VLEN;
            ATL_vmac(rC0_0, rA0, rB0);
               ATL_vld(rB1, pB1);
               pB1 += ATL_VLEN;
            ATL_vmac(rC1_0, rA1, rB0);
            ATL_vmac(rC2_0, rA2, rB0);
            ATL_vmac(rC3_0, rA3, rB0);
            ATL_vmac(rC0_1, rA0, rB1);
               ATL_vld(rA0, pA0);
               pA0 += ATL_VLEN;
            ATL_vmac(rC1_1, rA1, rB1);
               ATL_vld(rA1, pA1);
               pA1 += ATL_VLEN;
            ATL_vmac(rC2_1, rA2, rB1);
               ATL_vld(rA2, pA2);
               pA2 += ATL_VLEN;
            ATL_vmac(rC3_1, rA3, rB1);
               ATL_vld(rA3, pA3);
               pA3 += ATL_VLEN;
         }
         #if ATL_VLEN == 2
            ATL_vvrsum2(rC0_0, rC1_0);
            ATL_vvrsum2(rC2_0, rC3_0);
            ATL_vvrsum2(rC0_1, rC1_1);
            ATL_vvrsum2(rC2_1, rC3_1);
            #ifdef TCPLX
               #ifdef BETA0
                  *pC0   = rC0_0[0];
                  pC0[2] = rC0_0[1];
                  pC0[4] = rC2_0[0];
                  pC0[6] = rC2_0[1];
                  *pC1   = rC0_1[0];
                  pC1[2] = rC0_1[1];
                  pC1[4] = rC2_1[0];
                  pC1[6] = rC2_1[1];
               #elif defined(BETA1)
                  *pC0   += rC0_0[0];
                  pC0[2] += rC0_0[1];
                  pC0[4] += rC2_0[0];
                  pC0[6] += rC2_0[1];
                  *pC1   += rC0_1[0];
                  pC1[2] += rC0_1[1];
                  pC1[4] += rC2_1[0];
                  pC1[6] += rC2_1[1];
               #else
                  *pC0   = rC0_0[0] + beta * *pC0;
                  pC0[2] = rC0_0[1] + beta * pC0[2];
                  pC0[4] = rC2_0[0] + beta * pC0[4];
                  pC0[6] = rC2_0[1] + beta * pC0[6];
                  *pC1   = rC0_1[0] + beta * *pC1;
                  pC1[2] = rC0_1[1] + beta * pC1[2];
                  pC1[4] = rC2_1[0] + beta * pC1[4];
                  pC1[6] = rC2_1[1] + beta * pC1[6];
               #endif
            #else
               ATL_vbeta(pC0, rC0_0);
               ATL_vbeta(pC0+2, rC2_0);
               ATL_vbeta(pC1, rC0_1);
               ATL_vbeta(pC1+2, rC2_1);
            #endif
         #elif ATL_VLEN == 4
            #ifdef TCPLX
               ATL_vvrsum4(rC0_0, rC2_0, rC1_0, rC3_0);// rC00={r3, r1, r2, r0}
               ATL_vvrsum4(rC0_1, rC2_1, rC1_1, rC3_1);// rC01={r3, r1, r2, r0}
               #ifdef BETA0
                  ATL_vuld(rC1_0, pC0+1);              // rC10={r2, i1, r1, i0}
                  ATL_vunpckLO(rC2_0, rC0_0, rC1_0);   // rC20={i0, r3, i0, r2}
                  ATL_vust(pC0, rC2_0);
                  ATL_vuld(rC1_0, pC0+ATL_VLEN);       // rC10={i3, r3, i2, r2}
                  ATL_vunpckHI(rC0_0, rC0_0, rC1_0);   // rc00={i3, r3, i2, r2}
                  ATL_vust(pC0+ATL_VLEN, rC0_0);
                  ATL_vuld(rC1_0, pC1+1);              // rC10={r2, i1, r1, i0}
                  ATL_vunpckLO(rC2_1, rC0_1, rC1_0);   // rC21={i0, r3, i0, r2}
                  ATL_vust(pC1, rC2_1);
                  ATL_vuld(rC1_0, pC1+ATL_VLEN);       // rC11={i3, r3, i2, r2}
                  ATL_vunpckHI(rC0_1, rC0_1, rC1_0);   // rc01={i3, r3, i2, r2}
                  ATL_vust(pC1+ATL_VLEN, rC0_1);
               #else
                  ATL_vzero(rC1_0);                    // rC10={ 0,  0,  0,  0}
                  ATL_vunpckHI(rC2_0, rC0_0, rC1_0);   // rC20={ 0, r3,  0, r2}
                  ATL_vunpckLO(rC0_0, rC0_0, rC1_0);   // rC00={ 0, r1,  0, r0}
                  ATL_vbeta(pC0, rC0_0);
                  ATL_vbeta(pC0+ATL_VLEN, rC2_0);
                  ATL_vunpckHI(rC2_1, rC0_1, rC1_0);   // rC21={ 0, r3,  0, r2}
                  ATL_vunpckLO(rC0_1, rC0_1, rC1_0);   // rC01={ 0, r1,  0, r0}
                  ATL_vbeta(pC1, rC0_1);
                  ATL_vbeta(pC1+ATL_VLEN, rC2_1);
               #endif
            #else
               ATL_vvrsum4(rC0_0, rC1_0, rC2_0, rC3_0);
               ATL_vbeta(pC0, rC0_0);
               ATL_vvrsum4(rC0_1, rC1_1, rC2_1, rC3_1);
               ATL_vbeta(pC1, rC0_1);
            #endif
         #elif ATL_VLEN == 8 && defined(ATL_AVX) && defined(SREAL)
            ATL_vvrsum8(rC0_0, rC1_0, rC2_0, rC3_0, rC0_1, rC1_1, rC2_1, rC3_1);
            #ifndef BETA0
               rC0_1 =_mm256_insertf128_ps(rC0_1, _mm_loadu_ps(pC0), 0);
               rC0_1 =_mm256_insertf128_ps(rC0_1, _mm_loadu_ps(pC1), 1);
               #ifdef BETAX
                  ATL_vmul(rC0_1, rC0_1, vBE);
               #endif
               ATL_vadd(rC0_0, rC0_0, rC0_1);
            #endif
            _mm_storeu_ps(pC0, _mm256_extractf128_ps(rC0_0, 0));
            _mm_storeu_ps(pC1, _mm256_extractf128_ps(rC0_0, 1));
         #elif ATL_VLEN == 8 && defined(SCPLX)
            ATL_vvrsum8(rC0_0, rC1_0, rC2_0, rC3_0, rC0_1, rC1_1, rC2_1, rC3_1);
            #ifdef BETA0
               pC0[0] = rC0_0[0];
               pC0[2] = rC0_0[1];
               pC0[4] = rC0_0[2];
               pC0[6] = rC0_0[3];
               pC1[0] = rC0_0[4];
               pC1[2] = rC0_0[5];
               pC1[4] = rC0_0[6];
               pC1[6] = rC0_0[7];
            #elif defined(BETA1)
               pC0[0] += rC0_0[0];
               pC0[2] += rC0_0[1];
               pC0[4] += rC0_0[2];
               pC0[6] += rC0_0[3];
               pC1[0] += rC0_0[4];
               pC1[2] += rC0_0[5];
               pC1[4] += rC0_0[6];
               pC1[6] += rC0_0[7];
            #else
               pC0[0] = rC0_0[0] + beta*pC0[0];
               pC0[2] = rC0_0[1] + beta*pC0[2];
               pC0[4] = rC0_0[2] + beta*pC0[4];
               pC0[6] = rC0_0[3] + beta*pC0[6];
               pC1[0] = rC0_0[4] + beta*pC1[0];
               pC1[2] = rC0_0[5] + beta*pC1[2];
               pC1[4] = rC0_0[6] + beta*pC1[4];
               pC1[6] = rC0_0[7] + beta*pC1[6];
            #endif
         #else
            #error "VLEN NOT SUPPORTED!"
         #endif
         pC0 += 4 SHIFT;
         pC1 += 4 SHIFT;
         pB0 = B;
         pB1 = B + ldb;
         A += incAm;
         pA0 = A;
         pA1 = A + lda;
         pA2 = pA1 + lda;
         pA3 = pA2 + lda;
      }  /* end of loop over M */
      A = aa;
      pA0 = A;
      pA1 = A + lda;
      pA2 = pA1 + lda;
      pA3 = pA2 + lda;
      C += incC;
      pC0 = C;
      pC1 = C + (ldc SHIFT);
      B += incBn;
      pB0 = B;
      pB1 = B + ldb;
   }  /* end of loop over N */
}
