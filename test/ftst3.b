ROUTINE fsimple;
PARAMS :: f1, f2;
DOUBLE :: f1, f2;
ROUT_LOCALS;
   DOUBLE :: two, pf, tmp;
   CONST_INIT :: two=2.0, pf=0.5;
ROUT_BEGIN;
// Add para + const
   tmp = f1 + pf;
// add para2 + const
   f2 += two;
// Produce total sum
   tmp += f2;
// Return sum
   RETURN tmp;
ROUT_END;
