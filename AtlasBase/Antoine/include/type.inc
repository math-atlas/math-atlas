@skip
@skip
@lan F
@type intg
  @define pre         @I@
  @define upr         @I@
  @define type_decl   @INTEGER           @
  @define utyp_decl   @INTEGER           @
  @define type_comm   @INTEGER@
  @define utyp_comm   @INTEGER@
@type real
  @define pre         @S@
  @define upr         @S@
  @define type_decl   @REAL              @
  @define utyp_decl   @REAL              @
  @define type_comm   @REAL@
  @define utyp_comm   @REAL@
@type dble
  @define pre         @D@
  @define upr         @D@
  @define type_decl   @DOUBLE PRECISION  @
  @define utyp_decl   @DOUBLE PRECISION  @
  @define type_comm   @DOUBLE PRECISION@
  @define utyp_comm   @DOUBLE PRECISION@
@type cplx chex
  @define pre         @C@
  @define upr         @S@
  @define type_decl   @COMPLEX           @
  @define utyp_decl   @REAL              @
  @define type_comm   @COMPLEX@
  @define utyp_comm   @REAL@
@type zplx zhex
  @define pre         @Z@
  @define upr         @D@
  @define type_decl   @COMPLEX*16        @
  @define utyp_decl   @DOUBLE PRECISION  @
  @define type_comm   @COMPLEX*16@
  @define utyp_comm   @DOUBLE PRECISION@
@type real dble
  @define uc          @@
@type cplx zplx
  @define uc          @U@
@type real dble cplx zplx
  @define syhe        @SY@
  @define sbhb        @SB@
  @define sphp        @SP@
  @define syhe_comm   @symmetric@
@type chex zhex
  @define uc          @C@
  @define syhe        @HE@
  @define sbhb        @HB@
  @define sphp        @HP@
  @define syhe_comm   @Hermitian@
@type real cplx chex
  @define exp         @E@
@type cplx chex
  @define real_intr   @REAL@
  @define imag_intr   @AIMAG@
  @define cnjg_intr   @CONJG@
@type dble zplx zhex
  @define exp         @D@
@type zplx zhex
  @define real_intr   @DBLE@
  @define imag_intr   @DIMAG@
  @define cnjg_intr   @DCONJG@
@type !
@skip
@skip
@lan C
  @define F77_int_decl        @F77_INTEGER               @
  @define F77_int_comm        @F77_INTEGER@
@type intg
  @define pre                 @i@
  @define upr                 @i@
  @define type                @int@
  @define utyp                @int@
  @define type_decl           @int                       @
  @define utyp_decl           @int                       @
  @define cst_type_decl       @const int                 @
  @define cst_utyp_decl       @const int                 @
  @define reg_type_decl       @register int              @
  @define reg_utyp_decl       @register int              @
  @define reg_cst_type_decl   @register const int        @
  @define reg_cst_utyp_decl   @register const int        @
  @define type_comm           @int@
  @define utyp_comm           @int@
  @define cst_type_comm       @const int@
  @define cst_utyp_comm       @const int@
@type real
  @define pre                 @s@
  @define upr                 @s@
  @define type                @float@
  @define utyp                @float@
  @define type_decl           @float                     @
  @define utyp_decl           @float                     @
  @define cst_type_decl       @const float               @
  @define cst_utyp_decl       @const float               @
  @define reg_type_decl       @register float            @
  @define reg_utyp_decl       @register float            @
  @define reg_cst_type_decl   @register const float      @
  @define reg_cst_utyp_decl   @register const float      @
  @define sta_cst_type_decl   @static const float        @
  @define sta_cst_utyp_decl   @static const float        @
  @define type_comm           @float@
  @define utyp_comm           @float@
  @define size_comm           @sizeof(   float   )@
  @define rsiz_comm           @sizeof(   float   )@
@type dble
  @define pre                 @d@
  @define upr                 @d@
  @define type                @double@
  @define utyp                @double@
  @define type_decl           @double                    @
  @define utyp_decl           @double                    @
  @define cst_type_decl       @const double              @
  @define cst_utyp_decl       @const double              @
  @define reg_type_decl       @register double           @
  @define reg_utyp_decl       @register double           @
  @define reg_cst_type_decl   @register const double     @
  @define reg_cst_utyp_decl   @register const double     @
  @define sta_cst_type_decl   @static const double       @
  @define sta_cst_utyp_decl   @static const double       @
  @define type_comm           @double@
  @define utyp_comm           @double@
  @define size_comm           @sizeof(   double  )@
  @define rsiz_comm           @sizeof(   double  )@
@type real dble
  @define sca_in_comm         @const @(type_comm)@
  @define sca_in_out_comm     @@(type_comm) *@
  @define sca_out_comm        @@(type_comm) *@
  @define rca_in_comm         @const @(utyp_comm)@
  @define rca_in_out_comm     @@(utyp_comm) *@
  @define rca_out_comm        @@(utyp_comm) *@
  @define arr_in_comm         @const @(type_comm) *@
  @define arr_in_out_comm     @@(type_comm) *@
  @define arr_out_comm        @@(type_comm) *@
@type cplx chex
  @define pre                 @c@
  @define upr                 @s@
  @define type                @float@
  @define utyp                @float@
  @define type_decl           @float                     @
  @define utyp_decl           @float                     @
  @define cst_type_decl       @const float               @
  @define cst_utyp_decl       @const float               @
  @define reg_type_decl       @register float            @
  @define reg_utyp_decl       @register float            @
  @define reg_cst_type_decl   @register const float      @
  @define reg_cst_utyp_decl   @register const float      @
  @define sta_cst_type_decl   @static const float        @
  @define sta_cst_utyp_decl   @static const float        @
  @define type_comm           @float@
  @define utyp_comm           @float@
  @define size_comm           @sizeof( float [2] )@
  @define rsiz_comm           @sizeof(   float   )@
@type zplx zhex
  @define pre                 @z@
  @define upr                 @d@
  @define type                @double@
  @define utyp                @double@
  @define type_decl           @double                    @
  @define utyp_decl           @double                    @
  @define cst_type_decl       @const double              @
  @define cst_utyp_decl       @const double              @
  @define reg_type_decl       @register double           @
  @define reg_utyp_decl       @register double           @
  @define reg_cst_type_decl   @register const double     @
  @define reg_cst_utyp_decl   @register const double     @
  @define sta_cst_type_decl   @static const double       @
  @define sta_cst_utyp_decl   @static const double       @
  @define type_comm           @double@
  @define utyp_comm           @double@
  @define size_comm           @sizeof( double[2] )@
  @define rsiz_comm           @sizeof(   double  )@
@type cplx zplx chex zhex
  @define sca_in_comm         @const @(utyp_comm) *@
  @define sca_in_out_comm     @@(utyp_comm) *@
  @define sca_out_comm        @@(utyp_comm) *@
  @define rca_in_comm         @const @(utyp_comm)@
  @define rca_in_out_comm     @@(utyp_comm) *@
  @define rca_out_comm        @@(utyp_comm) *@
  @define arr_in_comm         @const @(utyp_comm) *@
  @define arr_in_out_comm     @@(utyp_comm) *@
  @define arr_out_comm        @@(utyp_comm) *@
@type real dble
  @define uc                  @@
@type cplx zplx
  @define uc                  @u@
@type real dble cplx zplx
  @define syhe                @sy@
  @define sbhb                @sb@
  @define sphp                @sp@
  @define syhe_comm           @symmetric@
@type chex zhex
  @define uc                  @c@
  @define syhe                @he@
  @define sbhb                @hb@
  @define sphp                @hp@
  @define syhe_comm           @Hermitian@
@type !
@skip
@skip
@lan F C
@skip
@skip
