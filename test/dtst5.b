ROUTINE arr_tst;
PARAMS     :: i, dp;
INT        :: i;
DOUBLE_PTR :: dp;
ROUT_LOCALS
   DOUBLE :: d0;
ROUT_BEGIN
   d0 = dp[i+1];
   RETURN d0;
ROUT_END
