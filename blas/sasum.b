ROUTINE ATL_UASUM;
   PARAMS :: N, X, incX;
   FLOAT_PTR :: X;
   INT :: N, incX;
ROUT_LOCALS
   INT :: i;
   FLOAT :: x, sum;
   CONST_INIT :: sum = 0.0e0;
ROUT_BEGIN
   LOOP i = 0, N
   LOOP_BODY
      x = X[0];
      x = ABS x;
      sum += x;
      X += 1;
   LOOP_END
   RETURN sum;
ROUT_END
