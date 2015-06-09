ROUTINE sloop;
ROUT_LOCALS
   INT :: i, sum;
ROUT_BEGIN
   sum = 0;
   LOOP i = 0, 10, 1
     LIVE_SCALARS_IN  :: sum;
     LIVE_SCALARS_OUT :: sum;
   LOOP_BODY
//    start loop body
      sum += i;
//    done loop body
   LOOP_END
   RETURN sum;
ROUT_END
