#define NO_STEXTERN
#include "fko.h"

char **STname;
union valoff *SToff;
int *STflag;
static int N=0, Nalloc=0;
static int niloc=0, nlloc=0, nfloc=0, ndloc=0, nvfloc=0, nvdloc=0;
int    LOCSIZE=0, LOCALIGN=0, NPARA=0;

#define STCHUNK 256
#define DTCHUNK 256

static void GetNewSymtab(int chunk)
{
   char **nam;
   int *flg;
   union valoff *off;
   int i, n;
   n = Nalloc + chunk;
   nam = malloc(sizeof(char*)*n);
   assert(nam);
   off = malloc(n*sizeof(union valoff));
   flg = malloc(n*sizeof(int));
   assert(off && flg);
   if (Nalloc > 0)
   {
      for (i=0; i < N; i++)
      {
         nam[i] = STname[i];
         off[i].d = SToff[i].d;
         flg[i] = STflag[i];
      }
      free(STname);
      free(SToff);
      free(STflag);
   }
   STname = nam;
   SToff = off;
   STflag = flg;
   Nalloc = n;
}

static short STnew(char *name, int flag, union valoff off)
{
   if (N == Nalloc) GetNewSymtab(STCHUNK);
   if (name)
   {
      STname[N] = malloc(strlen(name)+1);
      assert(STname[N]);
      strcpy(STname[N], name);
   }
   else STname[N] = NULL;
   STflag[N] = flag;
   SToff[N] = off;
   return(++N);
}

short STdef(char *name, int flag, int off)
{
   union valoff offset;
   offset.i = off;
   return(STnew(name, flag, offset));
}

short STdconstlookup(double f)
/*
 * Searches for fp constant f in symbol table, allocating new entry if not
 * already there. 
 * RETURNS: symtab index.
 */
{
   short i;
   union valoff val;
   static int ndc=0;
   char name[16];
   for (i=0; i != N; i++)
   {
      if (IS_CONST(STflag[i]) && IS_DOUBLE(STflag[i]) && SToff[i].d == f)
         return(i);
   }
   sprintf(name, "_FPDC_%d", ndc);
   ndc++;
   val.d = f;
   return(STnew(name, GLOB_BIT | CONST_BIT | T_DOUBLE, val));
}

short STfconstlookup(float f)
/*
 * Searches for fp constant f in symbol table, allocating new entry if not
 * already there. 
 * RETURNS: symtab index.
 */
{
   short i;
   static int nfc=0;
   char name[16];
   union valoff val;
   for (i=0; i != N; i++)
   {
      if (SToff[i].f == f && IS_CONST(STflag[i]) && IS_FLOAT(STflag[i]))
         return(i+1);
   }
   sprintf(name, "_FPFC_%d", nfc);
   nfc++;
   val.f = f;
   return(STnew(name, GLOB_BIT | CONST_BIT | T_FLOAT, val));
}


short STlconstlookup(long ic)
/*
 * Searches for long constant ic in symbol table, allocating new entry if not
 * already there. 
 * RETURNS: symtab index.
 */
{
   short i;
   union valoff val;
   for (i=0; i != N; i++)
   {
      if (SToff[i].l == ic && IS_CONST(STflag[i]) && IS_INT(STflag[i]))
         return(i+1);
   }
   val.l = ic;
   return(STnew(NULL, CONST_BIT | T_SHORT, val));
}

short STiconstlookup(int ic)
/*
 * Searches for int constant ic in symbol table, allocating new entry if not
 * already there. 
 * RETURNS: symtab index.
 */
{
   short i;
   union valoff val;
   for (i=0; i != N; i++)
   {
      if (SToff[i].i == ic && IS_CONST(STflag[i]) && IS_INT(STflag[i]))
         return(i+1);
   }
   val.i = ic;
   return(STnew(NULL, CONST_BIT | T_INT, val));
}

short STstrlookup(char *str)
/*
 * Searches for string str in symbol table.
 * RETURNS: symtab index.
 */
{
   short i;
   for (i=0; i != N; i++)
   {
      if (STname[i] && !IS_CONST(STflag[i]) && !strcmp(str, STname[i]))
         return(i+1);
   }
   return(0);
}

short STFindLabel(char *str)
{
   short i;
   for (i=0; i != N; i++)
   {
      if (STname[i] && IS_LABEL(STflag[i]) && !strcmp(str, STname[i]))
         return(i+1);
   }
   return(0);
}
short STlabellookup(char *str)
/*
 * Searches for label with name str in symtab, allocating new entry if
 * not already there.
 * RETURNS: symtab index.
 */
{
   short i;
   i = STFindLabel(str);
   if (!i) 
     i = STdef(str, T_LABEL, 0);
   return(i);
}

short STstrconstlookup(char *str)
/*
 * Searches for string constant str in symbol table, allocating new entry if
 * not lready there. 
 * RETURNS: symtab index.
 */
{
   short i;
   for (i=0; i != N; i++)
   {
      if (STname[i] && IS_CONST(STflag[i]) && !strcmp(str, STname[i]))
         return(i+1);
   }
   return(STdef(str, CONST_BIT | T_CHAR, 0));
}

char *STi2str(short i)
{
   #if IFKO_DEBUG_LEVEL > 0
      if (i < 1 || i > N)
         fko_error(__LINE__, "ST Index %d out of range (1,%d), file=%s",
                   i, N, __FILE__);
   #endif
   return(STname[i-1] ? STname[i-1] : "NULL");
}

void STsetoffi(short i, int off)
{
    SToff[i-1].i = off;
}
void STsetflag(short i, int flag)
{
    STflag[i-1] = flag;
}

void CreateFPLocals()
/*
 * iFKO handles floating point constants by allocating them as locals on
 * the stack frame, and writing their values during the function prologue.
 * This function adds the fp consts as locals, so that NumberLocalsByType
 * will contain the fp consts.  CreatePrologue then writes the appropriate
 * values to the frame later, looking for ST entries that are both local
 * and constant.  For these entries, finds original ST entry in offset,
 * and takes const value from there.
 */
{
   short k, n=0, i;
   int fl;
   char ln[256];

   for (k=0; k < N; k++)
   {
      fl = STflag[k];
      if (IS_CONST(fl) && (IS_DOUBLE(fl) || IS_FLOAT(fl)))
      {
         sprintf(ln, "_FPC_%d\n", n++);
         i = STdef(ln, (fl & (!GLOB_BIT)) | LOCAL_BIT, k+1);
      }
   }
}

short FindLocalFPConst(short gfp)
/*
 * Assuming that stack frame has been fully qualified, finds the local
 * variable corresponding to the global floating point const gfp
 */
{
   int flag, fptype;
   short k;
   fptype = FLAG2TYPE(STflag[gfp-1]);
   for (k=0; k < N; k++)
   {
      flag = STflag[k];
      if (FLAG2TYPE(flag) == fptype && IS_LOCAL(flag) && IS_CONST(flag) &&
          SToff[k].sa[1] == gfp)
         return(k+1);
   }
   return(0);
}

void NumberLocalsByType()
/*
 * Searches symbol table for all locals and parameters, and assigns
 * them a slot in the type-specific section of the frame
 */
{
   short k;
   int fl, type;

   for (k=0; k != N; k++)
   {
      fl = STflag[k];
      if ((IS_PARA(fl) || IS_LOCAL(fl)) && SToff[SToff[k].sa[2]-1].sa[0])
      {
         type = FLAG2PTYPE(fl);
         switch(type)
         {
         case T_INT:
            SToff[k].sa[1] = niloc++;
            break;
         case T_FLOAT:
            if (IS_VEC(fl)) SToff[k].sa[1] = nvfloc++;
            else SToff[k].sa[1] = nfloc++;
            break;
         case T_DOUBLE:
            if (IS_VEC(fl)) SToff[k].sa[1] = nvdloc++;
            else SToff[k].sa[1] = ndloc++;
            break;
         }
         #if IFKO_DEBUG_LEVEL > 1
            fprintf(stderr, "%c: %s gets slot %d\n", IS_PARA(fl) ? 'P' : 'L', 
                    STname[k], SToff[k].sa[1]);
         #endif
      }
   }
}

void CreateLocalDerefs()
/*
 * This routine creates placeholder deref entries for each local variable.
 * The correct offsets will be figured as essentially the last step before
 * lil-to-assembly conversion. 
 * For paras, moves para # to SToff[].sa[0].
 */
{
   short k;
   int fl, nl=0;
   extern int NPARA;
   for (k=0; k != N; k++)
   {
      fl = STflag[k];
      if (IS_PARA(fl))
      {
         SToff[k].sa[0] = SToff[k].i;
         SToff[k].sa[2] = AddDerefEntry(-REG_SP, k+1, -k-1, 0);
      }
      else if (IS_LOCAL(fl))
         SToff[k].sa[2] = AddDerefEntry(-REG_SP, k+1, -k-1, 0);
   }
}

void UpdateLocalDerefs(int isize)
/*
 * Given numbered locals, creates derefs for local access, assuming local
 * area starts at the stack pointer.  Puts DT[i+2] = -1 to denote that
 * the address is not yet fully formed (since local area will almost
 * always be offset from stack pointer).
 * isize is the size in bytes of an integer on the arch in question.
 */
{
   short k, off, h, i;
   int fl;
   int GetArchAlign(int nvd, int nvf, int nd, int nf, int nl, int ni);

   for (k=0; k != N; k++)
   {
      fl = STflag[k];
      if ((IS_PARA(fl) || IS_LOCAL(fl)) && SToff[SToff[k].sa[2]-1].sa[0])
      {
/* fprintf(stderr, "Updating local %s\n", STname[k]); */
         switch(FLAG2PTYPE(fl))
         {
         case T_INT:
            off = SToff[k].sa[1]*isize + ndloc*8 + 
                  nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4;
            break;
         case T_FLOAT:
            if (IS_VEC(fl))
               off = SToff[k].sa[1]*FKO_SVLEN*4 + nvdloc*FKO_DVLEN*8;
            else
               off = SToff[k].sa[1]*4 + nvdloc*FKO_DVLEN*8 + 
                     nvfloc*FKO_SVLEN*4 + ndloc*8 + niloc*isize;
            break;
         case T_DOUBLE:
            if (IS_VEC(fl))
               off = SToff[k].sa[1]*FKO_DVLEN*8;
            else
               off = SToff[k].sa[1]*8 + nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4;
            break;
         default:
            fprintf(stderr, "%d: Unknown type %d!\n", __LINE__, FLAG2PTYPE(fl));
            exit(-1);
         }
         SToff[SToff[k].sa[2]-1].sa[3] = off;
      }
   }
   LOCALIGN = GetArchAlign(nvdloc, nvfloc, ndloc, nfloc, nlloc, niloc);
   LOCSIZE = nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4 + 
             ndloc*8 + niloc*isize + nfloc*4;
}

void CorrectLocalOffsets(int ldist)
/*
 * After stack frame is fully resolved, we know the distance between the
 * stack pointer, and the start of the local area, ldist.  Add this to
 * our offset within the local area to get the correct local addressing.
 * NOTE: ldist includes any required padding to get to highest required
 *       local alignment.
 */
{
   int i;
   for (i=0; i != N; i++)
   {
      if (IS_DEREF(STflag[i]) && SToff[i].sa[0] == -REG_SP && 
          SToff[i].sa[1] > 0 && SToff[i].sa[2] < 0)
      {
/* fprintf(stderr, "correcting local %d, off=%d+%d!\n", i+1, SToff[i].sa[3], ldist); */
         SToff[i].sa[2] = 1;
         SToff[i].sa[3] += ldist;
      }
   }
}

void CorrectParamDerefs(struct locinit *libase, int rsav, int fsize)
/*
 * Given the frame size (fsize) and a queue of parameters derefs to
 * correct (libase), update the derefences by setting DT[0] = rsav and
 * adding frame size to the constant
 */
{
   struct locinit *lp;
   short k;

   for (lp = libase; lp; lp = lp->next)
   {
      k = lp->id-1;
/* fprintf(stderr, "correcting para %d, off=%d+%d!\n", lp->id, SToff[k].sa[3], fsize); */
      SToff[k].sa[0] = rsav;
      SToff[k].sa[2] = 1;
      SToff[k].sa[3] += fsize;
   }
}

void MarkUnusedLocals(BBLOCK *bbase)
/*
 * Looks at all derefences in entire program; if a local/param is never used,
 * changes first DT entry (presently -REG_SP) to 0
 * So, before this routine locals are marked by -REG_SP in base of DT, and
 * < 0 in mul; after this routine it is < 0 in mul, but -REG_SP in base
 * for used, and 0 in base for unused.
 */
{
   INSTQ *ip;
   short k, i;
   BBLOCK *bp;
/*
 * Start out by marking all locals as unused
 */
   for (k=0; k != N; k++)
   {
      if (IS_DEREF(STflag[k]) && SToff[k].sa[0] == -REG_SP &&
          SToff[k].sa[2] < 0)
      {
         SToff[k].sa[0] = 0;
      }
   }
   for (bp=bbase; bp; bp = bp->down)
   {
      for (ip=bp->inst1; ip; ip = ip->next)
      {
         for (k=1; k <= 3; k++)
         {
            i = ip->inst[k]-1;
            if (i >= 0 && IS_DEREF(STflag[i]) && !SToff[i].sa[0] && 
                SToff[i].sa[2] < 0)
            {
               SToff[i].sa[0] = -REG_SP;
            }
         }
      }
   }
}

int STlen(void)
{
   return(N);
}

#if 0
void AddStaticData(char *name, short align, short len, void *vbytarr)
{
   struct sdata *np;
   unsigned char *bytarr = vbytarr;
   short i;

   np = malloc(sizeof(struct sdata));
   assert(np);
   np->align = align;
   np->vals = malloc(len);
   assert(np->vals);
   np->name = name;
   np->len = len;
   for (i=0; i < len; i++) np->vals[i] = bytarr[i];
   np->next = SDhead;
   SDhead = np;
}

void KillStaticData(void)
{
   struct sdata *np;
   while(SDhead)
   {
      np = SDhead->next;
      free(np->vals);
      free(SDhead);
      SDhead = np;
   }
}
#endif

/* 
 * Every pointer dereference gets an entry in the dereference table.
 * Right now, deref table consists of 4 shorts:
 * <ptr> <reg> <mul> <con>
 * all of which except mul & con are indexes into the symbol table.  Mul and
 * con are simply short constants.
 * Addressing is: ptr+reg*mul+con
 */
short FindDerefEntry(short ptr, short ireg, short mul, short con)
{
   int i;
   for (i=0; i != N; i++)
   {
      if ( IS_DEREF(STflag[i]) && SToff[i].sa[0] == ptr && 
           SToff[i].sa[2] == mul && SToff[i].sa[3] == con &&
           (SToff[i].sa[1] == ireg || (!ireg && SToff[i].sa[1] > 0)) )
              return(i+1);
   }
   return(0);
}

short AddDerefEntry(short ptr, short reg, short mul, short con)
{
   int i;
   i = STdef("DT", DEREF_BIT | FLAG2TYPE(STflag[ptr-1]), 0) - 1;
   SToff[i].sa[0] = ptr;
   SToff[i].sa[1] = reg;
   SToff[i].sa[2] = mul;
   SToff[i].sa[3] = con;
   return(i+1);
}

short FindLocalFromDT(short dt)
/*
 * given a DT entry, searches ST for any local that uses that dereference
 * RETURNS: ST index of local using deref dt, or 0 if not found
 */
{
   int i;
   for (i=0; i != N; i++)
   {
      if (IS_LOCAL(STflag[i]))
      {
         if (SToff[i].sa[2] == dt)
            return(i+1);
      }
   }
   return(0);
}
static char flag2pre(int flag)
{
   char pre='?';
   if (IS_INT(flag)) pre = 'i';
   else if (IS_DOUBLE(flag)) pre = 'd';
   else if (IS_FLOAT(flag)) pre = 'f';
   return(pre);
}
void PrintST(FILE *fpout)
{
   int flag;
   short k;
   char pre;
   char *what;

   fprintf(fpout, "\n                                  SYMBOL TABLE:\n");
   fprintf(fpout, "                                                                 PTR,REG,MUL,CON\n");
   fprintf(fpout, "INDEX  FLAG              NAME                     WHAT                    VALUE\n");
   fprintf(fpout, "===== ===== ================================== ======= =========================\n\n");
   for (k=0; k != N; k++)
   {
      flag = STflag[k];
      fprintf(fpout, "%5d %5d %34.34s", k+1, flag, 
              STname[k] ? STname[k] : "NULL");
      pre = '?';
      if (IS_DEREF(flag))
         fprintf(fpout, "   DEREF %4d,%4d,%4d,%4d\n",
                 SToff[k].sa[0],SToff[k].sa[1],SToff[k].sa[2],SToff[k].sa[3]);
      else if (IS_LOCAL(flag) || IS_CONST(flag) || IS_PARA(flag))
      {
         if (IS_LOCAL(flag)) what = "LOCAL";
         else if (IS_PARA(flag)) what = "PARAM";
         else what = "CONST";

         if (IS_INT(flag))
            fprintf(fpout, " i%6.6s %25d\n", what, SToff[k].i);
         else if (IS_FLOAT(flag))
            fprintf(fpout, " f%6.6s %25.3f\n", what, SToff[k].f);
         else if (IS_DOUBLE(flag))
            fprintf(fpout, " d%6.6s %25.3lf\n", what, SToff[k].d);
         else 
            fprintf(fpout, " ?%6.6s UNKNOWN\n", what);
      }
      else if (IS_LABEL(flag))
         fprintf(fpout, "   LABEL\n");
      else if (IS_PTR(flag))
      {
         fprintf(fpout, "   %c PTR %4d,%4d,%4d,%4d", flag2pre(flag), 
	         SToff[k].sa[0],SToff[k].sa[1],SToff[k].sa[2],SToff[k].sa[3]);
      }
      else
         fprintf(fpout, " UNKNOWN UNKNOWN\n");
   }
}

static void KillSTStrings()
{
   int i;
   if (STname)
   {
      for (i=0; i < N; i++)
         if (STname[i]) free(STname[i]);
      free(STname);
   }
}

void ReadSTFromFile(char *fname)
{
   int i, len;
   FILE *fp;
   #ifdef X86
      extern int DTnzerod, DTabsd, DTnzero, DTabs, DTx87, DTx87d;
   #endif
   extern short STderef;

   fp = fopen(fname, "rb");
   assert(fp);
   KillSTStrings();
   if (SToff) free(SToff);
   if (STflag) free(STflag);
   Nalloc = N = 0;
   assert(fread(&len, sizeof(int), 1, fp) == 1);
   GetNewSymtab(((len+STCHUNK+1)/STCHUNK)*STCHUNK);
   N = len;
   assert(fread(&niloc, sizeof(int), 1, fp) == 1);
   assert(fread(&nlloc, sizeof(int), 1, fp) == 1);
   assert(fread(&nfloc, sizeof(int), 1, fp) == 1);
   assert(fread(&ndloc, sizeof(int), 1, fp) == 1);
   assert(fread(&nvfloc, sizeof(int), 1, fp) == 1);
   assert(fread(&nvdloc, sizeof(int), 1, fp) == 1);
   assert(fread(&LOCALIGN, sizeof(int), 1, fp) == 1);
   assert(fread(&LOCSIZE, sizeof(int), 1, fp) == 1);
   assert(fread(&NPARA, sizeof(int), 1, fp) == 1);
   #ifdef X86
      assert(fread(&DTnzerod, sizeof(int), 1, fp) == 1);
      assert(fread(&DTabsd, sizeof(int), 1, fp) == 1);
      assert(fread(&DTx87d, sizeof(int), 1, fp) == 1);
      assert(fread(&DTnzero, sizeof(int), 1, fp) == 1);
      assert(fread(&DTabs, sizeof(int), 1, fp) == 1);
      assert(fread(&DTx87, sizeof(int), 1, fp) == 1);
   #endif
   assert(fread(&STderef, sizeof(short), 1, fp) == 1);
   assert(fread(SToff, sizeof(union valoff), N, fp) == N);
   assert(fread(STflag, sizeof(int), N, fp) == N);
/* 
 * Must read in all strings as N <len,string> pairs
 */
   for (i=0; i < N; i++)
   {
      assert(fread(&len, sizeof(int), 1, fp) == 1);
      if (len > 0)
      {
         STname[i] = malloc(sizeof(char)*(len+1));
         assert(STname[i]);
         assert(fread(STname[i], sizeof(char), len, fp) == len);
         STname[i][len] = '\0';
      }
      else
         STname[i] = NULL;
   }
   fclose(fp);
}

void WriteSTToFile(char *fname)
{
   int i, len;
   FILE *fp;
   #ifdef X86
      extern int DTnzerod, DTabsd, DTnzero, DTabs, DTx87, DTx87d;
   #endif
   extern short STderef;

   fp = fopen(fname, "wb");
   assert(fp);
   assert(fwrite(&N, sizeof(int), 1, fp) == 1);
   assert(fwrite(&niloc, sizeof(int), 1, fp) == 1);
   assert(fwrite(&nlloc, sizeof(int), 1, fp) == 1);
   assert(fwrite(&nfloc, sizeof(int), 1, fp) == 1);
   assert(fwrite(&ndloc, sizeof(int), 1, fp) == 1);
   assert(fwrite(&nvfloc, sizeof(int), 1, fp) == 1);
   assert(fwrite(&nvdloc, sizeof(int), 1, fp) == 1);
   assert(fwrite(&LOCALIGN, sizeof(int), 1, fp) == 1);
   assert(fwrite(&LOCSIZE, sizeof(int), 1, fp) == 1);
   assert(fwrite(&NPARA, sizeof(int), 1, fp) == 1);
   #ifdef X86
      assert(fwrite(&DTnzerod, sizeof(int), 1, fp) == 1);
      assert(fwrite(&DTabsd, sizeof(int), 1, fp) == 1);
      assert(fwrite(&DTx87d, sizeof(int), 1, fp) == 1);
      assert(fwrite(&DTnzero, sizeof(int), 1, fp) == 1);
      assert(fwrite(&DTabs, sizeof(int), 1, fp) == 1);
      assert(fwrite(&DTx87, sizeof(int), 1, fp) == 1);
   #endif
   assert(fwrite(&STderef, sizeof(short), 1, fp) == 1);
   assert(fwrite(SToff, sizeof(union valoff), N, fp) == N);
   assert(fwrite(STflag, sizeof(int), N, fp) == N);
/* 
 * Must read in all strings as N <len,string> pairs
 */
   for (i=0; i < N; i++)
   {
      if (STname[i])
      {
         len = strlen(STname[i]);
         assert(fwrite(&len, sizeof(int), 1, fp) == 1);
         assert(fwrite(STname[i], sizeof(char), len, fp) == len);
      }
      else
      {
         len = 0;
         assert(fwrite(&len, sizeof(int), 1, fp) == 1);
      }
   }
   fclose(fp);
}
