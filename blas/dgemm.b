ROUTINE ATL_USERMM;
   PARAMS :: M, N, K, alpha, A0, lda, B, ldb, beta, C, ldc;
   INT    :: M, N, K, lda, ldb, ldc;
   DOUBLE :: alpha, beta;
   DOUBLE_PTR :: A0, B, C;
ROUT_LOCALS
   INT    :: i, j, k;
   DOUBLE :: a0, b0, c00;
   DOUBLE_PTR :: A;
ROUT_BEGIN
   ldc = ldc - M;
   lda = lda - M;
   j = N;
   NLOOP:
      A = A0;
      i = M;
      MLOOP:
         c00 = C[0];
         LOOP k = 0, K
         LOOP_BODY
            a0 = A[0];
            b0 = B[0];
            c00 += a0 * b0;
            A += 1;
            B += 1;
         LOOP_END
         C[0] = c00;
         A += lda;
         B = B - M;
         C += 1;
         i = i - 1;
      IF (i > 0) GOTO MLOOP;
      j = j - 1;
      B += ldb;
      C += ldc;
   IF (j > 0) GOTO NLOOP;
ROUT_END
