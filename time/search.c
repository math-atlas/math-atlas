#include "ifko.h"

#if 0
USAGE: [exe] [flags]
   -C <comp>  : full path to compiler
   -A "<args>": pass args unchanged to compiler
   -T <timer> : full path to timer to run
   -I <HIL>   : full path to source file
   -V 0 : do not vectorize
   -V 1 : vectorize
   -V 2 : try vector and scalar code
   Following describe different searches:
   -<flag> b <beg> <end>       : binary search between <beg> and <end>
   -<flag> l <beg> <end> <inc> : linear search between <beg> and <end>
   <flag> can be:
      -P : pipeline length
      -U : Loop unroll factor
#endif

void PrintUsage(char *name)
{
   fprintf(stderr, "USAGE: %s [flags]\n", name);
   fprintf(stderr, "   -C "args" : pass args to compiler\n");
   exit(-1);
}
void GetFlags(int nargs, char **args)
int main(int nargs, char **args)
{
   exit(0);
}
