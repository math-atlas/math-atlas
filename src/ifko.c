#define IFKO_DECLARE
#include "ifko.h"
#include "fko_arch.h"
#include "fko_l2a.h"

int FUNC_FLAG=0, DTnzerod=0, DTabsd=0, DTnzero=0, DTabs=0;
int main(int nargs, char **args)
{
   FILE *fpin;
   extern FILE *yyin;
   extern INSTQ *iqhead;
   struct assmln *abase;
   extern int LOCALIGN, LOCSIZE; 
   int i, savr[64];

   for (i=0; i < NIR; i++) savr[i] = i+2;
   fpin = fopen(args[1], "r");
   assert(fpin);
   yyin = fpin;
   yyparse();
   fclose(fpin);
   CreatePrologue(LOCALIGN, LOCSIZE, 0, NIR-1, savr, 0, NULL, 0, NULL);
   abase = lil2ass(iqhead);
   dump_assembly(stdout, abase);
   KillAllAssln(abase);
   #if 0
      abase = DumpData();
      dump_assembly(stdout, abase);
      KillAllAssln(abase);
   #endif
   KillAllInst(iqhead);
   return(0);
}
