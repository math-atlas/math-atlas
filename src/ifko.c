#define IFKO_DECLARE
#include "ifko.h"
#include "fko_arch.h"
#include "fko_l2a.h"
#include "fko_loop.h"

FILE *fpST=NULL, *fpIG=NULL, *fpLIL=NULL;
int FUNC_FLAG=0; 
int DTnzerod=0, DTabsd=0, DTnzero=0, DTabs=0, DTx87=0, DTx87d=0;
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

static void WriteShortArrayToFile(FILE *fp, short *sp)
{
   short n;
   if (sp)
   {
      n = sp[0] + 1;
      assert(fwrite(sp, sizeof(short), n, fp) == n);
   }
   else
   {
      n = 0;
      assert(fwrite(&n, sizeof(short), 1, fp) == 1);
   }
}

static short *ReadShortArrayFromFile(FILE *fp)
/*
 * Reads a short array of form <N> [<elt1>....<eltN>] into memory, and 
 * returns allocated array pointer.  <N> is put in as 1st element of this
 * N+1 element array
 */
{
   short n;
   short *sp=NULL;
   assert(fread(&n, sizeof(short), 1, fp) == 1);
   if (n > 0)
   {
      sp = malloc((n+1)*sizeof(short));
      assert(sp);
      sp[0] = n;
      assert(fread(sp, sizeof(short), n, fp) == n);
   }
   return(sp);
}

static void WriteMiscToFile(char *name)
{
   FILE *fp;
   short n;

   fp = fopen(name, "wb");
   assert(fp);
   if (optloop)
   {
      n = 1;
      assert(fwrite(&n, sizeof(short), 1, fp) == 1);
      assert(fwrite(optloop, sizeof(LOOPQ), 1, fp) == 1);
      WriteShortArrayToFile(fp, optloop->slivein);
      WriteShortArrayToFile(fp, optloop->sliveout);
      WriteShortArrayToFile(fp, optloop->adeadin);
      WriteShortArrayToFile(fp, optloop->adeadout);
      assert(!optloop->abalign);  /* fix this later */
   }
   else
   {
      n = 0;
      assert(fwrite(&n, sizeof(short), 1, fp) == 1);
   }
   fclose(fp);
}

static void ReadMiscFromFile(char *name)
{
   int i;
   short n;
   FILE *fp;

   if (optloop)
      KillLoop(optloop);
   KillAllLoops();
   optloop = NewLoop(0);
   fp = fopen(name, "rb");
   assert(fp);
   assert(fread(&n, sizeof(short), 1, fp) == 1);
   if (n)
   {
      assert(fread(optloop, sizeof(LOOPQ), 1, fp) == 1);
/* 
 *    Read in loop-markup arrays
 */
      optloop->slivein  = ReadShortArrayFromFile(fp);
      optloop->sliveout = ReadShortArrayFromFile(fp);
      optloop->adeadin  = ReadShortArrayFromFile(fp);
      optloop->adeadout = ReadShortArrayFromFile(fp);
      optloop->abalign = NULL;  /* handle this later */
      optloop->preheader = optloop->header = NULL;
      optloop->tails = optloop->posttails = NULL;
      if (optloop->blkvec)
         optloop->blocks = BitVec2BlockList(optloop->blkvec);
      else
         optloop->blocks = NULL;
      optloop->iglist = NULL;
      optloop->next = NULL;
   }
   else 
      optloop = NULL;
   fclose(fp);
}

static void RestoreFKOState(int isav)
{
   char *fST = "/tmp/iFKO_ST.",
        *fLIL = "/tmp/iFKO_LIL.",
        *fmisc =  "/tmp/iFKO_misc.";
   char ln[128];
   extern BBLOCK *bbbase;

   sprintf(ln, "%s%d", fST, isav);
   ReadSTFromFile(ln);
   sprintf(ln, "%s%d", fLIL, isav);
   ReadLILFromBinFile(ln);
   sprintf(ln, "%s%d", fmisc, isav);
   ReadMiscFromFile(ln);
/*
 * All annotation must be done afresh
 */
   CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = 0;
}

static void SaveFKOState(int isav)
/*
 * Writes out base LIL translation of routine so we can reread it in in order
 * to iteratively apply differing optimizations
 * Need to save:
 * 1. LIL
 * 2. ST: N, STname, SToff, STflag,
 *        niloc, nlloc, nfloc, ndloc, nvfloc, nvdloc, LOCALIGN, LOCSIZE, NPARA
 *    - WriteSTToFile, ReadSTFromFile in symtab.c
 * 3. Global vars: STderef, DTnzerod, DTabsd, DTx87d, DTnzero, DTabs, DTx87
 * 4. optloop
 * 5. Global vars: FKO_BVTMP, FKO_FLAG, 
 *    CFU2D,CFDOMU2D CFUSETU2D, INUSETU2D, INDEADU2D
 * 5. ... not finished looking ...
 *
 * DONE: 1(inst.c), 2,3(symtab.c)
 */
{
   char *fST = "/tmp/iFKO_ST.",
        *fLIL = "/tmp/iFKO_LIL.",
        *fmisc =  "/tmp/iFKO_misc.";
   char ln[128];
   extern BBLOCK *bbbase;
        
   sprintf(ln, "%s%d", fST, isav);
   WriteSTToFile(ln);
   sprintf(ln, "%s%d", fLIL, isav);
   WriteLILToBinFile(ln, bbbase);
   sprintf(ln, "%s%d", fmisc, isav);
   WriteMiscToFile(ln);
}

int PerformOpt(int SAVESP)
/*
 * Returns 0 on success, non-zero on failure
 */
{
   BLIST *bl, *lbase;
   BBLOCK *bp;
   int i, j, KeepOn, k;
   extern BBLOCK *bbbase;

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
      KeepOn = DoCopyProp(lbase);
      if (KeepOn)
        fprintf(stderr, "\n\nREAPPLYING GLOBAL OPTIMIZATIONS!!\n\n");
   }
   while(KeepOn);
   KillBlockList(lbase);
   INDEADU2D = CFUSETU2D = 0;
   return(0);
}

int GoToTown(int SAVESP)
{
   int i;
   extern BBLOCK *bbbase;
   extern struct locinit *ParaDerefQ;

   GenPrologueEpilogueStubs(bbbase, SAVESP);
   NewBasicBlocks(bbbase);
   FindLoops(); 
   CheckFlow(bbbase, __FILE__, __LINE__);
   if (optloop && 1)
      OptimizeLoopControl(optloop, 1, 1, NULL);
   CalcInsOuts(bbbase); 
   CalcAllDeadVariables();

   assert(!PerformOpt(SAVESP));

   if (!INDEADU2D)
      CalcAllDeadVariables();
   AddBlockComments(bbbase);
   AddLoopComments();
#if 0
   AddSetUseComments(bbbase);   
   AddDeadComments(bbbase); 
#endif
   i = FinalizePrologueEpilogue(bbbase, SAVESP);
   KillAllLocinit(ParaDerefQ);
   ParaDerefQ = NULL;
   if (i)
      return(1);
   CheckFlow(bbbase, __FILE__, __LINE__);
   return(0);
}

int main(int nargs, char **args)
{
   FILE *fpin, *fpout, *fpl;
   char *fin;
   char ln[512];
   struct assmln *abase;
   BBLOCK *bp;
   int i;
   extern FILE *yyin;
   extern BBLOCK *bbbase;

   GetFlags(nargs, args, &fin, &fpin, &fpout);
   yyin = fpin;
   bp = bbbase = NewBasicBlock(NULL, NULL);
   bp->inst1 = bp->instN = NULL;
   yyparse();
   fclose(fpin);
   SaveFKOState(0);
   if (GoToTown(0))
   {
      fprintf(stderr, "\n\nOut of registers for SAVESP, trying again!!\n");
      RestoreFKOState(0);
      assert(!GoToTown(IREGBEG+NIR-1));
   }

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
