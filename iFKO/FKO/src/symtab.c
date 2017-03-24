/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#define NO_STEXTERN
#include "fko.h"

/*
 * Majedul: FIXED: all the 4 pointers for symbol table are global and visible
 * from anyother files. any lookup function can overflow the size and change
 * the glabal pointers. So, using a statement where these pointers are accessed
 * while calling the lookup/add function is potentially dangarous.
 * Here is an example:
 *    SToff[k-1].sa[2] = AddDerefEntry(... ....);
 *    if AddDerefEntry() overflows and changes the SToff pointer, there would
 *    be an invalid memory write (as SToff may point the old address which is
 *    already freed)
 *
 */
char **STname;
union valoff *SToff;
int *STflag;
short *STpts2;
struct arrayinfo *STarr = NULL; /* to save information of multi-dim array*/
INT_DTC *DTcon = NULL;

static int N=0, Nalloc=0; /* for Symbol table */
static int Narr=0, TNarr=0; /* for array table */
static int Ndc=0, TNdc=0;

static int niloc=0, nlloc=0, nfloc=0, ndloc=0, nvfloc=0, nvdloc=0;
static int nviloc=0;

int LOCSIZE=0, LOCALIGN=0, NPARA=0;

#define STCHUNK 1024 
#define DTCHUNK 1024
#define DCCHUNK 256
#define MAXENTRY 32767 

static void GetNewSymtab(int chunk)
{
   char **nam;
   int *flg;
   short *pts2;
   union valoff *off;
   int i, n;
   n = Nalloc + chunk;
   nam = malloc(sizeof(char*)*n);
   assert(nam);
   off = malloc(n*sizeof(union valoff));
   flg = malloc(n*sizeof(int));
   pts2 = malloc(n*sizeof(short));
   assert(off && flg && pts2);
   if (Nalloc > 0)
   {
      for (i=0; i < N; i++)
      {
         nam[i] = STname[i];
         off[i].d = SToff[i].d;
         flg[i] = STflag[i];
         pts2[i] = STpts2[i];
      }
      free(STname);
      free(SToff);
      free(STflag);
      free(STpts2);
   }
   STname = nam;
   SToff = off;
   STflag = flg;
   STpts2 = pts2;
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
   STpts2[N] = 0;
#if 1
   assert(N!=MAXENTRY);
#endif
   return(++N);
}

short STdef(char *name, int flag, int off)
{
   union valoff offset;
   offset.i = off;
   return(STnew(name, flag, offset));
}

static void NewArrTable(int chunk)
/*
 * NOTE: In the arrtable, we keep the structure, not the struction pointer.
 * so, keep the chunk small. otherwise (TNarr - Narr) would be wasted
 */
{
   struct arrayinfo *new;
   int i,n;
   n = Narr + chunk;
   new = malloc(n*sizeof(struct arrayinfo));
   assert(new);
   if (TNarr > 0)
   {
      for (i=0; i < N; i++)
      {
         new[i].ptr = STarr[i].ptr;
         new[i].ndim = STarr[i].ndim;
         new[i].ldas = STarr[i].ldas; /* copy the list pointer */
         new[i].urlist = STarr[i].urlist;
         STarr[i].ldas = NULL;
         STarr[i].urlist = NULL;
      }
      free(STarr);
   }
   STarr = new;
   TNarr = n;
}

short AddSTarr(short ptr, short ndim, short *ldas)
/*
 * This function adds array info in the table and returns the index of the 
 * table. 
 * ptr = st index of base ptr
 * ndim = dimension of the array
 * ldas = list of STindex of lda (st entry can be integer variable or const) 
 *        number of element in the list is exactly ndim-1
 */
{
   int i;
   short *ls;

   if (Narr == TNarr) NewArrTable(8);
   
   ls = malloc((ndim-1)*sizeof(short));
   assert(ls);
   for (i=0; i < (ndim-1); i++)
      ls[i] = ldas[i];
   
   
   STarr[Narr].ptr = ptr;
   STarr[Narr].ndim = ndim;
   STarr[Narr].ldas = ls;
   STarr[Narr].urlist = NULL; /* unroll factor not updated at first */ 
/*
 * these list are to handle 2D array, wll be updated later
 */
   STarr[Narr].cldas = NULL; 
   STarr[Narr].colptrs = NULL; 
#if 1
   assert(Narr!=MAXENTRY);
#endif
   return (++Narr);
}

int UpdateSTarrUnroll(short id, short *ulist)
/*
 * our initial plan is to make unroll factor optional. so, we update 
 * unroll factor of each dimension for an array separately. 
 * returns 1 if successful, otherwise 0
 * NOTE: ulist is the list of unroll factor (ST id of const now) for each 
 * dimension : from higer dimension to lower dimension. 
 */
{
   int i;
   short ndim;
   short *urlist;
   if (id > Narr || id <= 0)  /* invalid index */
      return (0); 
   ndim = STarr[id-1].ndim;
   urlist = malloc(ndim*sizeof(short));
   assert(urlist);
   for (i=0; i < ndim; i++)
      urlist[i] = ulist[i];
   STarr[id-1].urlist = urlist;
   return(1);
}

static void GetNewDtcTable(int chunk)
{
   int i, n;
   INT_DTC *newdtc;
   n = TNdc + chunk;
   newdtc = malloc(sizeof(INT_DTC)*n);
   assert(newdtc);
   if (TNdc > 0)
   {
      for (i=0; i < Ndc; i++)
         newdtc[i] = DTcon[i];
      free(DTcon);
   }
   DTcon = newdtc;
   TNdc = n;
}

static int DTCnew(INT_DTC val)
{
   if (Ndc == TNdc)
      GetNewDtcTable(DCCHUNK);
   DTcon[Ndc] = val;
#if 1
   assert(Ndc!=MAXENTRY);
#endif
   return(++Ndc);
}

short DTClookup(INT_DTC val)
/*
 * Assumption: short is used to store the index of DTcon in DT table (sa[3]). 
 * So, we return short here. the assumption here is : we won't need more than
 * 2^15 entry of the table
 */
{
   int i;   
   for (i=0; i < Ndc; i++)
   {
      if (DTcon[i] == val)
         return(i+1);
   }
   i = DTCnew(val);
   assert(!((i<<1)&(1<<15))); /* <= 2^14-1*/
   return(i);
}

INT_DTC GetDTcon(int val)
{
   if (val & 1)
   {
      return(DTcon[(val>>1)-1]);
   }
   return(val);
}

void SetDTcon(int dt, INT_DTC con)
{
   int SHORT_MAX = 32767;
   int SHORT_MIN = -32768;
   INT_DTC val;
/*
 * we will save the const into a table if it is bigger than short type (16 bit)
 * or, the const is an odd number. Normally, const can't be odd since we 
 * multiply it with size of datatype. 
 */
   if ( (con > SHORT_MAX || con < SHORT_MIN)
         || (con & 1) )
   {
      val = DTClookup(con); 
      val = (val << 1) | 1;
      SToff[dt-1].sa[3] = val;
      /*fprintf(stderr, "***********con=%d, val=%d\n");*/
   }
   else /*FIXME: most common case, make it as if-statement */
   {
      /*assert(!(con&1));*/
      SToff[dt-1].sa[3] = con;
   }
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
/*
 *    Majedul: FIXED: should return (i+1), instead of i
 */
         return(i+1);
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
/*=============================================================================
 * Majedul: added the concept of new group of i/l constant: 'special purpose' 
 * for integer and long. normal i/l constlookup function would skip them from 
 * their search. Special purpose constant would store with a name in STname, 
 * not as NULL. 
 * NOTE: i/l constlookup functions now also check whether STname of that entry
 * is NULL.
 * Example: OL_NEINC 
 * ============================================================================
 */
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
#if 0      
      if (SToff[i].l == ic && IS_CONST(STflag[i]) && IS_INT(STflag[i]))
#else
      if (SToff[i].l == ic && IS_CONST(STflag[i]) && IS_INT(STflag[i])
          && !STname[i])
#endif
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
/*
 *    Majedul: normally all the constonts are stored with NULL in STname 
 *    except the special purpose one!
 *    Skip the special purpose const like: OL_NEINC in search where a str is
 *    saved in STname instead of NULL.
 *    =======================================================================
 *    need to use this new definiton for all other constant.
 */
#if 0      
      if (SToff[i].i == ic && IS_CONST(STflag[i]) && IS_INT(STflag[i]))
         return(i+1);
#else
      if (SToff[i].i == ic && IS_CONST(STflag[i]) && IS_INT(STflag[i]) 
          && !STname[i]) /* this extra checking is to skip special entry */
         return(i+1);
#endif
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

short STarrlookup(short ptr)
/*
 * searches for the ST id in STarr table, returns STarr index+1 if found,
 * if not, returns 0
 */
{
   short i;
   for (i=0; i != Narr; i++)
   {
      if (STarr[i].ptr == ptr)
         return (i+1);
   }
   return(0);
}

short STarrColPtrlookup(short colptr)
/*
 * searches column pointers and return STarr id+1 if found, 0 otherwise
 */
{
   int i, j;
   int n;
   for (i=0; i != Narr; i++)
   {
      n = STarr[i].colptrs[0];
      for (j=1; j <= n; j++)
      {
         if (colptr == STarr[i].colptrs[j])
            return(i+1);
      }
   }
   return(0);
}

short STarrlookupByname(char *name)
/*
 * searches for the array from STarr table, returns STarr index+1 if found, 
 * 0 if not
 */
{
   int id;
   assert(name);
   id = STstrlookup(name);
   if (id) 
      return(STarrlookup(id));
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
     i = STdef(str, LABEL_BIT, 0);
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

short FindVarFromName(char *name)
/*
 * RETURNS: ST index of var with name
 */
{
   int i;

   for (i=0; i < N; i++)
      if (STname[i] && IS_VAR(STflag[i]) && !strcmp(STname[i], name))
         return(i+1);
   return(0);
}

void STsetoffi(short i, int off)
{
    SToff[i-1].i = off;
}
void STsetflag(short i, int flag)
{
    STflag[i-1] = flag;
}

void STsetArray(short ptr, short ndim, short *ldas)
/*
 * setup additional info about the array: update STarr data structure  
 * NOTE: we can't keep the index in ST entry yet (SToff) but we can do it
 * inside CreateLocalDerefs() function as sa[3] will be available then. 
 */
{
   AddSTarr(ptr, ndim, ldas); 
}
void PrintSTarr(FILE *fpout)
{
   int i, j, dim, count;
   short lda, ur;
   assert(fpout);
   fprintf(fpout, "   PTR        DIMENSION          LDA              UNROLL");
   fprintf(fpout, "                NEWPTR    \n");
   fprintf(fpout, "==========   ===========   ==================="
                  " =================  =================\n");
   for (i=0; i < Narr; i++)
   {
      dim = STarr[i].ndim;
      fprintf(fpout, "   %s    %10d           ", STname[STarr[i].ptr-1], dim);
#if 0
      for (j=0; j < dim-1; j++)
      {
         lda = STarr[i].ldas[j];
         if (IS_CONST(STflag[lda-1]))
            fprintf(fpout, "%d,", SToff[lda-1].i);
         else
            fprintf(fpout, "%s,", STname[lda-1]);
      }
#else
      count = STarr[i].cldas[0];
      for (j=1; j <= count; j++)
      {
         lda = STarr[i].cldas[j];
         if (IS_CONST(STflag[lda-1]))
            fprintf(fpout, "%d,", SToff[lda-1].i);
         else
            fprintf(fpout, "%s,", STname[lda-1]);
      }
#endif
      fprintf(fpout, "\t\t  ");
      if (!STarr[i].urlist) 
         fprintf(fpout, " %d", 0);
      else
      {
         for (j=0; j < dim; j++)
         {
            ur = STarr[i].urlist[j];
            fprintf(fpout, "%d,", SToff[ur-1].i);
         }
         ur = STarr[i].urlist[0]; /* use the first ur only */
         ur = SToff[ur-1].i;
         if (STarr[i].colptrs)
         {
            count = STarr[i].colptrs[0];
            fprintf(fpout, "\t\t  ");
            for (j=1; j <= count; j++)
               fprintf(fpout, "%s, ", STname[STarr[i].colptrs[j]-1]);
         }
      }
      fprintf(fpout,"\n");
   }
}

short InsertNewLocal(char *name, int type )
/*
 * add new local var in symbol table. We should always use this function 
 * to insert any var for compiler's internal purpose, rather than manually 
 * update symbol table directly from outside. 
 */
{
   short k, j;
/*
 * NOTE: name string is copied and stored in Symbol Table
 * So, free the string from the caller function
 * FIXED: SToff is a global pointer and it can be updated from AddDerefEntry()
 * so, it's vary risky to use : SToff[k-1].sa[2] = AddDerefEntry(... ... );
 * which will ends up with invalid memory access!!!
 */
   k = STdef(name, type | LOCAL_BIT, 0);
   j =  AddDerefEntry(-REG_SP, k, -k, 0, k);
   SToff[k-1].sa[2] = j;
   return k;
}

short InsertNewLocalPtr(char *name, int type)
{
   short k, j;
   k = STdef(name, type | LOCAL_BIT | PTR_BIT, 0);
   j =  AddDerefEntry(-REG_SP, k, -k, 0, k);
   SToff[k-1].sa[2] = j;
   return k;
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
   /*int i;*/
   short k, n=0;
   int fl;
   char ln[256];

   for (k=0; k < N; k++)
   {
      fl = STflag[k];
      if (IS_CONST(fl) && (IS_DOUBLE(fl) || IS_FLOAT(fl)))
      {
         sprintf(ln, "_FPC_%d\n", n++);
         /*i = STdef(ln, (fl & (!GLOB_BIT)) | LOCAL_BIT, k+1);*/
         STdef(ln, (fl & (!GLOB_BIT)) | LOCAL_BIT, k+1);
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
      /*if ((IS_PARA(fl) || IS_LOCAL(fl)) && SToff[SToff[k].sa[2]-1].sa[0] 
            && !IS_VECELEM(fl))*/
      if ((IS_PARA(fl) || IS_LOCAL(fl)) && SToff[SToff[k].sa[2]-1].sa[0])
      {
         type = FLAG2PTYPE(fl);
         switch(type)
         {
         case T_INT:
            SToff[k].sa[1] = niloc++;
            break;
         case T_VFLOAT:
            SToff[k].sa[1] = nvfloc++;
            break;
         case T_FLOAT:
            SToff[k].sa[1] = nfloc++;
            break;
         case T_VDOUBLE:
            SToff[k].sa[1] = nvdloc++;
            break;
         case T_DOUBLE:
            SToff[k].sa[1] = ndloc++;
            break;
      #ifdef INT_VEC
         case T_VINT:
            SToff[k].sa[1] = nviloc++;
            break;
      #endif
         }
         #if IFKO_DEBUG_LEVEL > 1
            fprintf(stderr, "%c: %s(t=%d) gets slot %d\n", IS_PARA(fl) ? 'P' : 'L', 
                    STname[k], type, SToff[k].sa[1]);
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
   int fl;
   /*short st; */
   /*int i, vl, stype;*/
   /*char ln[512];*/

   for (k=0; k != N; k++)
   {
      fl = STflag[k];
      if (IS_PARA(fl))
      {
         SToff[k].sa[0] = SToff[k].i;
         SToff[k].sa[2] = AddDerefEntry(-REG_SP, k+1, -k-1, 0, k+1);
         SToff[k].sa[3] = 0; /* by default 0, remain 0 for 1D array */
      }
      else if (IS_LOCAL(fl))
      {
#if 0         
         if (!IS_VECELEM(fl))
            SToff[k].sa[2] = AddDerefEntry(-REG_SP, k+1, -k-1, 0, k+1);
         else
            SToff[k].sa[2] = AddDerefEntry(-REG_SP, SToff[k].sa[0], -k-1, 0, 
                                           k+1);
/*
 *       adding scalars for each vector elements
 *       For local, SToff[].sa[0] is unused. we used this to point the parent 
 *       vector ST
 */
         if (IS_VEC(fl))
         {
            SToff[k].sa[2] = AddDerefEntry(-REG_SP, k+1, -k-1, 0, k+1);
            SToff[k].sa[1] = 0; /* will be changed later! */
            vl = vtype2elem(FLAG2TYPE(fl));
            if (IS_VFLOAT(fl))
               stype = T_FLOAT;
            else if (IS_VDOUBLE(fl))
               stype = T_DOUBLE;
            else if (IS_VINT(fl))
               stype = T_INT;

            for (i=0; i < vl; i++)
            {
               sprintf(ln, "_%s_%d", STname[k], i);
               st = STdef(ln, VELEM_BIT | LOCAL_BIT | stype, 0);
               SToff[st-1].sa[0] = k+1;
               SToff[st-1].sa[1] = i+1;
            }
         }
         else if (IS_VECELEM(fl))
            SToff[k].sa[2] = AddDerefEntry(-REG_SP, SToff[k].sa[0],-k-1,0, k+1);
         else
            SToff[k].sa[2] = AddDerefEntry(-REG_SP, k+1, -k-1, 0, k+1);
#else
         SToff[k].sa[2] = AddDerefEntry(-REG_SP, k+1, -k-1, 0, k+1);
#endif
      }
   }
#if 0
   PrintST(stderr);
   exit(0);
#endif
}

#if 0
short FindSTVecElem(short vid, int el)
{
   short i;
   int fl;

   for (i=0; i != N; i++)
   {
      fl = STflag[i];
      if (IS_VECELEM(fl))
      {
         if (SToff[i].sa[0] == vid && el == SToff[i].sa[1])
            return(i+1);
      }
   }
   return(0);
}
#endif

void UpdateLocalDerefs(int isize)
/*
 * Given numbered locals, creates derefs for local access, assuming local
 * area starts at the stack pointer.  Puts DT[i+2] = -1 to denote that
 * the address is not yet fully formed (since local area will almost
 * always be offset from stack pointer).
 * isize is the size in bytes of an integer on the arch in question.
 */
{
   short k, off;
   int fl;
#if 1
   int GetArchAlign(int nvd, int nvf, int nvi, int nd, int nf, int nl, int ni);
#else
   int GetArchAlign(int nvd, int nvf, int nd, int nf, int nl, int ni);
#endif

   for (k=0; k != N; k++)
   {
      fl = STflag[k];
#if 0
      if (IS_VECELEM(fl) && SToff[SToff[k].sa[2]-1].sa[0]) /* vector element */
      {
         if (IS_FLOAT(fl)) 
         {
            SToff[SToff[k].sa[2]-1].sa[3] = 
               SToff[SToff[SToff[k].sa[0]-1].sa[2]-1].sa[3] 
               + (SToff[k].sa[1]-1) * 4;
         }
         else if (IS_DOUBLE(fl))
         {
            SToff[SToff[k].sa[2]-1].sa[3] = 
               SToff[SToff[SToff[k].sa[0]-1].sa[2]-1].sa[3] 
               + (SToff[k].sa[1]-1) * 8;
         }
         else
            fko_error(__LINE__, "vtype not imp yet");
      }
      else
#endif
      if ((IS_PARA(fl) || IS_LOCAL(fl)) && SToff[SToff[k].sa[2]-1].sa[0])
      {
/* fprintf(stderr, "Updating local %s\n", STname[k]); */
/*
 *       NOTE: this is the structure
 *
 *          VDOUBLE
 *          VFLOAT
 *          VINT
 *          double
 *          int
 *          float
 *       
 */
         switch(FLAG2PTYPE(fl))
         {
      #ifdef ArchHasVec            
         case T_VFLOAT:
            off = SToff[k].sa[1]*FKO_SVLEN*4 + nvdloc*FKO_DVLEN*8;
            break;
         case T_VDOUBLE:
            off = SToff[k].sa[1]*FKO_DVLEN*8;
            break;
         #ifdef INT_VEC
            case T_VINT:
               off = SToff[k].sa[1]*FKO_IVLEN*4 + nvdloc*FKO_DVLEN*8 + 
                     nvfloc*FKO_SVLEN*4;
               break;
            case T_DOUBLE:
               off = SToff[k].sa[1]*8 + nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4 +
                     nviloc*FKO_IVLEN*4 ;
               break;
            case T_INT:
               off = SToff[k].sa[1]*isize + ndloc*8 + 
                     nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4 + nviloc*FKO_IVLEN*4;
               break;
            case T_FLOAT:
               off = SToff[k].sa[1]*4 + nvdloc*FKO_DVLEN*8 + 
                     nvfloc*FKO_SVLEN*4 + nviloc*FKO_IVLEN*4 + ndloc*8 + 
                     niloc*isize;
               break;
         #else
            case T_DOUBLE:
               off = SToff[k].sa[1]*8 + nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4;
               break;
            case T_INT:
               off = SToff[k].sa[1]*isize + ndloc*8 + 
                     nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4;
               break;
            case T_FLOAT:
               off = SToff[k].sa[1]*4 + nvdloc*FKO_DVLEN*8 + 
                     nvfloc*FKO_SVLEN*4 + ndloc*8 + niloc*isize;
               break;
         #endif
      #else
         case T_DOUBLE:
            off = SToff[k].sa[1]*8;
            break;
         case T_INT:
            off = SToff[k].sa[1]*isize + ndloc*8;
            break;
         case T_FLOAT:
            off = SToff[k].sa[1]*4 + ndloc*8 + niloc*isize;
            break;
      #endif
         default:
            fprintf(stderr, "%d: Unknown type %d!\n", __LINE__, FLAG2PTYPE(fl));
            exit(-1);
         }
#if 0
         fprintf(stderr, "%s = %d\n", STname[k], off);
#endif
         SToff[SToff[k].sa[2]-1].sa[3] = off;
      }
   }
   LOCALIGN = GetArchAlign(nvdloc, nvfloc, nviloc, ndloc, nfloc, nlloc, niloc);
#ifdef ArchHasVec
   #ifdef INT_VEC
      LOCSIZE = nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4 + nviloc*FKO_IVLEN*4 + 
                ndloc*8 + niloc*isize + nfloc*4;
   #else
      LOCSIZE = nvdloc*FKO_DVLEN*8 + nvfloc*FKO_SVLEN*4 + ndloc*8 + 
                niloc*isize + nfloc*4;
   #endif
#else
   LOCSIZE = ndloc*8 + niloc*isize + nfloc*4;
#endif
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
         #if IFKO_DEBUG_LEVEL > 1
            fprintf(stderr, 
               "correcting local %d, off=%d+%d!\n", i+1, SToff[i].sa[3], ldist); 
         #endif
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
      #if IFKO_DEBUG_LEVEL > 1
         fprintf(stderr, 
            "correcting para %d, off=%d+%d!\n", lp->id, SToff[k].sa[3], fsize);
      #endif
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
         if (IS_KILLABLE(STflag[k]))
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

void MarkFinalUnusedLocals(BBLOCK *bbase)
/*
 * applied at FinalizePrologueEpilogue function, after repeatable optimization
 */
{
   INSTQ *ip;
   short k, i;
   /*enum inst inst;*/
   BBLOCK *bp;
   #ifdef X86_32
      extern int DTx87, DTx87d;
   #endif
   /*extern int DTabsd, DTnzerod, DTabs, DTnzero;*/
   /*extern int DTabsds, DTnzerods, DTabss, DTnzeros;*/

/*
 * Start out by marking all locals as unused, except the system const
 */
   for (k=0; k != N; k++)
   {
      if (IS_DEREF(STflag[k]) && SToff[k].sa[0] == -REG_SP &&
          SToff[k].sa[2] < 0)
      {
      #ifdef X86_32      
            if ( (DTx87 && k == SToff[DTx87-1].sa[2]-1)   
                 || (DTx87d && k == SToff[DTx87d-1].sa[2]-1) 
               )  
            ; /* do nothing for these system const */
         else
      #endif
/*
 *       register assignment is already done. so, no need to opt them out
 */
         SToff[k].sa[0] = 0;
      }
   }
/*
 * mark the used local DT by setting SToff[].sa[0] = -REG_SP 
 */
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
 * NOTE: con now can be an index of INT_DTC table
 */
short FindDerefEntry(short ptr, short ireg, short mul, short con)
{
   int i;
   for (i=0; i != N; i++)
   {
      if ( IS_DEREF(STflag[i]) && SToff[i].sa[0] == ptr && 
           SToff[i].sa[2] == mul && GetDTcon(SToff[i].sa[3]) == con &&
           (SToff[i].sa[1] == ireg || (!ireg && SToff[i].sa[1] > 0)) )
              return(i+1);
   }
   return(0);
}

short AddDerefEntry(short ptr, short reg, short mul, int con, short pts2)
{
   int i;
   /*int icon;
   int SHORT_MAX = 32767; 
   int SHORT_MIN = -32768; */
/*
 * Majedul: pts2 can be 0, see FinalizeEpilogue. 
 */
   if (pts2 > 0)
      i = STdef("DT", DEREF_BIT | FLAG2TYPE(STflag[pts2-1]) | 
                (UNKILL_BIT & STflag[pts2-1]), 0) - 1;
   else
      i = STdef("DT", DEREF_BIT,  0) - 1;

   SToff[i].sa[0] = ptr;
   SToff[i].sa[1] = reg;
   SToff[i].sa[2] = mul;
   STpts2[i] = pts2;
   SetDTcon(i+1, con);
   
   return(i+1);
}

short FindLocalFromDT(short dt)
/*
 * given a DT entry, searches ST for any local that uses that dereference
 * RETURNS: ST index of local using deref dt, or 0 if not found
 */
{
   int i, k;
   for (i=0; i != N; i++)
   {
      if (IS_LOCAL(STflag[i]) || IS_PARA(STflag[i]))
      {
         if (SToff[i].sa[2] == dt)
            break;
      }
   }
   if (i==N) i = 0;
   k = STpts2[dt-1];
   if (k > 0 && !IS_LOCAL(STflag[k-1]) && !IS_PARA(STflag[k-1]))
      k = 0;
   if (k != i+1)
      fprintf(stderr, "STpts2=%d, lookup=%d\n", k, i+1);
   assert(k == i+1);
   return(k);
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
void KillSTStrings()
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
/*
 * Majedul: FIXED: possible memory leak as STpts2 is not freed
 */
   if (STpts2) free(STpts2);
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
   assert(fread(STpts2, sizeof(short), N, fp) == N);
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
   assert(fwrite(STpts2, sizeof(short), N, fp) == N);
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

void PrintSymtabStaticMember(FILE *fpout)
{
   fprintf(fpout, "\n");
   fprintf(fpout, "INTERNAL MEMBER OF SYMTAB\n");
   fprintf(fpout, "-------------------------\n");
   fprintf(fpout, "N = %d\n", N);
   fprintf(fpout, "Nalloc = %d\n", Nalloc);
   fprintf(fpout, "niloc = %d\n", niloc);
   fprintf(fpout, "nlloc = %d\n", nlloc);
   fprintf(fpout, "ndloc = %d\n", ndloc);
   fprintf(fpout, "nvfloc = %d\n", nvfloc);
   fprintf(fpout, "nvdloc = %d\n", nvdloc);
   fprintf(fpout, "Narr = %d\n", Narr);
   fprintf(fpout, "TNarr = %d\n", TNarr);
   fprintf(fpout, "Ndc = %d\n", Ndc);
   fprintf(fpout, "TNdc = %d\n", TNdc);
   fprintf(fpout, "-------------------------\n");
}

/*=============================================================================
 *                TO MANAGE 2D ARRAY ACCESS 
 *
 *===========================================================================*/

short NewLDAS(short lda, short ptr, int mul)
/*
 * create new varibale _lda_s=lda*mul*size generating the statement/expression
 * return ST index of new variable
 */
{
   short ldas, sz;
   char ln[512];

   if (mul == 1) /* _lda_s = lda*size */
   {
/*
 *    create new variable _lda_S
 */
      sprintf(ln, "_%s_s", STname[lda-1]); 
      ldas = InsertNewLocal(ln, STflag[lda-1]);
/*
 *    findout the value to shift to generate ldas 
 */
      sz = STiconstlookup(type2shift(FLAG2TYPE(STflag[ptr-1])));
/*
 *    add instrucitons to calc ldas = lda * sizeof
 */
      DoArith(ldas, lda, '<', sz);
   }
   else if (mul == -1)
   {
      sprintf(ln, "_n_%s_s", STname[lda-1]); 
      ldas = InsertNewLocal(ln, STflag[lda-1]);
      sz = STiconstlookup(type2shift(FLAG2TYPE(STflag[ptr-1])));
      DoArith(ldas, lda, '<', sz);
      DoArith(ldas, ldas, 'n', 0);
   }
   else if (mul == 3)
   {
      sprintf(ln, "_3_%s_s", STname[lda-1]); 
      ldas = InsertNewLocal(ln, STflag[lda-1]);
      sz = STiconstlookup(type2len(FLAG2TYPE(STflag[ptr-1]))*mul);
      DoArith(ldas, lda, '*', sz);
   }
   else
      fko_error(__LINE__, "unknown mul, not implemented yet! ");
   return ldas;
}

short *CreateLDAs(short *ldas, short ptr)
/*
 * to create lda*size as new variable for non-optimized mode of 2D array
 */
{
   short lda0;
   short *cldas;     /* new generated ldas */

   lda0 = ldas[0]; /* for 2D array, we will have lda here */
   if (!IS_CONST(STflag[lda0-1]))
   {
/*
 *    create new list with ldas to update STarr
 */
      cldas = malloc(2*sizeof(short)); /* always one lda on non optimized arr */
      assert(cldas);
      cldas[0] = 1; /* lda count is one */
      cldas[1] = NewLDAS(lda0, ptr, 1); /* lda*sizeof */
   }
   else 
      fko_error(__LINE__, "No support for const array yet!");
   return cldas;
}

short *CreateOptLDAs(short *ldas, short ptr, short unroll)
/*
 * will create several ldas for optimized 2D array access, like; lda, -lda,
 * lda*3 , etc.this is to optimize pointer update for 2D access
 * 
 */
{
   int nl;
   short *cldas;

   switch(unroll)
   {
      case 1: 
      case 2:
         nl = 1; /* number of lda to be created */
         cldas = malloc((nl+1)*sizeof(short));
         assert(cldas);
         cldas[0] = nl;
         cldas[1] = NewLDAS(ldas[0], ptr, 1);
         break;
      case 3: 
      case 4: 
      case 5: 
      case 8: 
      case 9: 
      case 10: 
      case 11: 
      case 12: 
      case 15: 
      case 16: 
         nl = 2; /* number of lda to be created: lda, -lda */
         cldas = malloc((nl+1)*sizeof(short));
         assert(cldas);
         cldas[0] = nl;
         cldas[1] = NewLDAS(ldas[0], ptr, 1); /*  lda*sizeof */
         cldas[2] = NewLDAS(ldas[0], ptr, -1); /* -lda*sizeof */
         break;
      case 6: 
      case 7: 
      case 13: 
      case 14: 
         nl = 3; /* number of lda to be created: lda, -lda, 3*lda */
         cldas = malloc((nl+1)*sizeof(short));
         assert(cldas);
         cldas[0] = nl;
         cldas[1] = NewLDAS(ldas[0], ptr, 1); /*  lda*sizeof */
         cldas[2] = NewLDAS(ldas[0], ptr, -1); /* -lda*sizeof */
         cldas[3] = NewLDAS(ldas[0], ptr, 3); /*  3*lda*sizeof */
         break;
      default: 
         fko_error(__LINE__, "Unknown unroll for 2D array!");
   }
   return cldas;

}

short *CreateAllColPtrs(short base, short lda, int unroll)
/*
 * create all column pointers according to the unroll factor (non optimized),
 * note: lda is actually lda*sizeof here
 * return list of the colptrs
 */   
{
   int i;
   short *colptrs;
   char ln[512];
   
   assert(unroll);
   colptrs = malloc((unroll+1)*sizeof(short));
   assert(colptrs);
   colptrs[0] = unroll;
/*
 * time to create new pointers for unroll factor of the column
 */
   for (i=0; i<unroll; i++)
   {
      sprintf(ln, "_%s_%d", STname[base-1], i);
      colptrs[i+1] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
      if (!i)
         DoMove(colptrs[i+1], base); 
      else
         HandlePtrArithNoSizeofUpate(colptrs[i+1], colptrs[i], '+', lda); 
   }
   return colptrs;
}

short *CreateOptColPtrs(short base, short lda, short unroll)
/*
 * create column pointers needed for optimized 2D array access,
 * note: lda is actually lda*sizeof here
 * returns list of colptrs
 */
{
   int np;
   short *colptrs;
   short tmlda;
   char ln[512];

   switch(unroll)
   {
      case 1:
      case 2:  /* create P0 */
         np = 1;
         colptrs = malloc((np+1)*sizeof(short));
         assert(colptrs);
         colptrs[0] = np;
         sprintf(ln, "_%s_0", STname[base-1]);
         colptrs[1] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
         DoMove(colptrs[1], base); 
         break;
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:  /* create P2 */         
         np = 1;
         colptrs = malloc((np+1)*sizeof(short));
         assert(colptrs);
         colptrs[0] = np;
         sprintf(ln, "_%s_2", STname[base-1]);
         colptrs[1] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
/*
 *       create tmp lda, tmlda = lda * 2 
 */
         sprintf(ln, "_tm%s_2", STname[lda-1]);
         tmlda = InsertNewLocal(ln, FLAG2TYPE(STflag[lda-1]));
         DoArith(tmlda, lda, '<', STiconstlookup(1));
         HandlePtrArithNoSizeofUpate(colptrs[1], base, '+', tmlda); 
         break;
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:  /* create P2, P7 */
         np = 2;
         colptrs = malloc((np+1)*sizeof(short));
         assert(colptrs);
         colptrs[0] = np;
         
         sprintf(ln, "_%s_2", STname[base-1]);
         colptrs[1] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
         
         sprintf(ln, "_%s_7", STname[base-1]);
         colptrs[2] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
/*
 *       create tmp lda, tmlda = lda * 2 
 */
         sprintf(ln, "_tm%s_2_7", STname[lda-1]);
         tmlda = InsertNewLocal(ln, FLAG2TYPE(STflag[lda-1]));

         DoArith(tmlda, lda, '<', STiconstlookup(1));
         HandlePtrArithNoSizeofUpate(colptrs[1], base, '+', tmlda); 

         DoArith(tmlda, lda, '*', STiconstlookup(7));
         HandlePtrArithNoSizeofUpate(colptrs[2], base, '+', tmlda); 
         break;
      case 13:
      case 14:  /* create P2, P9 */
         np = 2;
         colptrs = malloc((np+1)*sizeof(short));
         assert(colptrs);
         colptrs[0] = np;
         
         sprintf(ln, "_%s_2", STname[base-1]);
         colptrs[1] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
         
         sprintf(ln, "_%s_9", STname[base-1]);
         colptrs[2] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
/*
 *       create tmp lda, tmlda = lda * 2 
 */
         sprintf(ln, "_tm%s_2_9", STname[lda-1]);
         tmlda = InsertNewLocal(ln, FLAG2TYPE(STflag[lda-1]));

         DoArith(tmlda, lda, '<', STiconstlookup(1));
         HandlePtrArithNoSizeofUpate(colptrs[1], base, '+', tmlda); 

         DoArith(tmlda, lda, '*', STiconstlookup(9));
         HandlePtrArithNoSizeofUpate(colptrs[2], base, '+', tmlda); 
         break;

      case 15:
      case 16:    /* create p2, P7, P12 */
         np = 3;
         colptrs = malloc((np+1)*sizeof(short));
         assert(colptrs);
         colptrs[0] = np;
         
         sprintf(ln, "_%s_2", STname[base-1]);
         colptrs[1] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
         
         sprintf(ln, "_%s_7", STname[base-1]);
         colptrs[2] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
         
         sprintf(ln, "_%s_12", STname[base-1]);
         colptrs[3] = InsertNewLocalPtr(ln, FLAG2TYPE(STflag[base-1]));
/*
 *       create tmp lda, tmlda = lda * 2 
 */
         sprintf(ln, "_tm%s_2_7_12", STname[lda-1]);
         tmlda = InsertNewLocal(ln, FLAG2TYPE(STflag[lda-1]));

         DoArith(tmlda, lda, '<', STiconstlookup(1));
         HandlePtrArithNoSizeofUpate(colptrs[1], base, '+', tmlda); 

         DoArith(tmlda, lda, '*', STiconstlookup(7));
         HandlePtrArithNoSizeofUpate(colptrs[2], base, '+', tmlda); 
         
         DoArith(tmlda, lda, '*', STiconstlookup(12));
         HandlePtrArithNoSizeofUpate(colptrs[3], base, '+', tmlda); 
         break;
      default:
         fko_error(__LINE__, "unknown unroll factor for 2D array access");
   }
   return colptrs;
}

void CreateArrColPtrs()
/*
 * this function creates new ldas=lda*size and updates the STarr with
 * new lda and also updates sa[3] ST entry to point the STarr
 * NOTE: it must be called after calling CreateLocalDerefs(), because 
 * sa[3] for parameter only be available after this function!!!
 */
{
   int i;
   short ptr, nd;
   short *lda;
   short ldaS;
   short ur;
   extern int FKO_FLAG;
  
   if (Narr)
      DoComment("Create ldas = lda * sizeof ");
   for (i=0; i < Narr; i++)
   {
      ptr = STarr[i].ptr;
      nd = STarr[i].ndim;
      assert(nd <= 2); /* we don't support 3D arrays yet */
      lda = STarr[i].ldas;
      if (STarr[i].urlist)
      {
         ur = STarr[i].urlist[0];
         ur = SToff[ur-1].i; /* unroll factor */
      }
      else
         ur = 0;
      SToff[ptr-1].sa[3] = i+1; /* sa[3] of ST should be free now */
/*
 *    depending on the optimization scheme, we will create various ldas. we will
 *    parameterize this later
 */
   #ifdef X86
      if (FKO_FLAG & IFF_OPT2DPTR)
      {
         STarr[i].cldas = CreateOptLDAs(lda, ptr, ur);
         ldaS = STarr[i].cldas[1]; /* always lda*size first */
         STarr[i].colptrs = CreateOptColPtrs(ptr,ldaS, ur);
      }
      else
      {
         STarr[i].cldas = CreateLDAs(lda, ptr);
         ldaS = STarr[i].cldas[1]; /* always lda*size first */
         STarr[i].colptrs = CreateAllColPtrs(ptr,ldaS, ur);
      }  
   #else /* no optimized scheme for non-x86 based system */
         STarr[i].cldas = CreateLDAs(lda, ptr);
         ldaS = STarr[i].cldas[1]; /* always lda*size first */
         STarr[i].colptrs = CreateAllColPtrs(ptr,ldaS, ur);
   #endif
   }
   if (Narr)
      DoComment("End ldas creation");
/*
 * based on unroll factor, we will create pointers to point the columns 
 * NOTE: we only support 2D array to create column pointers ... 
 */
#if 0
   //PrintST(stderr);
   PrintInst(stderr, bbbase);
   PrintSTarr(stderr);
   //exit(0);
#endif
}

void KillSTarr()
{
   int i;
   for (i=0; i < Narr; i++)
   {
      if (STarr[i].ldas)
         free(STarr[i].ldas);
      if (STarr[i].urlist)
         free(STarr[i].urlist);
      if (STarr[i].colptrs)
         free(STarr[i].colptrs);
      if (STarr[i].cldas)
         free(STarr[i].cldas);
   }
   if(STarr) free(STarr);
}
