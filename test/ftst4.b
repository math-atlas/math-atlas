ROUTINE arr_tst;
PARAMS     :: i, dp;
INT        :: i;
FLOAT_PTR :: dp;
ROUT_LOCALS
   FLOAT :: d0, d1;
ROUT_BEGIN
// d0 = dp[i]
   d0 = dp[i];
// return d0
   RETURN d0;
ROUT_END
