ROUTINE sloop;
   PARAMS :: ibeg, iend, ia;
   INT    :: ibeg, iend;
   INT_PTR :: ia;
ROUT_LOCALS
   INT :: i, sum, k;
ROUT_BEGIN
   sum = 0;
      ia += ibeg;
   LOOP i = ibeg, iend
     LIVE_SCALARS_IN  :: sum;
     LIVE_SCALARS_OUT :: sum;
   LOOP_BODY
//      k = ia[i];
         k = ia[0];
      sum += k;
         ia += 1;
   LOOP_END
   RETURN sum;
ROUT_END
