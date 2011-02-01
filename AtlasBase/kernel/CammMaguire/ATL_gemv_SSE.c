#include <stdio.h>
#include <stdlib.h>

#include "atlas_misc.h"

#include "camm_util.h"
#ifndef ATL_GAS_x8632
   #error "This kernel requires gas x86-32 assembler!"
#endif

#define COPY_B


#ifdef COPY_B
#define plb(a_,b_,c_) pla(a_,b_,c_)
#else
#define plb(a_,b_,c_) pl(a_,b_,c_)
#endif



#undef p1_4_gemvT_1
#define p1_4_gemvT_1(a_) \
      pls(SS(a_,MM(0,RS4)),bx,4) \
      pls(SS(a_,MM(0,RS4)),ax,0) \
      pls(SS(a_,MM(0,RS4)),si,2) \
      pmsr(4,0) \
      pasr(0,6) \
      pmsr(4,2) \
      pasr(2,7) 
#undef p1_2_gemvT_1
#define p1_2_gemvT_1(a_) \
      px(4) \
      pld(SS(a_,MM(0,RS4)),bx,4) \
      px(0) \
      pld(SS(a_,MM(0,RS4)),ax,0) \
      px(2) \
      pld(SS(a_,MM(0,RS4)),si,2) \
      pm(4,0) \
      pa(0,6) \
      pm(4,2) \
      pa(2,7) 
#undef p1_gemvT_1
#define p1_gemvT_1(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(0,RS4)),si,2) \
      pm(4,0) \
      pa(0,6) \
      pm(4,2) \
      pa(2,7) 
#undef p2_gemvT_1
#define p2_gemvT_1(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      plq(SS(a_,MM(0,RS4)),si,2) \
      plq(SS(a_,MM(1,RS4)),si,3) \
      pm(4,0) \
      pa(0,6) \
      pm(4,2) \
      pa(2,7) \
      pm(5,1) \
      pa(1,6) \
      pm(5,3) \
      pa(3,7) 
#undef p4_gemvT_1
#define p4_gemvT_1(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      f(nta,SS(a_,MM((SS(0,CL)),RS4)),ax) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      plq(SS(a_,MM(0,RS4)),si,2) \
      plq(SS(a_,MM(1,RS4)),si,3) \
      pm(4,0) \
      pa(0,6) \
      pm(4,2) \
      pa(2,7) \
      pm(5,1) \
      pa(1,6) \
      pm(5,3) \
      pa(3,7) \
      plb(SS(a_,MM(2,RS4)),bx,4) \
      plb(SS(a_,MM(3,RS4)),bx,5) \
      plq(SS(a_,MM(2,RS4)),ax,0) \
      plq(SS(a_,MM(3,RS4)),ax,1) \
      f(nta,SS(a_,MM((SS(0,CL)),RS4)),si) \
      plq(SS(a_,MM(2,RS4)),si,2) \
      plq(SS(a_,MM(3,RS4)),si,3) \
      pm(4,0) \
      pa(0,6) \
      pm(4,2) \
      pa(2,7) \
      pm(5,1) \
      pa(1,6) \
      pm(5,3) \
      pa(3,7) 
#undef lpgemvT_1
#define lpgemvT_1(a_) 
#undef dpgemvT_1
#define dpgemvT_1(a_) p4_gemvT_1(a_)
#undef plgemvT_1
#define plgemvT_1 16

#undef p1_4_gemvT_1_1
#define p1_4_gemvT_1_1(a_) \
      pls(SS(a_,MM(0,RS4)),bx,4) \
      pls(SS(a_,MM(0,RS4)),ax,0) \
      pmsr(4,0) \
      pasr(0,6) 
#undef p1_2_gemvT_1_1
#define p1_2_gemvT_1_1(a_) \
      px(4) \
      pld(SS(a_,MM(0,RS4)),bx,4) \
      px(0) \
      pld(SS(a_,MM(0,RS4)),ax,0) \
      pm(4,0) \
      pa(0,6) 
#undef p1_gemvT_1_1
#define p1_gemvT_1_1(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      pm(4,0) \
      pa(0,6) 
#undef p2_gemvT_1_1
#define p2_gemvT_1_1(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      pm(4,0) \
      pa(0,6) \
      pm(5,1) \
      pa(1,6) 
#undef p4_gemvT_1_1
#define p4_gemvT_1_1(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plb(SS(a_,MM(2,RS4)),bx,3) \
      f(nta,SS(a_,MM((SS(0,CL)),RS4)),ax) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      plq(SS(a_,MM(2,RS4)),ax,2) \
      pm(4,0) \
      pa(0,6) \
      plb(SS(a_,MM(3,RS4)),bx,4) \
      plq(SS(a_,MM(3,RS4)),ax,0) \
      pm(5,1) \
      pa(1,6) \
      pm(3,2) \
      pa(2,6) \
      pm(4,0) \
      pa(0,6) 
#undef lpgemvT_1_1
#define lpgemvT_1_1(a_) 
#undef dpgemvT_1_1
#define dpgemvT_1_1(a_) p4_gemvT_1_1(a_)
#undef plgemvT_1_1
#define plgemvT_1_1 16

#undef p1_4_gemvT_1_3
#define p1_4_gemvT_1_3(a_) \
      pls(SS(a_,MM(0,RS4)),bx,4) \
      pls(SS(a_,MM(0,RS4)),ax,0) \
      pmsr(4,0) \
      pasr(0,6) 
#undef p1_2_gemvT_1_3
#define p1_2_gemvT_1_3(a_) \
      px(4) \
      pld(SS(a_,MM(0,RS4)),bx,4) \
      px(0) \
      pld(SS(a_,MM(0,RS4)),ax,0) \
      pm(4,0) \
      pa(0,6) 
#undef p1_gemvT_1_3
#define p1_gemvT_1_3(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      pm(4,0) \
      pa(0,6) 
#undef p2_gemvT_1_3
#define p2_gemvT_1_3(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      pm(4,0) \
      pa(0,6) \
      pm(5,1) \
      pa(1,6) 
#undef p4_gemvT_1_3
#define p4_gemvT_1_3(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plb(SS(a_,MM(2,RS4)),bx,3) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      plq(SS(a_,MM(2,RS4)),ax,2) \
      f(nta,SS(a_,MM((SS(2,CL)),RS4)),ax) \
      pm(4,0) \
      pa(0,6) \
      plb(SS(a_,MM(3,RS4)),bx,4) \
      plq(SS(a_,MM(3,RS4)),ax,0) \
      pm(5,1) \
      pa(1,7) \
      pm(3,2) \
      pa(2,6) \
      pm(4,0) \
      pa(0,7) 
#undef p8_gemvT_1_3
#define p8_gemvT_1_3(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plb(SS(a_,MM(2,RS4)),bx,3) \
      f(nta,SS(a_,MM((SS(0,CL)),RS4)),ax) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      plq(SS(a_,MM(2,RS4)),ax,2) \
      pm(4,0) \
      pa(0,6) \
      plb(SS(a_,MM(3,RS4)),bx,4) \
      plq(SS(a_,MM(3,RS4)),ax,0) \
      pm(5,1) \
      pa(1,7) \
      pm(3,2) \
      pa(2,6) \
      pm(4,0) \
      pa(0,7) \
      plb(SS(a_,MM(4,RS4)),bx,4) \
      plb(SS(a_,MM(5,RS4)),bx,5) \
      plb(SS(a_,MM(6,RS4)),bx,3) \
      plq(SS(a_,MM(4,RS4)),ax,0) \
      plq(SS(a_,MM(5,RS4)),ax,1) \
      plq(SS(a_,MM(6,RS4)),ax,2) \
      pm(4,0) \
      pa(0,6) \
      plb(SS(a_,MM(7,RS4)),bx,4) \
      plq(SS(a_,MM(7,RS4)),ax,0) \
      pm(5,1) \
      pa(1,7) \
      pm(3,2) \
      pa(2,6) \
      pm(4,0) \
      pa(0,7) 
#undef lpgemvT_1_3
#define lpgemvT_1_3(a_) 
#undef dpgemvT_1_3
#define dpgemvT_1_3(a_) p4_gemvT_1_3(a_)
#undef plgemvT_1_3
#define plgemvT_1_3 16

#undef p1_4_gemvT_1_1c
#define p1_4_gemvT_1_1c(a_) 
#undef p1_2_gemvT_1_1c
#define p1_2_gemvT_1_1c(a_) \
      px(4) \
      pld(SS(a_,MM(0,RS4)),bx,4) \
      px(0) \
      pld(SS(a_,MM(0,RS4)),ax,0) \
      pc(4,2) \
      ps(CSHUF,4,4) \
      pm(0,2) \
      pa(2,6) \
      pm(0,4) \
      pa(4,7) 
#undef p1_gemvT_1_1c
#define p1_gemvT_1_1c(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      pc(4,2) \
      ps(CSHUF,4,4) \
      pm(0,2) \
      pa(2,6) \
      pm(0,4) \
      pa(4,7) 
#undef p2_gemvT_1_1c
#define p2_gemvT_1_1c(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      pc(4,2) \
      pc(5,3) \
      ps(CSHUF,4,4) \
      ps(CSHUF,5,5) \
      pm(0,2) \
      pa(2,6) \
      pm(0,4) \
      pa(4,7) \
      pm(1,3) \
      pa(3,6) \
      pm(1,5) \
      pa(5,7) 
#undef p4_gemvT_1_1c
#define p4_gemvT_1_1c(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      f(nta,SS(a_,MM((SS(0,CL)),RS4)),ax) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      pc(4,2) \
      pc(5,3) \
      ps(CSHUF,4,4) \
      ps(CSHUF,5,5) \
      pm(0,2) \
      pa(2,6) \
      pm(0,4) \
      pa(4,7) \
      pm(1,3) \
      pa(3,6) \
      pm(1,5) \
      pa(5,7) \
      plb(SS(a_,MM(2,RS4)),bx,4) \
      plb(SS(a_,MM(3,RS4)),bx,5) \
      plq(SS(a_,MM(2,RS4)),ax,0) \
      plq(SS(a_,MM(3,RS4)),ax,1) \
      pc(4,2) \
      pc(5,3) \
      ps(CSHUF,4,4) \
      ps(CSHUF,5,5) \
      pm(0,2) \
      pa(2,6) \
      pm(0,4) \
      pa(4,7) \
      pm(1,3) \
      pa(3,6) \
      pm(1,5) \
      pa(5,7) 
#undef lpgemvT_1_1c
#define lpgemvT_1_1c(a_) 
#undef dpgemvT_1_1c
#define dpgemvT_1_1c(a_) p4_gemvT_1_1c(a_)
#undef plgemvT_1_1c
#define plgemvT_1_1c 16

#undef p1_4_gemvT_3_1
#define p1_4_gemvT_3_1(a_) \
      pls(SS(a_,MM(0,RS4)),bx,3) \
      pls(SS(a_,MM(0,RS4)),ax,0) \
      plsx(SS(a_,MM(0,RS4)),ax,bp,1,1) \
      plsx(SS(a_,MM(0,RS4)),ax,bp,2,2) \
      pmsr(3,0) \
      pasr(0,4) \
      pmsr(3,1) \
      pasr(1,5) \
      pmsr(3,2) \
      pasr(2,6) 
#undef p1_2_gemvT_3_1
#define p1_2_gemvT_3_1(a_) \
      px(3) \
      px(0) \
      px(1) \
      px(2) \
      pld(SS(a_,MM(0,RS4)),bx,3) \
      pld(SS(a_,MM(0,RS4)),ax,0) \
      pldx(SS(a_,MM(0,RS4)),ax,bp,1,1) \
      pldx(SS(a_,MM(0,RS4)),ax,bp,2,2) \
      pm(3,0) \
      pa(0,4) \
      pm(3,1) \
      pa(1,5) \
      pm(3,2) \
      pa(2,6) 
#undef p1_gemvT_3_1
#define p1_gemvT_3_1(a_) \
      plb(SS(a_,MM(0,RS4)),bx,3) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plqx(SS(a_,MM(0,RS4)),ax,bp,1,1) \
      plqx(SS(a_,MM(0,RS4)),ax,bp,2,2) \
      pm(3,0) \
      pa(0,4) \
      pm(3,1) \
      pa(1,5) \
      pm(3,2) \
      pa(2,6) 
#undef p2_gemvT_3_1
#define p2_gemvT_3_1(a_) \
      plb(SS(a_,MM(0,RS4)),bx,3) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plqx(SS(a_,MM(0,RS4)),ax,bp,1,1) \
      plqx(SS(a_,MM(0,RS4)),ax,bp,2,2) \
      pm(3,0) \
      pa(0,4) \
      pm(3,1) \
      pa(1,5) \
      pm(3,2) \
      pa(2,6) \
      plb(SS(a_,MM(1,RS4)),bx,3) \
      plq(SS(a_,MM(1,RS4)),ax,0) \
      plqx(SS(a_,MM(1,RS4)),ax,bp,1,1) \
      plqx(SS(a_,MM(1,RS4)),ax,bp,2,2) \
      pm(3,0) \
      pa(0,4) \
      pm(3,1) \
      pa(1,5) \
      pm(3,2) \
      pa(2,6) 
#undef p4_gemvT_3_1
#define p4_gemvT_3_1(a_) \
      plb(SS(a_,MM(0,RS4)),bx,3) \
      f(nta,SS(a_,MM((SS(0,CL)),RS4)),ax) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plqx(SS(a_,MM(0,RS4)),ax,bp,1,1) \
      plqx(SS(a_,MM(0,RS4)),ax,bp,2,2) \
      pm(3,0) \
      pa(0,4) \
      pm(3,1) \
      pa(1,5) \
      pm(3,2) \
      pa(2,6) \
      plb(SS(a_,MM(1,RS4)),bx,3) \
      plq(SS(a_,MM(1,RS4)),ax,0) \
      pfx(nta,SS(a_,MM((SS(0,CL)),RS4)),ax,bp,1) \
      plqx(SS(a_,MM(1,RS4)),ax,bp,1,1) \
      plqx(SS(a_,MM(1,RS4)),ax,bp,2,2) \
      pm(3,0) \
      pa(0,4) \
      pm(3,1) \
      pa(1,5) \
      pm(3,2) \
      pa(2,6) \
      plb(SS(a_,MM(2,RS4)),bx,3) \
      plq(SS(a_,MM(2,RS4)),ax,0) \
      plqx(SS(a_,MM(2,RS4)),ax,bp,1,1) \
      pfx(nta,SS(a_,MM((SS(0,CL)),RS4)),ax,bp,2) \
      plqx(SS(a_,MM(2,RS4)),ax,bp,2,2) \
      pm(3,0) \
      pa(0,4) \
      pm(3,1) \
      pa(1,5) \
      pm(3,2) \
      pa(2,6) \
      plb(SS(a_,MM(3,RS4)),bx,3) \
      plq(SS(a_,MM(3,RS4)),ax,0) \
      plqx(SS(a_,MM(3,RS4)),ax,bp,1,1) \
      plqx(SS(a_,MM(3,RS4)),ax,bp,2,2) \
      pm(3,0) \
      pa(0,4) \
      pm(3,1) \
      pa(1,5) \
      pm(3,2) \
      pa(2,6) 
#undef lpgemvT_3_1
#define lpgemvT_3_1(a_) 
#undef dpgemvT_3_1
#define dpgemvT_3_1(a_) p4_gemvT_3_1(a_)
#undef plgemvT_3_1
#define plgemvT_3_1 16

#undef p1_4_gemvT_1_2
#define p1_4_gemvT_1_2(a_) \
      pls(SS(a_,MM(0,RS4)),bx,4) \
      pls(SS(a_,MM(0,RS4)),ax,0) \
      pmsr(4,0) \
      pasr(0,6) 
#undef p1_2_gemvT_1_2
#define p1_2_gemvT_1_2(a_) \
      px(4) \
      pld(SS(a_,MM(0,RS4)),bx,4) \
      px(0) \
      pld(SS(a_,MM(0,RS4)),ax,0) \
      pm(4,0) \
      pa(0,6) 
#undef p1_gemvT_1_2
#define p1_gemvT_1_2(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      pm(4,0) \
      pa(0,6) 
#undef p2_gemvT_1_2
#define p2_gemvT_1_2(a_) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plq(SS(a_,MM(0,RS4)),ax,0) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      pm(4,0) \
      pa(0,6) \
      pm(5,1) \
      pa(1,6) 
#undef p4_gemvT_1_2
#define p4_gemvT_1_2(a_) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      pm(0,4) \
      pa(4,6) \
      plb(SS(a_,MM(2,RS4)),bx,4) \
      plq(SS(a_,MM(2,RS4)),ax,0) \
      pm(1,5) \
      pa(5,6) \
      plb(SS(a_,MM(3,RS4)),bx,5) \
      plq(SS(a_,MM(3,RS4)),ax,1) \
      pm(0,4) \
      pa(4,6) \
      plb(SS(a_,MM(4,RS4)),bx,4) \
      f(nta,SS(a_,MM((SS(4,CL)),RS4)),ax) \
      plq(SS(a_,MM(4,RS4)),ax,0) \
      pm(1,5) \
      pa(5,6) 
#undef lpgemvT_1_2
#define lpgemvT_1_2(a_) \
      f(nta,SS(a_,MM((SS(0,CL)),RS4)),ax) \
      plb(SS(a_,MM(0,RS4)),bx,4) \
      plq(SS(a_,MM(0,RS4)),ax,0) 
#undef dpgemvT_1_2
#define dpgemvT_1_2(a_) \
      plb(SS(a_,MM(1,RS4)),bx,5) \
      plq(SS(a_,MM(1,RS4)),ax,1) \
      pm(0,4) \
      pa(4,6) \
      plb(SS(a_,MM(2,RS4)),bx,4) \
      plq(SS(a_,MM(2,RS4)),ax,0) \
      pm(1,5) \
      pa(5,6) \
      plb(SS(a_,MM(3,RS4)),bx,5) \
      plq(SS(a_,MM(3,RS4)),ax,1) \
      pm(0,4) \
      pa(4,6) \
      pm(1,5) \
      pa(5,6) 
#undef plgemvT_1_2
#define plgemvT_1_2 16



/* #define BITS 8 */
/* #define CL 56 */
#define NDPM 1

#define BITS 8
#if defined(SREAL) || defined(DREAL)
#define CL 56
#else
#define CL 32
#endif

/* #include "out.h" */
/* #include "foo.h" */


#define FN Mjoin(Mjoin(Mjoin(ATL_,PREC),gemv),Mjoin(FEXT,Mjoin(_a1_x1_,Mjoin(BL,_y1))))

#undef MY_FUNCTION
#define MY_FUNCTION FN


void 
MY_FUNCTION(int m,int n, const SCALAR alpha,const TYPE *a,
   int lda,const TYPE *b,int binc,
   const SCALAR beta,TYPE *c,int cinc) {

  NO_INLINE
  int ks;
#ifdef GCCWIN
  void *freeme[4]={NULL,NULL,NULL,NULL};
#endif
  const TYPE *bs[4]={NULL,NULL,NULL,NULL};
  const TYPE *ds[5]={NULL,NULL,NULL,NULL,NULL};

#undef AL
#define AL(a_) (((unsigned long)(a_))&0xf)

  const TYPE *at;
  long j;
  int k;

  at=a;
  for (k=0;k<4;at+=lda,k++) {

#ifdef REAL
    int l=AL(at)/4;
#else
    int l=0;
#endif
    
    if (bs[l])
      continue;

#ifdef COPY_B
    if (l!=AL(b)/4) {
    #ifndef GCCWIN
      bs[l]=alloca(n*sizeof(*b)+15);
    #else
      freeme[l] = bs[l]=malloc(n*sizeof(*b)+15);
    #endif
      ATL_assert(bs[l]);
      j=4*l-AL(bs[l]);
      j=j<0? j+16 : j;
      bs[l]=(void *)bs[l]+j;
      Mjoin(ATL_,Mjoin(PREC,copy))(n,(void *)b,1,(void *)bs[l],1);
#ifdef MDEBUG
      printf("Allocing:  a %p b %p X0 %p %d %d %d j %d %f %f\n",
	     at,b,bs[l],AL(at),AL(b),AL(bs[l]),j,b[0],*(TYPE *)(bs[l]));
#endif
    } else
#endif
      bs[l]=b;
  }

#if !defined(REAL)
  if (!bs[AL(a)/4])
    bs[AL(a)/4]=bs[0];
#if defined(SINGLE)
  if (!bs[AL(a+lda)/4])
    bs[AL(a+lda)/4]=bs[0];
#endif
#endif

#undef N
#define N main

    ds[0]=a+m*lda;
    ds[1]=(const TYPE *)(lda*sizeof(*a));
    ds[2]=(const TYPE *)sizeof(*c);

#if defined(BETAX) || defined(BETAXI0)
#if defined(REAL)
    ds[3]=&beta;
#else    
    ds[3]=beta;
#endif
#endif

#if !defined(REAL)
    ds[4]=signd;
#endif

#ifdef MDEBUG
    printf("bs is %p    %p %p %p %p\n",bs,bs[0],bs[1],bs[2],bs[3]);
    printf("ds is %p    %p %p %p %p\n",ds,ds[0],ds[1],ds[2],ds[3],ds[4]);
    printf("b is %p\n",b);
#endif

    ASM (
	 
	 "pushl %%ebx\n\t"
	 "movl %%esi,%%ebx\n\t"
	 
	 align
	 lab(a_loop)
	 
	 icmpr(ax,di)
	 je(a_end)
	 
	 push(ax)
	 push(bx)
	 push(dx)

	 "movl %%eax,%%esi\n\t"
	 "and $0xf,%%esi\n\t"
	 "movl (%%ebx,%%esi,1),%%ebx\n\t"
	 
	 px(6) px(7)

#undef VERS
#if defined(REAL)
#define VERS 3
#else
#define VERS 1c
#endif
#undef N
#define N Mjoin(Mjoin(gemvT_,1_),VERS)
#define ALIGN
#undef INC
#define INC(a_) a(a_,ax) a(a_,bx)
#undef LR
#define LR dx
#include "camm_tpipe.h"
#undef N
#define N main

#ifndef BETA0
	 f(nta,0,cx)
#endif

#if defined(REAL)
#if VERS == 3
 	 pa(7,6)
#endif

#else

	 "movl 16(%%edi),%%esi\n\t"
	 pl(0,si,1)
#ifdef Conj_
	 pm(1,7)
#else
	 pm(1,6)
#endif

	 pc(6,5)
	 pul(7,6)
	 puh(7,5)
	 pa(5,6)

#endif

#if defined(SINGLE)
	 px(5)
	 phl(6,5)
	 pa(5,6)
#endif

#if defined(REAL)
	 pc(6,5)
	 ps(1,6,6)
	 pasr(5,6)
#endif

#if !defined(BETA0)

#if defined(REAL)
	 pls(0,cx,0)
#elif defined(SINGLE)
	 px(0) pld(0,cx,0)
#else
	 pl(0,cx,0)
#endif

#if defined(BETAX) || defined(BETAXI0)
	 "movl 12(%%edi),%%esi\n\t"

#if defined(REAL) || defined(BETAXI0)
	 pls(0,si,5)
#elif defined(SINGLE)
	 px(5) pld(0,si,5)
#else
	 pl(0,si,5)
#endif

#if defined(REAL)
	 pmsr(5,0)
#elif defined(BETAXI0)
	 ps(0,5,5)
	 pm(5,0)
#else
 	 pc(0,2)
	 ps(CSHUF,0,0)
	 pm(5,2)
	 pm(5,0)
  	 pm(1,2) 
#if defined(SINGLE)
	 pul(0,2)
	 phl(2,0)
#else
	 pc(0,1)
	 pc(2,0)
	 pul(1,2)
	 puh(1,0)
#endif
#endif

#endif
#if defined(REAL)
	 pasr(0,6)
#else
	 pa(0,6)
#if defined(BETAX) && !defined(REAL)
	 pa(2,6)
#endif
#endif
#endif

#if defined(REAL)
	 pus(6,0,cx)
#elif defined(SINGLE)
	 pud(6,0,cx)
#else
	 pu(6,0,cx)
#endif

	 pop(dx)
	 pop(bx)
	 pop(ax)

	 "addl 4(%%edi),%%eax\n\t"
	 "addl 8(%%edi),%%ecx\n\t"

	 jmp(a_loop)
	 
	 lab(a_end)
	 
	 "movl %%ebx,%%esi\n\t"
	 "popl %%ebx\n\t"
	 
	 ::"a" (a),"S" (bs),"c" (c),"d" (n*DIV),"D" (ds)
	 :"memory");

    #ifdef GCCWIN
       for (k=0; k < 4; k++)
          if (freeme[k]) free(freeme[k]);
    #endif
#ifdef MDEBUG
    printf("bs is %p    %p %p %p %p\n",bs,bs[0],bs[1],bs[2],bs[3]);
    printf("ds is %p    %p %p %p %p\n",ds,ds[0],ds[1],ds[2],ds[3],ds[4]);


    printf("b is %p\n",b);
#endif


}
