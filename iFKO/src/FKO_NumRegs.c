/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

int FKO_NumRegsC(fko_archinfo_t *ap, int typ)
{
   assert(typ < FKO_NTYPES && typ >= 0);
   return(ap->numregs[typ]);
}

int FKO_NumRegs(int typ)
{
   return(FKO_NumRegsC(FKO_ARCHINF, typ));
}
