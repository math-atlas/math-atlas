#define IFKO_DECLARE
#include "ifko.h"
#include "fko_arch.h"
#include "fko_l2a.h"
#include "fko_loop.h"

int FUNC_FLAG=0, DTnzerod=0, DTabsd=0, DTnzero=0, DTabs=0;

int main(int nargs, char **args)
{
   FILE *fpin;
   struct assmln *abase;
   BBLOCK *bp;
   extern FILE *yyin;
   extern BBLOCK *bbbase;

   fpin = fopen(args[1], "r");
   assert(fpin);
   yyin = fpin;
   bp = bbbase = NewBasicBlock(NULL, NULL);
   bp->inst1 = bp->instN = NULL;
   yyparse();
   fclose(fpin);
   bp = FindBasicBlocks(bbbase);
   KillAllBasicBlocks(bbbase);
   bbbase = bp;
   FixFrame(bbbase);
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   NewBasicBlocks(bbbase);
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   FindLoops(); 
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   CheckFlow(bbbase, __FILE__, __LINE__);
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   CalcInsOuts(bbbase); 
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   CalcAllDeadVariables();
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
/*   if (optloop) DoLoopGlobalRegAssignment(optloop); */
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   AddBlockComments(bbbase);
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   AddLoopComments();
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
#if 0
   AddSetUseComments(bbbase);   
   AddDeadComments(bbbase);
#endif
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   abase = lil2ass(bbbase);
   KillAllBasicBlocks(bbbase);
   dump_assembly(stdout, abase);
   KillAllAssln(abase);
   return(0);
}
