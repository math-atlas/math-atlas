ROUTINE SVTST3
PARAMS :: X, Y, dval, fval;
DOUBLE :: dval;
FLOAT :: fval;
DOUBLE_PTR :: X;
FLOAT_PTR :: Y;

ROUT_LOCALS
   DOUBLE :: dflag;
   FLOAT :: fflag;
   CONST_INIT :: dflag = 2.0, fflag = 1.0e0;

ROUT_BEGIN
   
   dflag += dval;
   X[0] = dflag;

   fflag +=fval;
   Y[0] = fflag;

ROUT_END   
