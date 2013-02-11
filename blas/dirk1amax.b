ROUTINE FKO_IRK1AMAX;
   PARAMS :: M, s0, A0, lda;
   INT    :: M, lda;
   DOUBLE :: s0;
   DOUBLE_PTR :: A0;
ROUT_LOCALS
   INT    :: i, N, imax;
   DOUBLE :: t0, s1, t1, t2, t3, amax;
   DOUBLE_PTR :: A1;
   CONST_INIT :: amax = 0.0, imax=1;
ROUT_BEGIN
   A1 = A0 + lda;
   s1 = A1[0];
   
   A0 += 1;
   A1 += 1;
   N = M - 1;
   
   LOOP i =  0, N
   MUTUALLY_ALIGNED_FOR :: 32;
   LOOP_BODY
      t0 = A0[0];
      t1 = A1[0];
      t0 = t0 * s0;
      t2 = t0 * s1;
      t1 = t1 - t2;
      t3 = ABS t1;
      IF (t3 > amax) GOTO NEWMAX;
UPDATE:
      A0[0] = t0;
      A1[0] = t1;
ENDOFLOOP:
      A0 += 1;
      A1 += 1;
   LOOP_END
   RETURN imax;

NEWMAX:
   amax = t3;
   imax = i+1;
   GOTO UPDATE;
ROUT_END
