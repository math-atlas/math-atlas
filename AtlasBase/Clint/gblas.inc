@ifdef ! @rout
   @addkeys rout=refblas
@endifdef
@ifdef ! @lvl
   @addkeys lvl=all
@endifdef
@lvl 1 2 3
   @define lvl @@(@lvl)@
@lvl ! 1 2 3
   @define lvl @@
@lvl !
@ROUT REFBLAS
   @define han1 @ATL_@
   @define han2 @@
   @define rep1 @ATL_@
   @define rep2 @ref@
#ifndef ATLAS_REFALIAS@(lvl)_H
#define ATLAS_REFALIAS@(lvl)_H
@ROUT PTBLAS
   @define han1 @ATL_@
   @define han2 @@
   @define rep1 @ATL_@
   @define rep2 @pt@
   @define rep2c @t@
   @define rep2r @tomp@
@lvl 1 2
#define ATLAS_PTALIAS@(lvl)_H /* no threaded routs for Level 1 and 2 yet */
@lvl !
#ifndef ATLAS_PTALIAS@(lvl)_H
#define ATLAS_PTALIAS@(lvl)_H
@ROUT !
/*
 * Real BLAS
 */
@LVL 1 ALL
   #define @(han1)dsdot   @(rep1)ds@(rep2)dot
   #define @(han1)sdsdot  @(rep1)sds@(rep2)dot
@LVL !
@whiledef pre d s
   @LVL 3 ALL
      @whiledef rout gemm trsm trmm symm syrk syr2k
   #ifdef ATL_ANTOINE_THREADS
      #define @(han1)@(pre)@(han2)@6l@(rout) @(rep1)@(pre)@(rep2)@(rout)
   #else
      #define @(han1)@(pre)@(han2)@6l@(rout) @(rep1)@(pre)@(rep2c)@(rout)
   #endif
      @endwhile
@LVL ! 3
   @multidef rout 
      @LVL 2 ALL
         gemv gbmv symv sbmv spmv trmv tbmv tpmv trsv tbsv tpsv
         ger syr spr syr2 spr2 ger2
      @LVL 1 ALL
         rotg rotmg rot rotm swap scal copy axpy dot nrm2 asum
      @LVL ! 3
   @endmultidef
   @whiledef rout
   #define @(han1)@(pre)@(han2)@6l@(rout) @(rep1)@(pre)@(rep2)@(rout)
   @endwhile
@LVL 1 ALL
   #define @(han1)i@(pre)@(han2)amax  @(rep1)i@(pre)@(rep2)amax
@LVL !

@endwhile
/*
 * Complex BLAS
 */
@multidef upr d s
@whiledef pre z c
      @LVL 3 ALL
   @whiledef rout trsm gemm symm hemm syrk herk syr2k her2k trmm
   #ifdef ATL_ANTOINE_THREADS
      #define @(han1)@(pre)@(han2)@8l@(rout) @(rep1)@(pre)@(rep2)@(rout)
   #else
      #define @(han1)@(pre)@(han2)@8l@(rout) @(rep1)@(pre)@(rep2c)@(rout)
   #endif
   @endwhile
      @LVL 2 1
   @multidef rout
      @LVL 2 ALL
         gemv gbmv hemv hbmv hpmv trmv tbmv tpmv trsv tbsv tpsv
         geru gerc her hpr her2 hpr2 ger2u ger2c
      @LVL 1 ALL
         swap scal copy axpy dotu_sub dotc_sub
      @LVL ! 3
   @endmultidef
   @whiledef rout
   #define @(han1)@(pre)@(han2)@8l@(rout) @(rep1)@(pre)@(rep2)@(rout)
   @endwhile
@LVL 1 ALL
   #define @(han1)i@(pre)@(han2)amax    @(rep1)i@(pre)@(rep2)amax
   #define @(han1)@(pre)@(upr)@(han2)scal    @(rep1)@(pre)@(upr)@(rep2)scal
   #define @(han1)@(upr)@(pre)@(han2)nrm2    @(rep1)@(upr)@(pre)@(rep2)nrm2
   #define @(han1)@(upr)@(pre)@(han2)asum    @(rep1)@(upr)@(pre)@(rep2)asum
@LVL !

   @undef upr
@endwhile
#endif
