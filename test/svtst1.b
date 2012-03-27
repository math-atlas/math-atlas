ROUTINE SVTST1
PARAMS :: N, X, Y;
INT :: N;
DOUBLE_PTR :: X;
FLOAT_PTR :: Y;
ROUT_LOCALS
   INT :: i;
   DOUBLE :: x;
   FLOAT :: flag;
   CONST_INIT :: flag = 1.0e0;
ROUT_BEGIN
   LOOP i = 0, N
   LOOP_BODY
   x = X[0];
   x = ABS x;
   X[0] = x;
   Y[0] = flag;
   X +=1;
   Y +=1;
   LOOP_END
ROUT_END   
