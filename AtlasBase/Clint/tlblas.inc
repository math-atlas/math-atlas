@define pre @@low@(@pre)@
#ifndef ATLAS_CBLAS@up@(pre)ALIAS_H
   #define ATLAS_CBLAS@up@(pre)ALIAS_H

@PRE D S
   @multidef rout 
         gemm symm syrk syr2k trmm trsm
         gemv gbmv symv sbmv spmv trmv tbmv tpmv trsv tbsv tpsv
         ger syr spr syr2 spr2 ger2
         rotg rotmg rot rotm swap scal copy axpy dot nrm2 asum
   @endmultidef
   @whiledef rout
#define cblas_@6l@(rout) cblas_@(pre)@(rout)
   @endwhile
#define cblas_iamax  cblas_i@(pre)amax
@PRE C
   @define upr @s@
@PRE Z
   @define upr @d@
@PRE C Z
   @multidef rout
         gemm symm hemm syrk herk syr2k her2k trmm trsm
         gemv gbmv hemv hbmv hpmv trmv tbmv tpmv trsv tbsv tpsv
         geru gerc her hpr her2 hpr2 ger2u ger2c
         swap scal copy axpy dotu_sub dotc_sub
   @endmultidef
   @whiledef rout
#define cblas_@8l@(rout) cblas_@(pre)@(rout)
   @endwhile
#define cblas_iamax    cblas_i@(pre)amax
#define cblas_nrm2     cblas_@(upr)@(pre)nrm2
#define cblas_asum     cblas_@(upr)@(pre)asum
@undef upr
@PRE !
@undef pre

#endif
