#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void PrintUsage(char *name)
{
   fprintf(stderr, 
           "USAGE: %s -o <outfile> -c <flush kb> -k <kernel.s> -r <rout>\n",
           name);
   exit(-1);
}

FILE *GetFlags(int nargs, char **args, char *rout, char *kern, int *flush)
{
   FILE *fpout=NULL;
   int i;

   *kern = *rout = '\0';
   *flush = -1;
   for (i=1; i < nargs; i++)
   {
      if (args[i][0] != '-' || i == nargs-1)
         PrintUsage(args[0]);
      switch(args[i][1])
      {
      case 'c':
         *flush = atoi(args[++i]);
         break;
      case 'o':
         fpout = fopen(args[++i], "w");
         assert(fpout);
         break;
      case 'k':
         strcpy(kern, args[++i]);
         break;
      case 'r':
         strcpy(rout, args[++i]);
         break;
      }
   }
   return(fpout);
}

int main(int nargs, char **args)
{
   char ln[4096];
   char rout[128], URT[128], kern[512];
   FILE *fpin, *fpout;
   char *st;
   int i, flush;
   double time, mflop;

   fpout = GetFlags(nargs, args, rout, kern, &flush);
   for (i=0; URT[i] = toupper(rout[i]); i++);


   i = sprintf(ln, "make l1time rout=%s URT=%s kern=%s", rout, URT, kern);
   if (flush >= 0) 
      i += sprintf(ln+i, "cache='-C %d'", flush);
   ln[i] = '\n';
   ln[i+1] = '\0';
   fprintf(stderr, "EXEC %s", ln);
   assert(!system(ln));
   fpin = fopen("ifk2.tmp", "r");
   assert(fpin);
   fgets(ln, 4096, fpin);
   fclose(fpin);
   fprintf(stderr, "EXEC %s", ln);
   st = strstr(ln, "time=");
   assert(st);
   st += 5;
   assert(sscanf(st, "%le", &time)==1);
   st = strstr(ln, "mflop=");
   st += 6;
   assert(sscanf(st, "%lf", &mflop)==1);
   if (fpout)
   {
      fprintf(fpout, "%le, %lf", time, mflop);
      fclose(fpout);
   }
   fprintf(stderr, "\n\ntime=%le, %lf\n", time, mflop);
   exit(0);
}
