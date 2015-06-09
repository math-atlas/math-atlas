ROUTINE ATL_UIAMAX;
   PARAMS :: N, X, incX;
   INT    :: N, incX;
   FLOAT_PTR :: X;
ROUT_LOCALS
   INT    :: i, imax;
   FLOAT :: x, amax;
   CONST_INIT :: amax = 0.0e0, imax=0;
ROUT_BEGIN
   LOOP i = 0, N, 1
   LOOP_BODY
      x = X[0];
      x = ABS x;
      IF (x > amax) GOTO NEWMAX;
ENDOFLOOP:
      X += 1;
   LOOP_END
   RETURN imax;

NEWMAX:
   amax = x;
   imax = i;
   GOTO ENDOFLOOP;
ROUT_END
