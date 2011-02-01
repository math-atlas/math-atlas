@skip
@skip      C      V                      V                             V12345678
@skip
@inc uplosy
@lan F
*  UPLO    (input)                       CHARACTER*1
*          On entry, UPLO  specifies whether the upper or lower triangu-
*          lar part of the array @(Z) is to be referenced as follows:
*
*              UPLO = 'U' or 'u'   Only the upper triangular part of @(Z)
*                                  is to be referenced.
*
*              UPLO = 'L' or 'l'   Only the lower triangular part of @(Z)
*                                  is to be referenced.
*
*          Unchanged on exit.
@lan C
 * UPLO    (input)                       const enum ATLAS_UPLO
 *         On entry, UPLO  specifies whether the upper or lower triangu-
 *         lar part of the array @(Z) is to be referenced as follows:
 *
 *             UPLO = AtlasUpper   Only the upper triangular part of @(Z)
 *                                 is to be referenced.
 *
 *             UPLO = AtlasLower   Only the lower triangular part of @(Z)
 *                                 is to be referenced.
 *
 *         Unchanged on exit.
@lan F C
@inc uplosb
@lan F
*  UPLO    (input)                       CHARACTER*1
*          On entry, UPLO  specifies whether the upper or lower triangu-
*          lar part of the band matrix @(Z) is being supplied as follows:
*
*              UPLO = 'U' or 'u'   The upper triangular part of @(Z) is
*                                  being supplied.
*
*              UPLO = 'L' or 'l'   The lower triangular part of @(Z) is
*                                  being supplied.
*
*          Unchanged on exit. 
@lan C
 * UPLO    (input)                       const enum ATLAS_UPLO
 *         On entry, UPLO  specifies whether the upper or lower triangu-
 *         lar part of the band matrix @(Z) is being supplied as follows:
 *
 *             UPLO = AtlasUpper   The upper triangular part of @(Z) is
 *                                 being supplied.
 *
 *             UPLO = AtlasLower   The lower triangular part of @(Z) is
 *                                 being supplied.
 *
 *         Unchanged on exit. 
@lan F C
@inc uplosp
@lan F
*  UPLO    (input)                       CHARACTER*1
*          On entry, UPLO  specifies whether the upper or lower triangu-
*          lar part of the matrix @(Z) is supplied in the packed array A
*          as follows:
*
*              UPLO = 'U' or 'u'   The upper triangular part of @(Z) is
*                                  supplied in @(Z).
*
*              UPLO = 'L' or 'l'   The lower triangular part of @(Z) is
*                                  supplied in @(Z).
*
*          Unchanged on exit.
@lan C
 * UPLO    (input)                       const enum ATLAS_UPLO
 *         On entry, UPLO  specifies whether the upper or lower triangu-
 *         lar part of the matrix @(Z) is supplied in the packed array A
 *         as follows:
 *
 *             UPLO = AtlasUpper   The upper triangular part of @(Z) is
 *                                 supplied in @(Z).
 *
 *             UPLO = AtlasLower   The lower triangular part of @(Z) is
 *                                 supplied in @(Z).
 *
 *         Unchanged on exit.
@lan F C
@inc uplotr
@lan F
*  UPLO    (input)                       CHARACTER*1
*          On entry, UPLO  specifies whether  the  matrix is an upper or
*          lower triangular matrix as follows:
*
*              UPLO = 'U' or 'u'   @(Z) is an upper triangular matrix.
*
*              UPLO = 'L' or 'l'   @(Z) is a lower triangular matrix.
*
*          Unchanged on exit.
@lan C
 * UPLO    (input)                       const enum ATLAS_UPLO
 *         On entry, UPLO  specifies whether  the  matrix is an upper or
 *         lower triangular matrix as follows:
 *
 *             UPLO = AtlasUpper   @(Z) is an upper triangular matrix.
 *
 *             UPLO = AtlasLower   @(Z) is a lower triangular matrix.
 *
 *         Unchanged on exit.
@lan F C
@inc trns
@lan F
*  TRANS   (input)                       CHARACTER*1
*          On entry,  TRANS  specifies the  operation to be performed as
*          follows:
@lan C
@inc trns
 * TRANS   (input)                       const enum ATLAS_TRANS
 *         On entry,  TRANS  specifies the  operation to be performed as
 *         follows:
@lan F C
@inc trnsx
@lan F
*  TRANS@(Z)  (input)                       CHARACTER*1
*          On entry, TRANS@(Z)  specifies the form of op( A ) to be used in
*          the matrix multiplication as follows:
*
*             TRANS@(Z) = 'N' or 'n'  op( @(Z) ) = @(Z),
*
*             TRANS@(Z) = 'T' or 't'  op( @(Z) ) = @(Z)',
*
@type real dble
*             TRANS@(Z) = 'C' or 'c'  op( @(Z) ) = @(Z)'.
@type cplx zplx
*             TRANS@(Z) = 'C' or 'c'  op( @(Z) ) = conjg( @(Z)' ).
@type real dble cplx zplx
*
*          Unchanged on exit.
@lan C
 * TRANS@(Z)  (input)                       const enum ATLAS_TRANS
 *         On entry, TRANS@(Z)  specifies the form of op( A ) to be used in
 *         the matrix multiplication as follows:
 *
 *            TRANS@(Z) = AtlasNoTrans    op( @(Z) ) = @(Z),
 *
 *            TRANS@(Z) = AtlasTrans      op( @(Z) ) = @(Z)',
 *
@type real dble
 *            TRANS@(Z) = AtlasConjTrans  op( @(Z) ) = @(Z)'.
@type cplx zplx
 *            TRANS@(Z) = AtlasConjTrans  op( @(Z) ) = conjg( @(Z)' ).
@type real dble cplx zplx
 *
 *         Unchanged on exit.
@lan F C
@inc unit
@lan F
*  DIAG    (input)                       CHARACTER*1
*          On entry, DIAG specifies whether or not @(Z) is unit triangu-
*          lar as follows:
*
*             DIAG = 'U' or 'u'   @(Z) is assumed to be unit triangular.
*
*             DIAG = 'N' or 'n'   @(Z) is not assumed to be unit
*                                 triangular.
*
*          Unchanged on exit.
@lan C
 * DIAG    (input)                       const enum ATLAS_DIAG
 *         On entry, DIAG specifies whether or not @(Z) is unit triangu-
 *         lar as follows:
 *
 *            DIAG = AtlasUnit       @(Z) is assumed to be unit triangular,
 *
 *            DIAG = AtlasNonUnit    @(Z) is not assumed to be unit trian-
 *                                   gular.
 *
 *         Unchanged on exit.
@lan F C
@inc !
@skip      C      V                      D                             V12345678
