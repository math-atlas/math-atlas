ROUTINE FKO_COSINE;
   PARAMS :: N, X, Y;
   INT :: N;
   DOUBLE_PTR :: X, Y;

ROUT_LOCALS 
   INT :: i;
   DOUBLE :: x1, x, y, r, z;
   DOUBLE :: t0, t1, qx, a, hz;
   DOUBLE :: C1, C2, C3, C4, C5, C6, C7, C8, one, half, l1, l2, lmark;
   
   CONST_INIT ::
      one =  1.00000000000000000000e+00D,
      C1  =  4.16666666666666019037e-02D,
      C2  = -1.38888888888741095749e-03D,
      C3  =  2.48015872894767294178e-05D,
      C4  = -2.75573143513906633035e-07D,
      C5  =  2.08757232129817482790e-09D,
      C6  = -1.13596475577881948265e-11D,
      C7  =  2.8125e-1D,
      C8  =  4.0e+0D,
      lmark = 7.450580596923828125e-9D, 
      l1  = 3.0e-1D,
      l2 = 7.8125e-1D,                 
      half = 5.0e-1D;

ROUT_BEGIN
   LOOP i = 0, N
   LOOP_BODY
      x = X[0];
      y = Y[0];
      x1 = ABS x;
      IF (x1 < lmark) GOTO IF_TRUE; 
ELSE:
      z = x * x;
      t0 = z * C6;
      t0 += C5;
      
      t0 = z * t0;
      t0 += C4;

      t0 = z * t0;
      t0 += C3;

      t0 = z * t0;
      t0 += C2;

      t0 = z * t0;
      t0 += C1;

      r = z * t0;
      
      IF (x1 > l1) GOTO T2;
ELSE2:
      IF (x1 > l2) GOTO T3; 
   ELSE3:
      qx = x / C8;
   M3:
      t0 = half * z;
      hz = t0 - qx;
      
      a = one - qx;
      
      t0 = z * r;
      t1 = x * y;
      t0 = t0 - t1;
      t0 = hz - t0;
      r = a - t0;
ENDOFLOOP:
      Y[0] = r;
      X += 1;
      Y += 1;
   LOOP_END
   RETURN;

IF_TRUE:
   r = one;
   GOTO ENDOFLOOP;
T2:
   t0 = x * y;
   t1 = z * r;
   t0 = t1 - t0;
   t1 = half * z;
   t0 = t1 - t0;
   r = one - t0;
   GOTO ENDOFLOOP;
T3: 
   qx = C7;
   GOTO M3;

ROUT_END
