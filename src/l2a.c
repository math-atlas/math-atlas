#include "ifko.h"
#include "fko_arch.h"
#include <stdarg.h>

#define Mstr2(m) # m 
#define Mstr(m) Mstr2(m)

#define MyAssert(arg_) \
{ \
   if (!(arg_)) \
   { \
      fprintf(stderr, \
              "\n\nassertion '%s' failed on line %d of %s, hanging:\n\n", \
	      Mstr(arg_), __LINE__, __FILE__);\
      while(1); \
   } \
}
#undef assert
#define assert MyAssert
struct assmln *NewAssln(char *ln)
{
   struct assmln *ap;
   ap = malloc(sizeof(struct assmln));
   assert(ap);
   ap->ln = malloc(strlen(ln)+1);
   assert(ap->ln);
   strcpy(ap->ln, ln);
   ap->next = NULL;
   return(ap);
}

void KillAllAssln(struct assmln *base)
{
   struct assmln *next;
   while (base)
   {
      next = base->next;
      if (base->ln) free(base->ln);
      free(base);
      base = next;
   }
}

static struct assmln *PrintAssln(char *form, ...)
{
   char ln[256];
   va_list argptr;
   va_start(argptr, form);
   vsprintf(ln, form, argptr);
   return(NewAssln(ln));
}

static char *GetDeref(short id)
/*
 * Given a deref index D, print the appropriate format in assembly
 */
{
   static char ln[128];
   short ptr, reg, mul, con, HC;
   assert(id >= 1);
   id = (id-1)<<2;
   ptr = DT[id];
   reg = DT[id+1];
   mul = DT[id+2];
   con = DT[id+3];
   #ifdef X86
      if (!reg)
      {
         if (con) sprintf(ln,"%d(%s)", con, archiregs[-IREGBEG-ptr]);
         else sprintf(ln, "(%s)", archiregs[-IREGBEG-ptr]);
      }
      else if (mul)
      {
         if (con) /* if (reg && mul && con) */
            sprintf(ln, "%d(%s,%s,%d)", con, archiregs[-IREGBEG-ptr],
                    archiregs[-IREGBEG-reg], mul);
         else
            sprintf(ln, "(%s,%s,%d)", archiregs[-IREGBEG-ptr],
                    archiregs[-IREGBEG-reg], mul);
      }
      else /* reg && !mul */
      {
         if (con) 
            sprintf(ln, "%d(%s,%s)", con, archiregs[-IREGBEG-ptr],
                    archiregs[-IREGBEG-reg]);
         else
            sprintf(ln, "(%s,%s)", archiregs[-IREGBEG-ptr],
                    archiregs[-IREGBEG-reg]);
      }
   #elif defined(SPARC)
      if (con)
      {
         assert(!reg);
         sprintf(ln, "[%s+%d]", archiregs[-IREGBEG-ptr], con);
      }
      else if (reg) /*  && !con */
         sprintf(ln,"[%s+%s]", archiregs[-IREGBEG-ptr], 
                 archiregs[-IREGBEG-reg]);
      else /* !reg && !con */
         sprintf(ln, "[%s]", archiregs[-IREGBEG-ptr]);
   #elif defined(PPC)
      if (con)
      {
         assert(!reg);
         sprintf(ln, "%d(%s)", con, archiregs[-IREGBEG-ptr]);
      }
      else if (reg) /*  && !con */
         sprintf(ln, "%s,%s", archiregs[-IREGBEG-ptr], archiregs[-IREGBEG-reg]);
      else /* !reg && !con */
         sprintf(ln, "0(%s)", archiregs[-IREGBEG-ptr]);
   #endif
   return(ln);
}

#ifdef X86_64
static char *GetSregOrDeref(short id)
/*
 * given an id, return string containing register name if less than zero,
 * or a properly formated deref entry if greater than zero
 */
{
   if (id < 0)
      return(archsregs[-IREGBEG-id]);
   else if (id)
      return(GetDeref(id));
}
#endif

static char *GetIregOrDeref(short id)
/*
 * given an id, return string containing register name if less than zero,
 * or a properly formated deref entry if greater than zero
 */
{
   if (id < 0)
      return(archiregs[-IREGBEG-id]);
   else if (id)
      return(GetDeref(id));
}

static char *GetDregOrDeref(short id)
/*
 * given an id, return string containing register name if less than zero,
 * or a properly formated deref entry if greater than zero
 */
{
   if (id < 0)
   {
      if (-id >= DREGBEG)
         return(archdregs[-DREGBEG-id]);
      else
         return(archfregs[-FREGBEG-id]);
   }
   else if (id)
      return(GetDeref(id));
}

#ifdef X86_64
static char *GetSregOrConst(short id)
/*
 * Given a id, return string containing a register name if less than zero,
 * and a constant assuming greater than zero
 */
{
   static char ln[64];
   int flag;
   if (id < 0)
      return(archsregs[-IREGBEG-id]);
   else
   {
      assert(id != 0);
      id--;
      flag = STflag[id];
      assert(IS_CONST(flag));
      if (IS_INT(flag) || IS_SHORT(flag)) sprintf(ln, "$%d", SToff[id].i);
      else fko_error(__LINE__, "Integer constant expected!\n");
   }
   return(ln);
}
#endif

static char *GetIregOrConst(short id)
/*
 * Given a id, return string containing a register name if less than zero,
 * and a constant assuming greater than zero
 */
{
   static char ln[64];
   int flag;
   if (id < 0)
      return(archiregs[-IREGBEG-id]);
   else
   {
      assert(id != 0);
      id--;
      flag = STflag[id];
      assert( IS_CONST(flag));
      #ifdef X86
         if (IS_INT(flag) || IS_SHORT(flag)) sprintf(ln, "$%d", SToff[id].i);
/*         else if (IS_SHORT(flag)) sprintf(ln, "$%ld", SToff[id].l); */
      #else
         if (IS_INT(flag) || IS_SHORT(flag)) sprintf(ln, "%d", SToff[id].i);
/*         else if (IS_SHORT(flag)) sprintf(ln, "%ld", SToff[id].l); */
      #endif
         else fko_error(__LINE__, "Integer constant expected!\n");
   }
   return(ln);
}

#if 0
struct assmln *DumpData(void)
{
   extern struct sdata *SDhead;
   struct sdata *dp;
   struct assmln *ap, *aph=NULL;
   short align, i, n;

/*
 * Presently, only handling initialized read-only data (fp consts)
 */
   if (SDhead)
   {
      #ifdef SPARC
         ap = aph = NewAssln(".rodata");
      #else
         ap = aph = NewAssln(".data");
      #endif
      for (dp=SDhead; dp; dp = dp->next)
      {
         align = dp->align;
	 #ifdef OSX_PPC
	    if (align)
	    {
	       for (i=0; i < 16; i++)
	          if ((align ^ (1<<i)) == 0) break;
	       assert(i != 16);
	       align = i;
	    }
	 #endif
	 if (align)
	 {
            ap->next = PrintAssln(".align\t%d\n", align);
	    ap = ap->next;
         }
	 ap->next = PrintAssln("%s:\n", dp->name);
	 ap = ap->next;
	 for (n=dp->len, i=0; i < n; i++)
	 {
	    ap->next = PrintAssln("\t.byte %d, ", dp->vals[i]);
	    ap = ap->next;
	 }
      }
   }
   return(aph);
}
#endif

struct assmln *lil2ass(INSTQ *head)
{
   INSTQ *ip=head;
   struct assmln *ahead=NULL, *ap;
   short op1, op2, op3, k;
   #ifdef SPARC
      int SeenSave=0;
   #endif
   char ln[1024], *sptr;
   extern int DTabsd, DTnzerod, DTabs, DTnzero;

   ap = ahead = NewAssln(".text\n");
   do
   {
      op1 = ip->inst[1];
      op2 = ip->inst[2];
      op3 = ip->inst[3];
      switch(ip->inst[0])
      {
#if 0
      case UNIMP:
         ap->next = NewAssln("\tunimp\n");
         break;
#endif
      case NOP:
         ap->next = NewAssln("\tnop\n");
         break;
      case LABEL:
         #ifdef PPC
            if (IS_GLOB(STflag[--op1]))
            {
               ap->next = PrintAssln(".globl\t_%s\n", STname[op1]);
               ap = ap->next;
               ap->next = PrintAssln("_%s:\n", STname[op1]);
            }
            else ap->next = PrintAssln("%s:\n", STname[op1]);
         #else
            if (IS_GLOB(STflag[--op1]))
               ap->next = PrintAssln(".globl\t%s\n", STname[op1]);
            else
               ap->next = PrintAssln(".local\t%s\n", STname[op1]);
            ap = ap->next;
            ap->next = PrintAssln("%s:\n", STname[op1]);
         #endif
         break;
/*
 *    integer ops
 */
   #ifdef X86_64
      case LDS:
         ap->next = PrintAssln("\tmovl\t%s,%s\n", GetDeref(op2),
                                  archsregs[-IREGBEG-op1]);
         break;
   #endif
      case LD:
         #ifdef X86_64
            ap->next = PrintAssln("\tmovq\t%s,%s\n", GetDeref(op2),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(X86)
            ap->next = PrintAssln("\tmovl\t%s,%s\n", GetDeref(op2),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tld\t%s,%s\n", GetDeref(op2),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            k = (op2-1)<<2;
            if (DT[k+1])
               ap->next = PrintAssln("\tlwzx\t%s,%s\n", archiregs[-IREGBEG-op1],
                                     GetDeref(op2));
            else
               ap->next = PrintAssln("\tlwz\t%s,%s\n", archiregs[-IREGBEG-op1], 
                                     GetDeref(op2));
         #endif
         break;
      case FLDD:
         #ifdef X86
	    sptr = archdregs[-DREGBEG-op1];
	    if (sptr[1] == 's' && sptr[2] == 't')
               ap->next = PrintAssln("\tfldl\t%s\n", GetDeref(op2));
	    else
               ap->next = PrintAssln("\tmovsd\t%s,%s\n", GetDeref(op2),
                                     archdregs[-DREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tldd\t%s,%s\n", GetDeref(op2),
                                  archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            k = (op2-1)<<2;
            if (DT[k+1])
               ap->next = PrintAssln("\tlfdx\t%s,%s\n", archdregs[-DREGBEG-op1],
                                     GetDeref(op2));
            else
               ap->next = PrintAssln("\tlfd\t%s,%s\n", archdregs[-DREGBEG-op1], 
                                     GetDeref(op2));
         #endif
         break;
      case FLD:
         #ifdef X86
	    sptr = archfregs[-FREGBEG-op1];
	    if (sptr[1] == 's' && sptr[2] == 't')
               ap->next = PrintAssln("\tflds\t%s\n", GetDeref(op2));
	    else
               ap->next = PrintAssln("\tmovss\t%s,%s\n", GetDeref(op2),
                                     archfregs[-FREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tld\t%s,%s\n", GetDeref(op2),
                                  archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            k = (op2-1)<<2;
            if (DT[k+1])
               ap->next = PrintAssln("\tlfsx\t%s,%s\n", archfregs[-FREGBEG-op1],
                                     GetDeref(op2));
            else
               ap->next = PrintAssln("\tlfs\t%s,%s\n", archfregs[-FREGBEG-op1], 
                                     GetDeref(op2));
         #endif
         break;
   #ifdef X86_64
      case STS:
         ap->next = PrintAssln("\tmovl\t%s,%s\n", archsregs[-IREGBEG-op2],
                               GetDeref(op1));
         break;
   #endif
      case ST: 
         #ifdef X86_64
            ap->next = PrintAssln("\tmovq\t%s,%s\n", archiregs[-IREGBEG-op2],
                                  GetDeref(op1));
         #elif defined(X86)
	    #if 0
            ap->next = PrintAssln("\tmovl\t%s,%s\n", archiregs[-IREGBEG-op2],
                                  GetDeref(op1));
	    #elif 1
	    if (op1 == 0)
	       fprintf(stderr, "%d,%d,%d,%d; prevln = %s", 
	               ip->inst[0], ip->inst[1], ip->inst[2], ip->inst[3], ap->ln);
	    sprintf(ln, "\tmovl\t%s,%s\n", archiregs[-IREGBEG-op2], 
	            GetDeref(op1));
            ap->next = NewAssln(ln);
	    #else
	       ap->next = NewAssln("");
	    #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tst\t%s,%s\n", archiregs[-IREGBEG-op2],
                                  GetDeref(op1));
         #elif defined(PPC) /* HERE HERE */
            k = (op1-1)<<2;
            if (DT[k+1])
               ap->next = PrintAssln("\tstwx\t%s,%s\n",
                                     archiregs[-IREGBEG-op2], GetDeref(op1));
            else
               ap->next = PrintAssln("\tstw\t%s,%s\n", archiregs[-IREGBEG-op2], 
                                     GetDeref(op1));
         #endif
         break;
      case FSTD:
         #ifdef X86
            ap->next = PrintAssln("\tmovlpd\t%s,%s\n", archdregs[-DREGBEG-op2],
                                  GetDeref(op1));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tstd\t%s,%s\n", archdregs[-DREGBEG-op2],
                                  GetDeref(op1));
         #elif defined(PPC)
            k = (op1-1)<<2;
            if (DT[k+1])
               ap->next = PrintAssln("\tstfdx\t%s,%s\n",
                                     archdregs[-DREGBEG-op2], GetDeref(op1));
            else
               ap->next = PrintAssln("\tstfd\t%s,%s\n",archdregs[-DREGBEG-op2],
                                     GetDeref(op1));
         #endif
         break;
      case FST:
         #ifdef X86
            ap->next = PrintAssln("\tmovss\t%s,%s\n", archfregs[-FREGBEG-op2],
                                  GetDeref(op1));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tst\t%s,%s\n", archfregs[-FREGBEG-op2],
                                  GetDeref(op1));
         #elif defined(PPC)
            k = (op1-1)<<2;
            if (DT[k+1])
               ap->next = PrintAssln("\tstfsx\t%s,%s\n",
                                     archfregs[-FREGBEG-op2], GetDeref(op1));
            else
               ap->next = PrintAssln("\tstfs\t%s,%s\n", 
                                     archfregs[-FREGBEG-op2], GetDeref(op1));
         #endif
         break;
   #ifdef X86_64
      case SHLS:
         assert(op1 == op2);
         ap->next = PrintAssln("\tshll\t%s, %s\n", GetSregOrConst(op3), 
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
      case SHL:
         #ifdef X86_64
            assert(op1 == op2);
            ap->next = PrintAssln("\tshlq\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(X86)
            assert(op1 == op2);
            ap->next = PrintAssln("\tshll\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tsll\t%s, %s, %s\n", 
                                  archiregs[-IREGBEG-op2], GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tslwi\t%s, %s, %s\n",
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2], 
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tslw\t%s, %s, %s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
   #ifdef X86_64
      case SHRS:
         assert(op1 == op2);
         ap->next = PrintAssln("\tshrl\t%s, %s\n", GetSregOrConst(op3), 
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
      case SHR:
         #ifdef X86_64
            assert(op1 == op2);
            ap->next = PrintAssln("\tshrq\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(X86)
            assert(op1 == op2);
            ap->next = PrintAssln("\tshrl\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tsrl\t%s, %s, %s\n", 
                                  archiregs[-IREGBEG-op2], GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tsrwi\t%s, %s, %s\n",
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2], 
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tsrw\t%s, %s, %s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
   #ifdef X86_64
      case SARS:
         assert(op1 == op2);
         ap->next = PrintAssln("\tsarl\t%s, %s\n", GetSregOrConst(op3), 
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
      case SAR:
         #ifdef X86_64
            assert(op1 == op2);
            ap->next = PrintAssln("\tsarq\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(X86)
            assert(op1 == op2);
            ap->next = PrintAssln("\tsarl\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tsra\t%s, %s, %s\n", 
                                  archiregs[-IREGBEG-op2], GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tsrawi\t%s, %s, %s\n",
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2], 
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tsraw\t%s, %s, %s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
   #ifdef X86_64
      case NOTS:
         assert(op1 == op2 && op1 < 0);
         ap->next = PrintAssln("\tnotl\t%s\n", archsregs[-IREGBEG-op1]);
         break;
   #endif
      case NOT:
         assert(op1 == op2 && op1 < 0);
         ap->next = PrintAssln("\tnot\t%s\n", archiregs[-IREGBEG-op1]);
         break;
   #ifdef X86_64
      case XORS:
      case ORS:
         if (ip->inst[0] == XOR) sptr = "xorl";
         else sptr = "orl";
         assert(op1 == op2);
         ap->next = PrintAssln("\t%s\t%s, %s\n", sptr, GetSregOrConst(op3), 
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
      case XOR:
      case OR :
          #ifdef X86_64
             if (ip->inst[0] == XOR) sptr = "xorq";
             else sptr = "orq";
          #elif defined(X86)
             if (ip->inst[0] == XOR) sptr = "xorl";
             else sptr = "orl";
          #else
             if (ip->inst[0] == XOR) sptr = "xor";
             else sptr = "or";
          #endif
         #ifdef X86
            assert(op1 == op2);
            ap->next = PrintAssln("\t%s\t%s, %s\n", sptr, GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\t%s\t%s, %s, %s\n", sptr,
                                  archiregs[-IREGBEG-op2], GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\t%si\t%s, %s, %s\n", sptr,
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2], 
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\t%s\t%s, %s, %s\n", sptr,
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
   #ifdef X86_64
      case ADDS:
         assert(op1 == op2);
         ap->next = PrintAssln("\taddl\t%s, %s\n", GetSregOrConst(op3), 
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
      case ADD:
         #ifdef X86_64
            assert(op1 == op2);
            ap->next = PrintAssln("\taddq\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(X86)
            assert(op1 == op2);
            ap->next = PrintAssln("\taddl\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(SPARC)
/*
            fprintf(stderr, "reg2='%s'\n", archiregs[-IREGBEG-op2]);
            fprintf(stderr, "reg1='%s'\n", archiregs[-IREGBEG-op1]);
            fprintf(stderr, "const='%s'\n", GetIregOrConst(op3));
*/
#if 0
            sprintf(ln, "\tadd\t%s, %s, %s\n", archiregs[-IREGBEG-op2], 
                    GetIregOrConst(op3), archiregs[-IREGBEG-op1]);
            ap->next = NewAssln(ln);
#else
            ap->next = PrintAssln("\tadd\t%s, %s, %s\n", 
                                  archiregs[-IREGBEG-op2], GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op1]);
#endif
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\taddi\t%s, %s, %s\n",
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2], 
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tadd\t%s, %s, %s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
   #ifdef X86_64
      case SUBS:
         assert(op1 == op2);
         ap->next = PrintAssln("\tsubl\t%s, %s\n", GetSregOrConst(op3), 
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
      case SUB:
         #ifdef X86_64
            assert(op1 == op2);
            ap->next = PrintAssln("\tsubq\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(X86)
            assert(op1 == op2);
            ap->next = PrintAssln("\tsubl\t%s, %s\n", GetIregOrConst(op3), 
                                  archiregs[-IREGBEG-op1]);
         #elif defined(SPARC)
            if (-REG_SP == op1 && -REG_SP == op2 && !SeenSave)
            {
               SeenSave = 1;
               ap->next = PrintAssln("\tsave\t%s, -%s, %s\n", 
                                     archiregs[-IREGBEG-op2],
                                     GetIregOrConst(op3),
                                     archiregs[-IREGBEG-op1]);
            }
            else
               ap->next = PrintAssln("\tsub\t%s, %s, %s\n", 
                                     archiregs[-IREGBEG-op2],
                                     GetIregOrConst(op3),
                                     archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tsubi\t%s, %s, %s\n",
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2], 
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tsub\t%s, %s, %s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
      case MUL:
         #ifdef X86
            #ifdef X86_32
               sptr = "mull";
            #else
               sptr = "mulq";
            #endif
            if (op3 > 0)
            {
               ap->next = PrintAssln("\ti%s\t%s,%s,%s\n", sptr,
                                     GetIregOrConst(op3),
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op1]);
            }
            else
            {
               assert(op1 == op2);
               ap->next = PrintAssln("\ti%s\t%s,%s\n", sptr,
                                     archiregs[-IREGBEG-op3],
                                     archiregs[-IREGBEG-op1]);
            }
         #elif defined(SPARC)
            ap->next = PrintAssln("\tsmul\t%s,%s,%s\n", archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3), archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tmulli\t%s,%s,%s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tmullw\t%s,%s,%s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
      case UMUL:
         #ifdef X86
            #ifdef X86_32
               sptr = "imull";
            #else
               sptr = "imulq";
            #endif
/*
 * NOTE: we use signed mul for umul on the x86, so we can usual the
 * unrestricted form.  This means our total size may lose a bit
 */
            if (op3 > 0)
            {
               ap->next = PrintAssln("\t%s\t%s,%s,%s\n", sptr,
                                     GetIregOrConst(op3),
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op1]);
            }
            else
            {
               assert(op1 == op2);
               ap->next = PrintAssln("\t%s\t%s,%s\n", sptr,
                                     archiregs[-IREGBEG-op3],
                                     archiregs[-IREGBEG-op1]);
            }
         #elif defined(SPARC)
            ap->next = PrintAssln("\tumul\t%s,%s,%s\n", archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3), archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tmulli\t%s,%s,%s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tmullw\t%s,%s,%s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
      case DIV:
         #ifdef X86
/*
 * HERE HERE: need to have phase zero out %edx before this instruction
 */
            assert(op1 == -iName2Reg("%eax") && op2 == -iName2Reg("%edx"));
            ap->next = PrintAssln("\tidiv %s", GetIregOrDeref(op3));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tsdiv\t%s,%s,%s\n", archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3), archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            assert(op3 < 0);
            ap->next = PrintAssln("\tdivw\t%s,%s,%s\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  archiregs[-IREGBEG-op3]);
         #endif
         break;
      case UDIV:
         #ifdef X86
/*
 * HERE HERE: need to have phase zero out %edx before this instruction
 */
            assert(op1 == -iName2Reg("%eax") && op2 == -iName2Reg("%edx"));
            ap->next = PrintAssln("\tdiv %s", GetIregOrDeref(op3));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tudiv\t%s,%s,%s\n", archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3), archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            assert(op3 < 0);
            ap->next = PrintAssln("\tdivwu\t%s,%s,%s\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  archiregs[-IREGBEG-op3]);
         #endif
         break;
      case CMP:
         #ifdef X86
            ap->next = PrintAssln("\tcmp\t%s,%s\n", GetIregOrConst(op3),
                                  GetIregOrDeref(op2));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tsubcc\t%s,%s,%%g0\n", 
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tcmpwi\tcr0,%s,%s\n", 
                                     archiregs[-IREGBEG-op2],
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tcmpw\tcr0,%s,%s\n", 
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
  #ifdef X86_64
      case MOVS:
         ap->next = PrintAssln("\tmovl\t%s,%s\n", GetSregOrConst(op2),
                               archsregs[-IREGBEG-op1]);
      break;
  #endif
      case MOV:
         #ifdef X86_64
            ap->next = PrintAssln("\tmovq\t%s,%s\n", GetIregOrConst(op2),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(X86)
            ap->next = PrintAssln("\tmovl\t%s,%s\n", GetIregOrConst(op2),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tmov\t%s,%s\n", GetIregOrConst(op2),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op2 > 0)
               ap->next = PrintAssln("\tli\t%s,%s\n", archiregs[-IREGBEG-op1],
                                     GetIregOrConst(op2));
            else
               ap->next = PrintAssln("\tmr\t%s,%s\n", archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2]);
         #endif
         break;
   #ifdef X86_64
      case NEGS:
         assert(op1 == op2);
         ap->next = PrintAssln("\tnegl\t%s", GetIregOrDeref(op1));
         break;
   #endif
      case NEG:
         #ifdef X86_64
            assert(op1 == op2);
            ap->next = PrintAssln("\tnegq\t%s", GetIregOrDeref(op1));
         #elif defined(X86)
            assert(op1 == op2);
            ap->next = PrintAssln("\tnegl\t%s", GetIregOrDeref(op1));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tneg\t%s, %s", archiregs[-IREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tneg\t%s, %s", archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2]);
         #endif
         break;
   #ifdef X86_64
      case CVTSI:
         i = -iName2Reg("%rax")
         assert(op1 == i && op2 == i);
         ap->next = PrintAssln("cltq\n");
/*         ap->next = PrintAssln("cdqe\n"); */
         break;
      case CVTIS:
         ap->next = PrintAssln("movl\t%s,%s\n", archiregs[-IREGBEG-op2],
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
      case JMP:
         #ifdef X86
            ap->next = PrintAssln("\tjmp\t%s\n", STname[op1-1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tba\t%s\n\tnop\n", STname[op1-1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tb\t%s\n", STname[op1-1]);
         #endif
         break;
      case JLT:
         #ifdef X86
            ap->next = PrintAssln("\tjl\t%s\n", STname[op1-1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tbl\t%s\n", STname[op1-1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tblt\t%s\n", STname[op1-1]);
         #endif
         break;
      case RET:
         #ifdef X86
            ap->next = PrintAssln("\tret\n");
         #elif defined(SPARC)
            ap->next = PrintAssln("\tret\n\trestore\n");
         #elif defined(PPC)
            ap->next = PrintAssln("\tblr");
         #endif
         break;

      case FMACD:
         #ifdef X86
            ap->next = PrintAssln("\tUNIMP\n");
	    fko_error(__LINE__, "FMACD found in x86 code!");
         #elif defined(SPARC)
            ap->next = PrintAssln("\tUNIMP\n");
	    fko_error(__LINE__, "FMACD found in SPARC code!");
         #elif defined(PPC)
            ap->next = PrintAssln("\tfmadd\t%s,%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3], archdregs[-DREGBEG-op1]);
         #endif
         break;
      case FMAC:
         #ifdef X86
            ap->next = PrintAssln("\tUNIMP\n");
	    fko_error(__LINE__, "FMAC found in x86 code!");
         #elif defined(SPARC)
            ap->next = PrintAssln("\tUNIMP\n");
	    fko_error(__LINE__, "FMAC found in SPARC code!");
         #elif defined(PPC)
            ap->next = PrintAssln("\tfmadds\t%s,%s,%s,%s\n", 
	       archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
	       archfregs[-FREGBEG-op3], archfregs[-FREGBEG-op1]);
         #endif
         break;
      case FMULD:
         #ifdef X86
	    assert (op1 == op2);
            ap->next = PrintAssln("\tmulsd\t%s,%s\n", GetDregOrDeref(op3),
	                          archdregs[-DREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfmuld\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op3],
	       archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfmul\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FMUL:
         #ifdef X86
	    assert (op1 == op2);
            ap->next = PrintAssln("\tmulss\t%s,%s\n", GetDregOrDeref(op3),
	                          archfregs[-FREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfmuls\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op3],
	       archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfmuls\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FDIVD:
         #ifdef X86
	    assert (op1 == op2);
            ap->next = PrintAssln("\tdivsd\t%s,%s\n", GetDregOrDeref(op3),
	                          archdregs[-DREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfdivd\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op3],
	       archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfdiv\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FDIV:
         #ifdef X86
	    assert (op1 == op2);
            ap->next = PrintAssln("\tdivss\t%s,%s\n", GetDregOrDeref(op3),
	                          archfregs[-FREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfdivs\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op3],
	       archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfdivs\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
	       archfregs[-FREGBEG-op3]);
         #endif
         break;
      case FADDD:
         #ifdef X86
	    assert (op1 == op2);
            ap->next = PrintAssln("\taddsd\t%s,%s\n", GetDregOrDeref(op3),
	                          archdregs[-DREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfaddd\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op3],
	       archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfadd\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FADD:
         #ifdef X86
	    assert (op1 == op2);
            ap->next = PrintAssln("\taddss\t%s,%s\n", GetDregOrDeref(op3),
	                          archfregs[-FREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfadds\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op3],
	       archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfadds\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
	       archfregs[-FREGBEG-op3]);
         #endif
         break;
      case FSUBD:
         #ifdef X86
	    assert (op1 == op2);
            ap->next = PrintAssln("\tsubsd\t%s,%s\n", GetDregOrDeref(op3),
	                          archdregs[-DREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfsubd\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op3],
	       archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfsub\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FSUB:
         #ifdef X86
	    assert (op1 == op2);
            ap->next = PrintAssln("\tsubss\t%s,%s\n", GetDregOrDeref(op3),
	                          archfregs[-FREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfsubs\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op3],
	       archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfsubs\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
	       archfregs[-FREGBEG-op3]);
         #endif
         break;
      case FCMPD:
         #ifdef X86
            ap->next = PrintAssln("\tcomisd\t%s,%s\n", GetDregOrDeref(op2),
	                          archdregs[-DREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfcmped\t%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfcmpo\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FABS:
         #ifdef X86
	    assert(op1 == op2);
	    assert(DTabsd);
            ap->next = PrintAssln("\tandnpd\t%s,%s\n", GetDregOrDeref(DTabsd),
	                          archdregs[-DREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfabsd\t%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfabs\t%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2]);
         #endif
         break;
      case FABSD:
         #ifdef X86
	    assert(op1 == op2);
	    assert(DTabsd);
            ap->next = PrintAssln("\tandnpd\t%s,%s\n", GetDregOrDeref(DTabsd),
	                          archdregs[-DREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfabsd\t%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfabs\t%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2]);
         #endif
         break;
      case FNEGD:
         #ifdef X86
	    assert(op1 == op2);
	    assert(DTnzerod);
            ap->next = PrintAssln("\txorpd\t%s,%s\n", GetDregOrDeref(DTnzerod),
	                          archdregs[-DREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfabsd\t%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfabs\t%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2]);
         #endif
         break;
      case CMPFLAG:
         #ifdef X86
	    ap->next = PrintAssln("# CMPFLAG %d %d %d\n", op1, op2, op3);
         #elif defined(SPARC)
	    ap->next = PrintAssln("! CMPFLAG %d %d %d\n", op1, op2, op3);
         #elif defined(PPC)
	       ap->next = PrintAssln("# CMPFLAG %d %d %d\n", op1, op2, op3);
         #endif
         break;
      case FMOVD:
/*
 *       Handle fp constant moves seperately
 */
         if (op2 > 0)
	 {
	    assert(IS_GLOB(STflag[op2-1]));
	    #ifdef X86
	       ap->next = PrintAssln("\tmovlpd\t%s,%s\n", STname[op2-1],
	                             archdregs[-DREGBEG-op1]);
	    #elif defined(SPARC)
	       ap->next = PrintAssln("\tsethi\t@hi(%s),%s\n", STname[op2-1],
	                             archiregs[-IREGBEG-op3]);
	       ap->next = PrintAssln("\tor\t%s, @lo(%s), %s\n", 
	                             archiregs[-IREGBEG-op3], STname[op2-1],
	                             archiregs[-IREGBEG-op3]);
	       ap->next = PrintAssln("\tldd\t[%s], %s\n",
	                             archiregs[-IREGBEG-op3],
	                             archdregs[-DREGBEG-op1]);
            #elif defined(PPC)
	       ap->next = PrintAssln("\tlis\t%s, ha16(%s)\n", 
	                             archiregs[-IREGBEG-op3], STname[op2-1]);
	       ap->next = PrintAssln("\tori\t%s, lo16(%s)\n", 
	                             archiregs[-IREGBEG-op3], STname[op2-1]);
               ap->next = PrintAssln("\tlfd\t%s,%s\n",
		                     archdregs[-DREGBEG-op1],
				     archiregs[-IREGBEG-op3]);
	    #endif
	 }
	 else
	 {
            #ifdef X86
	       sptr = archdregs[-DREGBEG-op1];
	       if (sptr[1] == 's' && sptr[2] == 't')
	          fko_error(__LINE__, "WTF in %s!", __FILE__);
	       else
                  ap->next = PrintAssln("\tmovlpd\t%s,%s\n",
	                                archdregs[-DREGBEG-op2], sptr);
            #elif defined(SPARC)
               ap->next = PrintAssln("\tfmovd\t%s,%s\n",archdregs[-DREGBEG-op1],
                                     archdregs[-DREGBEG-op1]);
            #elif defined(PPC)
               ap->next = PrintAssln("\tfmr\t%s,%s\n", archdregs[-DREGBEG-op1],
                                     archdregs[-DREGBEG-op2]);
            #endif
            }
	 break;
      case COMMENT:
         #ifdef X86
            ap->next = PrintAssln("#%s\n", op1 ? STname[op1-1] : "");
         #elif defined(SPARC)
            ap->next = PrintAssln("!%s\n", op1 ? STname[op1-1] : "");
	 #elif defined(PPC)
            ap->next = PrintAssln("#%s\n", op1 ? STname[op1-1] : "");
         #endif
         break;
/*
 *  HERE HERE HERE:
 */
      case FCMP:
      case FNEG:
      case FMOV:
      case JEQ:
      case JNE:
      case JLE:
      case JGT:
      case JGE:
      case PREFR:
      case PREFW:
      case PREFRS:
      case PREFWS:
      case CVTFI:
      case CVTIF:
      case CVTDI:
      case CVTID:
      case CVTSF:
      case CVTSD:
   #ifdef X86_64
      case CVTDS:
      case CVTFS:
/*
 * This short inst not defined, since they are not used in preamble
 */
      case MULS:
      case UMULS:
      case DIVS:
      case UDIVS:
      case SHLCCS:
      case SHRCCS:
      case ADDCCS:
      case SUBCCS:
   #endif
      default:
         ap->next = PrintAssln("ERROR:\t%d %d %d %d\n", 
                               ip->inst[0], op1, op2, op3);
        fko_error(__LINE__, "Unknown instruction %d after %s\n", ip->inst[0],
                  ap->ln);
      }
      ip = ip->next;
      ap = ap->next;
   }
   while(ip != head);
   return(ahead);
}

void dump_assembly(FILE *fp, struct assmln *abase)
{
   int i;
   char *ln;
   while(abase)
   {
      ln = abase->ln;
      for (i=0; ln[i]; i++)
         if (ln[i] == '@') ln[i] = '\%';
      fputs(ln, fp);
      abase = abase->next;
   }
}
