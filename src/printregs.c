#define ARCH_DECLARE
#include "fko.h"
#include "fko_arch.h"

#if 1
char *Int2Reg(int i)
/*
 * Translates integral encoding to machine-specific registers
 */
{
   static char ln[128];

   if (i >= IREGBEG && i < IREGEND)
      sprintf(ln, "%s", archiregs[i-IREGBEG]);
   else if (i >= FREGBEG && i < FREGEND)
      sprintf(ln, "%s", archfregs[i-FREGBEG]);
   else if (i >= DREGBEG && i < DREGEND)
      sprintf(ln, "%s", archdregs[i-DREGBEG]);
   else if (i >= ICCBEG && i < ICCEND)
      sprintf(ln, "%s", ICCREGS[i-ICCBEG]);
   else if (i >= FCCBEG && i < FCCEND)
      sprintf(ln, "%s", FCCREGS[i-FCCBEG]);
   else if (i == PCREG)
      sprintf(ln, "%s", "PC");
   else
      sprintf(ln, "UNUSED");
   return(ln);
}
#endif

int main(int nargs, char **args)
{
   int i;
   char *sp;

   if (nargs > 1)
   {
      i = atoi(args[1]);
      printf("Register %d = %s\n", i, Int2Reg(i));
   }
   else
   {
      for (i=1; i <= TNREG; i++)
         printf("Register %d = %s\n",  i, Int2Reg(i));
   }
}
