/*
 * Copyright (C) 2015 R. Clint Whaley.
 */
#include "fko_info.h"

int FKO_system(char *outs, int len, const char *frm, ...)
{
   int ierr;
   va_list ap;
   va_start(ap, frm);

   va_start(ap, frm);
   #if defined(__STDC_VERSION__) && (__STDC_VERSION__/100 >= 1999)
      ierr = vsnprintf(outs, len, frm, ap);
   #else
      ierr = vsprintf(outs, frm, ap);
   #endif
   va_end(ap);
   assert (ierr < len);  /* not safe, but may catch some errs for C89 */
   ierr = system(outs);
   return(ierr);
}
