ROUTINE ATL_UASUM;
   PARAMS :: N, X, incX;
   DOUBLE_PTR :: X;
   INT :: N, incX;
ROUT_LOCALS
   INT :: i;
   DOUBLE :: x, sum;
   CONST_INIT :: sum = 0.0;
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
