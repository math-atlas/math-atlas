ROUTINE ATL_USERMM;
   PARAMS :: M, N, K, alpha, A, lda, B, ldb, beta, C, ldc;
   INT    :: M, N, K, lda, ldb, ldc;
   DOUBLE :: alpha, beta;
   DOUBLE_PTR :: A, B, C;
ROUT_LOCALS
   INT    :: i, j, k, ldam;
   DOUBLE :: a0, b0, c00, c10, c20, c30;
   DOUBLE_PTR :: A0, A1, A2, A3;
ROUT_BEGIN
   ldc = ldc - M;
   ldam = lda*4;
   ldam = ldam - M;
   j = N;
   NLOOP:
      A0 = A;
      i = M;
      MLOOP:
         A1 = A0 + lda;
         A2 = A1 + lda;
         A3 = A2 + lda;
         c00 = C[0];
         c10 = C[1];
         c20 = C[2];
         c30 = C[3];
         LOOP k = 0, K
         LOOP_BODY
            b0 = B[0];
            a0 = A0[0];
            c00 += a0 * b0;
            a0 = A1[0];
            c10 += a0 * b0;
            a0 = A2[0];
            c20 += a0 * b0;
            a0 = A3[0];
            c30 += a0 * b0;
            A0 += 1;
            A1 += 1;
            A2 += 1;
            A3 += 1;
            B += 1;
         LOOP_END
         C[0] = c00;
         C[1] = c10;
         C[2] = c20;
         C[3] = c30;
         A0 += ldam;
         B = B - M;
         C += 4;
         i = i - 4;
      IF (i > 0) GOTO MLOOP;
      j = j - 1;
      B += ldb;
      C += ldc;
   IF (j > 0) GOTO NLOOP;
ROUT_END
