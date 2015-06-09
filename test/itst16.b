ROUTINE sloop;
   PARAMS :: ibeg, iend, ia;
   INT    :: ibeg, iend;
   INT_PTR :: ia;
ROUT_LOCALS
   INT :: i, sum, k;
ROUT_BEGIN
   sum = 0;
   ia += ibeg;
   iend = iend - ibeg;
   LOOP i = iend, 0, -1
   LOOP_BODY
      k = ia[0];
      sum += k;
      ia += 1;
   LOOP_END
   RETURN sum;
ROUT_END
