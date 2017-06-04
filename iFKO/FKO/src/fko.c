/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#define IFKO_DECLARE
#include "fko.h"
#include "fko_arch.h"
#include "fko_l2a.h"
#include "fko_loop.h"

FILE *fpST=NULL, *fpIG=NULL, *fpLIL=NULL, *fpOPT=NULL, *fpLOOPINFO=NULL;
FILE *fpLRSINFO=NULL, *fpARCHINFO=NULL, *fpVECINFO=NULL, *fpVECINFO2=NULL;
FILE *fpBVNUM=NULL;
int FUNC_FLAG=0; 
int DTnzerod=0, DTabsd=0, DTnzero=0, DTabs=0, DTx87=0, DTx87d=0;
/*int DTnzerods=0, DTabsds=0, DTnzeros=0, DTabss=0;*/ /*not used anymore */
int FKO_FLAG;
int VECT_FLAG=0;  /* Majedul: for different vectorization methods */
/*
 * Majedul: 
 *    As we will introduce more and more new optimizations, I will keep 
 *    flag according to program states. 
 */
int STATE1_FLAG=0;
int STATE2_FLAG=0;
int STATE3_FLAG=0;
int path = -1; /* this is for speculation */
int FKO_MaxPaths = 0; /*number of paths to be analyzed. default 0 means all paths*/

int PFISKIP=0, PFINST=(-1), PFCHUNK=1;
int NWNT=0, NAWNT=0;
short *AEn=NULL;
char **ARRWNT=NULL, **AES=NULL;
short *SEn=NULL;    /* Majedul: this is for generalizing the Scalar Expansion */
char **SES=NULL;    /* Majedul: this is for generalizing the Scalar Expansion */
static char fST[1024], fLIL[1024], fmisc[1024];
static short *PFDST=NULL, *PFLVL=NULL;
static char **PFARR=NULL;
int  USEALLIREG=0; /* not used anymore */
int IWAY = 0;

int noptrec=0;
enum FKOOPT optrec[512];
short optchng[512];

void PrintUsageN(char *name)
{
   int i;
   fprintf(stderr, "\n%d repeatable optimization phases:\n", MaxOpt);
   for (i=0; i < MaxOpt; i++)
      fprintf(stderr, "%3d. %s   : %s\n", i+1, optabbr[i], optmnem[i]);
   fprintf(stderr, "\nUSAGE: %s [flags]\n", name);
   fprintf(stderr, 
           "  -I <LIL> <symtab> <misc> : start from intermediate files\n");
   fprintf(stderr, "  -v : verbose output\n");
   fprintf(stderr, "  -i <file> : generate loop info file and quit\n");
   fprintf(stderr, "  -iarch <file> : generate architectural info and quit\n");
   fprintf(stderr, "  -ilrs <file> : generate LR spilling info and quit\n");
   fprintf(stderr, "  -ivec <file> : generate vectorization info and quit\n");
   fprintf(stderr, 
         "  -ibvnum <file> : write vecapproach # of best vector method\n");
   fprintf(stderr, "  -ivec2 <file> : generate vectorization info and quit\n");
   fprintf(stderr, "  -o <outfile> : assembly output file\n");
   fprintf(stderr, "  -R [d,n] <directory/name> : restore path & base name\n");
   fprintf(stderr, "  -K 0 : suppress comments\n");
   fprintf(stderr, "  -o <outfile> : assembly output file\n");
/*
 * Majedul: not tested with new program states 
 */
   /*fprintf(stderr, "  -c <LIL> <symtab> <misc> : generate files and quit\n");
   fprintf(stderr, "  -t [S,I,L,o] : generate temporary files:\n");
   fprintf(stderr, "     S : dump symbol table to <file>.ST\n");
   fprintf(stderr, "     I : dump interference graph info to <file>.IG\n");
   fprintf(stderr, "     L : dump LIL <file>.L\n");
   fprintf(stderr, "     o : dump opt sequence to <file>.opt\n");*/
   fprintf(stderr, "  -a : Optimize pointer for array access\n");
   fprintf(stderr, "  -U <#> : Unroll main loop # of times\n");
   fprintf(stderr, "  -U all : Unroll main loop all the way\n");
   /*fprintf(stderr, "  -V : Vectorize (SIMD) main loop\n");*/
   fprintf(stderr, "  -vec : apply best possible vectorizaion\n");
   fprintf(stderr, "  -vecapproach <#> : apply vectorization based on <#>\n");
   fprintf(stderr, "  -LNZV : loop level no hazard vectorization\n");
   fprintf(stderr, "  -SV <path#> <nvlens> :" 
                   "Apply SpecVec to path# using nvlens bet\n" );
   fprintf(stderr, "  -SLP : basic block level no hazard vectorization\n");
   fprintf(stderr, "  -w <#> : indicate #way to init SLP vectorization\n");
   /*fprintf(stderr, "  -B : Stronger Bet unrolling for SV\n");*/
   fprintf(stderr, "  -M : Maximum paths to be analyzed in SV\n");
   /*fprintf(stderr, 
         "  -p [#]: path to speculate in speculative vectirization\n");*/
   fprintf(stderr, 
         "  -FPC [#]: path to make fall-through \n");
   /*fprintf(stderr, "  -rc : apply redundant computation to reduce path\n");
   fprintf(stderr, "  -mmr : apply max/min var reduction to reduce path\n");*/
   fprintf(stderr, "  -RC : apply redundant computation to reduce path\n");
   fprintf(stderr, "  -MMR : apply max/min var reduction to reduce path\n");
   fprintf(stderr, "  -W <name> : use cache write-through stores\n");
   fprintf(stderr, "  -DDE <0/1> : enable or disable dead code elimination\n");
   fprintf(stderr, "  -P <all/name> <cache level> <dist(bytes)>\n");
   fprintf(stderr, 
"  -Pa[r/w] [n,3,0] : non-temp, 3dnow, temp L1 (read/write) prefetch\n");
   fprintf(stderr, "   -Ps [b/a] [a,m,l,A] <iskip> <nclump>\n");
   fprintf(stderr, "  -PL <lvl> <linesize> : set cache linesize in bytes\n");
   fprintf(stderr, "  -[L,G] <blknum> <maxN> <nopt> <opt1> ... <optN>\n");
   fprintf(stderr, "     Loop or global optimization block\n");
   fprintf(stderr, "     <blknum> : integer identifier > 0\n");
   fprintf(stderr, 
"     <maxN>   : if maxN == 0, do opts only once, otherwise do them until\n");
   fprintf(stderr, "                there are no changes, or maxN iterations have been performed\n");
   fprintf(stderr, "     <nopt> : number of optimizations in this block\n");
   fprintf(stderr, 
           "     <opti> : Either alphabetic opt phase abbreviation, or\n");
   fprintf(stderr, 
           "              <blknum> of encapsulated opt block\n");
   fprintf(stderr, "  -C <blknum> <blk1> <blk2> <blk3>\n");
   fprintf(stderr, 
          "     if <blk1> causes changes, <blk2> is performed, else <blk3>.\n");
   fprintf(stderr, 
           "     0 for <blk2> or <blk3> means do no action in this case.\n");
   exit(-1);
}

struct optblkq *NewOptBlock(ushort bnum, ushort maxN, ushort nopt, ushort flag)
{
   /*int i;*/
   struct optblkq *op;
   op = calloc(1, sizeof(struct optblkq));
   assert(op);
   op->bnum = bnum;
   op->maxN = maxN;
   op->nopt = nopt;
   op->flag = flag;
   op->next = NULL;
#if 0
/*
 * added nspill to keep track of resgister spilling (# of live-range which 
 * doesn't get register), -1 means not calculated yet
 * FIXED: we will allocate memory when needed(inside DoOptList).
 */
   op->nspill = malloc(NTYPES*sizeof(int));
   assert(op->nspill);
   for (i = 0; i < NTYPES; i++)
      op->nspill[i] = -1;
#else
   op->nspill = NULL;
#endif 

   if (nopt > 0)
   {
      op->opts = calloc(nopt, sizeof(enum FKOOPT));
      assert(op->opts);
   }
   return(op);
}

struct optblkq *FindOptBlockUsingNext(struct optblkq *head, ushort bnum)
{
   struct optblkq *op;
   for (op=head; op && op->bnum != bnum; op = op->next) ;
   return(op);
}
struct optblkq *FindOptBlockUsingDown(struct optblkq *head, ushort bnum)
{
   struct optblkq *op;
   for (op=head; op && op->bnum != bnum; op = op->down);
   return(op);
}

void KillAllOptBlocks(struct optblkq *head)
{
   if (!head) return;

   if (head->next)
      KillAllOptBlocks(head->next);
   if (head->down)
      KillAllOptBlocks(head->down);
   if (head->ifblk)
      KillAllOptBlocks(head->ifblk);
   if (head->opts) free(head->opts);
/*
 * delete the newly added nspill  
 */
   if (head->nspill)
      free(head->nspill);
   free(head);
}

struct optblkq *SproutNode(struct optblkq *head, struct optblkq *seed)
/*
 * Changes opts list with numbers to pure opt + appropriate next, down
 */
{
   struct optblkq *new, *op;
   int i, n;
/*
 * Handle conditional blocks seperately
 */
   if (seed->ifblk)
   {
      new = NewOptBlock(seed->bnum, 0, 0, seed->flag);
/*
 *    How to handle scope!!!
 */
      new->ifblk = SproutNode(head, FindOptBlockUsingNext(head, seed->opts[0]));
      new->down  = SproutNode(head, FindOptBlockUsingNext(head, seed->opts[1]));
      new->next  = SproutNode(head, FindOptBlockUsingNext(head, seed->opts[2]));
      return(new);
   }
/*
 * Find if we've got an encapsulated opt block 
 */
   for (n=seed->nopt,i=0; i < n && seed->opts[i] < MaxOpt; i++);
   if (i || !n)
   {
      new = NewOptBlock(seed->bnum, seed->maxN, i, seed->flag);
      for (n=i,i=0; i < n; i++)
         new->opts[i] = seed->opts[i];
   }
/*
 * If first opt is a block
 */
   else
   {
      op = FindOptBlockUsingNext(head, seed->opts[0]-MaxOpt);
      assert(op);
      new = SproutNode(head, op);
      n = 1;
   }
/*
 * See if there are still optimizations to be applied
 */
   if (n != seed->nopt)
   {
      op = NewOptBlock(seed->bnum, 0, seed->nopt-n, seed->flag);
      for (i=n; i < seed->nopt; i++)
         op->opts[i-n] = seed->opts[i];
      new->down = SproutNode(head, op);
      if (!seed->maxN)
      {
         new->next = new->down;
         new->down = NULL;
      }
      free(op->opts);
      free(op);
   }
   return(new);
}
/*
 * Majedul: to print out the structure of opttree
 */
void PrintOptTree(FILE *fpout, struct optblkq *optree)
{
   int i;
   
   if (optree)
   {
      fprintf(fpout, "[%d,%d,%d,%d] ",optree->bnum, optree->maxN, 
            optree->nopt, optree->flag);
      
      if (optree->flag & IOPT_GLOB)
         fprintf(fpout, "(GLOBAL) :");
      else if (optree->flag & IOPT_SCOP)
         fprintf(fpout, "(SCOPED) :");
      else if (!optree->flag)
         fprintf(fpout, "(OPTLOOP) :");
      
      for(i=0; i < optree->nopt; i++)
         fprintf(fpout, "%s ", optabbr[optree->opts[i]]);
      
      if (optree->nspill)
      {
         fprintf(fpout, "(");
         for (i=0; i < NTYPES; i++)
            fprintf(fpout, "%d,", optree->nspill[i]);
         fprintf(fpout, ")\n");
      }
      else
         fprintf(fpout, "\n");
   }

   if (optree->next)
   {
      fprintf(fpout, " -> ");
      PrintOptTree(fpout, optree->next);
   }
   if (optree->down)
   {
      fprintf(fpout, "%d -^ ",optree->bnum);
      PrintOptTree(fpout, optree->down);
      fprintf(fpout, " %d ^  ",optree->bnum);
   }
   if (optree->ifblk)
   {
      fprintf(fpout, "%d ? ",optree->bnum);
      PrintOptTree(fpout, optree->ifblk);
      fprintf(fpout, " %d -?  ",optree->bnum);
   }
}

struct optblkq *OptBlockQtoTree(struct optblkq *head)
/*
 * Given a queue of unordered optblocks starting at head, and using only
 * next ptrs, create appropriate tree using both next and down
 * RETURNS: root of newly created tree
 * NOTE: deletes sequential queue when done
 */
{
   struct optblkq *root;
   root = FindOptBlockUsingNext(head, 1);
   assert(root);
   root = SproutNode(head, root);
   KillAllOptBlocks(head);
   return(root);
}

int KillDupPtrinfo(char **args, struct ptrinfo *pfb)
/* 
 * assumes pf->ptr is iarg of char string; deletes any pfb that
 * have the same string
 * RETURNS: number of unique strings
 */
{
   struct ptrinfo *pfK, *pfP, *pf;
   int n=0, nk=0;

   for (pf=pfb; pf; pf = pf->next)
   {
      n++;
      for (pfP=pf,pfK=pf->next; pfK; pfK = pfK->next)
      {
         if (!strcmp(args[pf->ptr], args[pfK->ptr]))
         {
            nk++;
            pfP->next = pfK->next;
            free(pfK);
            pfK = pfP;
         }
         pfP = pfK;
      }
   }
   return(n-nk);
}

char **PtrinfoToStrings(char **args, int n, struct ptrinfo *pfb)
{
   char **sarr;
   struct ptrinfo *pf;
   int i, j;

   sarr = malloc(sizeof(char*)*(n+1));
   assert(sarr);
   sarr[n] = NULL;
   for (pf=pfb,i=0; i < n; i++, pf=pf->next)
   {
      j = strlen(args[pf->ptr])+1;
      sarr[i] = malloc(sizeof(char)*j);
      assert(sarr[i]);
      strcpy(sarr[i], args[pf->ptr]);
   }
   return(sarr);
}

struct optblkq *GetFlagsN(int nargs, char **args, 
                          char **FIN, FILE **FPIN, FILE **FPOUT)
{
   char ln[256];
   FILE *fpin, *fpout;
   FILE **fpp;
   char *fin=NULL, *fout=NULL;
   struct optblkq *obq=NULL, *op;
   struct ptrinfo *pf, *pfb=NULL, *pf0, *aeb=NULL;
   struct ptrinfo *seb=NULL;    /* this is to generalize the scalar expansion */
   struct idlist  *id, *idb=NULL, *idP, *idK;
   char *sp, *rpath=NULL, *rname=NULL;
   int i, j, k, n;

   for (i=1; i < nargs; i++)
   {
      if (args[i][0] != '-')
      {
         if (fin) PrintUsageN(args[0]);
         else fin = args[i];
      }
      else
      {
         switch(args[i][1])
         {
         case 'P': /* prefetch  -P <name> <cache level> <bytesdist> */
            if (args[i][2] == 'a') /* alternate prefetch selection */
            {
               if (args[i][3] == 'r')
               {
                  i++;
                  FKO_FLAG &= ~(IFF_3DNOWR | IFF_TAR);
                  if (args[i][0] == '3')
                    FKO_FLAG |= IFF_3DNOWR;
                  else if (args[i][0] == '0')
                    FKO_FLAG |= IFF_TAR;
               }
               else if (args[i][3] == 'w')
               {
                  i++;
                  FKO_FLAG &= ~(IFF_3DNOWW | IFF_TAW);
                  if (args[i][0] == '3')
                    FKO_FLAG |= IFF_3DNOWW;
                  else if (args[i][0] == '0')
                    FKO_FLAG |= IFF_TAW;
               }
               else
               {
                  i++;
                  FKO_FLAG &= ~(IFF_3DNOWW | IFF_TAW | IFF_3DNOWR | IFF_TAR);
                  if (args[i][0] == '3')
                    FKO_FLAG |= (IFF_3DNOWR | IFF_3DNOWW);
                  else if (args[i][0] == '0')
                    FKO_FLAG |= (IFF_TAW | IFF_TAR);
               }
            }
            else if (args[i][2] == 'L') /* linesize specification*/
            {
               j = atoi(args[++i]);
               LINESIZE[j] = atoi(args[++i]);
            }
            else if (args[i][2] == 's') /* scheduling change */
            {
               PFCHUNK = atoi(args[i+4]);
               assert(PFCHUNK > 0);
               if (args[i+1][0] == 'b')
                  PFCHUNK = -PFCHUNK;
               PFISKIP = atoi(args[i+3]);
//               assert(PFISKIP >= 0);
               PFINST = args[i+2][0];
               i += 4;
            }
            else
            {
               pf = NewPtrinfo(i+1, atoi(args[i+3]), pfb);
               pf->nupdate = atoi(args[i+2]);
               pfb = pf;
               i += 3;
            }
            break;
/*
 *       2d array access scheme: optimized reg & ptr update, ptr for each 
 *       column, etc.
 */
         case 'a':
            FKO_FLAG |= IFF_OPT2DPTR; /* optimize 2d array access */
            break;
#if 0
/*
 *       Acc expansion using the old implementation
 */
         case 'A':
            aeb = NewPtrinfo(i+1, atoi(args[i+2]), aeb);
            i += 2;
            break;
#endif
#if 0
/*
 *       stronger bet unrolling for SV
 *       NOTE: added it to -SV flag
 */
         case 'B':
            FKO_SB = atoi(args[++i]);
            break;
#endif        
/*
 *       -M <#> :  max paths to be analyzed in SV and/or FPC
 *       -MMR : apply max/min reduction
 */
         case 'M':
            if (args[i][2] && args[i][3] 
                  && args[i][2] == 'M' && args[i][3] == 'R')
               STATE1_FLAG |= IFF_ST1_MMR; 
            else
               FKO_MaxPaths = atoi(args[++i]);
            break;
#if 0
/*
 *       Majedul: To generalize the AccumExpansion with Scalar Expansion
 *       -SE var #
 *       changed it to -RE 
 */
         case 'S':
            assert(args[i][2]=='E');
            seb = NewPtrinfo(i+1, atoi(args[i+2]), seb);
            i += 2;
            break;
#endif
/*
 *       SV path sbunroll 
 *       SLP = SLP vectorization, at first we only applied to loop
 */
         case 'S':
            if (args[i][2] && args[i][2]=='V')
            {
               FKO_FLAG |= IFF_VECTORIZE;
               if (args[i+1] && args[i+2])
               {
                  path = atoi(args[++i]);
                  FKO_SB = atoi(args[++i]);
               }
               else 
                  PrintUsageN(args[0]);
              if (!path || !FKO_SB) 
              {
                 fprintf(stderr, "Invalid path and/or nvlens!\n");
               PrintUsageN(args[0]);
              }
            }
            else if (args[i][2] && args[i][3] && args[i][2] == 'L' 
                     && args[i][3] == 'P')
            {
               FKO_FLAG |= IFF_VECTORIZE;
               VECT_FLAG |= VECT_SLP;
            }
            else
               PrintUsageN(args[0]);
            break;
         case 'w': /* #way to init SLP*/
             IWAY = atoi(args[++i]); 
             break;
         case 'W': /* use cache write-through stores */
            id = malloc(sizeof(struct idlist));
            id->name = args[++i];
            id->next = idb;
            idb = id;
            break;
/*
 *       changed -SE to -RE. 
 *       options:
 *          -RE : Reduce Expandable -RE <var> <#>
 *          -RC : redcomp optimization
 *          -R [d/n]: not tested by me recently
 */
         case 'R':
            if (args[i][2] && args[i][2] == 'E')
            {
               seb = NewPtrinfo(i+1, atoi(args[i+2]), seb);
               i += 2;
            }
            else if (args[i][2] && args[i][2] == 'C')
               STATE1_FLAG |= IFF_ST1_RC; 
            else if (args[i+1] && args[i+1][0] == 'd')
            {
               rpath = args[i+2];
               i += 2;
            }
            else /* no error check! */
            {
               rname = args[i+2];
               i += 2;
            }
            break;
         case 'v':
            if (!strcmp(args[i], "-vecapproach"))
               SetVecFlag(atoi(args[++i]));
            else if(!strcmp(args[i], "-vec"))
               FKO_FLAG |= IFF_BESTVEC;
            else if (!strcmp(args[i], "-v"))
               FKO_FLAG |= IFF_VERBOSE;
            else
               PrintUsageN(args[0]);
            break;
/*
 *       specify the path to speculate for speculative vectorization, this is
 *       optional.
 */
#if 0            
         case 'p' :
            if (args[i+1])
               path = atoi(args[i+1]);
            else 
            {
               fprintf(stderr, "Specify path number. \n");
               PrintUsageN(args[0]);
            }
            assert(path>0);
            i++;
            break;
#endif
         case 'F':
            if (args[i][2] && args[i][3] && args[i][2] == 'P' 
                && args[i][3] == 'C')
            {
               if (args[i+1])
                  path = atoi(args[i+1]);
               else 
               {
                  fprintf(stderr, "Specify path number. \n");
                  PrintUsageN(args[0]);
               }
               if (path <= 0) /* must started from 1*/
               {
                  fprintf(stderr, "Invalid path number. \n");
                  PrintUsageN(args[0]); 
               }
               i++;
            }
            else
               PrintUsageN(args[0]);
            break;    
#if 0
/*
 *       NOTE: splited the vectorization into two separate flags:
 *       -LNZV and -SV 
 */
         case 'V':
            FKO_FLAG |= IFF_VECTORIZE;
/*
 *          Majedul: Recognize different vector methods from command line
 *          changed the concept!
 */
   #if 0            
            if (args[i][2])
            {
               if (args[i][2] == 'm')
               {
                  i++;
                  if (i >= nargs)
                  {
                     fprintf(stderr, "Incomplete flag '%s'\n", args[i-1]);
                     PrintUsageN(args[0]);
                  }
                  else if (args[i] && strlen(args[i]) != 3)
                  {
                     fprintf(stderr, "Unknown flag '%s'\n", args[i]);
                     PrintUsageN(args[0]);
                  }
                  else if (args[i][0]=='S' && args[i][1]=='S' && args[i][2]=='V')
                     VECT_FLAG |= VECT_SSV;
                  else if (args[i][0]=='V' && args[i][1]=='R' && args[i][2]=='C')
                     VECT_FLAG |= VECT_VRC;
                  else if (args[i][0]=='V' && args[i][1]=='E' && args[i][2]=='M')
                     VECT_FLAG |= VECT_VEM;
                  else
                  {
                     fprintf(stderr, "Unknown flag '%s'\n", args[i]);
                     PrintUsageN(args[0]);
                  }
               }
               else
               {
                  fprintf(stderr, "Unknown flag '%s'\n", args[i]);
                  PrintUsageN(args[0]);
               }
            }
            else 
            {
               VECT_FLAG |= VECT_NCONTRL;
            }
   #endif            
            break;
#endif
         case 'c':
            fpIG = fpST = fpLIL = fpOPT = (FILE*) 1;
            FKO_FLAG |= IFF_GENINTERM;
            break;
         case 'i':
/*
 *          -i : loop info
 */
            if (!args[i][2])
               fpp = &fpLOOPINFO;
/*
 *          -ilrs : live range spilling info
 */
            else if (args[i][2] && args[i][2]=='l' && args[i][3]=='r' 
                     && args[i][4]=='s')
               fpp = &fpLRSINFO;
/*
 *          -iarch : architecture info
 */   
            else if (args[i][2] && args[i][2]=='a' && args[i][3]=='r' 
                     && args[i][4]=='c' && args[i][5]=='h')
               fpp = &fpARCHINFO;
/*
 *          -ivec2 : vectorization info with VecRank details
 */   
            else if (args[i][2] && args[i][2]=='v' && args[i][3]=='e' 
                     && args[i][4]=='c' && (args[i][5] && args[i][5] == '2'))
               fpp = &fpVECINFO2;
/*
 *          -ivec : vectorization info 
 */   
            else if (args[i][2] && args[i][2]=='v' && args[i][3]=='e' 
                     && args[i][4]=='c')
               fpp = &fpVECINFO;
            else if (!strcmp(args[i], "-ibvec"))
               fpp =&fpBVNUM;
            
            else PrintUsageN(args[0]);

            i++;
            if (!args[i] ) PrintUsageN(args[0]);
            else if (!strcmp(args[i], "stderr"))
               *fpp = stderr;
            else if (!strcmp(args[i], "stdout"))
               *fpp = stdout;
            else
               *fpp = fopen(args[i], "w");
            assert(*fpp);
/*
 *          no need to parse remaining flags, if we get -iarch
 */
            if (fpARCHINFO)
            {
               FeedbackArchInfo(fpARCHINFO);
               exit(0);
            }

            break;
/*
 *       enable or disable dead code elimination
 */
         case 'D':
            if (args[i][2] && args[i][3] 
                  && args[i][2] == 'D' && args[i][3] == 'E')
            {
               if (args[i+1])
               {
                  if (!atoi(args[i+1])) /*if no valid int, even then no DDE!! */
                  {
                     FKO_FLAG |= IFF_NODDE; 
                  }
               }
               else
                  PrintUsageN(args[0]);
               i++;
            }
            else
               PrintUsageN(args[0]);

            break;

         case 'I':
            fpIG = fpST = fpLIL = fpOPT = (FILE*) 1;
            FKO_FLAG |= IFF_READINTERM;
            break;
         case 'U':
            FKO_UR = atoi(args[++i]);
/*
 *          we may now have '-U all' as argument. check it. set FKO_UR to -1 
 */
            if (!FKO_UR)
               if (!strcmp(args[i], "all"))
                  FKO_UR = -1;
            break;
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
               case 'o':
                  fpOPT = (FILE *) 1;
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
/*
 *       Majedul: we changed the logic here.. by default, we won't show the 
 *       comments in code anymore, introduced IFF_SHOWCOMMENTS to show them. 
 */
         case 'K':
            j = atoi(args[++i]);
            if (!j) FKO_FLAG |= IFF_KILLCOMMENTS;
            else FKO_FLAG |= IFF_SHOWCOMMENTS;
            break;
         case 'L':
            if (args[i][2] && args[i][3] && args[i][4] && args[i][2] == 'N' 
                && args[i][3] == 'Z' && args[i][4] == 'V')
            {
               FKO_FLAG |= IFF_VECTORIZE;
               break;
            }
            /*
             * else do what 'G' does
             */
         case 'G':
            /*op = NewOptBlock(atoi(args[i+1]), atoi(args[i+2]), atoi(args[i+3]),
                             args[i][1] == 'G' ? IOPT_GLOB : 0);*/
            op = NewOptBlock(atoi(args[i+1]), atoi(args[i+2]), atoi(args[i+3]),
                             args[i][1] == 'G' ? IOPT_GLOB : IOPT_SCOP);
            i += 3;
            for (j=0; j < op->nopt; j++)
            {
               sp = args[++i];
               if (isdigit(*sp))
                  op->opts[j] = atoi(sp) + MaxOpt;
               else
               {
                  op->opts[j] = -1;
                  for (k=0; k < MaxOpt; k++)
                  {
                     if (!strcmp(optabbr[k], sp))
                     {
                        op->opts[j] = k;
                        break;
                     }
                  }
                  if (op->opts[j] == -1)
                     fko_error(__LINE__, "Unknown optimization '%s'", sp);
               }
#if 0
               switch(*sp)
               {
               case 'r':
                  if (sp[1] == 'a' && sp[2] == '\0')
                     op->opts[j] = RegAsg;
                  else goto ERR;
                  break;
               case 'c':
                  if (sp[1] == 'p' && sp[2] == '\0')
                     op->opts[j] = CopyProp;
                  else goto ERR;
                  break;
               case 'g':
                  if (sp[1] == 'a' && sp[2] == '\0')
                     op->opts[j] = GlobRegAsg;
                  else goto ERR;
                  break;
               case '0':
               case '1':
               case '2':
               case '3':
               case '4':
               case '5':
               case '6':
               case '7':
               case '8':
               case '9':
                  op->opts[j] = atoi(sp) + MaxOpt;
                  break;
               default:
ERR:
                  fko_error(__LINE__, "Unknown optimization '%s'", sp);
               }
#endif
            }
            op->next = obq;
            obq = op;
            break;
         case 'C':
            op = NewOptBlock(atoi(args[i+1]), 0, 3, 0);
            op->ifblk = (void*) 1;
            op->opts[0] = atoi(args[i+2]);
            op->opts[1] = atoi(args[i+3]);
            op->opts[2] = atoi(args[i+4]);
            op->next = obq;
            obq = op;
            i += 4;
            break;
/*
 *    Majedul:
 *       -rc = redundant computation
 *       -mmr = max/min reduction
 *    NOTE: new flag -ra is added to find out the register/value spilling
 *    NOTE: changed them to upper case, like: RC and MMR
 */
#if 0             
         case 'r':
            if (args[i][2] && args[i][2] == 'c')
            {
               STATE1_FLAG |= IFF_ST1_RC; 
            }
            else
            {
               fprintf(stderr, "Unknown flag '%s'\n", args[i]);
               PrintUsageN(args[0]);
            }
            break;
         case 'm':
            if (args[i][2] && args[i][3] 
                  && args[i][2] == 'm' && args[i][3] == 'r')
            {
               STATE1_FLAG |= IFF_ST1_MMR; 
            }
            else
            {
               fprintf(stderr, "Unknown flag '%s'\n", args[i]);
               PrintUsageN(args[0]);
            }
            break;
#endif
         default:
            fprintf(stderr, "Unknown flag '%s'\n", args[i]);
            PrintUsageN(args[0]);
         }
      }
   }

   if (idb)
   {
/*
 *    Keep only last declaration for given array
 */
      for (id=idb; id; id = id->next)
      {
         for (idP=id,idK=id->next; idK; idK = idK->next)
         {
            if (!strcmp(id->name, idK->name))
            {
               idP->next = idK->next;
               free(idK);
               idK = idP;
            }
            idP = idK;
         }
      }
/*
 *    Count # of such arrays, then store them in ARRWNT
 */
      for (id=idb,i=0; id; id = id->next, i++);
      NWNT = i;
      ARRWNT = malloc((i+1) * sizeof(char *));
      assert(ARRWNT);
      ARRWNT[i] = NULL;
      for (id=idb,i=0; i < NWNT; i++, id=id->next)
      {
         j = strlen(id->name)+1;
         ARRWNT[i] = malloc(sizeof(char)*j);
         assert(ARRWNT[i]);
         strcpy(ARRWNT[i], id->name);
      }
   }
   if (aeb)
   {
      n = KillDupPtrinfo(args, aeb);
      AES = PtrinfoToStrings(args, n, aeb);
      AEn = malloc(sizeof(short)*(n+1));
      AEn[0] = n;
      for (pf=aeb,i=1; i <= n; pf=pf->next,i++)
         AEn[i] = pf->flag;
      KillAllPtrinfo(aeb);
   }
/*
 * Majedul: This is to generalize the scalar expansion
 */
   if (seb)
   {
      STATE3_FLAG |= IFF_ST3_SE ; /* to keep track of the opt */
      n = KillDupPtrinfo(args, seb);
      SES = PtrinfoToStrings(args, n, seb);
      SEn = malloc(sizeof(short)*(n+1));
      SEn[0] = n;
      for (pf=seb,i=1; i <= n; pf=pf->next,i++)
         SEn[i] = pf->flag;
      KillAllPtrinfo(seb);
   }

   if (pfb)
   {
/*
 *    Keep only last declaration for given array
 */
      KillDupPtrinfo(args, pfb);
/*
 *    Count number of <name> refs, and find any default ref
 */
      for (i=0,pf0=NULL,pf=pfb; pf; pf = pf->next)
      {
         if (!strcmp(args[pf->ptr], "all"))
         {
            if (!pf0) pf0 = pf;
         }
         else
            i++;
      }
      j = pf0 ? i+1 : i;
      PFARR = malloc(sizeof(char *)*(j+1));
      PFDST = malloc(sizeof(short)*(j+1));
      PFLVL = malloc(sizeof(short)*(j+1));
      assert(PFARR && PFDST && PFLVL);
      PFARR[j] = NULL;
      PFDST[0] = PFLVL[0] = j;
      if (pf0)
      {
         i = 2;
         PFARR[0] = malloc(sizeof(char)*16);
         assert(PFARR[0]);
         strcpy(PFARR[0], "_default");
         PFDST[1] = pf0->flag;
         PFLVL[1] = pf0->nupdate;
      }
      else 
         i = 1;
      for (pf=pfb; pf; pf = pf->next)
      {
         if (strcmp(args[pf->ptr], "all"))
         {
            PFARR[i-1] = malloc(sizeof(char)*(strlen(args[pf->ptr])+1));
            assert(PFARR[i-1]);
            strcpy(PFARR[i-1], args[pf->ptr]);
            PFDST[i] = pf->flag;
            PFLVL[i] = pf->nupdate;
            i++;
         }
      }
      KillAllPtrinfo(pfb);
   }
   if (!rpath)
      rpath = "/tmp";
   if (!rname)
      rname = "FKO";
   sprintf(fST, "%s/%s_ST.", rpath, rname);
   sprintf(fLIL, "%s/%s_LIL.", rpath, rname);
   sprintf(fmisc, "%s/%s_misc.", rpath, rname);
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
   if (fpIG && !(FKO_FLAG & IFF_READINTERM))
   {
      ln[i] = 'I'; ln[i+1] = 'G'; ln[i+2] = '\0';
      if (FKO_FLAG & IFF_READINTERM)
         fpIG = fopen(ln, "r");
      else
         fpIG = fopen(ln, "w");
      assert(fpIG);
   }
   if (fpST)
   {
      ln[i] = 'S'; ln[i+1] = 'T'; ln[i+2] = '\0';
      if (FKO_FLAG & IFF_READINTERM)
         fpST = fopen(ln, "r");
      else
         fpST = fopen(ln, "w");
      assert(fpST);
   }
   if (fpLIL)
   {
      ln[i] = 'L'; ln[i+1] = '\0';
      if (FKO_FLAG & IFF_READINTERM)
         fpLIL = fopen(ln, "r");
      else
         fpLIL = fopen(ln, "w");
      assert(fpLIL);
   }
   if (fpOPT)
   {
      ln[i] = 'o'; ln[i+1] = 'p'; ln[i+2] = 't'; ln[i+3] = '\0';
      if (FKO_FLAG & IFF_READINTERM)
         fpOPT = fopen(ln, "r");
      else
         fpOPT = fopen(ln, "w");
      assert(fpOPT);
   }
   *FIN = fin;
   *FPIN = fpin;
   *FPOUT = fpout;
   return(obq);
}

static void WriteStringArrayToFile(FILE *fp, short n, char **s)
{
   short i, j;

   assert(fwrite(&n, sizeof(short), 1, fp) == 1);
   if (n)
   {
      assert(s);
      for (i=0; i < n; i++)
      {
         assert(s[i]);
         j = strlen(s[i]);
         assert(fwrite(&j, sizeof(short), 1, fp) == 1);
         assert(fwrite(s[i], sizeof(char), j, fp) == j);
      }
   }
}
static char **ReadStringArrayFromFile(FILE *fp)
{
   short n, i, j;
   char **s=NULL;

   assert(fread(&n, sizeof(short), 1, fp) == 1);
   if (n)
   {
      s = malloc((n+1)*sizeof(char*));
      assert(s);
      s[n] = NULL;
      for (i=0; i < n; i++)
      {
         assert(fread(&j, sizeof(short), 1, fp) == 1);
         s[i] = malloc(sizeof(char)*(j+1));
         assert(s[i]);
         assert(fread(s[i], sizeof(char), j, fp) == j);
         s[i][j] = '\0';
      }
   }
   return(s);
}
static void WriteShortArrayToFile(FILE *fp, short n, short *sp)
{
   assert(fwrite(&n, sizeof(short), 1, fp) == 1);
   if (n && sp)
      assert(fwrite(sp, sizeof(short), n, fp) == n);
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
      assert(fread(sp+1, sizeof(short), n, fp) == n);
   }
   return(sp);
}

static void WriteArrayOfShortArrayToFile(FILE *fp, short n, short **sp)
{
   short i;
   assert(fwrite(&n, sizeof(short), 1, fp) == 1);
   for (i=0; i < n; i++)
      WriteShortArrayToFile(fp, sp[i][0], &sp[i][1]);
}

static short **ReadArrayOfShortArrayFromFile(FILE *fp)
{
   short **sp=NULL;
   short i, n;

   assert(fread(&n, sizeof(short), 1, fp) == 1);
   if (n)
   {
      sp = malloc(sizeof(short*)*(n+1));
      assert(sp);
      sp[n] = NULL;
      for (i=0; i < n; i++)
         sp[i] = ReadShortArrayFromFile(fp);
   }
   return(sp);
}

static void WriteState0MiscToFile(char *name)
/*
 * Majedul: This is only to save the new state0
 */
{
   FILE *fp;
   short *sp;
   short n;
   struct locinit *lp;
   extern struct locinit *LIhead;

   fp = fopen(name, "wb");
   assert(fp);
/*
 * 1. Save optloop related information. We don't need to save recomputable info
 *    but need to save those data which are computed during parsing(HIL to LIL).
 *    it will provide us opportunity to restore the program state from files 
 *    offline.
 */
   if (optloop)
   {
      n = 1;
      assert(fwrite(&n, sizeof(short), 1, fp) == 1);
/*
 *    Need to save information which is computed during parsing:
 *    int flag 
 *    short I, beg, end, inc, body_label, end_label
 *    short maxunroll
 *    NOTE: We saved the full structure, but need to invalidated all the 
 *    pointers when restored.
 */
      assert(fwrite(optloop, sizeof(LOOPQ), 1, fp) == 1);
/*
 *    need to save all list coming from markup while parsing, like:
 *    aaligned, abalign, falign
 *    nopf is added now
 */
      n = optloop->aaligned ? optloop->aaligned[0]: 0;
      WriteShortArrayToFile(fp, n, optloop->aaligned+1);
      n = optloop->abalign ? optloop->abalign[0]: 0;
      WriteShortArrayToFile(fp, n, optloop->abalign+1);
      
      n = optloop->maaligned ? optloop->maaligned[0]: 0;
      WriteShortArrayToFile(fp, n, optloop->maaligned+1);
      n = optloop->mbalign ? optloop->mbalign[0]: 0;
      WriteShortArrayToFile(fp, n, optloop->mbalign+1);
      
      n = optloop->faalign ? optloop->faalign[0]: 0;
      WriteShortArrayToFile(fp, n, optloop->faalign+1);
      n = optloop->fbalign ? optloop->fbalign[0]: 0;
      WriteShortArrayToFile(fp, n, optloop->fbalign+1);

      n = optloop->nopf ? optloop->nopf[0]: 0;
      WriteShortArrayToFile(fp, n, optloop->nopf+1);
   }
   else
   {
      n = 0;
      assert(fwrite(&n, sizeof(short), 1, fp) == 1);
   }
/*
 * 2. Need to save structure for const initialization as it is constructed 
 *    during parsing.
 */
   if (LIhead)
   {
      for (n=0,lp=LIhead; lp; lp = lp->next) n++;
      sp = malloc(n*sizeof(short));
      assert(sp);
      for (n=0,lp=LIhead; lp; lp = lp->next)
         sp[n++] = lp->id;
      WriteShortArrayToFile(fp, n, sp);
      for (n=0,lp=LIhead; lp; lp = lp->next)
      {
/*         fprintf(stderr, "W id=%d, ptr=%d\n", lp->id, lp->con); */
         sp[n++] = lp->con;
      }
      WriteShortArrayToFile(fp, n, sp);
      free(sp);
   }
   else
   {
      n = 0;
      assert(fwrite(&n, sizeof(short), 1, fp) == 1);
   }
/*
 * 3. Need to save global data which is generated from GetFlagsN(). 
 *    We can restore the program state from files offline. 
 */
      n = PFDST ? PFDST[0] : 0;
      WriteStringArrayToFile(fp, n, PFARR);
      WriteShortArrayToFile(fp, n, PFLVL+1);
      WriteShortArrayToFile(fp, n, PFDST+1);
      WriteStringArrayToFile(fp, NWNT, ARRWNT);
      fclose(fp);
}


static void WriteMiscToFile(char *name)
{
   FILE *fp;
   short *sp;
   short n;
   struct locinit *lp;
   extern struct locinit *LIhead;

   fp = fopen(name, "wb");
   assert(fp);
   if (optloop)
   {
      n = 1;
      assert(fwrite(&n, sizeof(short), 1, fp) == 1);
      assert(fwrite(optloop, sizeof(LOOPQ), 1, fp) == 1);
      WriteShortArrayToFile(fp, optloop->varrs ? optloop->varrs[0] : 0, 
                            optloop->varrs+1);
      n = optloop->vscal ? optloop->vscal[0] : 0;
      WriteShortArrayToFile(fp, n, optloop->vscal+1);
      WriteShortArrayToFile(fp, n, optloop->vsflag+1);
      WriteShortArrayToFile(fp, n, optloop->vsoflag+1);
      WriteShortArrayToFile(fp, n, optloop->vvscal+1);
      n = optloop->pfarrs ? optloop->pfarrs[0] : 0;
      WriteShortArrayToFile(fp, n, optloop->pfarrs+1);
      WriteShortArrayToFile(fp, n, optloop->pfdist+1);
      n = optloop->ne ? optloop->ne[0] : 0;
      WriteShortArrayToFile(fp, n, optloop->ae+1);
      WriteShortArrayToFile(fp, n, optloop->ne+1);
      WriteArrayOfShortArrayToFile(fp, n, optloop->aes);
      assert(!optloop->abalign);  /* fix this later */
      n = PFDST ? PFDST[0] : 0;
      WriteStringArrayToFile(fp, n, PFARR);
      WriteShortArrayToFile(fp, n, PFLVL+1);
      WriteShortArrayToFile(fp, n, PFDST+1);
   }
   else
   {
      n = 0;
      assert(fwrite(&n, sizeof(short), 1, fp) == 1);
   }
   if (LIhead)
   {
      for (n=0,lp=LIhead; lp; lp = lp->next) n++;
      sp = malloc(n*sizeof(short));
      assert(sp);
      for (n=0,lp=LIhead; lp; lp = lp->next)
         sp[n++] = lp->id;
      WriteShortArrayToFile(fp, n, sp);
      for (n=0,lp=LIhead; lp; lp = lp->next)
      {
/*         fprintf(stderr, "W id=%d, ptr=%d\n", lp->id, lp->con); */
         sp[n++] = lp->con;
      }
      WriteShortArrayToFile(fp, n, sp);
      free(sp);
   }
   else
   {
      n = 0;
      assert(fwrite(&n, sizeof(short), 1, fp) == 1);
   }
   WriteStringArrayToFile(fp, NWNT, ARRWNT);
   fclose(fp);
}

static void KillArrayOfStr(int n, char **str)
{
   int i;
   if (!str)
      return;
   for (i=0; i < n; i++)
      if (str[i])
         free(str[i]);
   free(str);
}

static void ReinitAllStatic4bitvec()
/*
 * to initialize all the static vars after Killing the bitVec entry
 */
{
   extern INT_BVI FKO_BVTMP;
/*
 * Majedul: we have following functions which use local static variable for 
 * bit vectors:
 *    1. Reg2Regstate(-1); --- works
 *    2. FindLiveregs(NULL); ---works
 *    3. AllRegVec(-1); ---added, not tested yet
 *    4. Scope2BV(NULL); --- works
 *    5. CalcScopeIG(NULL); --- added, not tested yet
 *    6. FindReadUseType(NULL,0,0); --- added, not tested yet
 *    7. CalcBlocksDeadVariables(NULL);
 *    8. Array2BitVecFlagged(0, NULL,0, -1)....
 *    9. BlockList2BitVecFlagegd(NULL, -1).....
 */

   Reg2Regstate(-1); 
   FindLiveregs(NULL); 
   AllRegVec(-1); 
   Scope2BV(NULL); 
   CalcScopeIG(NULL); 
   FindReadUseType(NULL,0,0); 
   CalcBlocksDeadVariables(NULL);
   Array2BitVecFlagged(0, NULL,0, -1);
   BlockList2BitVecFlagged(NULL, -1);

   if (FKO_BVTMP) KillBitVec(FKO_BVTMP);
   FKO_BVTMP = 0;
}

static void ReadState0MiscFromFile(char *name)
{
   int i;
   short n;
   FILE *fp;
   short *sp, *s;
   extern struct locinit *LIhead;
   extern LOOPQ *loopq;
/*
 * Kill current optloop and restore the old one
 * NOTE: need to kill all loops in loopq
 * Majedul:
 * HERE HERE, We added a new loop killing function which would kill all the 
 * elements of loop. We don't need to preserve any info which is calculated 
 * during program states as we restore in program state0. Only the information 
 * which is added during parsing is needed; it can be saved and restore from 
 * file.
 */
#if 0   
   while(loopq)
   {
      if (loopq == optloop)
         loopq = loopq->next; /* delete optloop loop later */
      else      
      loopq = KillLoop(loopq);
   }

   if (optloop) KillLoop(optloop);  /* delete optloop safely now */
#else
   #if 0
      KillAllLoopsComplete();
   #else
   while(loopq)
   {
      if (loopq == optloop)
         loopq = loopq->next; /* delete optloop loop later */
      else      
      loopq = KillFullLoop(loopq);
   }
/*
 * FIXME: can't kill optloop like this now. We have optloop->falign
 * which is set from markup while parsing the array. we can't free them.
 * generalize all loop killers 
 */
   if (optloop) KillFullLoop(optloop);  /* delete optloop safely now */
   #endif
#endif
   optloop = NewLoop(0);
   fp = fopen(name, "rb");
   assert(fp);
   assert(fread(&n, sizeof(short), 1, fp) == 1);
   if (n)
   {
      assert(fread(optloop, sizeof(LOOPQ), 1, fp) == 1);
      optloop->aaligned = ReadShortArrayFromFile(fp);
      optloop->abalign = ReadShortArrayFromFile(fp);
      optloop->maaligned = ReadShortArrayFromFile(fp);
      optloop->mbalign = ReadShortArrayFromFile(fp);
      optloop->faalign = ReadShortArrayFromFile(fp);
      optloop->fbalign = ReadShortArrayFromFile(fp);
      optloop->nopf = ReadShortArrayFromFile(fp);
/*
 * Need to check whether all the information is loaded successfully
 * Ofcourse, need to mark all pointers as NULL
 * FIXME: For state0, we need to restore all the information which is set using
 * markup in parsing phase. 
 */
#if 0
      fprintf(stderr, "optloop->flag = %d\n", optloop->flag);
      fprintf(stderr, "optloop->I = %d\n", optloop->I);
      fprintf(stderr, "optloop->beg = %d\n", optloop->beg);
      fprintf(stderr, "optloop->end = %d\n", optloop->end);
      fprintf(stderr, "optloop->inc = %d\n", optloop->inc);
      fprintf(stderr, "optloop->body_label = %d\n", optloop->body_label);
      fprintf(stderr, "optloop->end_label = %d\n", optloop->end_label);
      fprintf(stderr, "optloop->maxunroll = %d\n", optloop->maxunroll);
#endif  
/*
 * initialize/invalidate all data which are recomputed.
 * 
 * NOTE:  need to figure out what would be the initial value for the following
 *        [all of these would be recomputed afterward ]
 *          optloop->vflag
 *          optloop->ndup
 *          optloop->depth
 *          optloop->CU_label
 *          optloop->PTCU_label
 *          optloop->NE_label
 *          optloop->loopnum
 *          optloop->writedd
 *          optloop->blkvec
 *          optloop->outs
 *          optloop->sets
 */
/*
 *    NOTE: There are options to set some elements from markup like: 
 *    nopf, aaligned, abalign, maxunroll, writedd, etc (see: hil_gram.y: 317)
 *    but those are not fully implemented. Need to update here during fixing 
 *    them. 
 */
/*
 *    NOTE: unmark the element which is set during parsing of the HIL.
 *    Other than those, reset all the elements.
 *    This is strickly for state0... in different state, we may need to 
 *    invalidate different pointer
 */
      optloop->blkvec = 0;
      optloop->outs = 0;
      optloop->sets = 0;
      optloop->ndup = 0;

      optloop->varrs = NULL;
      optloop->vscal = NULL;
      optloop->vsflag = NULL;
      optloop->vsoflag = NULL;
      optloop->vvscal = NULL;
      optloop->bvvscal = NULL;
      optloop->vvinit = NULL;
      optloop->pfarrs = NULL;
      optloop->pfdist = NULL;
      optloop->pfflag = NULL;
      optloop->ae = NULL;
      optloop->ne = NULL;
      optloop->aes = NULL;
      optloop->se = NULL;
      optloop->nse = NULL;
      optloop->ses = NULL;
      optloop->seflag = NULL;
      optloop->maxvars = NULL;
      optloop->minvars = NULL;
      optloop->preheader = NULL;
      optloop->header = NULL;
      optloop->tails = NULL;
      optloop->posttails = NULL;
      optloop->blocks = NULL;
      optloop->next = NULL;
   }
   else 
      optloop = NULL;
/*
 * Kill LIhead and restore the old one 
 */   
   KillAllLI(LIhead);
   LIhead = NULL;
   sp = ReadShortArrayFromFile(fp);
   if (sp)
   {
      s = ReadShortArrayFromFile(fp);
      for (i=1; i <= s[0]; i++)
      {
/*         fprintf(stderr, "R id=%d, ptr=%d\n", sp[i], s[i]); */
         LIhead = NewLI(sp[i], s[i], LIhead);
      }
      free(sp);
      free(s);
   }
/*
 * load all the global data which are needed to restore the states offline.
 * NOTE: need to store the num also to restore it ofline
 */
   i = PFDST ? PFDST[0] : 0; /* need to save it in file also*/
   KillArrayOfStr(i, PFARR);
   if (PFLVL)
      free(PFLVL);
   if (PFDST)
      free(PFDST);
   PFARR = ReadStringArrayFromFile(fp);
   PFLVL = ReadShortArrayFromFile(fp);
   PFDST = ReadShortArrayFromFile(fp);

   if (NWNT && ARRWNT)
   {
      for (i=0; i < NWNT; i++)
         if (ARRWNT[i])
            free(ARRWNT[i]);
      free(ARRWNT);
   }
   ARRWNT = ReadStringArrayFromFile(fp);
   if (ARRWNT)
      for (NWNT=0; ARRWNT[NWNT]; NWNT++);
   else NWNT = 0;
   fclose(fp);
/*
 * Now, we need to free all global data which are re-computed in later states
 * like: Bitvec. 
 * NOTE: must reinitialize all the static vars and Kill associate bit vector
 * before kill all.
 */
   ReinitAllStatic4bitvec();
   KillAllBitVec(); 
/*
 * what other info we need to free!!
 */

}

void KillAllGlobalData(struct optblkq *optblks)
/*
 * This function is called at the exit of the program to free all memory, 
 * by this way, we can check whether there is any memory leak using Valgrind.
 */
{
   int i;
   extern struct locinit *ParaDerefQ;
   extern struct locinit *LIhead;

   KillAllOptBlocks(optblks); /* delete optblkq*/ 
   if (ParaDerefQ)
   {
      KillAllLocinit(ParaDerefQ);
      ParaDerefQ = NULL;
   }
   KillAllLI(LIhead);
   LIhead = NULL;
   
   KillSTStrings();
   if (SToff) free(SToff);
   if (STflag) free(STflag);
   if (STpts2) free(STpts2);
   if (DTcon) free(DTcon);
   KillSTarr();
#if 0 
   KillAllLoopsComplete();
#else
   while(loopq)
   {
      if (loopq == optloop)
         loopq = loopq->next; /* delete optloop loop later */
      else      
      loopq = KillFullLoop(loopq);
   }
   if (optloop) KillFullLoop(optloop);  /* delete optloop safely now */
#endif
   KillAllBasicBlocks(bbbase);
   bbbase = NULL; /* whenever kill, make it NULL */
   ReinitAllStatic4bitvec();
   KillAllBitVec(); 
   
   i = PFDST ? PFDST[0] : 0;
   KillArrayOfStr(i, PFARR);
   if (PFLVL)
      free(PFLVL);
   if (PFDST)
      free(PFDST);
   
   if (NWNT && ARRWNT)
   {
      for (i=0; i < NWNT; i++)
         if (ARRWNT[i])
            free(ARRWNT[i]);
      free(ARRWNT);
   }
}

static void ReadMiscFromFile(char *name)
{
   int i;
   short n;
   FILE *fp;
   short *sp, *s;
   extern struct locinit *LIhead;

#if 0
   if (optloop)
   {
      for (lp=loopq; lp && lp != optloop; lp = lp->next);
      if (!lp)
         KillLoop(optloop);
   }
   KillAllLoops();
#else
   InvalidateLoopInfo();
   KillLoop(optloop);
#endif
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
      optloop->varrs = ReadShortArrayFromFile(fp);
      optloop->vscal  = ReadShortArrayFromFile(fp);
      optloop->vsflag  = ReadShortArrayFromFile(fp);
      optloop->vsoflag  = ReadShortArrayFromFile(fp);
      optloop->vvscal  = ReadShortArrayFromFile(fp);
      optloop->pfarrs  = ReadShortArrayFromFile(fp);
      optloop->pfdist  = ReadShortArrayFromFile(fp);
      optloop->ae      = ReadShortArrayFromFile(fp);
      optloop->ne      = ReadShortArrayFromFile(fp);
      optloop->aes     = ReadArrayOfShortArrayFromFile(fp);
      optloop->abalign = NULL;  /* handle this later */
      optloop->preheader = optloop->header = NULL;
      optloop->tails = optloop->posttails = NULL;
/*
 *    Majedul: constructing blocks from bitvec may not work. 
 *    We can recompute everything.
 *    FIXME: How can we get blkvec if it is not saved!!! 
 */
      if (optloop->blkvec)
         optloop->blocks = BitVec2BlockList(optloop->blkvec);
      else
         optloop->blocks = NULL;
      optloop->next = NULL;
      i = PFDST ? PFDST[0] : 0;
      KillArrayOfStr(i, PFARR);
      if (PFLVL)
         free(PFLVL);
      if (PFDST)
         free(PFDST);
      PFARR = ReadStringArrayFromFile(fp);
      PFLVL = ReadShortArrayFromFile(fp);
      PFDST = ReadShortArrayFromFile(fp);
   }
   else 
      optloop = NULL;
   KillAllLI(LIhead);
   LIhead = NULL;
   sp = ReadShortArrayFromFile(fp);
   if (sp)
   {
      s = ReadShortArrayFromFile(fp);
      for (i=1; i <= s[0]; i++)
      {
/*         fprintf(stderr, "R id=%d, ptr=%d\n", sp[i], s[i]); */
         LIhead = NewLI(sp[i], s[i], LIhead);
      }
      free(sp);
      free(s);
   }
   if (NWNT && ARRWNT)
   {
      for (i=0; i < NWNT; i++)
         if (ARRWNT[i])
            free(ARRWNT[i]);
      free(ARRWNT);
   }
   ARRWNT = ReadStringArrayFromFile(fp);
   if (ARRWNT)
      for (NWNT=0; ARRWNT[NWNT]; NWNT++);
   else NWNT = 0;
   fclose(fp);
}


void RestoreFKOState0()
/*
 * Majedul: 
 * This is only to restore program states from saved state0.
 * We need to kill/free all the unnecessary global data which is not already
 * computed in during state0
 */
{
   char ln[1024];
   extern BBLOCK *bbbase;
   extern struct locinit *ParaDerefQ;
/*
 * free ParaDerefQ if it is computed. ParaDerefQ is computed at the end during
 * finalizing the epilogue and prologue. 
 */
   if (ParaDerefQ)
   {
      KillAllLocinit(ParaDerefQ);
      ParaDerefQ = NULL;
   }
/*
 * No repeatable opt yet in state0
 */
   noptrec = 0;
/*
 * Restore related data structure for Symbol table: 
 *    1. global flag like: DTnzerod, Dtabsd, etc.
 *    2. glabal data like: STderef, N, niloc, nlloc, nfloc, ... NPARA, etc
 *    3. all data structure for Symbol table: SToff, STflag, STpts2, STName
 */
   sprintf(ln, "%s%d", fST, 0);
   ReadSTFromFile(ln);
/*
 * Restore basic block bbbase
 * NOTE: need to verify: is there any info related to optloop in bbbase
 */
   sprintf(ln, "%s%d", fLIL, 0);
   ReadLILFromBinFile(ln);
/*
 * Majedul: 
 * Restoring information like:
 *    1. Optloop: I, beg, end, inc, flag, body_label, end_label, CU_label
 *       TCU_label, NE_label, maxunroll, etc.
 *    2. LIhead: this is for const initialization which is built during parsing
 *    3. Global data structure for main program which are generated from 
 *       GetFlagsN() so that we can restore whole program from file 
 */
   sprintf(ln, "%s%d", fmisc, 0);
   ReadState0MiscFromFile(ln);
/*
 * All annotation must be done afresh
 */
   CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = CFLOOP = 0;
}


void RestoreFKOState(int isav)
{
   char ln[1024];
   extern BBLOCK *bbbase;
   extern struct locinit *ParaDerefQ;

   if (isav < 2 && ParaDerefQ)
   {
      KillAllLocinit(ParaDerefQ);
      ParaDerefQ = NULL;
   }
   noptrec = 0;
   sprintf(ln, "%s%d", fST, isav);
   ReadSTFromFile(ln);
   sprintf(ln, "%s%d", fLIL, isav);
   ReadLILFromBinFile(ln);
   sprintf(ln, "%s%d", fmisc, isav);
   ReadMiscFromFile(ln);
/*
 * All annotation must be done afresh
 */
   CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = CFLOOP = 0;
}

void SaveFKOState(int isav)
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
 * 7/20/04 : what about bitvecs?  Need to be written to file if we want to
 *           be able to restore from previously created files
 *
 * DONE: 1(inst.c), 2,3(symtab.c)
 */
{
   char ln[1024];
   extern BBLOCK *bbbase;
        
   sprintf(ln, "%s%d", fST, isav);
   WriteSTToFile(ln);
   sprintf(ln, "%s%d", fLIL, isav);
   WriteLILToBinFile(ln, bbbase);
   sprintf(ln, "%s%d", fmisc, isav);
   WriteMiscToFile(ln);
}

void SaveFKOState0()
/*
 * Majedul: 
 * this function is used only to save the state0. Saving and restoring multiple
 * states would complicate the procedure. So, right now, I just consider to save
 * state0 and recompute all states from state0 if needed.
 */
{
   char ln[1024];
   extern BBLOCK *bbbase;     

/*
 * saving following global data related to symbol table:
 *    1. global flag like: DTnzerod, Dtabsd, etc.
 *    2. glabal data like: STderef, N, niloc, nlloc, nfloc, ... NPARA, etc
 *    3. all data structure for Symbol table: SToff, STflag, STpts2, STName
 */
   sprintf(ln, "%s%d", fST, 0);
   WriteSTToFile(ln);
/*
 * saving bbbase block structure
 */
   sprintf(ln, "%s%d", fLIL, 0);
   WriteLILToBinFile(ln, bbbase);
/*
 * Majedul: 
 * we don't need to save re-computable information of optloop like: varrs, vscal,
 * ... ... ae, aes, etc. we just need to save all those marker which is found
 * while parsing the HIL to LIL. 
 * saving information like:
 *    1. Optloop: I, beg, end, inc, flag, body_label, end_label, CU_label
 *       TCU_label, NE_label, maxunroll, etc.
 *    2. LIhead: this is for const initialization which is built during parsing
 *    3. Global data structure for main program which are generated from 
 *       GetFlagsN() so that we can restore whole program from file 
 */

   sprintf(ln, "%s%d", fmisc, 0);
   WriteState0MiscToFile(ln); /* this function is diff than old one */
}

int GetNumLoopScopes()
/*
 * returns total number of loop scopes
 */
{
   int i;
   LOOPQ *lpq;
   extern LOOPQ *loopq;

   if (!CFLOOP)
      FindLoops();
   lpq = loopq;
   i = 0;
   while(lpq)
   {
      i++;
      lpq = lpq->next;
   }
   return(i);
}

BLIST *GetLoopScopebyNum(int id)
/*
 * returns blist for specific loop scope... scope number starts from 0.. 
 * If number of scopes is N, then id N means the global scope
 */
{
   int i;
   LOOPQ *lpq;
   BLIST *scope;
   BBLOCK *bp;
   extern LOOPQ *loopq;
   extern BBLOCK *bbbase;

   if (!CFLOOP)
      FindLoops();

   lpq = loopq;
   i = 0;
   while(lpq)
   {
      if (i == id)
      {
         /*scope = CopyBlockList(lpq->blocks);*/
         scope = NewReverseBlockList(lpq->blocks);
         return(scope);
      }
      i++;
      lpq = lpq->next;
   }

   if (i == id) /* id == N, means it's global*/
   {
      for (scope=NULL,bp=bbbase; bp; bp = bp->down)
         scope = AddBlockToList(scope, bp);
      scope = ReverseBlockList(scope);
   }
   else /* invalid id*/
   {
      scope = NULL;
   }
   
   return(scope);
}

BLIST **LoopBlocks(int *N)
/*
 * returns the blocklist of each loop: from depth most to upper 
 * (staring with optloop)
 */
{
   int i;
   LOOPQ *lpq;
   BLIST **scopes;
   extern LOOPQ *loopq;
   
   if (!CFLOOP)
      FindLoops();

   lpq = loopq;
   i = 0;
   while(lpq)
   {
      i++;
      lpq = lpq->next;
   }
   *N = i;

   scopes = malloc((*N)*sizeof(BLIST*));
   assert(scopes);

   for (i=0, lpq=loopq; i < *N; i++, lpq=lpq->next)
      scopes[i] = CopyBlockList(lpq->blocks);
   
   return(scopes);
}

int HasScopeIGReg(int nopt, enum FKOOPT *ops)
{
   int i;

   for (i=0; i < nopt; i++)
   {
      if (ops[i] == RegAsg)
         return 1;
   }
   return(0);
}

int CheckPreheaderPosttails(BLIST *scope)
{
   int nerr = 0, pred =0, npred = 0;
   BBLOCK *bp;
   BLIST *bl, *blp;
   INT_BVI blkvec;
   extern INT_BVI FKO_BVTMP;
   extern BBLOCK *bbbase;

#if 1   
   CheckFlow(bbbase, __FILE__, __LINE__);
#endif

   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(32);
   blkvec = FKO_BVTMP;
   SetVecAll(blkvec, 0);

   for (bl=scope; bl; bl=bl->next)
      SetVecBit(blkvec, bl->blk->bnum-1, 1);

   for (bl=scope; bl; bl=bl->next) 
   {
      bp = bl->blk;
/*
 *    check for single pre-header: 
 *    1. no other succ other than head
 *    2. succ/head has no other pred than prehead
 */
      pred = 0;
      for (blp=bp->preds; blp; blp=blp->next)
      {
         if (!BitVecCheck(blkvec, blp->blk->bnum-1)) /* pred not in scope */
         {
            pred = 1;
            if ( (blp->blk->usucc && blp->blk->usucc != bp) 
                  || (blp->blk->csucc && blp->blk->csucc != bp) )
            {
               nerr++;
            /*fprintf(stderr,"preheader blk-%d has successor other header-%d\n", 
                       blp->blk->bnum, bp->bnum);*/
               fko_warn(__LINE__,
                     "preheader blk-%d has successor other header-%d\n",
                       blp->blk->bnum, bp->bnum);
            }
         }
      }
      if (pred)
         npred++;
/*
 *    check for posttails: posttail should not have pred other than bp 
 */
      if (bp->usucc && !BitVecCheck(blkvec, bp->usucc->bnum-1))/*not in scope*/
      {
         if (bp->usucc->preds->next) // bp->usucc->preds->blk != bp 
         {
            nerr++;
            fko_warn(__LINE__, "Posttail blk-%d has pred other than blk-%d\n",
                   bp->usucc->bnum, bp->bnum);
         }
      }
      
      if (bp->csucc && !BitVecCheck(blkvec, bp->csucc->bnum-1)) /*not in scope*/
      {
         if (bp->csucc->preds->next) // bp->csucc->preds->blk != bp 
         {
            nerr++;
            fko_warn(__LINE__, "Posttail blk-%d has pred other than blk-%d\n",
                   bp->csucc->bnum, bp->bnum);
         }
      }
      
   }
   if (npred > 1)
   {
      nerr++;
      fko_warn(__LINE__, "More than one preheader for the scope\n");
   }
#if 0
   if (nerr)
   {
      fprintf(stderr, "scope=%s\n", PrintBlockList(scope));
      extern BBLOCK *bbbase;
      ShowFlow("cfg-error.dot", bbbase);
   }
#endif
   return nerr;
}

void AddPredBlk(INT_BVI scopeblks, BBLOCK *blk)
/*
 * Add preheader block in CFG if there is no preheader in the scope
 */
{
   short lb;
   INSTQ *ip;
   BBLOCK *bp, *bnew;
   char label[64];
   extern BBLOCK *bbbase;
   
/*
 * label of the header is unique. add PH_ as a prefix with this label to create'
 * the unique label of preheader
 */
   assert(blk->ilab); 
   sprintf(label, "PH_%s", STname[blk->ilab-1]);
   lb = STlabellookup(label);
   
   bnew = NewBasicBlock(blk->up, blk);
   InsNewInst(bnew, NULL, NULL, LABEL, lb, 0, 0 );
   
   blk->up->down = bnew;
   blk->up = bnew;
/*
 * update all label from ilab to lb which is out of scope
 */

   for (bp=bbbase; bp; bp=bp->down)
   {
      if (!BitVecCheck(scopeblks, bp->bnum-1))
      {
         for (ip=bp->ainst1; ip; ip=ip->next)
         {
            if (ip->inst[0] == JMP && ip->inst[2] == blk->ilab)
               ip->inst[2] = lb;
            else if (IS_COND_BRANCH(ip->inst[0]) && ip->inst[3] == blk->ilab)
               ip->inst[3] = lb;
         }
      }
   }

}


void FixPreheaderPosttails(BLIST *scope)
{
   int npred = 0;
   BBLOCK *bp, *bnew;
   BLIST *bl, *blp;
   INT_BVI blkvec;
   char label[64];
   extern INT_BVI FKO_BVTMP;
   
   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(32);
   blkvec = FKO_BVTMP;
   SetVecAll(blkvec, 0);

   for (bl=scope; bl; bl=bl->next)
      SetVecBit(blkvec, bl->blk->bnum-1, 1);


   for (bl=scope; bl; bl=bl->next) 
   {
      bp = bl->blk;
/*
 *    checking for single preheader
 */
      npred = 0;      
      for (blp=bp->preds; blp; blp=blp->next)
      {
         if (!BitVecCheck(blkvec, blp->blk->bnum-1)) /* pred not in scope */
         {
            if ( (blp->blk->usucc && blp->blk->usucc != bp) 
                  || (blp->blk->csucc && blp->blk->csucc != bp) )
            {
               npred++;        
            }
         }
      }
      if (npred)
      {
         AddPredBlk(blkvec, bp);
      }
/*
 *    checking for posttails: if they are usucc of tails
 */
      if (bp->usucc && !BitVecCheck(blkvec, bp->usucc->bnum-1))/*not in scope*/
      {
         if (bp->usucc->preds->next) // bp->usucc->preds->blk != bp 
         {
            /*fprintf(stderr, "adding blk after %d before %d\n", bp->bnum, 
                  bp->usucc->bnum);*/
            fko_warn(__LINE__, "Adding blk after %d before %d\n", bp->bnum, 
                  bp->usucc->bnum);
/*
 *          label of the tail is unique. add PT_ as a prefix with this label to
 *          create the unique label of posttail
 */
            assert(bp->usucc->ilab);
            sprintf(label, "PT_%s", STname[bp->usucc->ilab-1]);
            bnew = NewBasicBlock(bp, bp->usucc);
            InsNewInst(bnew, NULL, NULL, LABEL, STlabellookup(label), 0, 0);
            bp->usucc->up = bnew;
            bp->down = bnew;
         }
      }
/*
 *    posttail can be a csucc of tail, but we won't have any usecase of it 
 *    right now. So, I skipped that. Creating posttail of such case would be
 *    similar like the above.
 */
      if (bp->csucc && !BitVecCheck(blkvec, bp->csucc->bnum-1))/*not in scope*/
      {
         if (bp->csucc->preds->next) // bp->usucc->preds->blk != bp 
         {
            fko_error(__LINE__, 
                  "Posttail error: need to create posttail for this case");
         }
      }

   }
}

int DoOptList(int nopt, enum FKOOPT *ops, int iscope0, int global, int **nspill)
/*
 * Performs the nopt optimization in the sequence given by ops, returning
 * 0 if no transformations applied, nonzero if some changes were made
 * Now, we have two states
 *    1. IOPT_GLOB = all blocks inside the routine
 *    2. IOP_SCOP = blk list specified in optblks->blocks
 *  0  =  optloop is not used anymore
 */
{
   int i, j, k, nchanges=0, nc0;
   int *nsp;
   BLIST *scope;
   BBLOCK *bp;
   static short nlab=0, labs[4];
   /*static int iopt = 0, bv = 0;*/ /* Majedul: for opt logger, bv -> */
   static int iopt = 0, bv = 0;
   /*extern LOOPQ *optloop;*/
   extern BBLOCK *bbbase;;
  
/*
 * Form scope based on global setting
 */
   scope = NULL;

   if (!global) /* IOPT_SCOP */ 
   {
      scope = GetLoopScopebyNum(iscope0);
      if (!scope)
         return(0);
      else /* has loops, valid scope */
      {
/*
 *       if we have IgReg in the packet, we need to check whether it has proper
 *       preheader/posttails
 */
         if ( HasScopeIGReg(nopt, ops) && CheckPreheaderPosttails(scope))
         {
            FixPreheaderPosttails(scope);
/*
 *          update CFG now
 */
            InvalidateLoopInfo();
            bbbase = NewBasicBlocks(bbbase);
            CheckFlow(bbbase, __FILE__, __LINE__);
            FindLoops();
            CheckFlow(bbbase, __FILE__, __LINE__);
/*
 *          delete previous scope and recompute it again       
 */
            free(scope);
            scope = GetLoopScopebyNum(iscope0);
         }
      }
   }
   else /* IOPT_GLOB */
   {
      for (scope=NULL,bp=bbbase; bp; bp = bp->down)
         scope = AddBlockToList(scope, bp);
      scope = ReverseBlockList(scope);
   }
#if 0
   fprintf(stderr, "DoOpt scope = %s\n",PrintBlockList(scope));
   fprintf(stderr, "******************************************\n\n");
#endif
/*
 * NOTE: Need a way to eliminate _IFKO_EPILOGUE iff this is last optimization
 */
   if (!nlab)
   {
      nlab = 2;
      labs[0] = STlabellookup(rout_name);
      labs[1] = STlabellookup("_IFKO_EPILOGUE");
   }
   if (!scope)
      return(0);
/*
 * Now, perform the optimizations specified in the optblk
 */
   for (i=0; i < nopt; i++)
   {
      nc0 = nchanges;
      k = ops[i];
      switch(k)
      {
      case GlobRegAsg:
/*
 *       NOTE: it is normally not used when vect is applied !!!!!! 
 *       I made this obsolete for scalar code too. So, this opt has no longer
 *       been used!
 */
         fko_error(__LINE__, "\n Global Reg Asg is obsolete now, "
                   "change optblk's configuration !!!\n\n");
         /* nchanges += DoLoopGlobalRegAssignment(optloop);*/
         break;
      case RegAsg:
/*
 *       create space for reg info
 */
         nsp = malloc(NTYPES*sizeof(int));
         assert(nsp);
/*
 *       now, perform the IG reg asg 
 */
         nchanges += DoScopeRegAsg(scope, global ? 2:1, &j, nsp); 
/*
 *       update nspill 
 *       NOTE: we will save nspill only for the optloop when IOPT_SCOP is 
 *       applied, otherwise for global scope. we don't save nspill for other
 *       scopes now, but can do it by changing the following code.
 */
         if (global || !iscope0) /* scope no. 0 represents the optloop */
         {
            if (*nspill) /* already has old reg info, delete it */
            {
               for (j=0; j < NTYPES; j++)
                  (*nspill)[j] = nsp[j];
               free(nsp);
            }
            else 
               *nspill = nsp;
         }
         else 
            free(nsp); /* delete other nspill info */
         break;
      case CopyProp:
         nchanges += DoCopyProp(scope);
         #if IFKO_DEBUG_LEVEL >= 1         
            CheckUseSet();
         #endif         
         break;
      case RemoveOneUseLoads:
         nchanges += DoRemoveOneUseLoads(scope);
         break;
      case LastUseLoadRemoval:
         nchanges += DoLastUseLoadRemoval(scope);
         break;
      case ReverseCopyProp:
         nchanges += DoReverseCopyProp(scope);
         #if IFKO_DEBUG_LEVEL >= 1         
            CheckUseSet();
         #endif         
         break;
      case EnforceLoadStore:
         nchanges += DoEnforceLoadStore(scope);
         break;
      case DoNothing:  /* dummy opt does nothing */
         break;
      case UselessLabElim:
         nchanges += DoUselessLabelElim(nlab, labs);
         break;
      case UselessJmpElim:
         nchanges += DoUselessJumpElim();
         break;
      case BranchChain:
         nchanges += DoBranchChaining();
         break;
      default:
         fko_error(__LINE__, "Unknown optimization %d\n", ops[i]);
      }
      optchng[noptrec] = nchanges - nc0;
      optrec[noptrec++] = global ? k+MaxOpt : k;

   #if 0
/*
 *    to print the log of all optimizations who make changes
 */
      if (nchanges-nc0)
      {
         PrintOptInst(stdout, ++iopt, k, scope, global, nchanges-nc0);
      }
   #endif
   }
   if (scope) KillBlockList(scope);
   return(nchanges);
}

#if 0
LOOPQ *LoopOrder()
{
   int i;
   extern LOOPQ *loopq;
   LOOPQ *lp, *lp0;
  
   lp0 = loopq;
   
   fprintf(stderr, "%d -> ", lp0->loopnum);
   
   for (i=loopq->depth-1; i > 0; i--)
   {
      for (lp=loopq; lp; lp=lp->next)
      {
         if (lp->depth == i && FindBlockInList(lp->blocks, lp0->header))
         {
            fprintf(stderr, "%d -> ", lp->loopnum);
            break;
         }
      }
      lp0 = lp;
   }
   return(lp0);
}
#endif

int DoOptBlock(int igscope, int ilscope, struct optblkq *op)
/*
 * returns: nchanges applied
 */
{
   int i, nc, tnc=0, global;
   struct optblkq *dp;
   int iscope, maxN;

   global = op->flag & IOPT_GLOB;
   iscope = global ? igscope : ilscope;
/*
 * if we have conditional optimization block, handle it
 */   
   if (op->ifblk)
   {
      tnc = DoOptBlock(igscope, ilscope, op->ifblk);
      if (tnc)
      {
         if (op->down)
            tnc += DoOptBlock(igscope, ilscope, op->down);
      }
      else if (op->next)
         tnc += DoOptBlock(igscope, ilscope, op->next);
   }
/*
 * If we've got a one time list, handle it
 */
   else if (!op->maxN)
      nc = DoOptList(op->nopt, op->opts, iscope, global, &(op->nspill));
/*
 * otherwise, we have while(changes) optblk ... using down branch
 */
   else
   {
      maxN = op->maxN;
      for (i=0; i < maxN; i++)
      {
         nc = DoOptList(op->nopt, op->opts, iscope, global, &(op->nspill));
         for (dp=op->down; dp; dp = dp->down)
            nc += DoOptBlock(igscope, ilscope, dp);
         if (!nc)
            break;
         tnc += nc;
      }
      if (nc && (FKO_FLAG & IFF_VERBOSE))
         fprintf(stderr, "On last (%d) iteration, still had %d changes!\n",
                 maxN, nc);
   }
   
   if (op->next)
      tnc += DoOptBlock(igscope, ilscope, op->next);
   return(tnc);
}
#if 0
int PerformOptN(int SAVESP, struct optblkq *optblks)
/*
 * Returns: # of changes
 */
{
   BLIST *lbase;
   extern BBLOCK *bbbase;
   extern LOOPQ *optloop;
   BBLOCK *bp;
   int nc;
/*
 * NOTE: Now, we have three different scopes:
 *       IOPT_GLOB = all the blocks inside the routines
 *       IOP_SCOP  = block list specified in optnlks->blocks 
 *       0         = optloop // not used seperately any more 
 */
#if 0
   ShowFlow("cc0.dot", bbbase);
   FindScope();
   exit(0);
#endif

#if 0
   for (lbase=NULL,bp=bbbase; bp; bp = bp->down)
      lbase = AddBlockToList(lbase, bp);
/*
 * NOTE: no change here. send only all, or optloop
 */
   nc = DoOptBlock(lbase, optloop ? optloop->blocks : NULL, optblks);
   KillBlockList(lbase);
#else
   nc = DoOptBlock(optblks);
#endif
   INDEADU2D = CFUSETU2D = 0;
   return(nc);
}

#else

int PerformOptN(struct optblkq *optblks)
{
   int i, n;
   int nc = 0;
/*
 * Assumption: repeatable optimization may change the CFG, but can't get rid of 
 * any loop all together. It's fundamental optimization where we can use loop 
 * unroll to get rid of any loop. So, the number of scopes will never change, 
 * despite the change of CFG.
 */
   n = GetNumLoopScopes();
/*
 * apply optblks for each loop scopes
 */
   if (n)
   {
      for (i=0; i < n; i++)
         nc += DoOptBlock(n, i, optblks); 
   }
   else
      nc = DoOptBlock(n, -1, optblks); /* -1 means no loop scope */ 
      /*nc = DoOptBlock(n, 0, optblks);*/

   INDEADU2D = CFUSETU2D = 0; /* is it necessary? */
   return(nc);
}

#endif

int PerformOpt(int SAVESP)
/*
 * Returns 0 on success, non-zero on failure
 */
{
   int *nspill;
   BLIST *lbase;
   BBLOCK *bp;
   int i, j, KeepOn;
   extern BBLOCK *bbbase;

   nspill = malloc(NTYPES*sizeof(int));
   assert(nspill);
/*
 * Perform optimizations on special loop first
 */
   if (optloop && 1)
   {
      DoLoopGlobalRegAssignment(optloop);  
      optrec[noptrec++] = GlobRegAsg;
      do
      {
         j = DoScopeRegAsg(optloop->blocks, 1, &i, nspill);   
         KeepOn = j != i;
         KeepOn &= DoCopyProp(optloop->blocks); 
         if (KeepOn)
           fprintf(stderr, "\n\nREAPPLYING LOOP OPTIMIZATIONS!!\n\n");
         optrec[noptrec++] = RegAsg;
         optrec[noptrec++] = CopyProp;
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
      j = DoScopeRegAsg(lbase, 2, &i, nspill);   
      KeepOn = DoCopyProp(lbase);
      if (KeepOn)
        fprintf(stderr, "\n\nREAPPLYING GLOBAL OPTIMIZATIONS!!\n\n");
      optrec[noptrec++] = RegAsg + MaxOpt;
      optrec[noptrec++] = CopyProp+MaxOpt;
   }
   while(KeepOn);
   KillBlockList(lbase);
   INDEADU2D = CFUSETU2D = 0;
   return(0);
}
#if 0
int GoToTown(int SAVESP, int unroll, struct optblkq *optblks)
{
   int i, j;
   extern BBLOCK *bbbase;
   extern struct locinit *ParaDerefQ;

   if (optloop)
   {
      if (unroll > 1)
      {
#if 0
         fprintf(stdout, "LIL before unroll \n");
         PrintInst(stdout, bbbase);
         exit(0);
#endif
         UnrollLoop(optloop, unroll);
#if 0
         fprintf(stdout, "LIL after unroll \n");
         PrintInst(stdout, bbbase);
         exit(0);
#endif
         InvalidateLoopInfo();
         bbbase = NewBasicBlocks(bbbase);
         CheckFlow(bbbase, __FILE__, __LINE__);
         FindLoops();
         CheckFlow(bbbase, __FILE__, __LINE__);
      }
      else
      {
         if (DO_VECT(FKO_FLAG))
         {
            UnrollCleanup(optloop, 1);
            InvalidateLoopInfo();
            bbbase = NewBasicBlocks(bbbase);
            CheckFlow(bbbase, __FILE__, __LINE__);
            FindLoops();
            CheckFlow(bbbase, __FILE__, __LINE__);
         }
         else
            OptimizeLoopControl(optloop, 1, 1, NULL);
      }
#if 0 
      fprintf(stdout, "\n LIL After Unroll cleanup and before AE\n");
      PrintInst(stdout,bbbase);
#endif
/*
 *    Do accumulator expansion if requested
 */
      if (optloop->ae)
         DoAllAccumExpansion(optloop, unroll, DO_VECT(FKO_FLAG));
#if 0
      if (VECT_FLAG & (VECT_SSV | VECT_VRC | VECT_VEM))
      {
         if (optloop->se)
            DoAllScalarExpansion(optloop, unroll, DO_VECT(FKO_FLAG));
         if (optloop->ae)
            DoAllAccumExpansion(optloop, unroll, DO_VECT(FKO_FLAG));
      }
      else
         if (optloop->ae)
            DoAllAccumExpansion(optloop, unroll, DO_VECT(FKO_FLAG));
#endif 

#if 0
      fprintf(stdout, "\n LIL AFTER AE\n");
      PrintInst(stdout,bbbase);
      exit(0);
#endif
/*
 *    Add any prefetch inst to header of loop
 */
      if (optloop->pfarrs)
         AddPrefetch(optloop, unroll);
   }
   else
   {
      bbbase = NewBasicBlocks(bbbase);
      CheckFlow(bbbase, __FILE__, __LINE__);
      FindLoops();
      CheckFlow(bbbase, __FILE__, __LINE__);
   }
   CalcInsOuts(bbbase); 
   CalcAllDeadVariables();
/*
 * Majedul: Revealing hidden mem use for X86 before optimization
 */
   RevealArchMemUses();
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase); 
      CalcAllDeadVariables();
   }

#if 0 
      fprintf(stdout, "\n LIL Before OptN\n");
      PrintInst(stdout,bbbase);
#endif

#if 1
   PerformOptN(SAVESP, optblks);
#else
   assert(!PerformOpt(SAVESP));
#endif

   if (NWNT)
   {
      NAWNT = DoStoreNT(NULL);
/*
      for (j=0; j < NWNT; j++)
         free(ARRWNT[j]);
      free(ARRWNT);
      NWNT = 0;
*/
   }
   INUSETU2D = INDEADU2D = CFUSETU2D = 0;
   if (!INDEADU2D)
      CalcAllDeadVariables();
   if (!CFLOOP)
      FindLoops();
#if 1
   if (FKO_FLAG & IFF_SHOWCOMMENTS)
   {
      AddBlockComments(bbbase);
      AddLoopComments();
      AddSetUseComments(bbbase);   
      AddDeadComments(bbbase); 
   }
#endif
   i = FinalizePrologueEpilogue(bbbase, SAVESP);
   KillAllLocinit(ParaDerefQ);
   ParaDerefQ = NULL;
   if (i)
      return(1);
   CheckFlow(bbbase, __FILE__, __LINE__);
   return(0);
}
#endif

void DumpOptsPerformed(FILE *fpout, int verbose)
{
   int i, k, j=1;
   char ch;
   if (verbose)
   {
      fprintf(fpout, "\n%d optimization phases performed:\n", noptrec+j);
      if (DO_VECT(FKO_FLAG))
         fprintf(fpout, "%3d. %c %20.20s\n", j++, 'L', 
                 "Loop SIMD Vectorization");
      if (FKO_UR > 1)
         fprintf(fpout, "%3d. %c %20.20s : %d\n", j++, 'L', "Loop Unroll",
                 FKO_UR);
      if (optloop && optloop->pfarrs)
         fprintf(fpout, "%3d. %c %20.20s : %d\n", j++, 'L', "Prefetch",
                 optloop->pfarrs[0]);
      for (i=0; i < noptrec; i++)
      {
         k = optrec[i];
         if (k >= MaxOpt)
         {
            ch = 'G';
            k -= MaxOpt;
         }
         else
            ch = 'L';
         fprintf(fpout, "%3d. %c %20.20s : %d\n", j+i, ch, optmnem[k],
                 optchng[i]);
      }
      if (NAWNT)
      {
         fprintf(fpout, "%3d. %c %20.20s : %d\n", j+i+1, 'G', 
                 "Non-cached writes", NAWNT);
      }
   }
}

struct optblkq *DefaultOptBlocks(void)
/*
 * New defualts: -L : scoped (not local/optloop), -G : Global
 * Defaults to command-line flags of:
 *     -L 1 0 4 ls 2 3 4  
 *     -G 2 10 3 bc uj ul 
 *     -L 3 10 5 ra cp rc u1 lu
 *     -G 4 10 5 ra cp rc u1 lu // 
 *     -G 5 10 3 bc uj ul 
 */
{
   int i = 1;
   struct optblkq *base, *op;

/*
 * use encapsulated blk so that sprout node can gen the tree on next
 */
   op = base = NewOptBlock(i++, 0, 5, IOPT_SCOP);
   op->opts[0] = EnforceLoadStore;
   op->opts[1] = MaxOpt+2;
   op->opts[2] = MaxOpt+3;
   op->opts[3] = MaxOpt+4;
   op->opts[4] = MaxOpt+5;
   
   op->next = NewOptBlock(i++, 20, 3, IOPT_GLOB);
   op = op->next;
   op->opts[0] = BranchChain;
   op->opts[1] = UselessJmpElim;
   op->opts[2] = UselessLabElim;

   op->next = NewOptBlock(i++, 20, 5, IOPT_SCOP);
   op = op->next;
   op->opts[0] = RegAsg;
   op->opts[1] = CopyProp;
   op->opts[2] = ReverseCopyProp;
   op->opts[3] = RemoveOneUseLoads;
   op->opts[4] = LastUseLoadRemoval;
#if 1 
   op->next = NewOptBlock(i++, 20, 5, IOPT_GLOB);
   op = op->next;
   op->opts[0] = RegAsg;
   op->opts[1] = CopyProp;
   op->opts[2] = ReverseCopyProp;
   op->opts[3] = RemoveOneUseLoads;
   op->opts[4] = LastUseLoadRemoval;
#endif   
   op->next = NewOptBlock(i++, 20, 3, IOPT_GLOB);
   op = op->next;
   op->opts[0] = BranchChain;
   op->opts[1] = UselessJmpElim;
   op->opts[2] = UselessLabElim;
   
   return(base);
}

static short FindNameMatch(int n, short *pool, char *name)
/*
 * Given pool of ST entries, returns ST entry with STname matching name
 */
{
   int i;
   char *p;
   assert(name);
   for (i=0; i < n; i++)
   {
      p = STname[pool[i]-1];
      if (p && !strcmp(p, name))
         return(pool[i]);
   }
   return(0);
}

short *NamesToSTs(int n, char **names, int N, short *pool)
/*
 * Finds names from pool of symbol table (ST) indices
 * RETURNS: array of ST indices corresponding to names
 */
{
   short *sp;
   int i;
   short k;

   sp = malloc(sizeof(short)*n+1);
   assert(sp);
   sp[0] = n;

   for (i=1; i <= n; i++)
   {
      k = FindNameMatch(N, pool, names[i-1]);
      assert(k);
      sp[i] = k;
   }
   return(sp);
}

void UpdateNamedLoopInfo()
{
   int n, np, i, j, k, N, ncp;
   short *sp;
   short *aptrs;
   short sta, ptr;

   if (!optloop)
      return;
   if (PFARR)
   {
/*
 *    If we don't have a default distance, be sure arrays are moving in loop
 */
      if (PFARR[0][0] != '_')
      {
         n = PFDST[0] + 1;
         optloop->pfarrs = malloc(sizeof(short)*n);
         assert(optloop->pfarrs);
         optloop->pfarrs[0] = n-1;
         for (i=1; i < n; i++)
         {
#if 1
/*
 *          special checking for 2D array. varrs has column ptrs
 */
            sta = STarrlookupByname(PFARR[i-1]);
            if (sta && STarr[sta-1].ndim > 1)
            {
               for (j=1, ncp=STarr[sta-1].colptrs[0]; j <= ncp; j++ )
               {
                  k = FindInShortList(optloop->varrs[0], optloop->varrs+1, 
                                    STarr[sta-1].colptrs[j]);
                  assert(k);
               }
               k = STarr[sta-1].ptr;
            }
            else
            {
               k = FindNameMatch(optloop->varrs[0],optloop->varrs+1,PFARR[i-1]);
               assert(k);
            }
#else
            k = FindNameMatch(optloop->varrs[0],optloop->varrs+1,PFARR[i-1]);
            assert(k);
#endif
            optloop->pfarrs[i] = k;
         }
         optloop->pfdist = PFDST;
         optloop->pfflag = PFLVL;
         N = n = PFDST[0];
      }
/*
 *    If we've got default prefetch info for all arrays
 */
      else
      {
#if 1
/*
 *       FIXME: in case of 2D arrays, varrs saves the moving column pointers
 *       But we should add the array pointer here instead of column ptrs!
 */
         n = optloop->varrs[0];
         aptrs = malloc(sizeof(short)*(n+1));
         assert(aptrs);
         np = 0;
         for (i=1; i <= n; i++)
         {
            ptr = optloop->varrs[i];
            sta = STarrColPtrlookup(ptr);
            if (sta && STarr[sta-1].ndim > 1)
               ptr = STarr[sta-1].ptr;
            if (!FindInShortList(np, aptrs, ptr))
               np = AddToShortList(np, aptrs, ptr);
         }
   #if 0
         for (i=0; i < np; i++)
            fprintf(stderr, "%s, ", STname[aptrs[i]-1]);
         fprintf(stderr, "\n");
   #endif
         
         optloop->pfarrs = malloc(sizeof(short)*(np+1));
         optloop->pfdist = malloc(sizeof(short)*(np+1));
         optloop->pfflag = malloc(sizeof(short)*(np+1));
         optloop->pfarrs[0] = optloop->pfdist[0] = optloop->pfflag[0] = np;
/*
 *       Set default prefetch info for each moving pointer
 */
         for (i=1; i <=np; i++)
         {
            optloop->pfarrs[i] = aptrs[i-1];
            optloop->pfdist[i] = PFDST[1];
            optloop->pfflag[i] = PFLVL[1];
         }
         
#else
         n = optloop->varrs[0];
         optloop->pfarrs = malloc(sizeof(short)*(n+1));
         optloop->pfdist = malloc(sizeof(short)*(n+1));
         optloop->pfflag = malloc(sizeof(short)*(n+1));
         optloop->pfarrs[0] = optloop->pfdist[0] = optloop->pfflag[0] = n;
/*
 *       Set default prefetch info for each moving pointer
 */
         for (i=1; i <=n; i++)
         {
            optloop->pfarrs[i] = optloop->varrs[i];
            optloop->pfdist[i] = PFDST[1];
            optloop->pfflag[i] = PFLVL[1];
         }
#endif
/*
 *       Override default for non-def arrays
 */
         for (n=PFDST[0]+1,i=2; i < n; i++)
         {
            k = FindNameMatch(optloop->pfarrs[0],optloop->pfarrs+1,PFARR[i-1]);
            assert(k);
            k = FindInShortList(optloop->pfarrs[0], optloop->pfarrs+1, k);
            assert(k);
            optloop->pfdist[k] = PFDST[i];
            optloop->pfflag[k] = PFLVL[i];
         }
         n = PFDST[0];
         N = n - 1;
         free(PFDST);
         free(PFLVL);
      }
      for (i=0; i < n; i++)
         if (PFARR[i])
            free(PFARR[i]);
      free(PFARR);
      PFARR = NULL;
      PFDST = PFLVL = NULL;
/*
 *    Remove any mention of pointers that we are told not to prefetch
 *    (level == -1)
 */
/*
 *    FIXED: invalid read when applied following flags for sin:
 *    -Ps b A 0 3 -P all 0 128 -P X -1 0 -P Y -1 0 -P iY -1 0 
 */
      sp = optloop->pfflag+1;
      n = optloop->pfflag[0]; /* added to prevent the invalid read */
      for (k=i=0; i < n; i++)
         if (sp[i] == -1)
            k++;
      if (k)
      {
         if (N == k)
         {
            free(optloop->pfarrs);
            free(optloop->pfflag);
            free(optloop->pfdist);
            optloop->pfarrs = optloop->pfflag = optloop->pfdist = NULL;
         }
         else
         {
            for (j=i=1; i <= N; i++)
            {
               if (optloop->pfflag[i] != -1)
               {
                  optloop->pfarrs[j] = optloop->pfarrs[i];
                  optloop->pfdist[j] = optloop->pfdist[i];
                  optloop->pfflag[j] = optloop->pfflag[i];
                  j++;
               }
            }
            optloop->pfarrs[0] = optloop->pfdist[0] = optloop->pfflag[0] = j-1;
         }
      }
   }
}
#if 0
void DoStage2(int SAVESP, int SVSTATE)
/*
 * Assumes stage 0 has been achieved, writes files for stage 1
 */
{
   struct ptrinfo *pi, *pi0;
   int i, n, k;
   short *sp;

   GenPrologueEpilogueStubs(bbbase, SAVESP);
   NewBasicBlocks(bbbase);
   FindLoops(); 
   CheckFlow(bbbase, __FILE__, __LINE__);
#if 0
   fprintf(stdout, "LIL before LOOP UNROLL \n");
   PrintInst(stdout,bbbase);
#endif
/*
 * Having found the optloop, save which arrays to prefetch
 */
   if (optloop)
   {
      if (PFARR && !optloop->varrs)
      {
         KillLoopControl(optloop);
         pi0 = FindMovingPointers(optloop->blocks);
         for (n=0,pi=pi0; pi; pi=pi->next,n++);
         sp = malloc(sizeof(short)*(n+1));
         assert(sp);
         sp[0] = n;
         for (i=1,pi=pi0; i <= n; i++,pi=pi->next)
            sp[i] = pi->ptr;
         KillAllPtrinfo(pi0);
         RestoreFKOState(0);
         optloop->varrs = sp;
         GenPrologueEpilogueStubs(bbbase, SAVESP);
         NewBasicBlocks(bbbase);
         FindLoops(); 
         CheckFlow(bbbase, __FILE__, __LINE__);
      }
      UpdateNamedLoopInfo();
   }
   if (SVSTATE)
      SaveFKOState(2);
#if 0 
   fprintf(stdout, "LIL AFTER DO STAGE 2 \n");
   PrintInst(stdout, bbbase);
#endif
}
#endif

#if 0
void AddOptSTEntries()
/*
 * Some optimizations require additional locals to be allocated.  This routine
 * adds them to ST, though they may not be used if optimization is not applied
 * Must call this routine before initial save state, so args remain in ST for
 * later use.
 */
{
   char ln[1024];
   int i, n, VEC, k;
   short st;
   short *sp, **asp;

   VEC = DO_VECT(FKO_FLAG);
   if (optloop && AEn)
   {
      n = AEn[0];
      sp = malloc((n+1)*sizeof(short));
      asp = malloc((n+1)*sizeof(short*));
      assert(sp && asp);
      asp[n] = NULL;
      sp[0] = n;
      for (i=1; i <= n; i++)
      {
         sp[i] = FindVarFromName(AES[i-1]);
         assert(sp[i]);
         asp[i-1] = DeclareAE(VEC, AEn[i], sp[i]);
      }
      optloop->ae = sp;
      optloop->ne = AEn;
      AEn = NULL;
      optloop->aes = asp;
   }
}
#endif

void AddOptST4SE()
/*
 * Majedul: This is for generalizing the scalar expansion. 
 * NOTE: if it is accumulator, we should update that before state0. it will
 * be needed in normal vectorization. But if it is max/min var, we will need
 * special transformation before vectorization. 
 * 
 * FIXME: Need to solved the problem with saving the temporary LIL and 
 * restoring it later even after updating the bitvectors.  See SaveFKOstate
 * function for details.
 *
 * NOTE: AES/SES is 0 indexed (means SES[0] has valid element) but AEn/SEn
 * is 1 indexed (SEn[0] contains the number of elements)
 *
 * PrevNote: Some optimizations require additional locals to be allocated.  
 * This routine adds them to ST, though they may not be used if optimization 
 * is not applied Must call this routine before initial save state, so args 
 * remain in ST for later use. 
 */
{
   int i, n, VEC;
   short st;
   int *scf;

   short *se;   /* [0] = number of elements */
   short **ses; /* element from 0 position */
   
   se = NULL;
   ses = NULL;
   scf = NULL;

   VEC = DO_VECT(FKO_FLAG);
   if (optloop && SEn)
   {
      n = SEn[0];
      scf = calloc(n, sizeof(int));
      se = malloc((n+1)*sizeof(short));
      ses = malloc((n+1)*sizeof(short*));
      scf = malloc((n+1)*sizeof(int));
      assert(scf && se && ses && scf);
      ses[n] = NULL;
      se[0] = n;
      for (i=1 ; i <= n; i++)
      {
         st = FindVarFromName(SES[i-1]);
         assert(st);
         se[i] = st;
/*
 *       HERE HERE, f we applied -rc before, there is no way to determine a var
 *       as max/min. So, check the optloop->maxvars and optloop->minvars before 
 *       testing for Accumulator!!!
 *       UpdateOptLoopSTWithMaxMinVars must be applied irrespective of any
 *       transformation!!!
 */
         if (optloop->maxvars && 
             FindInShortList(optloop->maxvars[0], optloop->maxvars+1, st))
         {
            scf[i] = SC_MAX;
            ses[i-1] = DeclareMaxE(VEC, SEn[i], st);
         }
         else if (optloop->minvars && 
                  FindInShortList(optloop->minvars[0], optloop->minvars+1, st))
         {
            scf[i] = SC_MIN;
            ses[i-1] = DeclareMinE(VEC, SEn[i], st);
         }
         else if (VarIsAccumulator(optloop->blocks, st))
         {
            scf[i] = SC_ACC;
            ses[i-1] = DeclareAE(VEC, SEn[i], st);
         }
/*
 *    NOTE: -rc transformation modifies the max/min structure. So, following
 *    max/min testing will not work. 
 */
#if 0         
         else if (VarIsMaxOrMin(optloop->blocks, st, 1, 0))
         {
            scf[i] = SC_MAX;
            ses[i-1] = DeclareMaxE(VEC, SEn[i], st);
         }
         else if (VarIsMaxOrMin(optloop->blocks, st, 0, 1))
         {
            scf[i] = SC_MIN;
            ses[i-1] = DeclareMinE(VEC, SEn[i], st);
         }
#endif         
         else
         {
/*
 *          FIXME: check for snrm2 with scal as SE 
 */
            fko_error(__LINE__, "%s not a candidate for scalar expansion\n",
                     SES[i-1]);
            /*fko_warn(__LINE__, "%s not a candidate for scalar expansion\n",
                     SES[i-1]);*/
         }
      }
/*
 *    update optloop 
 */
      optloop->se = se;
      optloop->nse = SEn; /* number of expansion */
      SEn = NULL;
      optloop->ses = ses;
      optloop->seflag = scf;
   }
    
}


#if 0
void GenAssenblyApplyingOpt4SSV(FILE *fpout, struct optblkq *optblks, 
                                struct assmln *abase)
/*
 * Apply all repeatable optimization for SSV
 * NOTE: right now, we skip Unrolling, AE, etc optimization for SSV. So, we
 * skip more general function GoToTown and use this one. It is temporary func.
 * We will generalize it later.
 */
{

   int i;

   /*UnrollCleanup(optloop,1);*/
   UnrollCleanup2(optloop,1);
/*
 * Everything is messed up already. So, re-make the control flow and check it 
 */
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);

/*
 * Update prefetch information, done inside vectorization
 */
#if 0   
   UpdateNamedLoopInfo();
#endif

/*
 * Attempt to generate assembly after repeatable optimization
 */
#if 0 
   fprintf(stdout, "\nUnoptimized LIL\n");
   PrintInst(stdout, bbbase);
#endif

/*
 * add Prefetch
 */
#if 1
   if (optloop->pfarrs)
   AddPrefetch(optloop, 1);
#endif

   extern struct locinit *ParaDerefQ;
   CalcInsOuts(bbbase);
   CalcAllDeadVariables();
   RevealArchMemUses(); /* to handle ABS in X86 */
   PerformOptN(0, optblks);

   #if 0
      fprintf(stdout, "Optimized LIL\n");
      PrintInst(stdout, bbbase);
   #endif
   
   if (NWNT)
   {
      NAWNT = DoStoreNT(NULL);
   }
   INUSETU2D = INDEADU2D = CFUSETU2D = 0;
   if (!INDEADU2D)
      CalcAllDeadVariables();
   if (!CFLOOP)
      FindLoops();
/*
 * Majedul: adding comments significantly slow down the compilation after 
 * adding unrolling for SV (specially for -U 32)
 */
#if 0
   AddBlockComments(bbbase);
   AddLoopComments();
#endif   
   i = FinalizePrologueEpilogue(bbbase,0 );
   KillAllLocinit(ParaDerefQ);
   ParaDerefQ = NULL;
   if (i)
      fprintf(stderr, "ERR from PrologueEpilogue\n");
   CheckFlow(bbbase, __FILE__,__LINE__);
   DumpOptsPerformed(stderr, FKO_FLAG & IFF_VERBOSE);
   #if 1
      fprintf(stdout, "Optimized LIL Before Assembly\n");
      PrintInst(stdout, bbbase);
      fprintf(stdout, "SYmbol Table\n");
      PrintST(stdout);
   #endif
   abase = lil2ass(bbbase);
   KillAllBasicBlocks(bbbase);
   bbbase = NULL;
   dump_assembly(fpout, abase);
   KillAllAssln(abase);
   /*exit(0);*/
   return ;
}
#endif

void FeedbackLValSpill(FILE *fpout, struct optblkq *optblks)
/*
 * FIXME: not working fine with current PerformOptN structure if we have more 
 * than one loop in loopnest; overwritten by the data of uppermost loop !!!
 */
{
   int i, ln;
   struct optblkq *op;
   const int nreg = 6;
   char *treg[] = {"i","f", "d", "vi", "vf", "vd"}; 
   enum tregs { R_INT, R_FLOAT, R_DOUBLE, R_VINT, R_VFLOAT, R_VDOUBLE}; 
   int ropt[nreg];
   int rglob[nreg];
   int nropt=0, nrglob=0, tnr=0;

   for (op=optblks; op; op=op->next)
   {
/*
 *    Assumption: 1st op is always RegIG in the bundle
 *    record the last one 
 */
      if ( (op->flag & IOPT_SCOP) && op->opts[0] == RegAsg) /* 1st op is reg */
      {
         /*int,short,char */
         ropt[R_INT] = op->nspill[T_INT] + op->nspill[T_SHORT] + 
                       op->nspill[T_CHAR];
         ropt[R_FLOAT] = op->nspill[T_FLOAT];
         ropt[R_DOUBLE] = op->nspill[T_DOUBLE];
         ropt[R_VINT] = op->nspill[T_VINT];
         ropt[R_VFLOAT] = op->nspill[T_VFLOAT];
         ropt[R_VDOUBLE] = op->nspill[T_VDOUBLE];
         nropt = ropt[R_INT] + ropt[R_FLOAT] + ropt[R_DOUBLE] + ropt[R_VINT] 
                 + ropt[R_VFLOAT] + ropt[R_VDOUBLE];
      }
      else if ( (op->flag & IOPT_GLOB) && op->opts[0] == RegAsg)
      {
         rglob[R_INT] = op->nspill[T_INT] + op->nspill[T_SHORT] + op->nspill[T_CHAR];
         rglob[R_FLOAT] = op->nspill[T_FLOAT];
         rglob[R_DOUBLE] = op->nspill[T_DOUBLE];
         rglob[R_VINT] = op->nspill[T_VINT];
         rglob[R_VFLOAT] = op->nspill[T_VFLOAT];
         rglob[R_VDOUBLE] = op->nspill[T_VDOUBLE];
         tnr = nrglob = rglob[R_INT] + rglob[R_FLOAT] + rglob[R_DOUBLE] 
            + rglob[R_VINT] + rglob[R_VFLOAT] + rglob[R_VDOUBLE];

      }
   }
/*
 * print all info
 */
   ln = (nropt > 0) + (nrglob > 0);
   fprintf(fpout, "LRSPILLS=%d\n",ln);
   if (tnr)
   {
      if (nropt)
      {
         fprintf(fpout, "   OPTLOOP:");
         for (i=0; i < nreg; i++)
            if(ropt[i]) fprintf(fpout, " %s=%d", treg[i], ropt[i]);
         fprintf(fpout, "\n");
      }
      if (nrglob)
      {
         fprintf(fpout, "   GLOBAL:");
         for (i=0; i < nreg; i++)
            if(rglob[i]) fprintf(fpout, " %s=%d", treg[i], rglob[i]);
         fprintf(fpout, "\n");
      }
   }
   
}

struct optblkq *PerformRepeatableOpts(struct optblkq *optblks)
{
/*
 * generate optblks if not specified command line
 */
   if (!optblks)
      optblks = DefaultOptBlocks();
   optblks = OptBlockQtoTree(optblks);
/*
 * update use/set and dead variables if needed 
 */
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
/*
 * Perform repeatable optimizations based on optblks 
 */
#if 0
   PrintSymtabStaticMember(stderr);
   PrintST(stdout);
   exit(0);
#endif
#if 0
      fprintf(stdout, "\n LIL Repeatable opt\n");
      PrintInst(stdout,bbbase);
      exit(0);
#endif
   PerformOptN(optblks);

   return(optblks);
}

void GenAssembly(FILE *fpout)
/*
 * finalize prologue/epilogue and generate assembly from final LIL 
 */
{
   int i; 
   struct assmln *abase;
   extern struct locinit *ParaDerefQ;
/*
 * redo all the data flow analysis, may be needed to finalize prologue
 */
   INUSETU2D = INDEADU2D = CFUSETU2D = 0;
   if (!INDEADU2D)
      CalcAllDeadVariables();
   if (!CFLOOP)
      FindLoops();
/*
 * add comments in code if requested
 */
   if (FKO_FLAG & IFF_SHOWCOMMENTS)
   {
      AddBlockComments(bbbase);
      AddLoopComments();
      /*AddSetUseComments(bbbase);*/   
      /*AddDeadComments(bbbase); */
   }
/*
 * finalize prologue epilogue
 */
   i = FinalizePrologueEpilogue(bbbase,0 );
   KillAllLocinit(ParaDerefQ);
   ParaDerefQ = NULL;
   if (i)
      fprintf(stderr, "ERR from PrologueEpilogue\n");
   CheckFlow(bbbase, __FILE__,__LINE__);
/*
 * add repeatable opts log if requested
 */
   if ( (FKO_FLAG & IFF_VERBOSE) && 0) /* skipped for now */
      DumpOptsPerformed(stderr, FKO_FLAG & IFF_VERBOSE);
/*
 * LIL to assembly 
 */
   abase = lil2ass(bbbase);
   KillAllBasicBlocks(bbbase);
   bbbase=NULL;                  /* whenever Kill it here, make it NULL */
   dump_assembly(fpout, abase);
   KillAllAssln(abase);
}

void FinalStage(FILE *fpout, struct optblkq *optblks)
/*
 * final stage to generate assembly after performing repeatable optimizations
 */
{
#ifdef X86
   BLIST *bl;
   BBLOCK *bp;
#endif
/*
 * Need to reveal arch mem uses before DeadDefElim, otherwise, those defs would
 * be deleted from prologue
 */
#ifdef X86
   RevealArchMemUses(); /* to handle ABS in X86 */
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#endif
/*
 * NOTE: we need dead store elimination before applying repeatable optimization
 * like, register assignment
 */
   if (!(FKO_FLAG & IFF_NODDE) )
      DoDeadDefElim();

#if defined(X86) && 1
/*
 * peephole optimization: replace shl/add with lea
 */
   for (bp=bbbase, bl=NULL; bp; bp=bp->down)
      bl = AddBlockToList(bl, bp);
   DoOptArchSpecInst(bl);
   KillBlockList(bl);
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#endif
/*
 * perform repeatable optimization
 */
   optblks = PerformRepeatableOpts(optblks);
/*
 * feedback live range spilling info if requested 
 */
   if (fpLRSINFO)
   {
      FeedbackLValSpill(fpLRSINFO, optblks);
      KillAllGlobalData(optblks);
      return;
   }
/*
 * Non Temporal writes if requested
 */
   if (NWNT)
      NAWNT = DoStoreNT(NULL);
/*
 * finalize prologue/epilogue and generate assembly
 */
   GenAssembly(fpout);
/*
 * kill all global data structures 
 */
   KillAllGlobalData(optblks); 
}

int IsControlFlowInLoop(BLIST *lpblks, BBLOCK *header)
{
   BLIST *bl;

   for (bl = lpblks; bl; bl = bl->next)
   {
      if (bl->blk->csucc && bl->blk->csucc != header)
         return(1);
   }
   return 0;
}

void AddOptWithOptimizeLC(LOOPQ *lp)
/*
 * Optimize loop control only for UR =1 and update the optloop->varrs
 */
{
   struct ptrinfo *pi, *pi0;
   int i, n;
   short *sp;

   if (!lp) return;
/*
 * killed the loop control to make the analysis easier, we will update the
 * loop control with OptimizeLoopControl again!
 */
   KillLoopControl(optloop);
   pi0 = FindMovingPointers(lp->blocks);
   for (n=0,pi=pi0; pi; pi=pi->next,n++);
   sp = malloc(sizeof(short)*(n+1));
   assert(sp);
   sp[0] = n;
   for (i=1,pi=pi0; i <= n; i++,pi=pi->next)
      sp[i] = pi->ptr;
   KillAllPtrinfo(pi0);
  
   if (!lp->varrs)
      lp->varrs = sp;
   else assert(0);
/*
 * NOTE: it will not be applied if the code is vectorized or UR > 1 
 */
   OptimizeLoopControl(lp, 1, 0, NULL); /* lc is already killed */
}

int IsAlignLoopSpecNeeded(LOOPQ *lp)
{
   int i,j,k,n;
   short *spa[2], *spb[2];
   short id;
   const int na = 2;
/*
 * we don't need loop specialization for alignment if there is only one ptr
 * loop peeling would make it aligned to vlen
 */
   if (!lp->varrs || lp->varrs[0] < 2)
   {
      fko_warn(__LINE__, "NO SPECIALIZATION FOR ALIGNMENT: "
                     "we only have one PTR to manage");
      return 0;
   }

/*
 * no need of loop specialization for alignment if:
 * 1. All moving ptrs are aligned to atleast vlen
 *    - even if aligned to smaller than vlen, specialization can be avoided. but
 *      we don't consider that right now. 
 * 2. All moving ptrs are mutually aligned to atleast vlen
 */
   spa[0] = lp->aaligned;
   spa[1] = lp->maaligned;
   spb[0] = lp->abalign;
   spb[1] = lp->mbalign;
   
   for (i=0; i <na; i++)
   {
      if (!spa[i] && spb[i]) /* ALIGNED() :: *; */
      {
         if (spb[i][1] >= type2len(lp->vflag)) // ALIGNED(VB) :: *;
         {
            fko_warn(__LINE__, "NO SPECIALIZATION FOR ALIGNMENT: "
                     "all are aligned to vlen or greater");
            return 0;
         }
      }
      else if (spa[i]) /* alignment is specified for some ptr */
      {
         for (n=lp->varrs[0], j=1; j <= n; j++)
         {
            k = lp->varrs[j];
/*
 *          need to consider 2D array
 */
            if (!(k = FindInShortList(spa[i][0], spa[i]+1, k)))
            {
               id = STarrColPtrlookup(lp->varrs[j]);
               if (id && !FindInShortList(spa[i][0], spa[i]+1, id))
                  break;
               else
                  break;
            }
            else if (spb[i][k] < type2len(lp->vflag)) /*k = index of aptr */
               break;
         }
         if (j>n) /* no moving ptr which is not aligned to vlen */
         {
            fko_warn(__LINE__, "NO SPECIALIZATION FOR ALIGNMENT: "
                     "all are mutually aligned to vlen or greater");
            return 0;
         }
      }
   }

   return(1);
}

int GetLoopVtype(LOOPQ *lp)
{
   int i, j, k, n, N;
   BLIST *bl;
   short *sp;
   INT_BVI iv;
   int type;
   extern short STderef;
   extern INT_BVI FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);

   if (!CFUSETU2D )
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
   for (bl=lp->blocks; bl; bl=bl->next)
   {
      iv = BitVecComb(iv, iv, bl->blk->uses, '|'); 
      iv = BitVecComb(iv, iv, bl->blk->defs, '|'); 
   }
   
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);
   
   sp = BitVec2Array(iv, 1-TNREG);
   for (N=sp[0],n=0, i=1; i <= N; i++)
   {
      if (IS_FP(STflag[sp[i]-1]))
         sp[n++] = sp[i];
   }
   j = FLAG2TYPE(STflag[sp[0]-1]);
   for (i=1; i < n; i++)
   {
      k = FLAG2TYPE(STflag[sp[i]-1]);
      if (k != j)
         fko_error(__LINE__, "mixed type prevents vectorization!");
   }
   free(sp); 
   if (j == T_FLOAT)
      type = T_VFLOAT;
   else 
      type = T_VDOUBLE;

   return(type);
}

int IsSimpleLoopNestVec(int vflag)
{
   int k, type;
   int lpnest = 0;
   LOOPQ *lpq;
   extern LOOPQ *optloop;
   
   lpq = optloop;

   while (lpq)
   {
      lpnest++;
      lpq = lpq->preheader->loop;
   }
/*
 * we don't have loopnest, return false
 */
   if (lpnest <= 1)
   {
      fko_warn(__LINE__, "Must have atleast 2 nested loops for loop-nest vect\n");
      return(0);
   }
/*
 * fko supports vectorization for x86 only so far
 */
#ifndef X86
   return(0);
#endif
/*
 * if we want to apply loopnest vectorization on rolled loop (not by SLP), we 
 * can't have cleanup and loop peeling for alignment right now
 */   
   if ( !(vflag & VECT_SLP) )
   {
/*
 *    FIXME: use iter_mult markup to determine whether cleanup is not necessary
 *    need to figure out the data type used in optloop.. hence the vlen iter!
 */
/*
 *    get type from loop ptr, assuming all of them are of same types. We will
 *    analyze it later in vector analysis stage
 */
#if 0
      if ( (optloop->LMU_flag & LMU_NO_CLEANUP) )
      { 
      }
      else
      {
         fko_warn(__LINE__, "Must not have cleanup\n");
         return(0);
      }
#else
      type = GetLoopVtype(optloop);
      k = vtype2elem(type);
      if (!optloop->itermul || (optloop->itermul % k) )
      {
         fko_warn(__LINE__, "Must have no cleanup: (%d, %d)", optloop->itermul,
                  k);
         return(0);
      }
      /*else 
         fprintf(stderr, "no need of cleanup in vec");*/
#endif
/*
 *    which vectorization technique? 
 */
      if (IsSpeculationNeeded())
         VECT_FLAG |= VECT_SV;
      else
         VECT_FLAG |= VECT_NCONTRL;
   }
/*
 * passed all conditions
 */
   /*fprintf(stderr, "loop nest vectorization can be applicatable\n");*/
   return(1);
}

int IsPtrMinPossible(BLIST *scope)
/*
 * Main idea: if any block in the scope has any ptr with const update, we can 
 * minimize the updates or atleast move it at the end for this block... 
 */
{
   int nbp = 0;
   BLIST *bl;
   struct ptrinfo *pi;

   for (bl=scope; bl; bl=bl->next)
   {
      pi = FindConstMovingPtr(bl->blk);
      if (pi)
      {
         nbp++;
         KillAllPtrinfo(pi);
         /*return(1);*/
      }
   }
   if (nbp)
   {
      /*fprintf(stderr, " nbp = %d\n");*/
      return(1);
   }
   return(0);
}

int main(int nargs, char **args)
/*
 * ==========================================================================
 * Majedul:
 * I have changed the program states of FKO. Here is short description
 * of that:
 *
 * State 0: Plain LIL (with no prologue/epilogue), initial ST
 *          no update in ST and optloop for AE/SE/RE (Scalar Expansion)
 *
 * State 1:  
 *    a) Generate initial Prologue/Epilogue, so that we can create CFG and 
 *       update optloop info
 *    b) Fall-thru optimization, if requested
 *    c) We will apply some optimization which is the pre-requisit for some 
 *       some vectorization method. e.g.- 
 *       i) If conversion with Redundant Computation
 *       ii) Max/Min variable reduction.
 * 
 * State 2:      
 *    a) Vector Analysis: able to perform path based vector analysis
 *    b) Vectorization: Normal and Speculative (SV)
 *    c) Loop peeling for Alignment (if applicable)
 *    d) complete cleanup for vectorization (may be updated after unrolling)
 *         
 * State 3:
 *    a) Unrolling with updated cleanup 
 *    b) Scalar Expansion (supports Accumulator, Max/Min expansion)
 *    c) Prefetching
 *
 * State 3.1:
 *    a) Loop specialization for vector alignment: done after doing all the 
 *    fundamental opts to keep them intact in duplicated loops too.
 *
 * State 4: (Common Optimization State)
 *    a) Reveal Architectural memory usage (Enable or disable)
 *    b) Repeatable Optimization
 *    b) Finalize Prologue and Epilogue
 *    c) LIL to Assembly
 *
 * Relationship of states if all applied (state0 and state4 are must):
 *    State0 --> State1 --> State2 --> State3 --> State4
 *
 * NOTE: 
 * I will fix the Save/Restore function for states. Initial plan: all states
 * are completely separate and the complete program states for each can be 
 * saved and restored. Right now, only state0 can be saved and restored.
 *===========================================================================
 */
{
   int ur, bvlpl;
   FILE *fpin, *fpout;
   char *fin;
   struct optblkq *optblks;
   BBLOCK *bp;
   BLIST *bl;
   /*int RCapp;*/
   LPLIST *lpnest;
   short *ptrs, *aptr;
   extern FILE *yyin;
   extern BBLOCK *bbbase;
   extern int SKIP_PEELING;
/*
 * Update flags using command line options and create Optimization block 
 * structures. Note: It's not CFG block, it's block for opts and will be used
 * later in repeatable optimization.
 */
   optblks = GetFlagsN(nargs, args, &fin, &fpin, &fpout);
/*
 * If we have already saved the program state into files, load that (if it is
 * requested from command line).
 * 
 * Previous Requirement:
 * Files: IG, ST, LIL, OPT 
 * Normally files saved/restored from /tmp/FKO_ST.0/2/3, /tmp/FKO_LIL.0/2/3,
 * /tmp/FKO_misc.0/2/3 
 * NOTE: I was not able to successfully restore program state from command
 * line. 
 *
 * Future Plan: 
 * ------------ 
 *    I have 4 intermidiate steps. All of them would be saved/restored using 
 *    the appropriate prefix (0~3). Updated for state0 only.
 * 
 * NOTE: We need to find a way to continue the program flow after retrieving
 * the program states.
 */
   if (FKO_FLAG & IFF_READINTERM)
   {
      fprintf(stderr, 
             "Need to call restore function. Only supported for state 0\n");
      exit(0);
   }
/*===========================================================================
 *    STATE 0:
 *       1. Generate 1st LIL after parsing the HIL (Simple ld/st LIL).
 *       2. No CFG, No Prolgue/Epilogue, no extra info
 *==========================================================================*/
#if 0
   FKO_FLAG |= IFF_OPT2DPTR; /* applying optimize 2d array access by default */
#endif
   yyin = fpin;
   bp = bbbase = NewBasicBlock(NULL, NULL);
   bp->inst1 = bp->instN = NULL;
   yyparse();
   fclose(fpin);
/*
 * save state0 so that we can restore problem from that  
 */
   if (fpLOOPINFO || fpVECINFO || fpVECINFO2 || fpBVNUM)
      SaveFKOState0(); /* this function works for state0. */
/*
 * if we need information for tunning, generate and return those info.
 */
   if (fpLOOPINFO)
   {
      FeedbackLoopInfo();
      KillAllGlobalData(optblks); 
      exit(0);
   }
   else if (fpVECINFO || fpVECINFO2)
   {
      VecInfo(0); /* 0 means print not only best one nut also others */ 
      KillAllGlobalData(optblks); 
      exit(0);
   }
   else if (fpBVNUM)
   {
      fprintf(fpBVNUM, "BEST-VECTOR-METHOD: %d\n", VecInfo(1));
      KillAllGlobalData(optblks); 
      exit(0);
   }
/*
 * if we want to apply compiler's suggested best vectorization methods, analyze
 * it and set flags
 */
   if (FKO_FLAG & IFF_BESTVEC)
   {
      SaveFKOState0(); /* this function works for state0. */
      SetVecFlag(VecInfo(1)); 
      RestoreFKOState0();
   }
/*===========================================================================
 *    STATE 1:
 *       1. Generate initial prologue/epilogue, CFG, find loops
 *       2. Fall-thru optimization, if requested
 *       3. If conversion with Redundant computation if requested
 *       4. Max/Min Reduction, if requested
 *
 * NOTE: introduce new 2 command line flags: -rc, -mmr 
 *==========================================================================*/
/*
 * Get code into standard form for analysis
 */
   GenPrologueEpilogueStubs(bbbase, 0); /* rsav = 0, normal case */
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase,__FILE__,__LINE__);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
/*
 * NOTE: if there is no optloop, we can't perform transformation of 
 * State1~State3. So, we will jump to State4 to generate code
 */
   if (!optloop)
   {
/*
 *    perform repeatable optimization, NWNT and genrate assembly
 */
      FinalStage(fpout, optblks);
      return(0);
   }
/*
 * Apply fall thru transformation if mandated in commandline 
 */
   if (path != -1)
   {
      /*PrintFallThruLoopPath(optloop); */
      /*PrintLoopPaths();*/
      TransformFallThruPath(path);
      /*PrintFallThruLoopPath(optloop); */
/*
 *    Re-structure the CFG and loop info 
 */
      InvalidateLoopInfo();
      bbbase = NewBasicBlocks(bbbase);
      CheckFlow(bbbase, __FILE__,__LINE__);
      FindLoops();
      CheckFlow(bbbase, __FILE__, __LINE__);
      /*ShowFlow("fall.dot",bbbase);*/
   }
/*
 * Right now, we will consider only optloop for theses transformation, but
 * it can be applied anywhere. 
 * It will be applied only if there exists a control flow inside the loop.
 * NOTE: 
 *    1st MMR then RC to be applied. 
 */
   if (optloop && IsControlFlowInLoop(optloop->blocks, optloop->header))
   {
/*
 *    After this state, the structure of Max/Min will be lost. So, update 
 *    optloop with that. We can use this info later (both in this state and SE )
 */
      UpdateOptLoopWithMaxMinVars();
      if (STATE1_FLAG & IFF_ST1_MMR)
      {
/*
 *       this optimiozation tries to eliminate the branch if that is only used
 *       to findout max/min value
 */
         ElimMaxMinIf(1,1); /* both max and min */
      }
/*
 *    we always try to move max/min variable before applying RC 
 */
      if (STATE1_FLAG & IFF_ST1_RC)
      {
         /*assert(!IterativeRedCom());*/
         MovMaxMinVarsOut(1,1); /* both max/min */
         assert(!IterativeRedCom());
      } 
   }
   else
   {
      if (STATE1_FLAG & IFF_ST1_RC || STATE1_FLAG & IFF_ST1_MMR)
         fko_error(__LINE__, "No Control Flow in optloop to transform!!!\n");
   }

#if 0
   if (FKO_FLAG & IFF_GENINTERM && state1) /*flag state1 is not impl yet */
   {
      SaveFKOState(1);
      exit(0);
   }
#endif 
/*--------------------------------------------------------------------------
 * new fundamental optimization: loop unswitching... it will make the nested
 * loop simple to apply our loop nest vectorization
 * we will apply this only if all the if-conditionals can be removed for now
 *----------------------------------------------------------------------------*/
   if (optloop)
   {
      lpnest = FindLoopNest(optloop);
      if (optloop && IsLoopUnswitchable(lpnest))
      {
         /*fprintf(stderr, "Loop unswitchable!!\n");*/
         LoopUnswitch(lpnest);
         InvalidateLoopInfo();
         bbbase = NewBasicBlocks(bbbase);
         CheckFlow(bbbase, __FILE__,__LINE__);
         FindLoops();
         CheckFlow(bbbase, __FILE__, __LINE__);
      }
      KillAllLPlist(lpnest);
   }
/*-----------------------------------------------------------------------------
 * new fundamental optimization: Minimize Ptr update
 * right now, we will apply this opt when any one pointer is updated
 * by const
 *----------------------------------------------------------------------------*/
   if (optloop)
   {
      lpnest = FindLoopNest(optloop);
      if (IsPtrMinPossible(lpnest->loop->blocks)) /* blocks of outermost loop */
      {
         /*fprintf(stderr, "MPU possible!\n");*/
         LocalMinPtrUpdate(lpnest->loop->blocks);
      }
      KillAllLPlist(lpnest);
   }
/*============================================================================
 *    STATE2 : Vectorization 
 *    [This state is dedicated for vectorization. Right now, we deal with two 
 *    types of vectorization: normal, Speculative. Later, we will apply 
 *    speculation before vectorization. Then, vectorization will be straight 
 *    forward.]
 *
 *    1. Vector Analysis (Path based vector analyzer)
 *    2. Vectorization (Normal, Speculative - SV ) with loop peeling for 
 *       Alignment and cleanup
 *    
 *    NOTE: during vector analysis, we have updated optloop with necessary info
 *    like: varrs which is needed later.
 *
 *    NOTE: for speculative vectorization, unrolling is not implemented yet
 *===========================================================================*/

/*
 * we need to update vflag, varrs etc for HIL intrinsic vector loop
 */
   if (VECT_FLAG & VECT_INTRINSIC)
   {
      if (FKO_FLAG & IFF_VECTORIZE)
         fko_warn(__LINE__, "Already vectorize by HIL intrinsic\n");
      UpdateVecLoop(optloop);
   }
/*
 * Vectorization for Simple loop nest
 * can be any of them: SLP, SV and no hazard....
 */   
   else if ( (FKO_FLAG & IFF_VECTORIZE) && IsSimpleLoopNestVec(VECT_FLAG) )
   {
      if ((IWAY-STN_VEC_SLP) > 0)
      {
         ptrs = OrderedVecPtrAccess(optloop);
         if (ptrs && (IWAY-STN_VEC_SLP) <= ptrs[0])
         {
            aptr = malloc(2*sizeof(short));
            assert(aptr);
            aptr[0] = 1;
            aptr[1] = ptrs[IWAY-STN_VEC_SLP];
            /*fprintf(stderr, "ptr = %s(%d)\n", STname[aptr[1]-1], IWAY);*/
            assert(!LoopNestVec(aptr, &bvlpl));
            free(ptrs);
            free(aptr);
         }
         else
            fko_error(__LINE__, "Unknown way for SLP (-w #)\n");
      }
      else
         assert(!LoopNestVec(NULL, &bvlpl));
   }
   else if ( (FKO_FLAG & IFF_VECTORIZE) && (VECT_FLAG & VECT_SLP) )
   {
/*
 *    apply SLP vectorization method, use iway to init pack for SLP 
 */
      if ( (IWAY-STN_VEC_SLP) > 0)
      {
         ptrs = OrderedVecPtrAccess(optloop);
         if (ptrs && (IWAY-STN_VEC_SLP) <= ptrs[0])
         {
            aptr = malloc(2*sizeof(short));
            assert(aptr);
            aptr[0] = 1;
            aptr[1] = ptrs[IWAY-STN_VEC_SLP];
            assert(!LoopNestVec(aptr, &bvlpl));
            free(ptrs);
            free(aptr);
         }
         else
            fko_error(__LINE__, "Unknown way for SLP (-w #)\n");
      }
      else
         assert(!LoopNestVec(NULL, &bvlpl));
      /*CheckUseSet();*/
   }
   else if (FKO_FLAG & IFF_VECTORIZE)
   {
      if (IsSpeculationNeeded())
      {
         #if IFKO_DEBUG_LEVEL > 1
            fprintf(stderr,"Vectorization Method: SV \n");
         #endif
/*
 *       update flag for later use
 */
         VECT_FLAG &= ~VECT_NCONTRL;
         VECT_FLAG |= VECT_SV;
         
         assert(!SpeculativeVectorAnalysis());
         SpecSIMDLoop(FKO_SB);
         
         FinalizeVectorCleanup(optloop, FKO_SB);
         InvalidateLoopInfo();
         bbbase = NewBasicBlocks(bbbase);
         CheckFlow(bbbase, __FILE__,__LINE__);
         FindLoops();
         CheckFlow(bbbase, __FILE__, __LINE__);
/*
 *       Want to make the vector code fall-thru always
 */
         ReshapeFallThruCode(); 
         /* FKO_UR = 1;*/  /* forced to unroll factor 1*/
      }
      else /*if(RCapp)*/ /* use RcVec as a general pupose vec method*/
      {
         VECT_FLAG &= ~VECT_SV;
         VECT_FLAG |= VECT_NCONTRL;
         assert(!RcVectorAnalysis(optloop));
         assert(!RcVectorization(optloop));
         FinalizeVectorCleanup(optloop, 1);
      }
/*
 *    recompute the optloop and CFG
 */
      InvalidateLoopInfo();
      bbbase = NewBasicBlocks(bbbase);
      CheckFlow(bbbase, __FILE__,__LINE__);
      FindLoops();
      CheckFlow(bbbase, __FILE__, __LINE__);
   }
/*=============================================================================
 *    STATE3 : Transformation and optimization which should be applied after 
 *    vectorization (if applied with it)
 *    1. Unrolling with cleanup
 *    2. OptimizeLoopControl -> if neither Vect or Unroll
 *    3. Scalar Expansion
 *    4. Prefetching
 *
 *============================================================================*/
/*
 * unrolling ... ... LC optimization if neither unrolling nor Vectorization
 * NOTE: as after unrolling cleanup would be introduced. Traditional Killloop
 * control and OptimizeLoopControl will not work!!! Hence, update optloop also.
 * So, we should do the moving ptr analysis before/at the loop unrolling.
 * NOTE: if it is vectorized, this analysis is already done in vector analysis.
 *
 * NOTE: moving ptr analysis is essential for unrolling too. Why not we update
 * varrs at that time (if vectorization is not applied. )
 * NOTE: loop control may be changed. So, re-construct the CFG
 */
   if (FKO_UR > 1)
   {
/*
 *    NOTE: to use blind unroll, we need to reshape the scalar Restart code.
 */
      if (VECT_FLAG & VECT_SV)
      {
/*
 *       NOTE: running fall-thru after unrolling is not a good idea as it will
 *       traverse all the paths. If we use SB (strong bet) 8 for a double kernel
 *       there would be atleast 8*4 = 32 branch in scalar restart which means
 *       2^32 seperate paths!!!
 */
         /*TransformFallThruPath(1);*/

         ReshapeFallThruCode(); /* it requires very less time */
         
         InvalidateLoopInfo();
         bbbase = NewBasicBlocks(bbbase);
         CheckFlow(bbbase, __FILE__,__LINE__);
         FindLoops();
         CheckFlow(bbbase, __FILE__, __LINE__);
      }
/*
 *    NOTE: for complex cfg, we may need to apply TransformFallThruPath first;
 *    even not done the speculative vect.
 */
      UnrollLoop(optloop, FKO_UR); /* with modified cleanup */
   }
/*
 * apply unroll all the way. we will get rid of optloop at the end of 
 * fundamental opts
 */
   else if (FKO_UR == -1)
   {
      ur = CountUnrollFactor(optloop);
      if(ur > 1) 
         UnrollLoop(optloop, ur); /* with modified cleanup */
      else 
         FKO_UR = 0;
      
   }
   /* neither vectorize nor unrolled ! */
   else if (!DO_VECT(FKO_FLAG) && !(VECT_FLAG & VECT_INTRINSIC) ) 
   {
      AddOptWithOptimizeLC(optloop); 
      /*OptimizeLoopControl(optloop, 1, 1, NULL);*/
   }
   else ;
/*
 * reconstruct CFG for next opt
 */
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__,__LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
/*
 * State3: Scalar Expansion ... ... 
 */
   if (STATE3_FLAG & IFF_ST3_SE)
   {
      AddOptST4SE();
      if (optloop->se)
      {
/*
 *       Restrict SE if there is control flow but Rc or mmc is not applied
 *       Need to check for others later !!!
 *       FIXED: Scalar Expansion can't be applied for SV and Shadow VRC.
 *       need to check for shadow VRC to skip SE
 */
         if (IsControlFlowInLoop(optloop->blocks, optloop->header) && 
             !( (STATE1_FLAG & IFF_ST1_RC) || (STATE1_FLAG & IFF_ST1_MMR)) )
         {
/*
 *          NOTE: we need to describe why it is not allowed! 
 */
            fprintf(stderr, "SE not Applied right now!!!\n");
         }
         else if (VECT_FLAG & VECT_SHADOW_VRC)
         {
            fprintf(stderr, "SE not supported with shadow VRC !!!\n");
         }
         else
            DoAllScalarExpansion(optloop, FKO_UR, DO_VECT(FKO_FLAG));   
      }
   }
/*
 * Prefetching ... ...
 * NOTE: To apply prefetching, we need to update optloop->varrs info. 
 * if vectorization is applied, it is already propulated in analysis phase. 
 */
  
   if (PFARR )  /* request for prefetch... IFF_ST3_PREF */
   {
      if (optloop->varrs)
         UpdateNamedLoopInfo();
      else assert(0); /* must have moving ptr for prefetch */
         
      if (optloop->pfarrs)
      { 
         if (!FKO_SB || !(VECT_FLAG & VECT_SV)) FKO_SB = 1;
         if (!FKO_UR) FKO_UR = 1;
         AddPrefetch(optloop, FKO_UR*FKO_SB);
      }
   }
/*
 * special stage: loop specialization for memory alignment for SIMD 
 * duplicate the optloop with unaligned load to handle all the
 * memory alignment. 
 * NOTE: this is done after all the fundamental optimizations sothat we can 
 * keep them too in the duplicated loop. 
 */
#if 1   
   if (VECT_FLAG 
         && VECT_FLAG != VECT_INTRINSIC 
         && VECT_FLAG != VECT_SLP
         && !SKIP_PEELING
         && IsAlignLoopSpecNeeded(optloop))
      UnalignLoopSpecialization(optloop);
#endif
/*
 * applied optloop all the way, now time to delete the optloop control 
 */
   if (FKO_UR == -1)
   {
      /*UnrollAllTheWay();*/
      DelLoopControl(optloop);
   }
/*
 * FINAL STAGE: 
 *    0. Apply arch specific peep hole optimization
 *    1. Apply repeatable optimization
 *    2. Non Temporal writes if requested
 *    3. Finalize the prologue epilogue
 *    4. General assembly from final LIL
 *    5. Free all memory for global data 
 * Now, it's time to apply repeatable optimizations 
 */
   FinalStage(fpout, optblks);
   
   return(0);
}

