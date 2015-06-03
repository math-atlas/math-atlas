#include "fko_archinfo.h"

int FKO_NumRegsC(fko_archinfo_t *ap, int typ1)
{
   switch(typ1)
   {
   case FKO_TINT:
      return(ap->numregs[FKO_TINT]);
   case FKO_TFLT:
      return(ap->numregs[FKO_TFLT]);
   case FKO_TDBL:
      return(ap->numregs[FKO_TDBL]);
   case FKO_TVINT:
      return(ap->numregs[FKO_TVINT]);
   case FKO_TVFLT:
      return(ap->numregs[FKO_TVFLT]);
   case FKO_TVDBL:
      return(ap->numregs[FKO_TVDBL]);
   default:
      fprintf(stderr, "INVALID TYPE=%d\n", typ1);
      assert(0);

   }
   return(0);
}
int FKO_NumRegs(int typ1)
{
   return(FKO_NumRegsC(FKO_ARCHINF, typ1));
}
