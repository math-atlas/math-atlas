//
// Simple imul via loop test, primarily for debugging loop optimization
//
ROUTINE mul_tst;
PARAMS :: m0, m1;
INT    :: m0, m1;
ROUT_LOCALS
   INT :: imul, i;
ROUT_BEGIN
   imul = 0;
   LOOP i = 0, m1
   LOOP_BODY
      imul += m0;
   LOOP_END
   RETURN imul;
ROUT_END
