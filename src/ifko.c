#define IFKO_DECLARE
#include "ifko.h"
#include "fko_arch.h"
#include "fko_l2a.h"
#include "fko_loop.h"

FILE *fpST=NULL, *fpIG=NULL, *fpLIL=NULL;
int FUNC_FLAG=0, DTnzerod=0, DTabsd=0, DTnzero=0, DTabs=0;
int FKO_FLAG;

void PrintUsage(char *name)
{
   fprintf(stderr, "USAGE: %s [flags] <HIL file>\n", name);
   fprintf(stderr, "  -o <outfile>\n");
   fprintf(stderr, "  -C 0 : suppress comments\n");
   fprintf(stderr, "  -t [S,I,L,s] : generate temporary files:\n");
   fprintf(stderr, "     S : dump symbol table to <file>.ST\n");
   fprintf(stderr, "     I : dump interference graph info to <file>.IG\n");
   fprintf(stderr, "     L : dump LIL <file>.L\n");
   exit(-1);
}

void GetFlags(int nargs, char **args, char **FIN, FILE **FPIN, FILE **FPOUT)
{
   FILE *fpin, *fpout;
   char *fin=NULL, *fout=NULL;
   char ln[512];
   int i, j;

   FKO_FLAG = 0;
   for (i=1; i < nargs; i++)
   {
      if (args[i][0] != '-')
      {
         if (fin) PrintUsage(args[0]);
         else fin = args[i];
      }
      else
      {
         switch(args[i][1])
         {
         case 't':
            for(++i, j=0; args[i][j]; j++)
            {
               switch(args[i][j])
               {
               case 'I': /* IG */
                  fpIG = (FILE *) 1;
                  break;
               case 'S': /* symbol table */
                  fpST = (FILE *) 1;
                  break;
               case 'L': /* LIL */
                  fpLIL = (FILE *) 1;
                  break;
               default :
                  fprintf(stderr, "Unknown temp label %c ignored!!\n\n",
                          args[i][j]);
               }
            }
            break;
         case 'o':
            fout = args[++i];
            break;
         case 'C':
            j = atoi(args[++i]);
            if (!j) FKO_FLAG |= IFF_KILLCOMMENTS;
            break;
         default:
            fprintf(stderr, "Unknown flag '%s'\n", args[i]);
            PrintUsage(args[0]);
         }
      }
   }
   if (!fin) fpin = stdin;
   else
   {
      fpin = fopen(fin, "r");
      assert(fpin);
   }
   if (!fout)
   {
      fpout = stdout;
      strcpy(ln, "ifko_temp.");
      i = 10;
   }
   else
   {
      fpout = fopen(fout, "w");
      assert(fpout);
      for (i=0; fout[i]; i++);
      if (i > 2 && fout[i-1] == 'l' && fout[i-2] == '.')
         FKO_FLAG |= IFF_NOASS | IFF_LIL;
      for (i=0; fout[i]; i++) ln[i] = fout[i];
      for (i--; i > 0 && ln[i] != '.'; i--);
      if (ln[i] != '.')
      {
         for (i=0; ln[i]; i++);
         ln[i++] = '.';
      }
      else ln[++i] = '\0';

   }
   if (fpIG)
   {
      ln[i] = 'I'; ln[i+1] = 'G'; ln[i+2] = '\0';
      fpIG = fopen(ln, "w");
      assert(fpIG);
   }
   if (fpST)
   {
      ln[i] = 'S'; ln[i+1] = 'T'; ln[i+2] = '\0';
      fpST = fopen(ln, "w");
      assert(fpST);
   }
   if (fpLIL)
   {
      ln[i] = 'L'; ln[i+1] = '\0';
      fpLIL = fopen(ln, "w");
      assert(fpLIL);
   }
   *FIN = fin;
   *FPIN = fpin;
   *FPOUT = fpout;
}

int main(int nargs, char **args)
{
   FILE *fpin, *fpout, *fpl;
   char *fin;
   char ln[512];
   int i, j, KeepOn;
   struct assmln *abase;
   BBLOCK *bp;
   extern FILE *yyin;
   extern BBLOCK *bbbase;
   BLIST *bl, *lbase;

   GetFlags(nargs, args, &fin, &fpin, &fpout);
   yyin = fpin;
   bp = bbbase = NewBasicBlock(NULL, NULL);
   bp->inst1 = bp->instN = NULL;
   yyparse();
   fclose(fpin);
   GenPrologueEpilogueStubs(bbbase);
   NewBasicBlocks(bbbase);
   FindLoops(); 
   CheckFlow(bbbase, __FILE__, __LINE__);
   CalcInsOuts(bbbase); 
   CalcAllDeadVariables();
/*
 * Perform optimizations on special loop first
 */
   if (optloop)
   {
      DoLoopGlobalRegAssignment(optloop);  
      do
      {
         j = DoScopeRegAsg(optloop->blocks, 1, &i);   
         KeepOn = j != i;
         KeepOn &= DoCopyProp(optloop->blocks); 
         if (KeepOn)
           fprintf(stderr, "\n\nREAPPLYING LOOP OPTIMIZATIONS!!\n\n");
      }
      while(KeepOn);
   }

   PrintInst(fopen("tmp.err", "w"), bbbase);
/*
 * Perform global optimizations on whole function
 */
   for (lbase=NULL,bp=bbbase; bp; bp = bp->down)
      lbase = AddBlockToList(lbase, bp);
   do
   {
/*
 *    Do reg asg on whole function
 */
      j = DoScopeRegAsg(lbase, 2, &i);   
      KeepOn = j != i;
fprintf(stderr, "\n%s(%d): KeepOn = %d\n", __FILE__,__LINE__, KeepOn);
      KeepOn = KeepOn && DoCopyProp(lbase); 
fprintf(stderr, "\n%s(%d): KeepOn = %d\n", __FILE__,__LINE__, KeepOn);
      if (KeepOn)
        fprintf(stderr, "\n\nREAPPLYING GLOBAL OPTIMIZATIONS!!\n\n");
   }
   while(KeepOn);
   KillBlockList(lbase);
   INDEADU2D = CFUSETU2D = 0;
   if (!INDEADU2D)
      CalcAllDeadVariables();

fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   AddBlockComments(bbbase);
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   AddLoopComments();
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
#if 1
   AddSetUseComments(bbbase);   
   AddDeadComments(bbbase); 
#endif
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
   FinalizePrologueEpilogue(bbbase);
   if (fpLIL)
   {
      PrintInst(fpLIL, bbbase);
      fclose(fpLIL);
   }
   if (fpST)
   {
      PrintST(fpST);
      fclose(fpST);
   }
#if 0
/* Sometime, rewrite IG to take global table, rather than sorted table */
   if (fpIG)
   {
      PrintIG(fpIG);
      fclose(fpIG);
   }
#endif
   if (DO_LIL(FKO_FLAG))
   {
      if (!DO_ASS(FKO_FLAG))
         PrintInst(fpout, bbbase);
      else
      {
         if (!fin)
            fpl = fopen("ifko_tmp.l", "w");
         else
         {
            for (i=0; ln[i] = fin[i]; i++);
            for (i--; i > 0 && ln[i] != '.'; i--);
            if (ln[i] == '.')
            {
               ln[i+1] = 'l';
               ln[i+2] = '\0';
            }
            else strcat(ln, ".l");
            fpl = fopen(ln, "w");
         }
         assert(fpl);
         PrintInst(fpl, bbbase);
      }
   }
   if (DO_ASS(FKO_FLAG))
   {
      abase = lil2ass(bbbase);
      KillAllBasicBlocks(bbbase);
      dump_assembly(fpout, abase);
      KillAllAssln(abase);
   }
   return(0);
}
