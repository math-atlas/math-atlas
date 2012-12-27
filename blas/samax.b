ROUTINE ATL_UAMAX;
   PARAMS :: N, X, incX;
   INT    :: N, incX;
   FLOAT_PTR :: X;
ROUT_LOCALS
   INT    :: i;
   FLOAT :: x, amax;
   CONST_INIT :: amax = 0.0e0;
ROUT_BEGIN
   LOOP i = 0, N
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
