ROUTINE ATL_UGEMV;
   PARAMS :: M, N, A, lda, X, Y;
   INT :: M, N, lda;
   DOUBLE_PTR :: X, Y;
   DOUBLE_ARRAY [*][lda] :: A;
   UNROLL_ARRAY :: A(*,16);
ROUT_LOCALS
   INT :: i, j, ldam;
   DOUBLE :: y0, y1, y2, y3, y4, y5, y6, y7, y8, y9, y10, y11, y12, y13, y14, y15; 
   DOUBLE :: a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15,x0;
ROUT_BEGIN  
   ldam = lda * 16;
   ldam = ldam - M;
   j = N;
   NLOOP:
      y0 = Y[0];
      y1 = Y[1];
      y2 = Y[2];
      y3 = Y[3];
      y4 = Y[4];
      y5 = Y[5];
      y6 = Y[6];
      y7 = Y[7];
      y8 = Y[8];
      y9 = Y[9];
      y10 = Y[10];
      y11 = Y[11];
      y12 = Y[12];
      y13 = Y[13];
      y14 = Y[14];
      y15 = Y[15];

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

         a4 = A[0][4];
         y4 += a4 * x0;
         
         a5 = A[0][5];
         y5 += a5 * x0;
         
         a6 = A[0][6];
         y6 += a6 * x0;
         
         a7 = A[0][7];
         y7 += a7 * x0;
         
         a8 = A[0][8];
         y8 += a8 * x0;
         
         a9 = A[0][9];
         y9 += a9 * x0;
         
         a10 = A[0][10];
         y10 += a10 * x0;
         
         a11 = A[0][11];
         y11 += a11 * x0;
         
         a12 = A[0][12];
         y12 += a12 * x0;
         
         a13 = A[0][13];
         y13 += a13 * x0;
         
         a14 = A[0][14];
         y14 += a14 * x0;
         
         a15 = A[0][15];
         y15 += a15 * x0;
         
         A += 1;
         X += 1;
      LOOP_END
      Y[0] = y0;
      Y[1] = y1;
      Y[2] = y2;
      Y[3] = y3;
      Y[4] = y4;
      Y[5] = y5;
      Y[6] = y6;
      Y[7] = y7;
      Y[8] = y8;
      Y[9] = y9;
      Y[10] = y10;
      Y[11] = y11;
      Y[12] = y12;
      Y[13] = y13;
      Y[14] = y14;
      Y[15] = y15;
      
      A += ldam;
      Y += 16;
      X = X-M;
      j = j - 16;
      IF (j > 0) GOTO NLOOP;
ROUT_END

