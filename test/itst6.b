ROUTINE arr_tst;
PARAMS  :: i, ip;
INT     :: i;
INT_PTR :: ip;
ROUT_LOCALS;
   INT :: i0, i1;
ROUT_BEGIN
   i0 = ip[i];
   RETURN i0;
ROUT_END
