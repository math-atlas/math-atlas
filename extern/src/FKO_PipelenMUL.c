#include "fko_info.h"

int FKO_PipelenMULC(fko_archinfo_t *ap, int typ)
{
   assert(typ >= 0 && typ < FKO_NTYPES);
   return(ap->pipelen_mul[typ]);
}

int FKO_PipelenMUL(int typ)
{
   return(FKO_PipelenMULC(FKO_ARCHINF, typ));
}
