/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

int FKO_NumCachesC(fko_archinfo_t *ap)
{
   return(ap->ncaches);
}

int FKO_NumCaches(void)
{
   return(FKO_NumCachesC(FKO_ARCHINF));
}
