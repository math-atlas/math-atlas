ROUTINE MSVIAMAX
PARAMS :: N, X, Y;
INT :: N;
DOUBLE_PTR :: X;
FLOAT_PTR :: Y;
ROUT_LOCALS
   INT :: i, imax;
   DOUBLE :: x, amax;
   FLOAT :: flag;
   CONST_INIT :: amax = 0.0, imax = 0;
   CONST_INIT :: flag = 1.0e0;
ROUT_BEGIN
   LOOP i = N, 0, -1
   LOOP_BODY
   x = X[0];
   x = ABS x;
   IF (x > amax) GOTO ASGVAL;
ENDOFLOOP:
   X +=1;
   Y +=1;
   LOOP_END
   RETURN  imax;

ASGVAL:
   amax = x;
   imax = N-i;
   Y[0] = flag;
   GOTO ENDOFLOOP;
ROUT_END   
