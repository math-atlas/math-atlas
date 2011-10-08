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
@lvl 1
#define ATLAS_PTALIAS@(lvl)_H /* no threaded routs for Level 1 yet */
@lvl !
#ifndef ATLAS_PTALIAS@(lvl)_H
#define ATLAS_PTALIAS@(lvl)_H
@ROUT !
/*
 * Real BLAS
 */
@LVL 1 ALL
   #define @(han1)dsdot   @(rep1)ds@(rep2c)dot
   #define @(han1)sdsdot  @(rep1)sds@(rep2c)dot
@LVL !
@whiledef pre d s
   @LVL 3 ALL
      @whiledef rout gemm trsm trmm symm syrk syr2k
   #define @(han1)@(pre)@(han2)@6l@(rout) @(rep1)@(pre)@(rep2c)@(rout)
      @endwhile
@LVL ! 3
   @multidef rout 
      @LVL 2 ALL
         gemv ger
@skip         gemv gbmv symv sbmv spmv trmv tbmv tpmv trsv tbsv tpsv
@skip         ger syr spr syr2 spr2 ger2
      @LVL 1 ALL
         rotg rotmg rot rotm swap scal copy axpy dot nrm2 asum
      @LVL ! 3
   @endmultidef
   @whiledef rout
   #define @(han1)@(pre)@(han2)@6l@(rout) @(rep1)@(pre)@(rep2c)@(rout)
   @endwhile
@LVL 1 ALL
   #define @(han1)i@(pre)@(han2)amax  @(rep1)i@(pre)@(rep2c)amax
@LVL !

@endwhile
/*
 * Complex BLAS
 */
@multidef upr d s
@whiledef pre z c
      @LVL 3 ALL
   @whiledef rout trsm gemm symm hemm syrk herk syr2k her2k trmm
   #define @(han1)@(pre)@(han2)@8l@(rout) @(rep1)@(pre)@(rep2c)@(rout)
   @endwhile
      @LVL 2 1
   @multidef rout
      @LVL 2 ALL
         geru gerc gemv
@skip         gemv gbmv hemv hbmv hpmv trmv tbmv tpmv trsv tbsv tpsv
@skip         geru gerc her hpr her2 hpr2 ger2u ger2c
      @LVL 1 ALL
         swap scal copy axpy dotu_sub dotc_sub
      @LVL ! 3
   @endmultidef
   @whiledef rout
   #define @(han1)@(pre)@(han2)@8l@(rout) @(rep1)@(pre)@(rep2c)@(rout)
   @endwhile
@LVL 1 ALL
   #define @(han1)i@(pre)@(han2)amax    @(rep1)i@(pre)@(rep2c)amax
   #define @(han1)@(pre)@(upr)@(han2)scal    @(rep1)@(pre)@(upr)@(rep2c)scal
   #define @(han1)@(upr)@(pre)@(han2)nrm2    @(rep1)@(upr)@(pre)@(rep2c)nrm2
   #define @(han1)@(upr)@(pre)@(han2)asum    @(rep1)@(upr)@(pre)@(rep2c)asum
@LVL !

   @undef upr
@endwhile
#endif
