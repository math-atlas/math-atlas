ROUTINE multiply
   PARAMS :: i1, i2;
   INT    :: i1, i2;
ROUT_LOCALS
   INT :: i, res;
ROUT_BEGIN
   res = 0;
   LOOP i = 0, i2, 1
   LOOP_BODY
      res += i1;
   LOOP_END
   RETURN res;
ROUT_END
