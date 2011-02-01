@skip
@skip   General ATLAS macro definitions
@skip
@define F77comp  @	$(F77) -o $@ -c $(F77FLAGS)@
@define CCcomp   @	$(ICC) -o $@ -c $(ICCFLAGS)@
@define CCptcomp @	$(ICC) -o $@ -c $(ICCFLAGS) -DATL_USEPTHREADS@
@define arch     @$(ARCHIVER)@
@define archflgs @$(ARFLAGS)@
@define ranlib   @$(RANLIB)@
@define atllib   @$(ATLASlib)@
@define ptxlib   @$(ATLASlib)@
@define f77lib   @$(F77BLASlib)@
@define reflib   @$(REFBLASlib)@
@define ptf77lib @$(PTF77BLASlib)@
@skip
@define VER_ATLAS_MAK          @-b @(incdir)vers.inc lan=M@
@lan F
@skip
@skip   Version definition macros
@skip
@define VER_ATLAS_TST_DRIVER   @-b @(incdir)vers.inc lan=F inc=AtlasTstDrv@
@define VER_ATLAS_TST          @-b @(incdir)vers.inc lan=F inc=AtlasTstRout@
@define VER_ATLAS_TIM_DRIVER   @-b @(incdir)vers.inc lan=F inc=AtlasTimDrv@
@define VER_ATLAS_TIM          @-b @(incdir)vers.inc lan=F inc=AtlasTimRout@
@define VER_ATLAS_SRC          @-b @(incdir)vers.inc lan=F inc=AtlasSrcRout@
@define VER_ATLAS_AUX          @-b @(incdir)vers.inc lan=F inc=AtlasAuxRout@
@skip
@define VER_ATLAS_F77_SRC      @-b @(incdir)vers.inc lan=F inc=AtlasF77Rout@
@skip
@skip   Comment file definition macros
@skip
@define ATLAS_F77_LIN_COMMENT  @-addkeys -b @(comdir)atlF77lin.txt  lan=F@
@define ATLAS_F77_SRC_COMMENT  @-addkeys -b @(comdir)atlF77blas.txt lan=F@
@skip
@skip   Comment definition macros
@skip
@define DOTS                   @-b @(incdir)misc.inc lan=F inc=dotdot@
@define SCALAR_ARGUMENT        @-b @(incdir)misc.inc lan=F inc=sarg@
@define ARRAY_ARGUMENT         @-b @(incdir)misc.inc lan=F inc=aarg@
@define FUNCTION_ARGUMENT      @-b @(incdir)misc.inc lan=F inc=farg@
@define SUBROUTINE_ARGUMENT    @-b @(incdir)misc.inc lan=F inc=subarg@
@define PURPOSE                @-b @(incdir)misc.inc lan=F inc=purp@
@define NOTES                  @-b @(incdir)misc.inc lan=F inc=note@
@define ARGUMENTS              @-b @(incdir)misc.inc lan=F inc=arg@
@define INTERNAL_PARAMETER     @-b @(incdir)misc.inc lan=F inc=in_param@
@define FURTHER_DETAILS        @-b @(incdir)misc.inc lan=F inc=detail@
@define LINE                   @-b @(incdir)misc.inc lan=F inc=comline@
@define PARAMETER              @-b @(incdir)misc.inc lan=F inc=param@
@define LOCAL_SCALAR           @-b @(incdir)misc.inc lan=F inc=sloc@
@define LOCAL_ARRAY            @-b @(incdir)misc.inc lan=F inc=aloc@
@define LOCAL_VARIABLE         @-b @(incdir)misc.inc lan=F inc=vloc@
@define EXTERNAL_SUBROUTINE    @-b @(incdir)misc.inc lan=F inc=exsub@
@define EXTERNAL_FUNCTION      @-b @(incdir)misc.inc lan=F inc=exfunc@
@define INTRINSIC_FUNCTION     @-b @(incdir)misc.inc lan=F inc=infunc@
@define STATEMENT_FUNCTION     @-b @(incdir)misc.inc lan=F inc=statfunc@
@define COMMON_BLOCK           @-b @(incdir)misc.inc lan=F inc=common@
@define DATA_STATEMENT         @-b @(incdir)misc.inc lan=F inc=data@
@define SAVE_STATEMENT         @-b @(incdir)misc.inc lan=F inc=save@
@define EQUIVALENCE_STATEMENT  @-b @(incdir)misc.inc lan=F inc=equiv@
@define EXECUTABLE_STATEMENT   @-b @(incdir)misc.inc lan=F inc=exstat@
@define TESTINPUT_COMMENT      @-b @(incdir)misc.inc lan=F inc=tstin@
@define QUICKRETURN_COMMENT    @-b @(incdir)misc.inc lan=F inc=quick@
@define STARTOP_COMMENT        @-b @(incdir)misc.inc lan=F inc=start@
@define GETTYPE_COMMENT        @-b @(incdir)misc.inc lan=F inc=gettype@
@define END_OF_ROUTINE         @-b @(incdir)misc.inc lan=F inc=endofrout@
@skip
@define UPLO_SY_COMMENT        @-b @(incdir)comm.inc lan=F inc=uplosy -def Z@
@define UPLO_SB_COMMENT        @-b @(incdir)comm.inc lan=F inc=uplosb -def Z@
@define UPLO_SP_COMMENT        @-b @(incdir)comm.inc lan=F inc=uplosp -def Z@
@define UPLO_TR_COMMENT        @-b @(incdir)comm.inc lan=F inc=uplotr -def Z@
@define TRANS_COMMENT          @-b @(incdir)comm.inc lan=F inc=trns@
@define TRANSX_COMMENT         @-b @(incdir)comm.inc lan=F inc=trnsx  -def Z@
@define DIAG_COMMENT           @-b @(incdir)comm.inc lan=F inc=unit   -def Z@
@skip
@skip   Declaration definition macros
@skip
@define EXTERNAL               @"      EXTERNAL           "@
@define INTRINSIC              @"      INTRINSIC          "@
@define INTEGER                @"      INTEGER            "@
@define LOGICAL                @"      LOGICAL            "@
@define CHARACTER              @"      CHARACTER          "@
@define FTYPE                  @"      @(type) "@
@define FUTYP                  @"      @(utyp) "@
@lan C
@skip
@skip   Version definition macros
@skip
@define VER_ATLAS_TST_DRIVER   @-b @(incdir)vers.inc lan=C inc=AtlasTstDrv@
@define VER_ATLAS_TST          @-b @(incdir)vers.inc lan=C inc=AtlasTstRout@
@define VER_ATLAS_TIM_DRIVER   @-b @(incdir)vers.inc lan=C inc=AtlasTimDrv@
@define VER_ATLAS_TIM          @-b @(incdir)vers.inc lan=C inc=AtlasTimRout@
@define VER_ATLAS_LIN          @-b @(incdir)vers.inc lan=C inc=AtlasLinRout@
@define VER_ATLAS_SRC          @-b @(incdir)vers.inc lan=C inc=AtlasSrcRout@
@define VER_ATLAS_AUX          @-b @(incdir)vers.inc lan=C inc=AtlasAuxRout@
@skip
@skip   Comment file definition macros
@skip
@define ATLAS_WRP_COMMENT      @-addkeys -b @(comdir)atlwrpblas.txt lan=C@
@define ATLAS_REF_COMMENT      @-addkeys -b @(comdir)atlrefblas.txt lan=C@
@define ATLAS_LV2_COMMENT      @-addkeys -b @(comdir)atllv2blas.txt lan=C@
@define ATLAS_RC2_COMMENT      @-addkeys -b @(comdir)atlrc2blas.txt lan=C@
@define ATLAS_LV3_COMMENT      @-addkeys -b @(comdir)atllv3blas.txt lan=C@
@define ATLAS_RC3_COMMENT      @-addkeys -b @(comdir)atlrc3blas.txt lan=C@
@define ATLAS_PTM_COMMENT      @-addkeys -b @(comdir)atlptmblas.txt lan=C@
@define ATLAS_PT1_COMMENT      @-addkeys -b @(comdir)atlpt1blas.txt lan=C@
@define ATLAS_PT2_COMMENT      @-addkeys -b @(comdir)atlpt2blas.txt lan=C@
@define ATLAS_PT3_COMMENT      @-addkeys -b @(comdir)atlpt3blas.txt lan=C@
@skip
@skip   Comment definition macros
@skip
@define INCLUDE                @-b @(incdir)misc.inc lan=C inc=incf@
@define DOTS                   @-b @(incdir)misc.inc lan=C inc=dotdot@
@define SCALAR_ARGUMENT        @-b @(incdir)misc.inc lan=C inc=sarg@
@define ARRAY_ARGUMENT         @-b @(incdir)misc.inc lan=C inc=aarg@
@define SUBROUTINE_ARGUMENT    @-b @(incdir)misc.inc lan=C inc=subarg@
@define FUNCTION_ARGUMENT      @-b @(incdir)misc.inc lan=C inc=farg@
@define PURPOSE                @-b @(incdir)misc.inc lan=C inc=purp@
@define NOTES                  @-b @(incdir)misc.inc lan=C inc=note@
@define ARGUMENTS              @-b @(incdir)misc.inc lan=C inc=arg@
@define INTERNAL_PARAMETER     @-b @(incdir)misc.inc lan=C inc=in_param@
@define FURTHER_DETAILS        @-b @(incdir)misc.inc lan=C inc=detail@
@define LINE                   @-b @(incdir)misc.inc lan=C inc=comline@
@define PARAMETER              @-b @(incdir)misc.inc lan=C inc=param@
@define LOCAL_SCALAR           @-b @(incdir)misc.inc lan=C inc=sloc@
@define LOCAL_ARRAY            @-b @(incdir)misc.inc lan=C inc=aloc@
@define LOCAL_VARIABLE         @-b @(incdir)misc.inc lan=C inc=vloc@
@define EXTERNAL_SUBROUTINE    @-b @(incdir)misc.inc lan=C inc=exsub@
@define EXTERNAL_FUNCTION      @-b @(incdir)misc.inc lan=C inc=exfunc@
@define INTRINSIC_FUNCTION     @-b @(incdir)misc.inc lan=C inc=infunc@
@define STATEMENT_FUNCTION     @-b @(incdir)misc.inc lan=C inc=statfunc@
@define COMMON_BLOCK           @-b @(incdir)misc.inc lan=C inc=common@
@define DATA_STATEMENT         @-b @(incdir)misc.inc lan=C inc=data@
@define SAVE_STATEMENT         @-b @(incdir)misc.inc lan=C inc=save@
@define EQUIVALENCE_STATEMENT  @-b @(incdir)misc.inc lan=C inc=equiv@
@define EXECUTABLE_STATEMENT   @-b @(incdir)misc.inc lan=C inc=exstat@
@define TESTINPUT_COMMENT      @-b @(incdir)misc.inc lan=C inc=tstin@
@define QUICKRETURN_COMMENT    @-b @(incdir)misc.inc lan=C inc=quick@
@define STARTOP_COMMENT        @-b @(incdir)misc.inc lan=C inc=start@
@define GETTYPE_COMMENT        @-b @(incdir)misc.inc lan=C inc=gettype@
@define END_OF_ROUTINE         @-b @(incdir)misc.inc lan=C inc=endofrout@
@skip
@define UPLO_SY_COMMENT        @-b @(incdir)comm.inc lan=C inc=uplosy -def Z@
@define UPLO_SB_COMMENT        @-b @(incdir)comm.inc lan=C inc=uplosb -def Z@
@define UPLO_SP_COMMENT        @-b @(incdir)comm.inc lan=C inc=uplosp -def Z@
@define UPLO_TR_COMMENT        @-b @(incdir)comm.inc lan=C inc=uplotr -def Z@
@define TRANS_COMMENT          @-b @(incdir)comm.inc lan=C inc=trns@
@define TRANSX_COMMENT         @-b @(incdir)comm.inc lan=C inc=trnsx  -def Z@
@define DIAG_COMMENT           @-b @(incdir)comm.inc lan=C inc=unit   -def Z@
@skip
@skip   Declaration definition macros
@skip
@define EXTERNAL               @"      EXTERNAL           "@
@define INTRINSIC              @"      INTRINSIC          "@
@define INTEGER                @"      INTEGER            "@
@define LOGICAL                @"      LOGICAL            "@
@define CHARACTER              @"      CHARACTER          "@
@define FTYPE                  @"      @(type) "@
@define FUTYP                  @"      @(utyp) "@
@lan F C
