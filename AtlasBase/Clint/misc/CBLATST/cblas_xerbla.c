#include <stdio.h>
#include <stdarg.h>
#include "atlas_misc.h"
#include "cblas_test.h"

/* void ATL_xerbla(int info, char *rout, char *form, ...) */
int cblas_errprn(int ierr, int info, char *form, ...)
{
   if (ierr < info) return(ierr);
   else return(info);
}
void cblas_xerbla(int info, char *rout, char *form, ...)
{
   extern int cblas_lerr, cblas_info, cblas_ok;
   extern char *cblas_rout;
   va_list argptr;

   /* DCHKE hasn't been invoked */
   if (cblas_ok == UNDEFINED) {
      va_start(argptr, form);
      if (info)
         printf("Parameter %d to routine %s was incorrect\n", info, rout);
      vfprintf(stderr, form, argptr);
      va_end(argptr);
      exit(-1);
   }
   
   if (info != cblas_info){
      printf("***** XERBLA WAS CALLED WITH INFO = %d INSTEAD OF %d in %s *******\n",info, cblas_info, rout);
      cblas_lerr = PASSED;
      cblas_ok = FALSE;
   } else cblas_lerr = FAILED;
   if (cblas_rout != NULL && strstr(cblas_rout, rout) == 0){
      printf("***** XERBLA WAS CALLED WITH SRNAME = %s INSTEAD OF %s *******\n", rout, cblas_rout);
      cblas_ok = FALSE;
   }
}
