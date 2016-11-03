#ifndef FKO_SYMTAB_H
#define FKO_SYMTAB_H

#define NTYPES 9           /* number of types suported here */

#define T_INT        0x0
#define T_SHORT      0x1
#define T_FLOAT      0x2
#define T_DOUBLE     0x3
#define T_CHAR       0x4
#define T_FUNC       0x5
#define T_VFLOAT     0x6
#define T_VDOUBLE    0x7

#if 0
#define IS_INT(flag_)  (((flag_) & 0x7) == T_INT)
#define IS_SHORT(flag_)  (((flag_) & 0x7) == T_SHORT)
#define IS_FLOAT(flag_)  (((flag_) & 0x7) == T_FLOAT)
#define IS_DOUBLE(flag_)  (((flag_) & 0x7) == T_DOUBLE)
#define IS_FP(flag_)      (((flag_) & 0x7) & (T_DOUBLE | T_FLOAT))
#define IS_VFLOAT(flag_)  (((flag_) & 0x7) == T_VFLOAT)
#define IS_VDOUBLE(flag_)  (((flag_) & 0x7) == T_VDOUBLE)
#define IS_CHAR(flag_)  (((flag_) & 0x7) == T_CHAR)
#define IS_FUNC(flag_)  (((flag_) & 0x7) == T_FUNC)
#define FLAG2TYPE(flag_) ((flag_) & 0x7)
#define IS_VEC(flag_)  (IS_VFLOAT(flag_) | IS_VDOUBLE(flag_))
#endif
/*
 * Majedul: adding new vector type for int: V_INT. we need to use 4 bits to 
 * track type which changes previous definition
 * .... Should we have two different types like: V_SHORT, V_INT !!!!
 * NOTE: normally, we don't use V_SHORT and V_INT together. So, I decided to
 * use only one type V_INT in my implementation.
 * NOTE: they are also used in fko_types.h... see flag for SLP_VECTOR
 */
#define T_VINT       0x8
/*#define T_VSHORT       0x8
#define T_VINT       0x9*/

#define IS_INT(flag_)  (((flag_) & 0xF) == T_INT)
#define IS_SHORT(flag_)  (((flag_) & 0xF) == T_SHORT)
#define IS_FLOAT(flag_)  (((flag_) & 0xF) == T_FLOAT)
#define IS_DOUBLE(flag_)  (((flag_) & 0xF) == T_DOUBLE)
#define IS_FP(flag_)      (((flag_) & 0xF) & (T_DOUBLE | T_FLOAT))
#define IS_VFLOAT(flag_)  (((flag_) & 0xF) == T_VFLOAT)
#define IS_VDOUBLE(flag_)  (((flag_) & 0xF) == T_VDOUBLE)
#define IS_VINT(flag_)  (((flag_) & 0xF) == T_VINT)
#define IS_CHAR(flag_)  (((flag_) & 0xF) == T_CHAR)
#define IS_FUNC(flag_)  (((flag_) & 0xF) == T_FUNC)
#define FLAG2TYPE(flag_) ((flag_) & 0xF) /* 9-15 not defined yet */
#define IS_VEC(flag_)  (IS_VFLOAT(flag_) | IS_VDOUBLE(flag_) | IS_VINT(flag_))


#define PTR_BIT      0x10
#define CONST_BIT    0x20
#define GLOB_BIT     0x40
#define PARA_BIT     0x80
#define LABEL_BIT    0x100
#define UNSIGNED_BIT 0x200
#define LOCAL_BIT    0x400
#define DEREF_BIT    0x800
#define UNKILL_BIT   0x1000   /* local must be allocated */
#define ARRAY_BIT    0x2000   /* indicates an array */
/*#define VELEM_BIT    0x4000 */  /* scalar for vector element, no new var */

#define IS_PTR(flag_)   ((flag_) & PTR_BIT)
#define IS_CONST(flag_) ((flag_) & CONST_BIT)
#define IS_GLOB(flag_)  ((flag_) & GLOB_BIT)
#define IS_PARA(flag_)  ((flag_) & PARA_BIT)
#define IS_UNSIGNED(flag_)   ((flag_) & UNSIGNED_BIT)
#define IS_LOCAL(flag_) ((flag_) & LOCAL_BIT)
#define IS_DEREF(flag_) ((flag_) & DEREF_BIT)
#define IS_LABEL(flag_) ((flag_) & LABEL_BIT)
#define IS_VAR(flag_) (!((flag_) & (CONST_BIT | LABEL_BIT)))
#define IS_KILLABLE(flag_) (!((flag_) & UNKILL_BIT))
#define IS_SETUSE(flag_) (IS_LOCAL(flag_))
#if 0
#define FLAG2PTYPE(flag_) ( IS_PTR(flag_) ? T_INT : ((flag_) & 0x7) )
#endif
#define FLAG2PTYPE(flag_) ( IS_PTR(flag_) ? T_INT : ((flag_) & 0xF) )
#define IS_ARRAY(flag_) ((flag_) & ARRAY_BIT)
/*#define IS_VECELEM(flag_) ((flag_) & VELEM_BIT)*/


#define REG_SP   1
/*
 * Majedul: AVX is defined in another header file. We can't ensure that it is 
 * defined before this header file. So these are shifted to fko_arch
 */
/*#if defined(X86) && defined(AVX)*/
/*
#if defined(AVX) || 1
   #define FKO_DVLEN 4  
   #define FKO_SVLEN 8  
#else
   #define FKO_DVLEN 2  
   #define FKO_SVLEN 4  
#endif
*/
#ifndef NO_STEXTERN
   extern char         **STname;
   extern union valoff *SToff;
   extern int          *STflag;
   extern short        *STpts2;
   extern  struct arrayinfo *STarr;
   extern INT_DTC *DTcon;
#endif

short FindDerefEntry(short ptr, short ireg, short mul, short con);
short AddDerefEntry(short ptr, short reg, short mul, int con, short pts2);
short STdef(char *name, int flag, int off);
short STdconstlookup(double f);
short STfconstlookup(float f);
short STiconstlookup(int ic);
short STlconstlookup(long ic);
short STstrlookup(char *str);
short STstrconstlookup(char *str);
void STsetoffi(short i, int off);
void STsetflag(short i, int flag);
void NumberLocalsByType(void);
void CorrectLocalOffsets(int ldist);
int STlen(void);
char *STi2str(short i);
void CreateLocalDerefs(void);
void UpdateLocalDerefs(int);
void AddStaticData(char *name, short align, short len, void *vbytarr);
void KillStaticData(void);
void MarkUnusedLocals(BBLOCK *bbase);
void MarkFinalUnusedLocals(BBLOCK *bbase);
void CorrectParamDerefs(struct locinit *libase, int rsav, int fsize);
short FindLocalFromDT(short dt);
short FindVarFromName(char *name);
short InsertNewLocal(char *name, int type );
short STarrlookup(short id);
short STarrColPtrlookup(short colptr);
short STarrlookupByname(char *name);
void KillSTStrings();
void ReadSTFromFile(char *fname);
void WriteSTToFile(char *fname);
short STlabellookup(char *str);
void CreateArrColPtrs();
int UpdateSTarrUnroll(short id, short *ulist);
void STsetArray(short ptr, short ndim, short *ldas);
void PrintST(FILE *fpout);
void PrintSTarr(FILE *fpout);
void KillSTarr();
void SetDTcon(int dt, INT_DTC con);
INT_DTC GetDTcon(int val);
#endif
