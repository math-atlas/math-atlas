ROUTINE dot;
   PARAMS :: N, X, Y;
   INT    :: N;
   DOUBLE_PTR :: X, Y;
ROUT_LOCALS
   INT    :: i;
   DOUBLE :: x, y, ddot;
   CONST_INIT :: ddot = 0.0;
ROUT_BEGIN
   LOOP i = 0, N
      LIVE_SCALARS_IN  :: ddot;
      LIVE_SCALARS_OUT :: ddot;
   LOOP_BODY
//    Load x
      x = X[i];
//    Load y
      y = Y[i];
//    multiply x * y
      x = x * y;
//    add into ddot
      ddot += x;
   LOOP_END
   RETURN ddot;
ROUT_END
