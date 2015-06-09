ROUTINE ptr_tst;
PARAMS :: ip;
INT_PTR :: ip;
ROUT_LOCALS;
   INT :: i;
ROUT_BEGIN
   i = ip[0];
   i = i + i;
   ip[0] = i;
ROUT_END
