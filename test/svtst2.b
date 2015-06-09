ROUTINE SVTST2
PARAMS :: X, fval,dval,N;
INT :: N;
FLOAT :: fval;
DOUBLE :: dval;
FLOAT_PTR :: X;

ROUT_LOCALS
INT :: i;
FLOAT :: x;
DOUBLE :: dlocal, oned;
CONST_INIT :: dlocal = 2.0, oned = 1.0;

ROUT_BEGIN
   LOOP i = 0, N
   LOOP_BODY
   x = X[0];
   IF (x > fval) GOTO ASGVAL;
   dlocal += oned;
ENDOFLOOP:
   X += 1;
   LOOP_END
   
   RETURN dlocal;

ASGVAL:
   dlocal += dval;
   GOTO ENDOFLOOP;
ROUT_END
