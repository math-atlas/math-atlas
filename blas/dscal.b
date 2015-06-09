ROUTINE ATL_USCAL
   PARAMS :: N, alpha, X, incX;
   INT :: N, incX;
   DOUBLE :: alpha;
   DOUBLE_PTR :: X;
ROUT_LOCALS
   INT :: i;
   DOUBLE :: x, y;
ROUT_BEGIN
   LOOP i = 0, N
   LOOP_BODY
      x = X[0];
      x = x * alpha;
      X[0] = x;
      X += 1;
   LOOP_END
ROUT_END
