ROUTINE arr_tst;
PARAMS     :: i, dp;
INT        :: i;
FLOAT_PTR :: dp;
ROUT_LOCALS
   FLOAT :: d0;
ROUT_BEGIN
   d0 = dp[i+1];
   RETURN d0;
ROUT_END
