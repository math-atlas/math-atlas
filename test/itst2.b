ROUTINE simple;
PARAMS :: i1, i2;
INT :: i1, i2;
ROUT_LOCALS;
   INT :: j;
ROUT_BEGIN
   j = i1 + i2;
   j += 2;
   RETURN j;
ROUT_END
