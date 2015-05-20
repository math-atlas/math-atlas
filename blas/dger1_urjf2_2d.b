ROUTINE ATL_UGERK
   PARAMS :: M, N, X, Y, A, lda;
   INT :: M, N, lda;
   DOUBLE_PTR :: A, X, Y;
   DOUBLE_ARRAY[lda][*] :: A;
   UNROLL_ARRAY :: A(2,*);

ROUT_LOCALS
   INT :: i, j, ldam;
   DOUBLE :: y0, y1, a0, a1, x0;

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
         a1 = A[1][0];

         a0 += x0 * y0;
         a1 += x0 * y1;
         
         A[0][0] = a0;
         A[1][0] = a1;
         
         A += 1;
         
         X += 1;
      LOOP_END
      A += ldam;
      X = X - M;
      Y += 2;
      j = j - 2;
      IF (j > 0) GOTO NLOOP;
ROUT_END

