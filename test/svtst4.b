ROUTINE SVTST4
PARAMS :: N, X, Y;
INT :: N;
DOUBLE_PTR :: X, Y;
ROUT_LOCALS
   INT :: i;
   DOUBLE :: x;
   DOUBLE :: flag;
   CONST_INIT :: flag = 1.0;
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
