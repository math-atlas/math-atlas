ROUTINE myabs
PARAMS :: dval;
FLOAT  :: dval;
ROUT_LOCALS
   FLOAT  :: zero;
   CONST_INIT :: zero = 0.0e0;
ROUT_BEGIN
   IF (dval < zero) GOTO NEGNUM;
   RETURN dval;
NEGNUM:
   dval = -dval;
   RETURN dval;
ROUT_END
