ROUTINE FKO_SINE;
   PARAMS :: N, X, Y;
   INT :: N;
   FLOAT_PTR :: X, Y;

ROUT_LOCALS 
   INT :: i;
   FLOAT :: x1, x, y, r, z, v;
   FLOAT :: t0, t1;
   FLOAT :: s1, s2, s3, s4, s5, s6, lmark, half, fzero;
   CONST_INIT :: s1 = -1.6666667163e-1, s2 = 8.3333337680e-3, 
   s3 = -1.9841270114e-4, s4 = 2.7557314297e-6, s5 = -2.5050759689e-8, 
   s6 = 1.5896910177e-10, lmark = 7.4505806e-9, half = 5.0e-1, fzero = 0.0e0;;

ROUT_BEGIN
   LOOP i = 0, N, 1
   LOOP_BODY
      x = X[0];
      y = Y[0];
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

      IF (y != fzero) GOTO NZERO;
ZEROV:
   
   t0 = s1;
   t0 += z * r;
//   t1 = x;
//   t1 += v * t0;
//   Y[0] = t1;
   r = x;
   r += v * t0;

ENDOFLOOP:
      Y[0] = r;
      X += 1;
      Y += 1;
   LOOP_END

   RETURN;

IF_TRUE:
   //Y[0] = x;
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
//      t1 = x - t1;
//      Y[0] = t1;
      r = x - t1;

   GOTO ENDOFLOOP;

ROUT_END
