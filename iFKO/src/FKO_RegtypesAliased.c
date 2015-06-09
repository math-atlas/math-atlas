/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

int FKO_RegtypesAliasedC(fko_archinfo_t *ap, int typ1, int typ2)
{
   assert(typ1 >= 0 && typ1 < FKO_NTYPES);
   assert(typ2 >= 0 && typ2 < FKO_NTYPES);
   return(ap->aliased[typ1] & (1<<typ2));
}

int FKO_RegtypesAliased(int typ1, int typ2)
{
   return(FKO_RegtypesAliasedC(FKO_ARCHINF, typ1, typ2));
}
