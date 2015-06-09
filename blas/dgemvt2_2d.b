ROUTINE ATL_UGEMV;
   PARAMS :: M, N, A, lda, X, Y;
   INT :: M, N, lda;
   DOUBLE_PTR :: X, Y;
   DOUBLE_ARRAY[*][lda] :: A;
   UNROLL_ARRAY :: A(*,2);
ROUT_LOCALS
   INT :: i, j, ldam;
   DOUBLE :: y0, y1, a0, a1, a2, x0;
ROUT_BEGIN  
   ldam = lda * 2;
   ldam = ldam - M;
   j = N;
   NLOOP:
      y0 = Y[0];
      y1 = Y[1];

      LOOP i = 0, M
      MUTUALLY_ALIGNED(32) :: *;
      LOOP_BODY
         x0 = X[0];

         a0 = A[0][0];
         y0 += a0 * x0;

         a1 = A[0][1];
         y1 += a1 * x0;

         A += 1; 
         
         X += 1;
      LOOP_END
      Y[0] = y0;
      Y[1] = y1;
      
      A += ldam;
      X = X-M;
      Y += 2;
      j = j - 2;
      IF (j > 0) GOTO NLOOP;
ROUT_END

