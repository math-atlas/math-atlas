ROUTINE ATL_UIAMAX;
   PARAMS :: N, X, incX;
   INT    :: N, incX;
   FLOAT_PTR :: X;
ROUT_LOCALS
   INT    :: i, imax;
   FLOAT :: x, amax;
   CONST_INIT :: amax = 0.0e0, imax=0;
ROUT_BEGIN
//   N = N - 1;
   LOOP i = N, 0, -1
   LOOP_BODY
//    Load x
      x = X[0];
//    x = abs(x)
      x = ABS x;
//    Branch if we have a new maximum
      IF (x > amax) GOTO NEWMAX;
ENDOFLOOP:
      X += 1;
   LOOP_END
   RETURN imax;

NEWMAX:
   amax = x;
   imax = N-i;
   GOTO ENDOFLOOP;
ROUT_END
