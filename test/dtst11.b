ROUTINE iamax;
   PARAMS :: N, X;
   INT    :: N;
   DOUBLE_PTR :: X;
ROUT_LOCALS
   INT    :: i, imax;
   DOUBLE :: x, amax;
   CONST_INIT :: amax = 0.0, imax=0;
ROUT_BEGIN
   LOOP i = 0, N
      LIVE_SCALARS_IN  :: imax, amax;
      LIVE_SCALARS_OUT :: imax, amax;
   LOOP_BODY
//    Load x
      x = X[i];
//    x = abs(x)
      x = ABS x;
//    Branch if we have a new maximum
      IF (x > amax) GOTO NEWMAX;
ENDOFLOOP:
   LOOP_END
   RETURN imax;

NEWMAX:
   amax = x;
   imax = i;
   GOTO ENDOFLOOP;
ROUT_END
