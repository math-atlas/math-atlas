ROUTINE ATL_UAMAX;
   PARAMS :: N, X, incX;
   INT    :: N, incX;
   DOUBLE_PTR :: X;
ROUT_LOCALS
   INT    :: i;
   DOUBLE :: x, amax;
   CONST_INIT :: amax = 0.0;
ROUT_BEGIN
   LOOP i = 0, N, 1
   LOOP_BODY
      x = X[0];
      x = ABS x;
      IF (x > amax) GOTO NEWMAX;
ENDOFLOOP:
      X += 1;
   LOOP_END
   RETURN amax;

NEWMAX:
   amax = x;
   GOTO ENDOFLOOP;
ROUT_END
