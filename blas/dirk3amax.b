ROUTINE FKO_IRK3AMAX;
   PARAMS :: M, s0, A0, lda;
   INT    :: M, lda;
   DOUBLE :: s0;
   DOUBLE_PTR :: A0;
ROUT_LOCALS
   INT    :: i, N, imax;
   DOUBLE :: t0, t1, t2, t3, t4, s1, s2, s3, s4, amax;
   DOUBLE_PTR :: A1, A2, A3;
   CONST_INIT :: amax = 0.0, imax=3;
ROUT_BEGIN
   
   A1 = A0 + lda;
   A2 = A1 + lda;
   A3 = A2 + lda;

   s1 = A3[0];
   s1 = - s1;
   
   s2 = A3[1];
   s3 = A3[2];

   s4 = A0[1];
   s2 += s1 * s4;

   A3[1] = s2;
   s2 = -s2;
   s4 = A0[2];
   s3 += s4 * s1;
   s4 = A1[2];
   s3 += s4 * s2;
   A3[2] = s3;
   s3 = -s3;

   A0 += 3;
   A1 += 3;
   A2 += 3;
   A3 += 3;

   N = M - 3;
   
   LOOP i =  0, N
   MUTUALLY_ALIGNED(32) :: *;
   LOOP_BODY
      t0 = A0[0];
      t1 = A1[0];
      t2 = A2[0];
      t3 = A3[0];
      
      t2 = t2 * s0;

      t3 += t0 * s1;
      t3 += t1 * s2;
      t3 += t2 * s3;

      t4 = ABS t3;
      IF (t4 > amax) GOTO NEWMAX;
UPDATE:
      A2[0] = t2;
      A3[0] = t3;

ENDOFLOOP:
      A0 += 1;
      A1 += 1;
      A2 += 1;
      A3 += 1;
   LOOP_END
   RETURN imax;

NEWMAX:
   amax = t4;
   imax = i+3;
   GOTO UPDATE;
ROUT_END
