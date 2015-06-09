/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

int FKO_PipelenMACC(fko_archinfo_t *ap, int typ)
{
   assert(typ >= 0 && typ < FKO_NTYPES);
   return(ap->pipelen_mac[typ]);
}

int FKO_PipelenMAC(int typ)
{
   return(FKO_PipelenMACC(FKO_ARCHINF, typ));
}
