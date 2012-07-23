ROUTINE ATL_USSQ;
   PARAMS :: N, X, incX, scal0, ssq0;
   INT    :: N, incX;
   DOUBLE_PTR :: X, scal0, ssq0;
ROUT_LOCALS
   INT    :: i;
   DOUBLE :: ax, ssq, scal, t0, t1, ATL_rzero, ATL_rone;
   CONST_INIT :: ATL_rone = 1.0;
ROUT_BEGIN
   ssq = ssq0[0];
   scal = scal0[0];
   LOOP i = 0, N
   LOOP_BODY
      ax = X[0];
      ax = ABS ax;
      IF (ax > scal) GOTO SCAL_UPDATE;
         t0 = ax/scal;
         ssq += t0 * t0;
ENDOFLOOP:      
      X += 1;
   LOOP_END
   ssq0[0] = ssq;
   scal0[0] = scal;
   RETURN ;
SCAL_UPDATE:
   t0 = scal/ax;
   t0 = t0 * t0; //t0 *= t0 ??
   t1 = ssq * t0; 
   ssq = ATL_rone + t1;
   scal = ax;
   GOTO ENDOFLOOP;
ROUT_END
