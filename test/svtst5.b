ROUTINE SVTST5
PARAMS :: N, X, Y;
INT :: N;
DOUBLE_PTR :: X;
FLOAT_PTR :: Y;
ROUT_LOCALS
   INT :: i;
   DOUBLE :: x, dflag;
   FLOAT :: flag;
   CONST_INIT :: flag = 1.0e0;
   CONST_INIT :: dflag = -1.0;
ROUT_BEGIN
   LOOP i = 0, N
   LOOP_BODY
   x = X[0];
   x = x * dflag;
   X[0] = x;
   Y[0] = flag;
   X +=1;
   Y +=1;
   LOOP_END
ROUT_END   
