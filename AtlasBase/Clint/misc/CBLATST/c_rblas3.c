#include "atlas_f77.h"
#include "cblas_test.h"
#include "cblas.h"

int cblas_info; 
int cblas_lerr=PASSED, cblas_ok=UNDEFINED;
char *cblas_rout;

void get_transpose_type(char *type, enum CBLAS_TRANSPOSE *trans) {
  if( (strncmp( type,"n",1 )==0)||(strncmp( type,"N",1 )==0) )
        *trans = CblasNoTrans;
  else if( (strncmp( type,"t",1 )==0)||(strncmp( type,"T",1 )==0) )
        *trans = CblasTrans;
  else if( (strncmp( type,"c",1 )==0)||(strncmp( type,"C",1 )==0) )
        *trans = CblasConjTrans;
  else *trans = UNDEFINED;
}


void get_uplo_type(char *type, enum CBLAS_UPLO *uplo) {
  if( (strncmp( type,"u",1 )==0)||(strncmp( type,"U",1 )==0) )
        *uplo = CblasUpper;
  else if( (strncmp( type,"l",1 )==0)||(strncmp( type,"L",1 )==0) )
        *uplo = CblasLower;
  else *uplo = UNDEFINED;
}
void get_diag_type(char *type, enum CBLAS_DIAG *diag) {
  if( (strncmp( type,"u",1 )==0)||(strncmp( type,"U",1 )==0) )
        *diag = CblasUnit;
  else if( (strncmp( type,"n",1 )==0)||(strncmp( type,"N",1 )==0) )
        *diag = CblasNonUnit;
  else *diag = UNDEFINED;
}
void get_side_type(char *type, enum CBLAS_SIDE *side) {
  if( (strncmp( type,"l",1 )==0)||(strncmp( type,"L",1 )==0) )
        *side = CblasLeft;
  else if( (strncmp( type,"r",1 )==0)||(strncmp( type,"R",1 )==0) )
        *side = CblasRight;
  else *side = UNDEFINED;
}

void chkxer(void) {
   extern int cblas_ok, cblas_lerr, cblas_info;
   extern char *cblas_rout;
   if (cblas_lerr == PASSED) {
      printf("***** ILLEGAL VALUE OF PARAMETER NUMBER %d NOT DETECTED BY %s *****\n", cblas_info, cblas_rout);
      cblas_ok = FALSE;
   }
   cblas_lerr = PASSED;
}

void F77_gemm( F77_INTEGER *order, F77_CHAR transpa, F77_CHAR transpb, 
               F77_INTEGER *m, F77_INTEGER *n, F77_INTEGER *k, 
               CBLAS_REAL *alpha, CBLAS_REAL *a, F77_INTEGER *lda, 
               CBLAS_REAL *b, F77_INTEGER *ldb, CBLAS_REAL *beta, 
               CBLAS_REAL *c, F77_INTEGER *ldc ) 
{

  CBLAS_REAL *A, *B, *C;
  int i,j,LDA, LDB, LDC;
  enum CBLAS_TRANSPOSE transa, transb;
   #ifdef ATL_FunkyInts
      int F77_m = *m, F77_n = *n, F77_k = *k, F77_lda = *lda, F77_ldb = *ldb;
      int F77_ldc = *ldc, F77_order=*order;
   #else
      #define F77_order  *order
      #define F77_m    *m
      #define F77_n    *n
      #define F77_k    *k
      #define F77_lda  *lda
      #define F77_ldb  *ldb
      #define F77_ldc  *ldc
   #endif

  get_transpose_type(ATL_STR_PTR(transpa), &transa);
  get_transpose_type(ATL_STR_PTR(transpb), &transb);

  if (F77_order == TEST_ROW_MJR) {
     if (transa == CblasNoTrans) {
        LDA = F77_k+1;
        A = (CBLAS_REAL *)malloc( (F77_m)*LDA*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_m; i++ )
           for( j=0; j<F77_k; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     else {
        LDA = F77_m+1;
        A   = ( CBLAS_REAL* )malloc( LDA*(F77_k)*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_k; i++ )
           for( j=0; j<F77_m; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     if (transb == CblasNoTrans) {
        LDB = F77_n+1;
        B   = ( CBLAS_REAL* )malloc( (F77_k)*LDB*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_k; i++ )
           for( j=0; j<F77_n; j++ )
              B[i*LDB+j]=b[j*(F77_ldb)+i];
     }
     else {
        LDB = F77_k+1;
        B   = ( CBLAS_REAL* )malloc( LDB*(F77_n)*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_n; i++ )
           for( j=0; j<F77_k; j++ )
              B[i*LDB+j]=b[j*(F77_ldb)+i];
     }
     LDC = F77_n+1;
     C   = ( CBLAS_REAL* )malloc( (F77_m)*LDC*sizeof( CBLAS_REAL ) );
     for( j=0; j<F77_n; j++ )
        for( i=0; i<F77_m; i++ )
           C[i*LDC+j]=c[j*(F77_ldc)+i];

     cblas_gemm( CblasRowMajor, transa, transb, F77_m, F77_n, F77_k, *alpha, A, LDA,
                  B, LDB, *beta, C, LDC );
     for( j=0; j<F77_n; j++ )
        for( i=0; i<F77_m; i++ )
           c[j*(F77_ldc)+i]=C[i*LDC+j];
     free(A);
     free(B);
     free(C);
  }
  else if (F77_order == TEST_COL_MJR)
     cblas_gemm( CblasColMajor, transa, transb, F77_m, F77_n, F77_k, *alpha, a, F77_lda,
                  b, F77_ldb, *beta, c, F77_ldc );
  else
     cblas_gemm( UNDEFINED, transa, transb, F77_m, F77_n, F77_k, *alpha, a, F77_lda,
                  b, F77_ldb, *beta, c, F77_ldc );
}
void F77_symm( F77_INTEGER *order, F77_CHAR rtlf, F77_CHAR uplow, 
               F77_INTEGER *m, F77_INTEGER *n, CBLAS_REAL *alpha, 
               CBLAS_REAL *a, F77_INTEGER *lda, CBLAS_REAL *b, 
               F77_INTEGER *ldb, CBLAS_REAL *beta, CBLAS_REAL *c, 
               F77_INTEGER *ldc ) 
{

  CBLAS_REAL *A, *B, *C;
  int i,j,LDA, LDB, LDC;
  enum CBLAS_UPLO uplo;
  enum CBLAS_SIDE side;
   #ifdef ATL_FunkyInts
      int F77_m = *m, F77_n = *n, F77_lda = *lda, F77_ldb = *ldb;
      int F77_ldc = *ldc, F77_order=*order;
   #else
      #define F77_order  *order
      #define F77_m    *m
      #define F77_n    *n
      #define F77_lda  *lda
      #define F77_ldb  *ldb
      #define F77_ldc  *ldc
   #endif

  get_uplo_type(ATL_STR_PTR(uplow),&uplo);
  get_side_type(ATL_STR_PTR(rtlf),&side);

  if (F77_order == TEST_ROW_MJR) {
     if (side == CblasLeft) {
        LDA = F77_m+1;
        A   = ( CBLAS_REAL* )malloc( (F77_m)*LDA*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_m; i++ )
           for( j=0; j<F77_m; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     else{
        LDA = F77_n+1;
        A   = ( CBLAS_REAL* )malloc( (F77_n)*LDA*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_n; i++ )
           for( j=0; j<F77_n; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     LDB = F77_n+1;
     B   = ( CBLAS_REAL* )malloc( (F77_m)*LDB*sizeof( CBLAS_REAL ) );
     for( i=0; i<F77_m; i++ )
        for( j=0; j<F77_n; j++ )
           B[i*LDB+j]=b[j*(F77_ldb)+i];
     LDC = F77_n+1;
     C   = ( CBLAS_REAL* )malloc( (F77_m)*LDC*sizeof( CBLAS_REAL ) );
     for( j=0; j<F77_n; j++ )
        for( i=0; i<F77_m; i++ )
           C[i*LDC+j]=c[j*(F77_ldc)+i];
     cblas_symm( CblasRowMajor, side, uplo, F77_m, F77_n, *alpha, A, LDA, B, LDB, 
                  *beta, C, LDC );
     for( j=0; j<F77_n; j++ )
        for( i=0; i<F77_m; i++ )
           c[j*(F77_ldc)+i]=C[i*LDC+j];
     free(A);
     free(B);
     free(C);
  }
  else if (F77_order == TEST_COL_MJR)
     cblas_symm( CblasColMajor, side, uplo, F77_m, F77_n, *alpha, a, F77_lda, b, F77_ldb, 
                  *beta, c, F77_ldc );
  else
     cblas_symm( UNDEFINED, side, uplo, F77_m, F77_n, *alpha, a, F77_lda, b, F77_ldb, 
                  *beta, c, F77_ldc );
}

void F77_syrk( F77_INTEGER *order, F77_CHAR uplow, F77_CHAR transp, 
               F77_INTEGER *n, F77_INTEGER *k, CBLAS_REAL *alpha, 
               CBLAS_REAL *a, F77_INTEGER *lda, CBLAS_REAL *beta, 
               CBLAS_REAL *c, F77_INTEGER *ldc ) 
{
  int i,j,LDA,LDC;
  CBLAS_REAL *A, *C;
  enum CBLAS_UPLO uplo;
  enum CBLAS_TRANSPOSE trans;
   #ifdef ATL_FunkyInts
      int F77_n = *n, F77_k = *k, F77_lda = *lda, F77_ldb = *ldb;
      int F77_ldc = *ldc, F77_order=*order;
   #else
      #define F77_order  *order
      #define F77_n    *n
      #define F77_k    *k
      #define F77_lda  *lda
      #define F77_ldc  *ldc
   #endif

  get_uplo_type(ATL_STR_PTR(uplow),&uplo);
  get_transpose_type(ATL_STR_PTR(transp),&trans);

  if (F77_order == TEST_ROW_MJR) {
     if (trans == CblasNoTrans) {
        LDA = F77_k+1;
        A   = ( CBLAS_REAL* )malloc( (F77_n)*LDA*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_n; i++ )
           for( j=0; j<F77_k; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     else{
        LDA = F77_n+1;
        A   = ( CBLAS_REAL* )malloc( (F77_k)*LDA*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_k; i++ )
           for( j=0; j<F77_n; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     LDC = F77_n+1;
     C   = ( CBLAS_REAL* )malloc( (F77_n)*LDC*sizeof( CBLAS_REAL ) );
     for( i=0; i<F77_n; i++ )
        for( j=0; j<F77_n; j++ )
           C[i*LDC+j]=c[j*(F77_ldc)+i];
     cblas_syrk(CblasRowMajor, uplo, trans, F77_n, F77_k, *alpha, A, LDA, *beta, 
	         C, LDC );
     for( j=0; j<F77_n; j++ )
        for( i=0; i<F77_n; i++ )
           c[j*(F77_ldc)+i]=C[i*LDC+j];
     free(A);
     free(C);
  }
  else if (F77_order == TEST_COL_MJR)
     cblas_syrk(CblasColMajor, uplo, trans, F77_n, F77_k, *alpha, a, F77_lda, *beta, 
	         c, F77_ldc );
  else
     cblas_syrk(UNDEFINED, uplo, trans, F77_n, F77_k, *alpha, a, F77_lda, *beta, 
	         c, F77_ldc );
}

void F77_syr2k( F77_INTEGER *order, F77_CHAR uplow, F77_CHAR transp, 
                F77_INTEGER *n, F77_INTEGER *k, CBLAS_REAL *alpha, 
                CBLAS_REAL *a, F77_INTEGER *lda, CBLAS_REAL *b, 
                F77_INTEGER *ldb, CBLAS_REAL *beta, CBLAS_REAL *c, 
                F77_INTEGER *ldc ) 
{
  int i,j,LDA,LDB,LDC;
  CBLAS_REAL *A, *B, *C;
  enum CBLAS_UPLO uplo;
  enum CBLAS_TRANSPOSE trans;
   #ifdef ATL_FunkyInts
      int F77_n = *n, F77_k = *k, F77_lda = *lda, F77_ldb = *ldb;
      int F77_ldc = *ldc, F77_order=*order;
   #else
      #define F77_order  *order
      #define F77_n    *n
      #define F77_k    *k
      #define F77_lda  *lda
      #define F77_ldb  *ldb
      #define F77_ldc  *ldc
   #endif
  get_uplo_type(ATL_STR_PTR(uplow),&uplo);
  get_transpose_type(ATL_STR_PTR(transp),&trans);

  if (F77_order == TEST_ROW_MJR) {
     if (trans == CblasNoTrans) {
        LDA = F77_k+1;
        LDB = F77_k+1;
        A   = ( CBLAS_REAL* )malloc( (F77_n)*LDA*sizeof( CBLAS_REAL ) );
        B   = ( CBLAS_REAL* )malloc( (F77_n)*LDB*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_n; i++ )
           for( j=0; j<F77_k; j++ ) {
              A[i*LDA+j]=a[j*(F77_lda)+i];
              B[i*LDB+j]=b[j*(F77_ldb)+i];
           }
     }
     else {
        LDA = F77_n+1;
        LDB = F77_n+1;
        A   = ( CBLAS_REAL* )malloc( LDA*(F77_k)*sizeof( CBLAS_REAL ) );
        B   = ( CBLAS_REAL* )malloc( LDB*(F77_k)*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_k; i++ )
           for( j=0; j<F77_n; j++ ){
              A[i*LDA+j]=a[j*(F77_lda)+i];
              B[i*LDB+j]=b[j*(F77_ldb)+i];
           }
     }
     LDC = F77_n+1;
     C   = ( CBLAS_REAL* )malloc( (F77_n)*LDC*sizeof( CBLAS_REAL ) );
     for( i=0; i<F77_n; i++ )
        for( j=0; j<F77_n; j++ )
           C[i*LDC+j]=c[j*(F77_ldc)+i];
     cblas_syr2k(CblasRowMajor, uplo, trans, F77_n, F77_k, *alpha, A, LDA, 
		  B, LDB, *beta, C, LDC );
     for( j=0; j<F77_n; j++ )
        for( i=0; i<F77_n; i++ )
           c[j*(F77_ldc)+i]=C[i*LDC+j];
     free(A);
     free(B);
     free(C);
  }
  else if (F77_order == TEST_COL_MJR)
     cblas_syr2k(CblasColMajor, uplo, trans, F77_n, F77_k, *alpha, a, F77_lda, 
		   b, F77_ldb, *beta, c, F77_ldc );
  else
     cblas_syr2k(UNDEFINED, uplo, trans, F77_n, F77_k, *alpha, a, F77_lda, 
		   b, F77_ldb, *beta, c, F77_ldc );
}
void F77_trmm( F77_INTEGER *order, F77_CHAR rtlf, F77_CHAR uplow, 
               F77_CHAR transp, F77_CHAR diagn, F77_INTEGER *m, 
               F77_INTEGER *n, CBLAS_REAL *alpha, CBLAS_REAL *a, 
               F77_INTEGER *lda, CBLAS_REAL *b, F77_INTEGER *ldb) 
{
  int i,j,LDA,LDB;
  CBLAS_REAL *A, *B;
  enum CBLAS_SIDE side;
  enum CBLAS_DIAG diag;
  enum CBLAS_UPLO uplo;
  enum CBLAS_TRANSPOSE trans;
   #ifdef ATL_FunkyInts
      int F77_m = *m, F77_n = *n, F77_lda = *lda, F77_ldb = *ldb;
      int F77_order=*order;
   #else
      #define F77_order  *order
      #define F77_m    *m
      #define F77_n    *n
      #define F77_lda  *lda
      #define F77_ldb  *ldb
   #endif

  get_uplo_type(ATL_STR_PTR(uplow),&uplo);
  get_transpose_type(ATL_STR_PTR(transp),&trans);
  get_diag_type(ATL_STR_PTR(diagn),&diag);
  get_side_type(ATL_STR_PTR(rtlf),&side);

  if (F77_order == TEST_ROW_MJR) {
     if (side == CblasLeft) {
        LDA = F77_m+1;
        A   = ( CBLAS_REAL* )malloc( (F77_m)*LDA*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_m; i++ )
           for( j=0; j<F77_m; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     else{
        LDA = F77_n+1;
        A   = ( CBLAS_REAL* )malloc( (F77_n)*LDA*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_n; i++ )
           for( j=0; j<F77_n; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     LDB = F77_n+1;
     B   = ( CBLAS_REAL* )malloc( (F77_m)*LDB*sizeof( CBLAS_REAL ) );
     for( i=0; i<F77_m; i++ )
        for( j=0; j<F77_n; j++ )
           B[i*LDB+j]=b[j*(F77_ldb)+i];
     cblas_trmm(CblasRowMajor, side, uplo, trans, diag, F77_m, F77_n, *alpha, 
		 A, LDA, B, LDB );
     for( j=0; j<F77_n; j++ )
        for( i=0; i<F77_m; i++ )
           b[j*(F77_ldb)+i]=B[i*LDB+j];
     free(A);
     free(B);
  }
  else if (F77_order == TEST_COL_MJR)
     cblas_trmm(CblasColMajor, side, uplo, trans, diag, F77_m, F77_n, *alpha, 
		   a, F77_lda, b, F77_ldb);
  else
     cblas_trmm(UNDEFINED, side, uplo, trans, diag, F77_m, F77_n, *alpha, 
		   a, F77_lda, b, F77_ldb);
}

void F77_trsm( F77_INTEGER *order, F77_CHAR rtlf, F77_CHAR uplow, 
               F77_CHAR transp, F77_CHAR diagn, F77_INTEGER *m, 
               F77_INTEGER *n, CBLAS_REAL *alpha, CBLAS_REAL *a, 
               F77_INTEGER *lda, CBLAS_REAL *b, F77_INTEGER *ldb) 
{
  int i,j,LDA,LDB;
  CBLAS_REAL *A, *B;
  enum CBLAS_SIDE side;
  enum CBLAS_DIAG diag;
  enum CBLAS_UPLO uplo;
  enum CBLAS_TRANSPOSE trans;
   #ifdef ATL_FunkyInts
      int F77_m = *m, F77_n = *n, F77_lda = *lda, F77_ldb = *ldb;
      int F77_order=*order;
   #else
      #define F77_order  *order
      #define F77_m    *m
      #define F77_n    *n
      #define F77_lda  *lda
      #define F77_ldb  *ldb
   #endif

  get_uplo_type(ATL_STR_PTR(uplow),&uplo);
  get_transpose_type(ATL_STR_PTR(transp),&trans);
  get_diag_type(ATL_STR_PTR(diagn),&diag);
  get_side_type(ATL_STR_PTR(rtlf),&side);

  if (F77_order == TEST_ROW_MJR) {
     if (side == CblasLeft) {
        LDA = F77_m+1;
        A   = ( CBLAS_REAL* )malloc( (F77_m)*LDA*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_m; i++ )
           for( j=0; j<F77_m; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     else{
        LDA = F77_n+1;
        A   = ( CBLAS_REAL* )malloc( (F77_n)*LDA*sizeof( CBLAS_REAL ) );
        for( i=0; i<F77_n; i++ )
           for( j=0; j<F77_n; j++ )
              A[i*LDA+j]=a[j*(F77_lda)+i];
     }
     LDB = F77_n+1;
     B   = ( CBLAS_REAL* )malloc( (F77_m)*LDB*sizeof( CBLAS_REAL ) );
     for( i=0; i<F77_m; i++ )
        for( j=0; j<F77_n; j++ )
           B[i*LDB+j]=b[j*(F77_ldb)+i];
     cblas_trsm(CblasRowMajor, side, uplo, trans, diag, F77_m, F77_n, *alpha, 
		 A, LDA, B, LDB );
     for( j=0; j<F77_n; j++ )
        for( i=0; i<F77_m; i++ )
           b[j*(F77_ldb)+i]=B[i*LDB+j];
     free(A);
     free(B);
  }
  else if (F77_order == TEST_COL_MJR)
     cblas_trsm(CblasColMajor, side, uplo, trans, diag, F77_m, F77_n, *alpha, 
		   a, F77_lda, b, F77_ldb);
  else
     cblas_trsm(UNDEFINED, side, uplo, trans, diag, F77_m, F77_n, *alpha, 
		   a, F77_lda, b, F77_ldb);
}

void F77_3chke(F77_CHAR rout) {
   char *sf = ATL_STR_PTR(rout);
   CBLAS_REAL A[2] = {0.0,0.0}, 
              B[2] = {0.0,0.0}, 
              C[2] = {0.0,0.0}, 
              ALPHA=0.0, BETA=0.0;
   extern int cblas_info, cblas_lerr, cblas_ok;
   extern char *cblas_rout;

 
   cblas_ok = TRUE;
   cblas_lerr = PASSED;

   if (strncmp( sf,CBLAS_GEMM,11)==0) {
      cblas_rout = CBLAS_GEMM;
cblas_info = 1;
cblas_gemm( INVALID,  CblasNoTrans, CblasNoTrans, 0, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 1;
cblas_gemm( INVALID,  CblasNoTrans, CblasTrans, 0, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 1;
cblas_gemm( INVALID,  CblasTrans, CblasNoTrans, 0, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 1;
cblas_gemm( INVALID,  CblasTrans, CblasTrans, 0, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 2;
cblas_gemm( CblasColMajor,  INVALID, CblasNoTrans, 0, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 2;
cblas_gemm( CblasColMajor,  INVALID, CblasTrans, 0, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 3;
cblas_gemm( CblasColMajor,  CblasNoTrans, INVALID, 0, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 3;
cblas_gemm( CblasColMajor,  CblasTrans, INVALID, 0, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasNoTrans, INVALID, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasTrans, INVALID, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_gemm( CblasColMajor,  CblasTrans, CblasNoTrans, INVALID, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_gemm( CblasColMajor,  CblasTrans, CblasTrans, INVALID, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasNoTrans, 0, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasTrans, 0, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_gemm( CblasColMajor,  CblasTrans, CblasNoTrans, 0, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_gemm( CblasColMajor,  CblasTrans, CblasTrans, 0, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 6;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasNoTrans, 0, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 6;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasTrans, 0, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 6;
cblas_gemm( CblasColMajor,  CblasTrans, CblasNoTrans, 0, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 6;
cblas_gemm( CblasColMajor,  CblasTrans, CblasTrans, 0, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 9;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasNoTrans, 2, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 9;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasTrans, 2, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 9;
cblas_gemm( CblasColMajor,  CblasTrans, CblasNoTrans, 0, 0, 2, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 9;
cblas_gemm( CblasColMajor,  CblasTrans, CblasTrans, 0, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 9;
cblas_gemm( CblasRowMajor,  CblasNoTrans, CblasNoTrans, 0, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 9;
cblas_gemm( CblasRowMajor,  CblasNoTrans, CblasTrans, 0, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 9;
cblas_gemm( CblasRowMajor,  CblasTrans, CblasNoTrans, 2, 0, 0, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 9;
cblas_gemm( CblasRowMajor,  CblasTrans, CblasTrans, 2, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasNoTrans, 0, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_gemm( CblasColMajor,  CblasTrans, CblasNoTrans, 0, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasTrans, 0, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_gemm( CblasColMajor,  CblasTrans, CblasTrans, 0, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_gemm( CblasRowMajor,  CblasNoTrans, CblasNoTrans, 0, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_gemm( CblasRowMajor,  CblasTrans, CblasNoTrans, 0, 2, 0, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_gemm( CblasRowMajor,  CblasNoTrans, CblasTrans, 0, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_gemm( CblasRowMajor,  CblasTrans, CblasTrans, 0, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 14;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasNoTrans, 2, 0, 0, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 14;
cblas_gemm( CblasColMajor,  CblasNoTrans, CblasTrans, 2, 0, 0, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 14;
cblas_gemm( CblasColMajor,  CblasTrans, CblasNoTrans, 2, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 14;
cblas_gemm( CblasColMajor,  CblasTrans, CblasTrans, 2, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 14;
cblas_gemm( CblasRowMajor,  CblasNoTrans, CblasNoTrans, 0, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 14;
cblas_gemm( CblasRowMajor,  CblasNoTrans, CblasTrans, 0, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 14;
cblas_gemm( CblasRowMajor,  CblasTrans, CblasNoTrans, 0, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 14;
cblas_gemm( CblasRowMajor,  CblasTrans, CblasTrans, 0, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();

   } else if (strncmp( sf,CBLAS_SYMM,11)==0) {
     cblas_rout = CBLAS_SYMM;

cblas_info = 1;
cblas_symm( INVALID,  CblasRight, CblasLower, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 2;
cblas_symm( CblasColMajor,  INVALID, CblasUpper, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 3;
cblas_symm( CblasColMajor,  CblasLeft, INVALID, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_symm( CblasColMajor,  CblasLeft, CblasUpper, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_symm( CblasColMajor,  CblasRight, CblasUpper, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_symm( CblasColMajor,  CblasLeft, CblasLower, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_symm( CblasColMajor,  CblasRight, CblasLower, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_symm( CblasColMajor,  CblasLeft, CblasUpper, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_symm( CblasColMajor,  CblasRight, CblasUpper, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_symm( CblasColMajor,  CblasLeft, CblasLower, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_symm( CblasColMajor,  CblasRight, CblasLower, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_symm( CblasRowMajor,  CblasLeft, CblasUpper, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_symm( CblasRowMajor,  CblasRight, CblasUpper, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_symm( CblasRowMajor,  CblasLeft, CblasLower, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_symm( CblasRowMajor,  CblasRight, CblasLower, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_symm( CblasColMajor,  CblasLeft, CblasUpper, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_symm( CblasColMajor,  CblasRight, CblasUpper, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_symm( CblasColMajor,  CblasLeft, CblasLower, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_symm( CblasColMajor,  CblasRight, CblasLower, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_symm( CblasRowMajor,  CblasLeft, CblasUpper, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_symm( CblasRowMajor,  CblasRight, CblasUpper, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_symm( CblasRowMajor,  CblasLeft, CblasLower, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_symm( CblasRowMajor,  CblasRight, CblasLower, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_symm( CblasColMajor,  CblasLeft, CblasUpper, 2, 0, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_symm( CblasColMajor,  CblasRight, CblasUpper, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_symm( CblasColMajor,  CblasLeft, CblasLower, 2, 0, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_symm( CblasColMajor,  CblasRight, CblasLower, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_symm( CblasRowMajor,  CblasLeft, CblasUpper, 0, 2, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_symm( CblasRowMajor,  CblasRight, CblasUpper, 0, 2, ALPHA, A, 2, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_symm( CblasRowMajor,  CblasLeft, CblasLower, 0, 2, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_symm( CblasRowMajor,  CblasRight, CblasLower, 0, 2, ALPHA, A, 2, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_symm( CblasColMajor,  CblasLeft, CblasUpper, 2, 0, ALPHA, A, 2, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_symm( CblasColMajor,  CblasRight, CblasUpper, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_symm( CblasColMajor,  CblasLeft, CblasLower, 2, 0, ALPHA, A, 2, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_symm( CblasColMajor,  CblasRight, CblasLower, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();

   } else if (strncmp( sf,CBLAS_TRMM,11)==0) {
      cblas_rout = CBLAS_TRMM;

cblas_info = 1;
cblas_trmm( INVALID,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 2;
cblas_trmm( CblasColMajor,  INVALID, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 3;
cblas_trmm( CblasColMajor,  CblasLeft, INVALID, CblasNoTrans, CblasNonUnit, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 4;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, INVALID, CblasNonUnit, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 5;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, INVALID, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trmm( CblasColMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trmm( CblasColMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trmm( CblasColMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trmm( CblasColMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trmm( CblasColMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trmm( CblasColMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trmm( CblasColMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trmm( CblasColMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trmm( CblasColMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trmm( CblasColMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trmm( CblasColMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trmm( CblasColMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasRowMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasRowMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasRowMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasRowMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasRowMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasRowMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasRowMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasRowMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasColMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasColMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasColMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasColMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasColMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trmm( CblasColMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasRowMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasRowMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasRowMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasRowMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasRowMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasRowMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasRowMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasRowMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasColMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasColMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasColMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasColMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasColMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasColMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trmm( CblasColMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();

   } else if (strncmp( sf,CBLAS_TRSM,11)==0) {
      cblas_rout = CBLAS_TRSM;

cblas_info = 1;
cblas_trsm( INVALID,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 2;
cblas_trsm( CblasColMajor,  INVALID, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 3;
cblas_trsm( CblasColMajor,  CblasLeft, INVALID, CblasNoTrans, CblasNonUnit, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 4;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, INVALID, CblasNonUnit, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 5;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, INVALID, 0, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trsm( CblasColMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trsm( CblasColMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trsm( CblasColMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trsm( CblasColMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trsm( CblasColMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 6;
cblas_trsm( CblasColMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, INVALID, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trsm( CblasColMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trsm( CblasColMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trsm( CblasColMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trsm( CblasColMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trsm( CblasColMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 7;
cblas_trsm( CblasColMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 0, INVALID, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasRowMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasRowMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasRowMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasRowMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasRowMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasRowMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasRowMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasRowMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasColMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasColMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasColMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasColMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 2 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasColMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 10;
cblas_trsm( CblasColMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasRowMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasRowMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasRowMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasRowMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasRowMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasRowMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasRowMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 0, 2, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasRowMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 0, 2, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasColMajor,  CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasColMajor,  CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasColMajor,  CblasRight, CblasUpper, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasColMajor,  CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasColMajor,  CblasLeft, CblasLower, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 2, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasColMajor,  CblasRight, CblasLower, CblasNoTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();
cblas_info = 12;
cblas_trsm( CblasColMajor,  CblasRight, CblasLower, CblasTrans, CblasNonUnit, 2, 0, ALPHA, A, 1, B, 1 );
chkxer();

   } else if (strncmp( sf,CBLAS_SYRK,11)==0) {
      cblas_rout = CBLAS_SYRK;

cblas_info = 1;
cblas_syrk( INVALID,  CblasUpper, CblasNoTrans, 0, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 2;
cblas_syrk( CblasColMajor,  INVALID, CblasNoTrans, 0, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 3;
cblas_syrk( CblasColMajor,  CblasUpper, INVALID, 0, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_syrk( CblasColMajor,  CblasUpper, CblasNoTrans, INVALID, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_syrk( CblasColMajor,  CblasUpper, CblasTrans, INVALID, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_syrk( CblasColMajor,  CblasLower, CblasNoTrans, INVALID, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_syrk( CblasColMajor,  CblasLower, CblasTrans, INVALID, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_syrk( CblasColMajor,  CblasUpper, CblasNoTrans, 0, INVALID, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_syrk( CblasColMajor,  CblasUpper, CblasTrans, 0, INVALID, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_syrk( CblasColMajor,  CblasLower, CblasNoTrans, 0, INVALID, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_syrk( CblasColMajor,  CblasLower, CblasTrans, 0, INVALID, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_syrk( CblasRowMajor,  CblasUpper, CblasNoTrans, 0, 2, ALPHA, A, 1, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_syrk( CblasRowMajor,  CblasUpper, CblasTrans, 2, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_syrk( CblasRowMajor,  CblasLower, CblasNoTrans, 0, 2, ALPHA, A, 1, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_syrk( CblasRowMajor,  CblasLower, CblasTrans, 2, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_syrk( CblasColMajor,  CblasUpper, CblasNoTrans, 2, 0, ALPHA, A, 1, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_syrk( CblasColMajor,  CblasUpper, CblasTrans, 0, 2, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_syrk( CblasColMajor,  CblasLower, CblasNoTrans, 2, 0, ALPHA, A, 1, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_syrk( CblasColMajor,  CblasLower, CblasTrans, 0, 2, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_syrk( CblasRowMajor,  CblasUpper, CblasNoTrans, 2, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_syrk( CblasRowMajor,  CblasUpper, CblasTrans, 2, 0, ALPHA, A, 2, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_syrk( CblasRowMajor,  CblasLower, CblasNoTrans, 2, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_syrk( CblasRowMajor,  CblasLower, CblasTrans, 2, 0, ALPHA, A, 2, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_syrk( CblasColMajor,  CblasUpper, CblasNoTrans, 2, 0, ALPHA, A, 2, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_syrk( CblasColMajor,  CblasUpper, CblasTrans, 2, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_syrk( CblasColMajor,  CblasLower, CblasNoTrans, 2, 0, ALPHA, A, 2, BETA, C, 1 );
chkxer();
cblas_info = 11;
cblas_syrk( CblasColMajor,  CblasLower, CblasTrans, 2, 0, ALPHA, A, 1, BETA, C, 1 );
chkxer();

   } else if (strncmp( sf,CBLAS_SYR2K,12)==0) {
      cblas_rout = CBLAS_SYR2K;

cblas_info = 1;
cblas_syr2k( INVALID,  CblasUpper, CblasNoTrans, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 2;
cblas_syr2k( CblasColMajor,  INVALID, CblasNoTrans, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 3;
cblas_syr2k( CblasColMajor,  CblasUpper, INVALID, 0, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasNoTrans, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasTrans, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_syr2k( CblasColMajor,  CblasLower, CblasNoTrans, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 4;
cblas_syr2k( CblasColMajor,  CblasLower, CblasTrans, INVALID, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasNoTrans, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasTrans, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_syr2k( CblasColMajor,  CblasLower, CblasNoTrans, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 5;
cblas_syr2k( CblasColMajor,  CblasLower, CblasTrans, 0, INVALID, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_syr2k( CblasRowMajor,  CblasUpper, CblasNoTrans, 0, 2, ALPHA, A, 1, B, 2, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_syr2k( CblasRowMajor,  CblasUpper, CblasTrans, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_syr2k( CblasRowMajor,  CblasLower, CblasNoTrans, 0, 2, ALPHA, A, 1, B, 2, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_syr2k( CblasRowMajor,  CblasLower, CblasTrans, 2, 0, ALPHA, A, 1, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasNoTrans, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasTrans, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 8;
cblas_syr2k( CblasColMajor,  CblasLower, CblasNoTrans, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 8;
cblas_syr2k( CblasColMajor,  CblasLower, CblasTrans, 0, 2, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_syr2k( CblasRowMajor,  CblasUpper, CblasNoTrans, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 10;
cblas_syr2k( CblasRowMajor,  CblasUpper, CblasTrans, 2, 0, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_syr2k( CblasRowMajor,  CblasLower, CblasNoTrans, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 10;
cblas_syr2k( CblasRowMajor,  CblasLower, CblasTrans, 2, 0, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasNoTrans, 2, 0, ALPHA, A, 2, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 10;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasTrans, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 10;
cblas_syr2k( CblasColMajor,  CblasLower, CblasNoTrans, 2, 0, ALPHA, A, 2, B, 1, BETA, C, 2 );
chkxer();
cblas_info = 10;
cblas_syr2k( CblasColMajor,  CblasLower, CblasTrans, 0, 2, ALPHA, A, 2, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_syr2k( CblasRowMajor,  CblasUpper, CblasNoTrans, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_syr2k( CblasRowMajor,  CblasUpper, CblasTrans, 2, 0, ALPHA, A, 2, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_syr2k( CblasRowMajor,  CblasLower, CblasNoTrans, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_syr2k( CblasRowMajor,  CblasLower, CblasTrans, 2, 0, ALPHA, A, 2, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasNoTrans, 2, 0, ALPHA, A, 2, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_syr2k( CblasColMajor,  CblasUpper, CblasTrans, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_syr2k( CblasColMajor,  CblasLower, CblasNoTrans, 2, 0, ALPHA, A, 2, B, 2, BETA, C, 1 );
chkxer();
cblas_info = 13;
cblas_syr2k( CblasColMajor,  CblasLower, CblasTrans, 2, 0, ALPHA, A, 1, B, 1, BETA, C, 1 );
chkxer();
   }
   if (cblas_ok == TRUE)
       printf(" %-12s PASSED THE TESTS OF ERROR-EXITS\n", cblas_rout);
   else
       printf("***** %s FAILED THE TESTS OF ERROR-EXITS *******\n",cblas_rout);
}
