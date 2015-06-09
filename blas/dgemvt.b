ROUTINE ATL_UGEMV;
   PARAMS :: M, N, A, lda, X, Y;
   INT :: M, N, lda;
   DOUBLE_PTR :: A, X, Y;
ROUT_LOCALS
   INT :: i, j;
   DOUBLE :: y0, a0, x0;
   DOUBLE_PTR :: XX0, AA0;
ROUT_BEGIN   
   j = N;
   NLOOP:
      y0 = Y[0];
      XX0 = X;
      AA0 = A;
      LOOP i = 0, M
      MUTUALLY_ALIGNED(32) :: *;
      LOOP_BODY
         a0 = A[0];
         x0 = X[0];
         y0 += a0 * x0;
         A += 1;
         X += 1;
      LOOP_END
      Y[0] = y0;
      A = AA0;
      A += lda;
      Y += 1;
      X = XX0;
      j = j - 1;
      IF (j > 0) GOTO NLOOP;
ROUT_END

