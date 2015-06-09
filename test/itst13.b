ROUTINE sloop;
ROUT_LOCALS
   INT :: i, sum;
ROUT_BEGIN
   sum = 0;
   i = 0;
LOOPBEG:
//    Loop body begin
      sum += i;
      i += 1;
//    Loop body end
   IF (i < 10) GOTO LOOPBEG;
   RETURN sum;
ROUT_END
