ROUTINE FKO_IRK2AMAX;
   PARAMS :: M, s0, A0, lda;
   INT    :: M, lda;
   FLOAT :: s0;
   FLOAT_PTR :: A0;
ROUT_LOCALS
   INT    :: i, N, imax;
   FLOAT :: t0, t1, t2, t3, s1, s2, s3, amax;
   FLOAT_PTR :: A1, A2;
   CONST_INIT :: amax = 0.0e0, imax=2;
ROUT_BEGIN
   
   A1 = A0 + lda;
   A2 = A1 + lda;

   s1 = A2[0];
   s1 = - s1;

   s2 = A2[1];
   s3 = A0[1];
   s2 += s3 * s1;

   A2[1] = s2;
   s2 = -s2;

   A0 += 2;
   A1 += 2;
   A2 += 2;

   N = M - 2;
   
   LOOP i =  0, N
   MUTUALLY_ALIGNED(32) :: *;
   LOOP_BODY
      t0 = A0[0];
      t1 = A1[0];
      t2 = A2[0];

      t1 = t1 * s0;
      
      t2 += t0 * s1;
      t2 += t1 * s2;

      t3 = ABS t2;
      IF (t3 > amax) GOTO NEWMAX;
UPDATE:
      A1[0] = t1;
      A2[0] = t2;
ENDOFLOOP:
      A0 += 1;
      A1 += 1;
      A2 += 1;
   LOOP_END
   RETURN imax;

NEWMAX:
   amax = t3;
   imax = i+2;
   GOTO UPDATE;
ROUT_END
