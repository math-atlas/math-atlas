#include "ifko.h"
#define ARCH_DECLARE
#include "fko_arch.h"
main()
{
   int i=3, iret;
   iret = simple(i);
   printf("i=%d, iret=%d\n", i, iret);
}
