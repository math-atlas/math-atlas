/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

int FKO_TypestrToInt(char *str)
{
   if (!strcmp(str, "INT"))
      return(FKO_TINT);
   if (!strcmp(str, "DOUBLE"))
      return(FKO_TDBL);
   if (!strcmp(str, "FLOAT"))
      return(FKO_TFLT);
   if (!strcmp(str, "VINT"))
      return(FKO_TVINT);
   if (!strcmp(str, "VDOUBLE"))
      return(FKO_TVDBL);
   if (!strcmp(str, "VFLOAT"))
      return(FKO_TVFLT);
   fprintf(stderr, "UNKNOWN TYPE = '%s'\n", str);
   assert(0);
   return(0);
}
