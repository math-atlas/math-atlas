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
int PFISKIP=0, PFINST=(-1), PFCHUNK=1;
int NWNT=0;
short *AEn=NULL;
char **ARRWNT=NULL, **AES=NULL;
static char fST[1024], fLIL[1024], fmisc[1024];
static short *PFDST=NULL, *PFLVL=NULL;
static char **PFARR=NULL;

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
   fprintf(stderr, "  -V     : Vectorize (SIMD) main loop\n");
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
         pfP = pf;
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
         case 'V':
            FKO_FLAG |= IFF_VECTORIZE;
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
 *    Count # of such arrays, then store them in ARRNT
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
         nchanges += DoScopeRegAsg(scope, global ? 2:1, &j);
         break;
      case CopyProp:
         nchanges += DoCopyProp(scope);
         break;
      case RemoveOneUseLoads:
         nchanges += DoRemoveOneUseLoads(scope);
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
         UnrollLoop(optloop, unroll);
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
/*
 *    Do accumulator expansion if requested
 */
      if (optloop->ae)
         DoAllAccumExpansion(optloop, unroll, DO_VECT(FKO_FLAG));
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

#if 1
   PerformOptN(SAVESP, optblks);
#else
   assert(!PerformOpt(SAVESP));
#endif

   if (NWNT)
   {
      i = DoStoreNT(NULL);
      for (j=0; j < NWNT; j++)
         free(ARRWNT[j]);
      free(ARRWNT);
      NWNT = i;
   }
   if (!INDEADU2D)
      CalcAllDeadVariables();
   if (!CFLOOP)
      FindLoops();
   AddBlockComments(bbbase);
   AddLoopComments();
#if 1
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
      if (NWNT)
      {
         fprintf(fpout, "%3d. %c %20.20s : %d\n", j+i+1, 'G', 
                 "Non-cached writes", NWNT);
      }
   }
}

struct optblkq *DefaultOptBlocks(void)
/*
 * Defaults to command-line flags of:
 *     -L 1 0 6 ls gr 2 3 4 5 -G 2 10 3 bc uj ul -L 3 10 3 ra cp rc -G 4 0 1 ls 
 *     -G 5 10 3 ra cp rc
 */
{
   struct optblkq *base, *op;

   if (DO_VECT(FKO_FLAG))
   {
      op = base = NewOptBlock(1, 0, 5, 0);
      op->opts[0] = EnforceLoadStore;
      op->opts[1] = MaxOpt+2;
      op->opts[2] = MaxOpt+3;
      op->opts[3] = MaxOpt+4;
      op->opts[4] = MaxOpt+5;
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

   op->next = NewOptBlock(3, 10, 3, 0);
   op = op->next;
   op->opts[0] = RegAsg;
   op->opts[1] = CopyProp;
   op->opts[2] = ReverseCopyProp;

   op->next = NewOptBlock(4, 0, 1, IOPT_GLOB);
   op = op->next;
   op->opts[0] = EnforceLoadStore;

   op->next = NewOptBlock(5, 10, 3, IOPT_GLOB);
   op = op->next;
   op->opts[0] = RegAsg;
   op->opts[1] = CopyProp;
   op->opts[2] = ReverseCopyProp;

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
      sp = optloop->pfflag+1;
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
            for (j=i=1; i <= n; i++)
            {
               if (optloop->pfflag[i] != -1)
               {
                  optloop->pfarrs[j] = optloop->pfarrs[i];
                  optloop->pfdist[j] = optloop->pfdist[i];
                  optloop->pfflag[j] = optloop->pfflag[i];
                  j++;
               }
               optloop->pfarrs[0] = optloop->pfdist[0] = optloop->pfflag[0] =
                                    N-k;
            }
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

int main(int nargs, char **args)
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
      if (!fpLOOPINFO && (FKO_FLAG & IFF_VECTORIZE))
      {
         assert(!VectorizeStage1());
         assert(!VectorizeStage3(0, 0));
      }
      else
         DoStage2(0, 0);
   }
   if (fpLOOPINFO)
      PrintLoopInfo();
   if (FKO_FLAG & IFF_GENINTERM)
      exit(0);
/*
 * If we were unable to produce correct code, back off and save sp
 */
   if (GoToTown(0, FKO_UR, optblks))
   {
      fprintf(stderr, "\n\nOut of registers for SAVESP, trying again!!\n");
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
      abase = lil2ass(bbbase);
      KillAllBasicBlocks(bbbase);
      dump_assembly(fpout, abase);
      KillAllAssln(abase);
   }
   return(0);
}
