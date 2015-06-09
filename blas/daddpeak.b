ROUTINE addpeak;
   PARAMS :: N;
   INT :: N;
ROUT_LOCALS
   INT :: i;
   DOUBLE :: sum;
   CONST_INIT :: sum = 0.0;
ROUT_BEGIN
   LOOP i = 0, N, 1
   LOOP_BODY
      sum += sum;
   LOOP_END
   RETURN sum;
ROUT_END
