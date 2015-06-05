ROUTINE ATL_UGEMV;
   PARAMS :: M, N, A, lda, X, Y;
   INT :: M, N, lda;
   DOUBLE_PTR :: X, Y;
   DOUBLE_ARRAY[*][lda] :: A;
   UNROLL_ARRAY :: A(*,2);
ROUT_LOCALS
   INT :: i, j, ldam;
   DOUBLE :: y0, a0, x0;
   DOUBLE_PTR :: XX0, AA0;
ROUT_BEGIN   
   ldam = lda - M;
   j = N;
   NLOOP:
      y0 = Y[0];
      LOOP i = 0, M
      MUTUALLY_ALIGNED(32) :: *;
      LOOP_BODY
         a0 = A[0][0];
         x0 = X[0];
         y0 += a0 * x0;
         A += 1;
         X += 1;
      LOOP_END
      Y[0] = y0;
      A += ldam;
      X = X - M;
      Y += 1;
      j = j - 1;
      IF (j > 0) GOTO NLOOP;
ROUT_END

