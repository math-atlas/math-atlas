ROUTINE myabs
PARAMS :: dval;
DOUBLE :: dval;
ROUT_LOCALS
   DOUBLE :: zero;
   CONST_INIT :: zero = 0.0;
ROUT_BEGIN
// meaningless comment
   IF (dval < zero) GOTO NEGNUM;
   RETURN dval;
NEGNUM:
   dval = -dval;
   RETURN dval;
ROUT_END
