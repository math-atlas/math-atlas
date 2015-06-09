ROUTINE ATL_UGEMV;
   PARAMS :: M, N, A, lda, X, Y;
   INT :: M, N, lda;
   DOUBLE_PTR :: A, X, Y;
   DOUBLE_ARRAY [*][lda] :: A;
   UNROLL_ARRAY :: A(*,4);
ROUT_LOCALS
   INT :: i, j, ldam;
   DOUBLE :: y0, y1, y2, y3, a0, a1, a2, a3, a4, x0;
ROUT_BEGIN  
   ldam = lda * 4;
   ldam = ldam - M;
   j = N;
   NLOOP:
      y0 = Y[0];
      y1 = Y[1];
      y2 = Y[2];
      y3 = Y[3];

      LOOP i = 0, M
      MUTUALLY_ALIGNED(32) :: *;
      LOOP_BODY
         x0 = X[0];

         a0 = A[0][0];
         y0 += a0 * x0;

         a1 = A[0][1];
         y1 += a1 * x0;

         a2 = A[0][2];
         y2 += a2 * x0;

         a3 = A[0][3];
         y3 += a3 * x0;

         A += 1;
         X += 1;
      LOOP_END
      Y[0] = y0;
      Y[1] = y1;
      Y[2] = y2;
      Y[3] = y3;
      
      A += ldam;
      Y += 4;
      X = X-M;
      j = j - 4;
      IF (j > 0) GOTO NLOOP;
ROUT_END

