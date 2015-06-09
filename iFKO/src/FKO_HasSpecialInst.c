/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

int FKO_HasSpecialInstC(fko_archinfo_t *ap, int typ, int inst)
{
   assert(typ < FKO_NTYPES && typ >= 0);
   assert(inst < FKO_NSINST && inst >= 0);
   return(ap->spcinst[typ] & (1<<inst));
}

int FKO_HasSpecialInst(int typ, int inst)
{
   return(FKO_HasSpecialInstC(FKO_ARCHINF, typ, inst));
}
