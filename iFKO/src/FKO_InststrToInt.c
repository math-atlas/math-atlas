/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

int FKO_TypestrToInt(char *str)
{
   if (!strcmp(str, "max") || strcmp(str, "MAX"))
      return(FKO_SIMAX);
   if (!strcmp(str, "min") || strcmp(str, "MIN"))
      return(FKO_SIMIN);
   if (!strcmp(str, "cmov") || strcmp(str, "CMOV"))
      return(FKO_SICMOV);
   fprintf(stderr, "UNKNOWN INST = '%s'\n", str);
   assert(0);
   return(0);
}
