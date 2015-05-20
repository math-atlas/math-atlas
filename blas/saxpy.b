ROUTINE ATL_UAXPY;
   PARAMS :: N, alpha, X, incX, Y, incY;
   INT    :: N, incX, incY;
   FLOAT :: alpha;
   FLOAT_PTR :: X, Y;
ROUT_LOCALS
   INT    :: i;
   FLOAT :: x, y;
ROUT_BEGIN
   LOOP i = 0, N
   LOOP_BODY
      x = X[0];
      y = Y[0];
      y += x * alpha;
      Y[0] = y;
      X += 1;
      Y += 1;
   LOOP_END
ROUT_END
