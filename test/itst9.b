ROUTINE sloop;
   PARAMS :: ibeg, iend, ia;
   INT    :: ibeg, iend;
   INT_PTR :: ia;
ROUT_LOCALS
   INT :: i, sum, k;
ROUT_BEGIN
   sum = 0;
   LOOP i = ibeg, iend
     LIVE_SCALARS_IN  :: sum;
     LIVE_SCALARS_OUT :: sum;
   LOOP_BODY
      k = ia[i];
      sum += k;
   LOOP_END
   RETURN sum;
ROUT_END
