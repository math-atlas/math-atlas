ROUTINE ATL_UDOT;
   PARAMS :: N, X, incX, Y, incY;
   INT    :: N, incX, incY;
   FLOAT_PTR :: X, Y;
ROUT_LOCALS
   INT    :: i;
   FLOAT :: x, y, dot;
   CONST_INIT :: dot = 0.0e0;
ROUT_BEGIN
   LOOP i = 0, N
   LOOP_BODY
      x = X[0];
      y = Y[0];
      dot += x * y;
      X += 1;
      Y += 1;
   LOOP_END
   RETURN dot;
ROUT_END
