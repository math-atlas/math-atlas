#include "fko_info.h"

int FKO_VeclenC(fko_archinfo_t *ap, int typ)
{
   assert(typ >= 0 && typ < FKO_NTYPES);
   if (typ > FKO_TDBL)  /* make vector type into scalar type */
      typ -= FKO_TDBL;
   return(ap->vlen[typ]);
}

int FKO_Veclen(int typ)
{
   return(FKO_VeclenC(FKO_ARCHINF, typ));
}
