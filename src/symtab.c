#define NO_STEXTERN
#include "ifko.h"

char **STname;
union valoff *SToff;
int *STflag;
short *DT;
static int N=0, Nalloc=0, Ndt=0, Ndtalloc=0;
static int niloc=0, nlloc=0, nfloc=0, ndloc=0, nvfloc=0, nvdloc=0;
int    LOCSIZE=0, LOCALIGN=0, NPARA=0;
struct sdata *SDhead=NULL;

#define STCHUNK 256
#define DTCHUNK 256

/* 
 * Every pointer dereference gets an entry in the dereference table.
 * Right now, deref table consists of 4 shorts:
 * <ptr> <reg> <mul> <con>
 * all of which except mul are indexes into the symbol table.  Mul is simply
 * a short multiplier constant.
 * Addressing is: ptr+reg*mul+con
 * NOTE: this structure will likely expand.
 */
static void GetNewDereftab(int chunk)
{
   short *dtn;
   int i, n;
   Ndtalloc += chunk;
   dtn = malloc(4*Ndtalloc * sizeof(short));
   assert(dtn);
   if (DT)
   {
      for (n=4*Ndt, i=0; i != n; i++) dtn[i] = DT[i];
      free(DT);
   }
   DT = dtn;
}

void CleanDereftab()
/*
 * Searches and removes nullified deref entries
 */
{
   int i, j, h, k;
   for (i=j=0; i < Ndt; i++)
   {
      k = i << 2;
      if (DT[k])
      {
          if (i != j)
          {
             h = j << 2;
             DT[h] = DT[k];
             DT[h+1] = DT[k+1];
             DT[h+2] = DT[k+2];
             DT[h+3] = DT[k+3];
          }
          j++;
      }
   }
   Ndt = j;
}

short FindDerefEntry(short ptr, short ireg, short mul, short con)
{
   int i, n=4*Ndt;
   for (i=0; i != n; i += 4)
       if (DT[i] == ptr && DT[i+1] == ireg && DT[i+2] == mul && DT[i+3] == con)
          return((i>>2)+1);
   return(0);
}

short AddDerefEntry(short ptr, short reg, short mul, short con)
{
   int i = 4*Ndt;
   if (Ndt == Ndtalloc) GetNewDereftab(DTCHUNK);
   DT[i] = ptr;
   DT[i+1] = reg;
   DT[i+2] = mul;
   DT[i+3] = con;
   return(++Ndt);
}

#if 0
short AddFindDerefEntry(short ptr, short reg, short mul, short con)
{
   int i;
   i = FindDerefEntry(ptr, reg, mul, conI)
   if (!i)
   {
      if (Ndt == Ndtalloc) GetNewDereftab(DTCHUNK);
      i = 4*Ndt;
   }
   else i = (i-1)<<2;
   DT[i] = ptr;
   DT[i+1] = reg;
   DT[i+2] = mul;
   DT[i+3] = con;
   return((i>>2)+1);
}
#endif

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
   return(STnew(name, GLOB_BIT | CONST_BIT | T_DOUBLE, val));
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

short STstrconstlookup(char *str)
/*
 * Searches for string constant str in symbol table, allocating new entry if not
 * already there. 
 * RETURNS: symtab index.
 */
{
   short ret;
   ret = STstrlookup(str);
   if (!ret)
   {
      return(STdef(str, CONST_BIT | T_CHAR, 0));
   }
   return(ret);
}

char *STi2str(short i)
{
   return(STname[i-1]);
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
   short k, n=0;
   int fl;
   char ln[256];
   for (k=0; k < N; k++)
   {
      fl = STflag[k];
      if (IS_CONST(fl) && (IS_DOUBLE(fl) || IS_FLOAT(fl)))
      {
         sprintf(ln, "_FPC_%d\n", n);
         STdef(ln, (fl & (!GLOB_BIT)), k+1);
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
   int fl;

   for (k=0; k != N; k++)
   {
      fl = STflag[k];
      if (IS_PARA(fl) || IS_LOCAL(fl))
      {
         SToff[k].sa[0] = SToff[k].i;
         switch(FLAG2TYPE(fl))
         {
         case T_INT:
            SToff[k].sa[1] = niloc++;
            break;
         case T_LONG:
            SToff[k].sa[1] = nlloc++;
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
      }
   }
}

void CreateLocalDerefs()
/*
 * Given numbered locals, creates derefs for local access, assuming local
 * area starts at the stack pointer.  Puts DT[i+2] = -1 to denote that
 * the address is not yet fully formed (since local area will almost
 * always be offset from stack pointer).
 */
{
   short k, off;
   int fl;
   int GetArchAlign(int nvd, int nvf, int nd, int nf, int nl, int ni);

   for (k=0; k != N; k++)
   {
      fl = STflag[k];
      if (IS_PARA(fl))
      {
         SToff[k].sa[0] = SToff[k].i;
         switch(FLAG2TYPE(fl))
         {
         case T_INT:
            off = SToff[k].sa[1]*4 + nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4 + 
                  ndloc*8 + nlloc*8 + nfloc*4;
            break;
         case T_LONG:
            off = SToff[k].sa[1]*8 + nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4 + 
                  ndloc*8;
            break;
         case T_FLOAT:
            if (IS_VEC(fl))
               off = SToff[k].sa[1]*FKO_SVLEN*4 + nvdloc*FKO_DVLEN*8;
            else
               off = SToff[k].sa[1]*4 + nvdloc*FKO_DVLEN*8 + 
                     nvfloc*FKO_SVLEN*4 + ndloc*8 + nlloc*8;
            break;
         case T_DOUBLE:
            if (IS_VEC(fl))
               off = SToff[k].sa[1]*FKO_DVLEN*8;
            else
               off = SToff[k].sa[1]*8 + nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4;
            break;
         default:
            fprintf(stderr, "%d: Unknown type %d!\n", __LINE__, FLAG2TYPE(fl));
            exit(-1);
         }
         SToff[k].sa[2] = AddDerefEntry(-REG_SP, 0, -1, off);
         fprintf(stderr, "%s, DT#=%d\n", STname[k], SToff[k].sa[2]);
      }
   }
   LOCALIGN = GetArchAlign(nvdloc, nvfloc, ndloc, nfloc, nlloc, niloc);
/*
   if (nvdloc) LOCALIGN = FKO_DVLEN*8;
   else if (nvfloc) LOCALIGN = FKO_SVLEN*4;
   else if (ndloc || nlloc) LOCALIGN = 8;
   else if (nfloc || niloc) LOCALIGN = 4;
*/
   LOCSIZE = nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4 + 
             ndloc*8 + nlloc*8 + nfloc*4 + niloc*4;
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
   int i, k=(Ndt<<2);
   for (i=0; i != k; i += 4)
   {
      if (DT[i] == -REG_SP && DT[i+1] == 0 && DT[i+2] == -1)
      {
         DT[i+2] = 0;
         DT[i+3] += ldist;
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
