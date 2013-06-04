#define IFKO_DECLARE
#include "fko.h"
#include "fko_arch.h"
#include "fko_l2a.h"
#include "fko_loop.h"

FILE *fpST=NULL, *fpIG=NULL, *fpLIL=NULL, *fpOPT=NULL, *fpLOOPINFO=NULL;
int FUNC_FLAG=0; 
int DTnzerod=0, DTabsd=0, DTnzero=0, DTabs=0, DTx87=0, DTx87d=0;
int DTnzerods=0, DTabsds=0, DTnzeros=0, DTabss=0;
int FKO_FLAG;
int VECT_FLAG=0;  /* Majedul: for different vectorization methods */
/*
 * Majedul: 
 *    As we will introduce more and more new optimizations, I will keep 
 *    flag according to program states. 
 *    STATE1 has two optimization, hence 2 flag: 
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
int  USEALLIREG=0;

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
   fprintf(stderr, "  -c <LIL> <symtab> <misc> : generate files and quit\n");
   fprintf(stderr, "  -i <file> : generate loop info file and quit\n");
   fprintf(stderr, "  -o <outfile> : assembly output file\n");
   fprintf(stderr, "  -R [d,n] <directory/name> : restore path & base name\n");
   fprintf(stderr, "  -K 0 : suppress comments\n");
   fprintf(stderr, "  -t [S,I,L,o] : generate temporary files:\n");
   fprintf(stderr, "     S : dump symbol table to <file>.ST\n");
   fprintf(stderr, "     I : dump interference graph info to <file>.IG\n");
   fprintf(stderr, "     L : dump LIL <file>.L\n");
   fprintf(stderr, "     o : dump opt sequence to <file>.opt\n");
   fprintf(stderr, "  -U <#> : Unroll main loop # of times\n");
   fprintf(stderr, "  -V : Vectorize (SIMD) main loop\n");
   fprintf(stderr, "  -B : Stronger Bet unrolling for SV\n");
   fprintf(stderr, "  -M : Maximum paths to be analyzed in SV\n");
#if 0
   fprintf(stderr, 
"  -Vm [SSV,VRC,VEM]: Vectorize (SIMD) main loop with control flow\n");
   fprintf(stderr, "     SSV : Speculative vectorization \n");
   fprintf(stderr, "     VRC : Vector Redundant Computation \n");
   fprintf(stderr, "     VEM : Vector after elim of control flow for max\n");
#endif
   fprintf(stderr, 
         "  -p [#]: path to speculate in speculative vectirization\n");
   fprintf(stderr, "  -W <name> : use cache write-through stores\n");
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
   struct optblkq *op;
   op = calloc(1, sizeof(struct optblkq));
   assert(op);
   op->bnum = bnum;
   op->maxN = maxN;
   op->nopt = nopt;
   op->flag = flag;
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
   for (op=head; op && op->bnum != bnum; op = op->next);
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
   if (head->next)
      KillAllOptBlocks(head->next);
   if (head->down)
      KillAllOptBlocks(head->down);
   if (head->ifblk)
      KillAllOptBlocks(head->ifblk);
   if (head->opts) free(head->opts);
   free(head);
}

struct optblkq *SproutNode(struct optblkq *head, struct optblkq *seed)
/*
 * Changes opts list with numbers to pure opt + appropriate next, down
 */
{
   struct optblkq *new, *op;
   int n, nlist, i, j, k;
/*
 * Handle conditional blocks seperately
 */
   if (seed->ifblk)
   {
      new = NewOptBlock(seed->bnum, 0, 0, seed->flag);
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
   char *fin=NULL, *fout=NULL;
   struct optblkq *obq=NULL, *op;
   struct ptrinfo *pf, *pfb=NULL, *pf0, *pfK, *pfP, *aeb=NULL;
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
         case 'A':
            aeb = NewPtrinfo(i+1, atoi(args[i+2]), aeb);
            i += 2;
            break;
/*
 *       stronger bet unrolling for SV
 */
         case 'B':
            FKO_SB = atoi(args[++i]);
            break;
         
         case 'M':
            FKO_MaxPaths = atoi(args[++i]);
            break;
/*
 *       Majedul: To generalize the AccumExpansion with Scalar Expansion
 *       -SE var # 
 */
         case 'S':
            assert(args[i][2]=='E');
            seb = NewPtrinfo(i+1, atoi(args[i+2]), seb);
            i += 2;
            break;
         case 'W':
            id = malloc(sizeof(struct idlist));
            id->name = args[++i];
            id->next = idb;
            idb = id;
            break;
         case 'R':
            if (args[i+1][0] == 'd')
               rpath = args[i+2];
            else 
               rname = args[i+2];
            i += 2;
            break;
         case 'v':
            FKO_FLAG |= IFF_VERBOSE;
            break;
/*
 *       specify the path to speculate for speculative vectorization, this is
 *       optional.
 */
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
         case 'c':
            fpIG = fpST = fpLIL = fpOPT = (FILE*) 1;
            FKO_FLAG |= IFF_GENINTERM;
            break;
         case 'i':
            i++;
            if (!strcmp(args[i], "stderr"))
               fpLOOPINFO = stderr;
            else if (!strcmp(args[i], "stdout"))
               fpLOOPINFO = stdout;
            else
               fpLOOPINFO = fopen(args[i], "w");
            assert(fpLOOPINFO);
            break;
         case 'I':
            fpIG = fpST = fpLIL = fpOPT = (FILE*) 1;
            FKO_FLAG |= IFF_READINTERM;
            break;
         case 'U':
            FKO_UR = atoi(args[++i]);
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
         case 'K':
            j = atoi(args[++i]);
            if (!j) FKO_FLAG |= IFF_KILLCOMMENTS;
            break;
         case 'L':
         case 'G':
            op = NewOptBlock(atoi(args[i+1]), atoi(args[i+2]), atoi(args[i+3]),
                             args[i][1] == 'G' ? IOPT_GLOB : 0);
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
 *       -mmc = max/min reduction
 */
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
            assert(args[i][2] && args[i][3]);
            if (args[i][2] == 'm' && args[i][3] == 'r')
            {
               STATE1_FLAG |= IFF_ST1_MMR; 
            }
            else
            {
               fprintf(stderr, "Unknown flag '%s'\n", args[i]);
               PrintUsageN(args[0]);
            }
            break;
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
   short i, n;
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
 *    We saved the full structure, but need to invalidated all the pointers
 *    when restored.
 */
      assert(fwrite(optloop, sizeof(LOOPQ), 1, fp) == 1);
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
   short i, n;
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
   LOOPQ *lp;
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
/*
 * Need to check whether all the information is loaded successfully
 * Ofcourse, need to mark all pointers as NULL
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
      optloop->nopf = NULL;
      optloop->aaligned = NULL;
      optloop->abalign = NULL;
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

void KillAllGlobalData()
/*
 * This function is called at the exit of the program to free all memory, 
 * by this way, we can check whether there is any memory leak using Valgrind.
 */
{
   int i;
   extern struct locinit *ParaDerefQ;
   extern struct locinit *LIhead;

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
   LOOPQ *lp;
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


int DoOptList(int nopt, enum FKOOPT *ops, BLIST *scope0, int global)
/*
 * Performs the nopt optimization in the sequence given by ops, returning
 * 0 if no transformations applied, nonzero if some changes were made
 */
{
   BLIST *scope;
   BBLOCK *bp;
   int i, j, k, nchanges=0, nc0;
   static short nlab=0, labs[4];
   static int iopt = 0, bv = 0; /* Majedul: for opt logger, bv -> */
   BLIST *bl;
/*
 * Form scope based on global setting
 */
   if (!global)
   {
      if (!optloop)
         return(0);
/*
 *    Recalculate all optloop info if it has changed due to previous opt
 */
      if (!CFLOOP)
         FindLoops(); 
      scope = optloop->blocks;
   }
   else
      for (scope=NULL,bp=bbbase; bp; bp = bp->down)
         scope = AddBlockToList(scope, bp);
#if 0
   if (global)
   {
      fprintf(stdout, "Scope : " );
      for (bl=scope; bl; bl=bl->next )
          fprintf(stdout, " %d ",bl->blk->bnum);
      fprintf(stdout, "\n");
   }
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
   for (i=0; i < nopt; i++)
   {
      nc0 = nchanges;
      k = ops[i];
      switch(k)
      {
      case GlobRegAsg:
         assert(!global);
         nchanges += DoLoopGlobalRegAssignment(optloop);  
         break;
      case RegAsg:
         #if 0 
            fprintf(stderr, "%d: \n", ++bv);
            fprintf(stderr, "Before : ");
            PrintBVecInfo(stderr);
         #endif
         nchanges += DoScopeRegAsg(scope, global ? 2:1, &j);
         #if 0 
            fprintf(stderr, "After : ");
            PrintBVecInfo(stderr);
         #endif
         break;
      case CopyProp:
         nchanges += DoCopyProp(scope);
         break;
      case RemoveOneUseLoads:
         nchanges += DoRemoveOneUseLoads(scope);
         break;
      case LastUseLoadRemoval:
         nchanges += DoLastUseLoadRemoval(scope);
         break;
      case ReverseCopyProp:
         nchanges += DoReverseCopyProp(scope);
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
      PrintOptInst(stdout, ++iopt, k, scope, global, nchanges-nc0);
      fflush(stdout);  
#if 0
      if (k == RegAsg)
      {
         ShowFlow("cfg.dot", bbbase);
         exit(0);
      }
#endif      
      /*char file[20];*/
      /*sprintf(file, "cfg/%s%d.dot", "cfg", iopt);*/
      /*ShowFlow(file,bbbase);*/
#endif
   }
   return(nchanges);
}

int DoOptBlock(BLIST *gscope, BLIST *lscope, struct optblkq *op)
/*
 * Returns: nchanges applied
 */
{
   int i, nc, tnc=0, global;
   int maxN;
   struct optblkq *dp;
   BLIST *scope;

   global = op->flag & IOPT_GLOB;
   scope = global ? gscope : lscope;
/*
 * if we have conditional optimization block, handle it
 */
   if (op->ifblk)
   {
      tnc = DoOptBlock(gscope, lscope, op->ifblk);
      if (tnc)
      {
         if (op->down)
            tnc += DoOptBlock(gscope, lscope, op->down);
      }
      else if (op->next)
         tnc += DoOptBlock(gscope, lscope, op->next);
   }
/*
 * If we've got a one-time list, handle it
 */
   else if (!op->maxN)
      nc = DoOptList(op->nopt, op->opts, scope, global);
/*
 * Otherwise, we have normal while(changes) optblk
 */
   else
   {
      maxN = op->maxN;
      for (i=0; i < maxN; i++)
      {
         nc = DoOptList(op->nopt, op->opts, scope, global);
         for (dp=op->down; dp; dp = dp->down)
            nc += DoOptBlock(gscope, lscope, op);
         if (!nc)
            break;
         tnc += nc;
      }
      if (nc && (FKO_FLAG & IFF_VERBOSE))
         fprintf(stderr, "On last (%d) iteration, still had %d changes!\n",
                 maxN, nc);
   }
   if (op->next)
      tnc += DoOptBlock(gscope, lscope, op->next);
   return(tnc);
}

int PerformOptN(int SAVESP, struct optblkq *optblks)
/*
 * Returns: # of changes
 */
{
   BLIST *bl, *lbase;
   extern BBLOCK *bbbase;
   BBLOCK *bp;
   int nc;

   for (lbase=NULL,bp=bbbase; bp; bp = bp->down)
      lbase = AddBlockToList(lbase, bp);
   nc = DoOptBlock(lbase, optloop ? optloop->blocks : NULL, optblks);
   KillBlockList(lbase);
   INDEADU2D = CFUSETU2D = 0;
   return(nc);
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
   if (optloop && 1)
   {
      DoLoopGlobalRegAssignment(optloop);  
      optrec[noptrec++] = GlobRegAsg;
      do
      {
         j = DoScopeRegAsg(optloop->blocks, 1, &i);   
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
      j = DoScopeRegAsg(lbase, 2, &i);   
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
 * Defaults to command-line flags of:
 *     -L 1 0 6 ls gr 2 3 4 5 -G 2 10 3 bc uj ul -L 3 10 5 ra cp rc u1 lu
 *     -G 4 10 3 ra cp rc -L 5 10 5 ra cp rc u1 lu
 */
{
   struct optblkq *base, *op;

   if (DO_VECT(FKO_FLAG))
   {
/*
 * Majedul: consider EnforceLoadStore as global optimization, otherwise FABS in
 * Cleanup could not be updated ... solved this issue with a extra stage.
 * 
 */
#if 1
      op = base = NewOptBlock(1, 0, 5, 0);
      op->opts[0] = EnforceLoadStore;
      op->opts[1] = MaxOpt+2;
      op->opts[2] = MaxOpt+3;
      op->opts[3] = MaxOpt+4;
      op->opts[4] = MaxOpt+5;
#else
      op = base = NewOptBlock(1, 0, 5, IOPT_GLOB);
      op->opts[0] = EnforceLoadStore;
      op->opts[1] = MaxOpt+2;
      op->opts[2] = MaxOpt+3;
      op->opts[3] = MaxOpt+4;
      op->opts[4] = MaxOpt+5;
#endif
   }
   else
   {
      op = base = NewOptBlock(1, 0, 6, 0);
      op->opts[0] = EnforceLoadStore;
      op->opts[1] = GlobRegAsg;
      op->opts[2] = MaxOpt+2;
      op->opts[3] = MaxOpt+3;
      op->opts[4] = MaxOpt+4;
      op->opts[5] = MaxOpt+5;
   }

   op->next = NewOptBlock(2, 10, 3, IOPT_GLOB);
   op = op->next;
   op->opts[0] = BranchChain;
   op->opts[1] = UselessJmpElim;
   op->opts[2] = UselessLabElim;

   op->next = NewOptBlock(3, 10, 5, 0);
   op = op->next;
   op->opts[0] = RegAsg;
   op->opts[1] = CopyProp;
   op->opts[2] = ReverseCopyProp;
   op->opts[3] = RemoveOneUseLoads;
   op->opts[4] = LastUseLoadRemoval;

   op->next = NewOptBlock(4, 10, 5, IOPT_GLOB);
   op = op->next;
   op->opts[0] = RegAsg;
   op->opts[1] = CopyProp;
   op->opts[2] = ReverseCopyProp;
   op->opts[3] = RemoveOneUseLoads;
   op->opts[4] = LastUseLoadRemoval;

   op->next = NewOptBlock(5, 10, 5, 0);
   op = op->next;
   op->opts[0] = RegAsg;
   op->opts[1] = CopyProp;
   op->opts[2] = ReverseCopyProp;
   op->opts[3] = RemoveOneUseLoads;
   op->opts[4] = LastUseLoadRemoval;

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
   int n, i, j, k, N;
   short *sp;

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
            k = FindNameMatch(optloop->varrs[0],optloop->varrs+1,PFARR[i-1]);
            assert(k);
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
 *    FIXME: invalid read when applied following flags for sin:
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

/*void AddOptSTEntries1()*/
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
         else if (VarIsMax(optloop->blocks, st))
         {
            scf[i] = SC_MAX;
            ses[i-1] = DeclareMaxE(VEC, SEn[i], st);
         }
         else if (VarIsMin(optloop->blocks, st))
         {
            scf[i] = SC_MIN;
            ses[i-1] = DeclareMinE(VEC, SEn[i], st);
         }
#endif         
         else
         {
            fko_error(__LINE__, "%s not a candidate for scalar expansion\n",
                     SES[i-1]);
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

void GenerateAssemblyWithCommonOpts(FILE *fpout, struct optblkq *optblks,
                                    struct assmln *abase)
/*
 * NOTE: this is used temporarily to genarate assembly and test the output
 * will formalize later ... fpout, optblks, abase
 */
{
   int i; 
   extern struct locinit *ParaDerefQ;

   CalcInsOuts(bbbase);
   CalcAllDeadVariables();
   
   RevealArchMemUses(); /* to handle ABS in X86 */
   if (!CFUSETU2D)
   {
#if 0
      bbbase = NewBasicBlocks(bbbase);
      CheckFlow(bbbase, __FILE__, __LINE__);
      FindLoops();
      CheckFlow(bbbase, __FILE__, __LINE__);
#endif
#if 0
      fprintf(stdout, "LIL before Repeatable opt\n");
      PrintInst(stdout, bbbase);
      exit(0);
#endif      
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#if 0
   fprintf(stdout, "LIL before Repeatable Opt \n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif  

   PerformOptN(0, optblks);

#if 0
   fprintf(stderr, "BVEC after OPTN\n\n");
   PrintBVecInfo(stderr);
#endif   

#if 0
   PrintST(stdout);
   fprintf(stdout, "LIL after Repeatable Opt \n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif   


/*
 * Non Temporal write
 */
   if (NWNT)
   {
      NAWNT = DoStoreNT(NULL);
/*
 * Need to free ???? 
 */
   }


   INUSETU2D = INDEADU2D = CFUSETU2D = 0;
   if (!INDEADU2D)
      CalcAllDeadVariables();
   if (!CFLOOP)
      FindLoops();
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
#if 0
   PrintST(stdout);
   fprintf(stdout, "Final LIL \n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif   
   DumpOptsPerformed(stderr, FKO_FLAG & IFF_VERBOSE);
   abase = lil2ass(bbbase);
   KillAllBasicBlocks(bbbase);
   bbbase=NULL;                  /* whenever Kill, make it NULL */
   dump_assembly(fpout, abase);
   KillAllAssln(abase);
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
   int i, n, k;
   short *sp;

   if (!lp) return;
/*
 * killed the loop control to make the analysis easier, we will update the
 * loop control with OptimizeLoopControl again!
 */
#if 0
   fprintf(stdout, "lil\n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif

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

int main(int nargs, char **args)
/*
 * ==========================================================================
 * Majedul:
 * I'm going to change the program states of FKO. Here is short description
 * of that:
 *
 * State 0: Plain LIL (with no prologue/epilogue), initial ST
 *          no update in ST and optloop for AE/SE (Scalar Expansion)
 *
 * State 1:  
 *    a) Generate initial Prologue/Epilogue, so that we can create CFG and 
 *       update optloop info
 *    b) We will apply some optimization which is pre-requisit for some 
 *       some vectorization method. Eg.- 
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
 * State 4: (Common Optimization State)
 *    a) Reveal Architectural memory usage (Enable or disable)
 *    b) Repeatable Optimization
 *    b) Finalize Prologue and Epilogue
 *    c) LIL to Assembly
 *
 * Relationship of states if all applied (state0 and state4 are must):
 *    State0 <-- State1 <-- State2 <-- State3 <-- State4
 *
 * NOTE: 
 * I will fix the Save/Restore function for states. Initial plan: all states
 * are completely separate and the complete program states for each can be 
 * saved and restored.
 *===========================================================================
 */
{
   FILE *fpin, *fpout, *fpl;
   char *fin;
   char ln[512];
   struct assmln *abase;
   struct optblkq *optblks;
   BBLOCK *bp;
   int i;
   extern FILE *yyin;
   extern BBLOCK *bbbase;

/*
 * Update flags using command line options and create Optimization block 
 * structures. Note: It's not CFG block, it's block for opts and will be used
 * later in repeatable optimization.
 */
   optblks = GetFlagsN(nargs, args, &fin, &fpin, &fpout);
   if (!optblks)
      optblks = DefaultOptBlocks();
   optblks = OptBlockQtoTree(optblks);
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
 *    the appropriate prefix (0~3). I will update it after fixing the 
 *    SaveFKOState and RestoreFKOState functions.
 * 
 * NOTE: We need to find a way to continue the program flow after retrieving
 * the program states.
 */
   if (FKO_FLAG & IFF_READINTERM)
   {
      fprintf(stderr, 
             "Need to call restore function. It is not implemented yet\n");
      exit(0);
   }

/*===========================================================================
 *    STATE 0:
 *       1. Generate 1st LIL after parsing the HIL (Simple ld/st LIL).
 *       2. No CFG, No Prolgue/Epilogue, no extra info
 *==========================================================================*/

   yyin = fpin;
   bp = bbbase = NewBasicBlock(NULL, NULL);
   bp->inst1 = bp->instN = NULL;
   yyparse();
   fclose(fpin);
/*
 * To provide feedback to the tuning scripts 
 */
#if 0
/*
 * old implementation
 */
   SaveFKOState(0); 
   if (fpLOOPINFO)
   {
      PrintLoopInfo();
   }
#else   
   SaveFKOState0(); /* this Save function works for state0. Fixed it later */
/*
 * if we need information for tunning, Generate and return those info.
 * NOTE: info returns can be based on any states. PrintLoopInfo will provide
 * That. I will change the function later accordingly. Even may be, I will 
 * implement a new function for this.
 */
   if (fpLOOPINFO)
   {
      FeedbackLoopInfo();
      exit(0);
   }
#endif

#if 0
   if (FKO_FLAG & IFF_GENINTERM && state0) /*flag state0 is not impl yet */
   {
      SaveFKOState(0);
      exit(0);
   }
#endif   

/*===========================================================================
 *    STATE 1:
 *       1. Generate initial prologue/epilogue, CFG, find loops
 *       2. If conversion with Redundant computation if requested
 *       3. Max/Min Reduction, if requested
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
#if 0
   fprintf(stdout, "1st LIL");
   PrintInst(stdout, bbbase);
   //ShowFlow("cfg.dot", bbbase);
   //PrintLoop(stderr,optloop);
   exit(0);
#endif   
/*
 * NOTE: if there is no optloop, we can't perform transformation of 
 * State1~State3. So, we will jump to State4 to generate code
 */
   if (!optloop)
   {
      GenerateAssemblyWithCommonOpts(fpout, optblks, abase );
      KillAllGlobalData(); 
      return(0);
   }

#if 0   
   fprintf(stdout, "Before Fall-thru conversion\n");
   PrintInst(stdout, bbbase);
   fflush(stdout);
#endif   
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
#if 0      
      PrintLoopPaths();
      ShowFlow("fall.dot",bbbase);
#endif      
   }
#if 0   
   fprintf(stdout, "After Fall-thru conversion\n");
   PrintInst(stdout, bbbase);
   ShowFlow("fall.dot",bbbase);
   exit(0);
#endif   

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
      UpdateOptLoopWithMaxMinVars1(optloop);
      if (STATE1_FLAG & IFF_ST1_MMR)
      {
/*
 *       Right now, we will transform the whole if blk, if this blk is used only
 *       for determing the max/min. We will extend this to figure out the 
 *       max/min var and strip it out from the loop. 
 */
         ElimMaxMinIf();
#if 0 
         fprintf(stdout, "LIL after ElimMax/MinIf\n");
         PrintInst(stdout, bbbase);
         GenerateAssemblyWithCommonOpts(fpout, optblks, abase );
#endif
      }

      if (STATE1_FLAG & IFF_ST1_RC)
      {
#if 0 
         fprintf(stdout, "LIL Before RC\n");
         PrintInst(stdout, bbbase);
         ShowFlow("cfg.dot", bbbase);
         exit(0);
#endif         
         /*assert(!IfConvWithRedundantComp());*/
         assert(!IterativeRedCom());
#if 0 
         fprintf(stdout, "LIL after RC\n");
         PrintInst(stdout, bbbase);
         exit(0);
         GenerateAssemblyWithCommonOpts(fpout, optblks, abase );
#endif         
      } 
   }
   else
   {
      if (STATE1_FLAG & IFF_ST1_RC || STATE1_FLAG & IFF_ST1_MMR)
         fko_error(__LINE__, "No Control Flow in optloop to transform!!!\n");
   }

#if 0
   if (FKO_FLAG & IFF_GENINTERM && state1) /*flag state0 is not impl yet */
   {
      SaveFKOState(1);
      exit(0);
   }
#endif   
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
   if (FKO_FLAG & IFF_VECTORIZE)
   {
#if 0
         fprintf(stdout, "LIL before speculation test \n");
         PrintInst(stdout, bbbase);
         exit(0);
#endif
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
 *       Want to reshape the vector code always
 */
         ReshapeFallThruCode(); 
#if 0
         fprintf(stdout, "LIL AFTER LARGER BET \n");
         PrintInst(stdout, bbbase);
         //PrintST(stdout);
         exit(0);
#endif         
         #if 0
            //GenAssenblyApplyingOpt4SSV(fpout, optblks, abase);
            GenerateAssemblyWithCommonOpts(fpout, optblks, abase );
            exit(0);
         #endif
/*
 *                NOTE: need to parameterize the vectorization.
 *                Haven't fixed the issue if there is outof Regs!!!
 */
                 /* FKO_UR = 1;*/  /* forced to unroll factor 1*/
      }
      else
      {
         VECT_FLAG &= ~VECT_SV;
         VECT_FLAG |= VECT_NCONTRL;
         
         /*VectorAnalysis();*/
         assert(!SpeculativeVectorAnalysis());
         Vectorization();
         FinalizeVectorCleanup(optloop, 1);
      }
/*
 *    NOTE:
 *    old UnrollCleanup will not work anymore after calling this function.
 *    new UnrollCleanup should recognize the changes and update accordingly
 *    NOTE: as this cleanup updates the loopcontrol, must invalidate the old 
 *    one
 *    HERE HERE .... shifted up 
 */
/*
 *    NOTE: There is a issue here. If we reconstruct the CFG after vectorization
 *    we can't call existing KillLoopControl function. checking for cleanup 
 *    will split the basic block. Existing KillLoopControl function always
 *    check preheader for the CF_LOOP_INIT flag. So, it will fail the assertion.
 *    There can be two way to solve this issue:
 *       1. Update the KillLoopControl, make it robust and check preheader->up
 *          blk also for CF_LOOP_INIT. Need to update the AddLoopControl func 
 *          also to find out the flag.
 *       2. Keep track of the New CFG construction. Avoid creating this before
 *          unroll loop if vectorization is applied. 
 *    sol-2 may not work after implementing the unroll for speculative 
 *    vectorization. We may need to reconstruct CFG after Vspec!!
 *    DONE: KillLoopControl and AddLoopControl function is updated.!
 */
#if 0      
      if (FKO_UR <= 1)
      {
         InvalidateLoopInfo();
         bbbase = NewBasicBlocks(bbbase);
         CheckFlow(bbbase, __FILE__,__LINE__);
         FindLoops();
         CheckFlow(bbbase, __FILE__, __LINE__);
      }
#else
      InvalidateLoopInfo();
      bbbase = NewBasicBlocks(bbbase);
      CheckFlow(bbbase, __FILE__,__LINE__);
      FindLoops();
      CheckFlow(bbbase, __FILE__, __LINE__);
#endif
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
#if 0
         fprintf(stdout, "LIL AFTER UNROLL \n");
         PrintInst(stdout, bbbase);
         //PrintST(stdout);
         exit(0);
#endif         
   }
   else if (!DO_VECT(FKO_FLAG)) /* neither vectorize nor unrolled ! */
   {
      AddOptWithOptimizeLC(optloop); 
      /*OptimizeLoopControl(optloop, 1, 1, NULL);*/
   }
   else ;
/*
 * reconstruct for next opt
 */
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__,__LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
#if 0 
         fprintf(stdout, "LIL after Unrolling \n");
         PrintInst(stdout, bbbase);
         //exit(0);
         GenerateAssemblyWithCommonOpts(fpout, optblks, abase );
         exit(0);
#endif         

/*
 * Scalar Expansion ... ... 
 */
   if (STATE3_FLAG & IFF_ST3_SE)
   {
      AddOptST4SE();
      if (optloop->se)
      {
/*
 *       Restrict SE if there is control flow but Rc or mmc is not applied
 *       Need to check for others later !!!
 */
         /*fprintf(stderr, "se!\n");*/
         if (IsControlFlowInLoop(optloop->blocks, optloop->header) && 
             !( (STATE1_FLAG & IFF_ST1_RC) || (STATE1_FLAG & IFF_ST1_MMR)) )
         {
/*
 *          NOTE: we need to describe why it is not allowed! 
 */
            fprintf(stderr, "SE not Applied right now!!!\n");
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

#if 1 
         #if 0
            fprintf(stdout, "LIL Before Repeat Opt \n");
            PrintInst(stdout, bbbase);
            PrintST(stdout);
            exit(0);
         #else
            //ShowFlow("cfg.dot",bbbase);
            GenerateAssemblyWithCommonOpts(fpout, optblks, abase );
            KillAllGlobalData(); 
            //exit(0);
         #endif
#endif         

   return(0);

#if 0
/*==========================================================================
 * Previous code with 3 states:
 * 
 * State0: plain LIL but with updates of ST and optloop for AE
 * State1 & State3:  Generating Epilogue and Prologue, Vector analysis,
 *        Resrtore State0 code, then vectorization.
 * State2 : Applied only if Vectorization is not applied.
 *        Scalar Vars analyis, Restore State0 code, Epilogue/Prologue 
 *        generation, updates optloop with prefetche info.
 *==========================================================================*/


   optblks = GetFlagsN(nargs, args, &fin, &fpin, &fpout);
   if (!optblks)
      optblks = DefaultOptBlocks();
   optblks = OptBlockQtoTree(optblks);
   if (FKO_FLAG & IFF_READINTERM)
   {
      if (FKO_FLAG & IFF_VECTORIZE)
         RestoreFKOState(3);
      else
         RestoreFKOState(2);
   }
   else
   {
      yyin = fpin;
      bp = bbbase = NewBasicBlock(NULL, NULL);
      bp->inst1 = bp->instN = NULL;
      yyparse();
      fclose(fpin);
      AddOptSTEntries();
      SaveFKOState(0);
#if 0
      fprintf(stdout,"FIRST LIL\n");
      PrintInst(stdout,bbbase);
      PrintST(stdout);
      exit(0);
#endif
/*
 *    Majedul: 
 *    Consider Max/Min vars detection as global 
 *    NOTE: It can't be done at the AddOptSTEntires as max/min var analysis 
 *    needs CFG formation. For this, we need to generate the prologue and 
 *    epilogue. 
 *
 *    Note that GenPrologueEpilogueStubs should be called only once
 *    so, removed it from IsSpeculationNeeded(). For normal vectorization, 
 *    FKOstate0 code is retrieved anyway. So, it will not create problem 
 *    it state0 code is save without generating PrologueEpilogue.
 *
 *    HERE HERE, if we need Max/Min var analysis in NCONTRL vectorization, this
 *    information will be lost!!! As we consider Max/Min only inside optloop, 
 *    NCONTRL vectorization can't be applied if there is a Max/Min var. 
 */
      if (optloop && FKO_FLAG & IFF_VECTORIZE)
         UpdateOptLoopWithMaxMinVars();
      AddOptSTEntries1(); /* these two will be merge later */

      if (!fpLOOPINFO && (FKO_FLAG & IFF_VECTORIZE))
      {
         if (IsSpeculationNeeded() && 1)
         {
            /*UpdateOptLoopWithMaxMinVars(optloop);*/
            
            if (VECT_FLAG & VECT_NCONTRL)
            {

              /* fprintf(stderr, "Control Flow exists in main loop!!\n");*/
/*
 *             keep backward compatibility right now
 */
               #if defined(SSV)
                  VECT_FLAG &= ~VECT_NCONTRL;
                  VECT_FLAG |= VECT_SSV;
                  fprintf(stderr, "Considering macro(SSV) for vector!!\n");
               #elif defined(VRC)
                  VECT_FLAG &= ~VECT_NCONTRL;
                  VECT_FLAG |= VECT_VRC;
                  /*fprintf(stderr, "Considering macro(VRC) for vector!!\n");*/
               #elif defined(VEM)
                  VECT_FLAG &= ~VECT_NCONTRL;
                  VECT_FLAG |= VECT_VEM;
                  fprintf(stderr, "Considering macro(VEM) for vector!!\n");
               #else
                  fko_error(__LINE__, "Unknown Vectorization");
               #endif

            }

            if (VECT_FLAG & VECT_SSV)
            {
               #if IFKO_DEBUG_LEVEL > 1
                  fprintf(stderr,"Vectorization Method: SSV \n");
               #endif
               SpeculativeVectorAnalysis();
               SpecSIMDLoop();
               #if 1
                  GenAssenblyApplyingOpt4SSV(fpout, optblks, abase);
                  exit(0);
               #else
/*
 *                NOTE: need to parameterize the vectorization.
 *                Haven't fixed the issue if there is outof Regs!!!
 */
                  FKO_UR = 1; /* forced to unroll factor 1*/
               #endif
            }
            else if (VECT_FLAG & VECT_VRC)
            {
               #if IFKO_DEBUG_LEVEL > 1
                  fprintf(stderr,"Vectorization Method: VRC \n");
               #endif
               VectorRedundantComputation();
                  /*FKO_UR = 1;*/ /* forced to unroll factor 1*/
            }
            else if (VECT_FLAG & VECT_VEM)
            {
               #if IFKO_DEBUG_LEVEL > 1
                  fprintf(stderr,"Vectorization Method: VEM \n");
               #endif
               VectorizeElimIFwithMaxMin();
                  /*FKO_UR = 1;*/  /* forced to unroll factor 1*/
            }
            else ; /* no other choice */
         }
         else
         {
            assert(!VectorizeStage1());
            assert(!VectorizeStage3(0, 0));
         }
      }
      else
         DoStage2(0, 0);
   }
   
#if 0 
   fprintf(stdout,"LIL before optimization\n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif

   if (fpLOOPINFO)
      PrintLoopInfo();
   if (FKO_FLAG & IFF_GENINTERM)
      exit(0);
/*
 * If we were unable to produce correct code, back off and save sp
 */
   if (GoToTown(0, FKO_UR, optblks))
   {
      fprintf(stderr, "\n\nOut of regs for SAVESP, forcing full save.\n");
      USEALLIREG = 1;
      if (FKO_FLAG & IFF_VECTORIZE)
      {
         RestoreFKOState(1);
         assert(!VectorizeStage3(0, 0));
      }
      else
      {
         RestoreFKOState(0);
         DoStage2(0, 0);
      }
      if (GoToTown(0, FKO_UR, optblks))
      {
         USEALLIREG = 0;
         fprintf(stderr, 
                 "\n\nOut of registers for SAVESP, reserving SAVESP!!\n");
         if (FKO_FLAG & IFF_VECTORIZE)
         {
            RestoreFKOState(1);
            assert(!VectorizeStage3(IREGBEG+NIR-1, 0));
         }
         else
         {
            RestoreFKOState(0);
            DoStage2(IREGBEG+NIR-1, 0);
         }
         assert(!GoToTown(IREGBEG+NIR-1, FKO_UR, optblks));
      }
      else
         USEALLIREG = 0;
   }
   DumpOptsPerformed(stderr, FKO_FLAG & IFF_VERBOSE);

   if (!(FKO_FLAG & IFF_READINTERM))
   {
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
   }
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
/*   ShowFlow("dot.out", bbbase); */
   if (DO_ASS(FKO_FLAG))
   {
#if 0
      fprintf(stdout, "\n LIL BEFORE L2A: \n");
      PrintInst(stdout,bbbase);
      fprintf(stdout, "\n ST BEFORE L2A: \n");
      PrintST(stdout,bbbase);
      //exit(0);
#endif
      abase = lil2ass(bbbase);
      KillAllBasicBlocks(bbbase);
      bbbase = NULL;
      dump_assembly(fpout, abase);
      KillAllAssln(abase);
   }
   return(0);
#endif   
}

