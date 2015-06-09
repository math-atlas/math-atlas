ROUTINE ATL_UCOPY;
   PARAMS :: N, X, incX, Y, incY;
   INT    :: N, incX, incY;
   FLOAT_PTR :: X, Y;
ROUT_LOCALS
   INT    :: i;
   FLOAT :: x;
ROUT_BEGIN
   LOOP i = 0, N
   LOOP_BODY
      x = X[0];
      Y[0] = x;
      X += 1;
      Y += 1;
   LOOP_END
ROUT_END
