/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

int FKO_CachelineSizeC(fko_archinfo_t *ap, int lvl)
{
   assert(lvl >= 0 && lvl < ap->ncaches);
   return(ap->clsz[lvl]);
}

int FKO_CachelineSize(int lvl)
{
   return(FKO_CachelineSizeC(FKO_ARCHINF, lvl));
}
