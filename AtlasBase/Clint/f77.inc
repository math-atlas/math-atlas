@WHAT ENUMS
      INTEGER ATLASROWMAJOR, ATLASCOLMAJOR
      PARAMETER (ATLASROWMAJOR=101, ATLASCOLMAJOR=102)
      INTEGER ATLASNOTRANS, ATLASTRANS, ATLASCONJTRANS
      PARAMETER (ATLASNOTRANS=111, ATLASTRANS=112, ATLASCONJTRANS=113)
      INTEGER ATLASUPPER, ATLASLOWER
      PARAMETER (ATLASUPPER=121, ATLASLOWER=122)
      INTEGER ATLASNONUNIT, ATLASUNIT
      PARAMETER (ATLASNONUNIT=131, ATLASUNIT=132)
      INTEGER ATLASLEFT, ATLASRIGHT
      PARAMETER (ATLASLEFT=141, ATLASRIGHT=142)
@WHAT ! type
@endextract
@WHAT TYPE
@TYPE SREAL
   @define type @REAL@
   @define utyp @REAL@
   @define pre @S@
   @define upr @S@
   @define one @1.0E+0@
   @define zero @0.0E+0@
   @define e    @E@
@TYPE DREAL
   @define type @DOUBLE PRECISION@
   @define utyp @DOUBLE PRECISION@
   @define pre @D@
   @define upr @D@
   @define one @1.0D+0@
   @define zero @0.0D+0@
   @define e    @D@
@TYPE SCPLX
   @define type @COMPLEX@
   @define utyp @REAL@
   @define pre @C@
   @define upr @S@
   @define one  @(1.0E+0, 0.0E+0)@
   @define zero @(0.0E+0, 0.0E+0)@
   @define e    @E@
@TYPE DCPLX
   @define type @COMPLEX*16@
   @define utyp @DOUBLE PRECISION@
   @define pre @Z@
   @define upr @D@
   @define one @(1.0D+0, 0.0D+0)@
   @define zero @(0.0D+0, 0.0D+0)@
   @define e    @D@
@TYPE SREAL DREAL
   @define comtyp @real@
   @define hesy @symmetric@
   @define HT @T@
@TYPE SCPLX DCPLX
   @define hesy @Hermitian@
   @define comtyp @complex@
   @define HT @H@
@TYPE !
