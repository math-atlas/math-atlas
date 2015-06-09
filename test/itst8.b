ROUTINE sloop;
   PARAMS :: ibeg, iend;
   INT    :: ibeg, iend;
ROUT_LOCALS
   INT :: i, sum;
ROUT_BEGIN
   sum = 0;
   LOOP i = ibeg, iend
     LIVE_SCALARS_IN  :: sum;
     LIVE_SCALARS_OUT :: sum;
   LOOP_BODY
      sum += i;
   LOOP_END
   RETURN sum;
ROUT_END
