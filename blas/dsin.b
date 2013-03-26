ROUTINE FKO_SINE;
   PARAMS :: N, X, Y, iY;
   INT :: N;
   DOUBLE_PTR :: X, Y, iY;

ROUT_LOCALS 
   INT :: i;
   DOUBLE :: x1, x, y, r, z, v, iy;
   DOUBLE :: t0, t1;
   DOUBLE :: s1, s2, s3, s4, s5, s6, lmark, half, fzero;   
   CONST_INIT ::  s1 = -1.66666666666666324348e-01D, 
                  s2 = 8.33333333332248946124e-03D, 
                  s3 = -1.98412698298579493134e-04D, 
                  s4 =  2.75573137070700676789e-06D, 
                  s5 = -2.50507602534068634195e-08D, 
                  s6 = 1.58969099521155010221e-10D, 
                  lmark = 7.450580596923828125e-9D, 
                  half = 5.0e-1D, 
                  fzero = 0.0e0D;

ROUT_BEGIN
   LOOP i = 0, N, 1
   LOOP_BODY
      x = X[0];
      y = Y[0];
      iy = iY[0];
      x1 = ABS x;
      IF (x1 < lmark) GOTO IF_TRUE; 
ELSE:
      z = x * x;
      v = z * x;

      t0 = s5;
      t0 += z * s6;

      t1 = s4;
      t1 += z * t0;

      t0 = s3;
      t0 += z * t1;

      r = s2;
      r += z * t0;

      IF (iy != fzero) GOTO NZERO;
ZEROV:
   
   t0 = s1;
   t0 += z * r;
   r = x;
   r += v * t0;

ENDOFLOOP:
      Y[0] = r;
      X += 1;
      Y += 1;
      iY += 1;
   LOOP_END

   RETURN;

IF_TRUE:
   r = x;
   GOTO ENDOFLOOP;

NZERO:
      t0 = half * y;
      t1 = v * r;
      t0 = t0 - t1;
      t1 = z * t0;
      t0 = t1 - y;
      t1 = v * s1;
      t1 = t0 - t1;
      r = x - t1;

   GOTO ENDOFLOOP;

ROUT_END
