#include "fko.h"
#include "fko_arch.h"
#include <stdarg.h>

#ifdef AVX
   #ifdef X86_64
      extern char *archxmmregs[NFR];
   #else
      extern char *archxmmregs[TNFR];
   #endif      
#endif

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
   char ln[4096];
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
   id--;
   assert (id >= 0 && IS_DEREF(STflag[id]));
   ptr = SToff[id].sa[0];
   reg = SToff[id].sa[1];
   mul = SToff[id].sa[2];
   con = SToff[id].sa[3];
   if (reg > 0) reg = 0;
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
   #elif FKO_ANSIC
      if (!reg)
      {
         if (con)
            sprintf(ln, "((char *)(%s+%d))", archiregs[-IREGBEG-ptr], con);
         else sprintf(ln, "(%s)", archiregs[-IREGBEG-ptr]);
      }
      else if (mul)
      {
         if (con) /* if (reg && mul && con) */
            sprintf(ln, "((char*)%s)+(%s*%d)+%d", archireg[-IREGBEG-ptr],
                    archireg[-IREGBEG-reg], mul, con);
         else
            sprintf(ln, "((char*)%s)+(%s*%d)", archireg[-IREGBEG-ptr],
                    archireg[-IREGBEG-reg], mul);
      }
      else /* reg && !mul */
      {
         if (con) 
            sprintf(ln, "((char*)(%s+%s)+%d)", archiregs[-IREGBEG-ptr],
                    archiregs[-IREGBEG-reg], con);
         else
            sprintf(ln, "((char*)(%s+%s))", archiregs[-IREGBEG-ptr],
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

static int GetDregID(short id)
{
/*
 * given an reg id, return appropriate positive index, otherwise -1
 */
   if (id < 0)
   {
      #ifdef VDREGBEG
         if(-id >= VDREGBEG)
            return(-VDREGBEG-id);
      #endif
      #ifdef VFREGBEG
         if(-id >= VFREGBEG)
            return(-VFREGBEG-id);
      #endif
      if (-id >= DREGBEG)
         return(-DREGBEG-id);
      else
         return(-FREGBEG-id);
   }
   else
      return -1;

}
static char *GetDregOrDeref(short id)
/*
 * given an id, return string containing register name if less than zero,
 * or a properly formated deref entry if greater than zero
 */
{
   if (id < 0)
   {
      #ifdef VDREGBEG
         if(-id >= VDREGBEG)
            return(archvdregs[-VDREGBEG-id]);
      #endif
      #ifdef VFREGBEG
         if(-id >= VFREGBEG)
            return(archvfregs[-VFREGBEG-id]);
      #endif
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
      if (IS_CONST(flag))
      {
         if (IS_INT(flag) || IS_SHORT(flag)) sprintf(ln, "$%d", SToff[id].i);
         else fko_error(__LINE__, "Integer constant expected!\n");
      }
      else
      {
        assert(IS_DEREF(flag));
        return(GetSregOrDeref(id+1));
      }
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
   static char ln[128];
   int flag;
   if (id < 0)
      return(archiregs[-IREGBEG-id]);
   else
   {
      assert(id != 0);
      id--;
      flag = STflag[id];
      if (IS_CONST(flag))
      {
         #ifdef X86_64
            if (IS_INT(flag)) sprintf(ln, "$%d", SToff[id].i);
            else if (IS_SHORT(flag)) sprintf(ln, "$0x%lx", SToff[id].l);
         #elif defined(X86)
            if (IS_INT(flag) || IS_SHORT(flag)) sprintf(ln, "$%d", SToff[id].i);
         #else
            if (IS_INT(flag) || IS_SHORT(flag)) sprintf(ln, "%d", SToff[id].i);
         #endif
            else fko_error(__LINE__, "Integer constant expected!\n");
      }
      else
      {
        assert(IS_DEREF(flag));
        return(GetIregOrDeref(id+1));
      }
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

static uchar *imap2cmap(int imap)
{
   static uchar ch[8];
   int i;

   for (i=0; i < 8; i++, imap >>= 4)
      ch[i] = imap & 0x0000000F;
   return(ch);
}
struct assmln *lil2ass(BBLOCK *bbase)
{
   INSTQ *ip;
   BBLOCK *bp;
   uchar *cp;
   struct assmln *ahead=NULL, *ap;
   short inst, op1, op2, op3, k;
   #ifdef SPARC
      int SeenSave=0;
   #endif
   int i, j;
   char ln[1024], *sptr;
   extern int DTabsd, DTnzerod, DTabs, DTnzero, DTx87, DTx87d;
   extern int DTabsds, DTnzerods, DTabss, DTnzeros;
   /* End of declaration */

   ap = ahead = NewAssln(".text\n");

   for (bp=bbase; bp; bp = bp->down)
   {
   for (ip=bp->inst1; ip; ip = ip->next)
   {
      inst = ip->inst[0] & 0x3FFF;
      op1 = ip->inst[1];
      op2 = ip->inst[2];
      op3 = ip->inst[3];
      switch(inst)
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
         #elif FKO_ANSIC
            ap->next = PrintAssln("%s:\n", STname[op1]);
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
            k = op2-1;
            if (SToff[k].sa[1] < 0)
               ap->next = PrintAssln("\tlwzx\t%s,%s\n", archiregs[-IREGBEG-op1],
                                     GetDeref(op2));
            else
               ap->next = PrintAssln("\tlwz\t%s,%s\n", archiregs[-IREGBEG-op1], 
                                     GetDeref(op2));
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = *((int*)(%s));\n", 
                                  archiregs[-IREGBEG-op1], GetDeref(op2));
         #endif
         break;
      case FLDD:
         #ifdef X86
	    sptr = archdregs[-DREGBEG-op1];
	    if (sptr[1] == 's' && sptr[2] == 't')
               ap->next = PrintAssln("\tfldl\t%s\n", GetDeref(op2));
	    else
               #ifdef AVX
                  /* vmovsd m8s,r16d  # rd[0]=ms[0]; rd[1:3]=0.0; */
                  ap->next = PrintAssln("\tvmovsd\t%s,%s\n", GetDeref(op2),
                                        archxmmregs[-DREGBEG-op1]);
               #else
                  ap->next = PrintAssln("\tmovsd\t%s,%s\n", GetDeref(op2),
                                        archdregs[-DREGBEG-op1]);
               #endif

         #elif defined(SPARC)
            ap->next = PrintAssln("\tldd\t%s,%s\n", GetDeref(op2),
                                  archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            if (SToff[op2-1].sa[1] < 0)
               ap->next = PrintAssln("\tlfdx\t%s,%s\n", archdregs[-DREGBEG-op1],
                                     GetDeref(op2));
            else
               ap->next = PrintAssln("\tlfd\t%s,%s\n", archdregs[-DREGBEG-op1], 
                                     GetDeref(op2));
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = *((double*)(%s));\n", 
                                  archdregs[-DREGBEG-op1], GetDeref(op2));
         #endif
         break;
      case FLD:
         #ifdef X86
	    sptr = archfregs[-FREGBEG-op1];
	    if (sptr[1] == 's' && sptr[2] == 't')
               ap->next = PrintAssln("\tflds\t%s\n", GetDeref(op2));
	    else
               #ifdef AVX
               /*vmovss ms,r16d # rd[0]=ms[0]; rd[1:7]=0.0 */
                  ap->next = PrintAssln("\tvmovss\t%s,%s\n", GetDeref(op2),
                                        archxmmregs[-FREGBEG-op1]);
               #else
                  ap->next = PrintAssln("\tmovss\t%s,%s\n", GetDeref(op2),
                                        archfregs[-FREGBEG-op1]);
               #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tld\t%s,%s\n", GetDeref(op2),
                                  archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            if (SToff[op2-1].sa[1] < 0)
               ap->next = PrintAssln("\tlfsx\t%s,%s\n", archfregs[-FREGBEG-op1],
                                     GetDeref(op2));
            else
               ap->next = PrintAssln("\tlfs\t%s,%s\n", archfregs[-FREGBEG-op1], 
                                     GetDeref(op2));
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = *((float*)(%s));\n", 
                                  archfregs[-FREGBEG-op1], GetDeref(op2));
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
	    #if 1
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
            if (SToff[op1-1].sa[1] < 0)
               ap->next = PrintAssln("\tstwx\t%s,%s\n",
                                     archiregs[-IREGBEG-op2], GetDeref(op1));
            else
               ap->next = PrintAssln("\tstw\t%s,%s\n", archiregs[-IREGBEG-op2], 
                                     GetDeref(op1));
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   *((int*)(%s)) = %s;\n", 
                                  GetDeref(op1), archiregs[-IREGBEG-op2]);
         #endif
         break;
      case FSTD:
         #ifdef X86
            #ifdef AVX
               /*vmovsd r16s, m8rd # mrd[0]=rs[0] ---- need to check vmovlpd */
               ap->next = PrintAssln("\tvmovsd\t%s,%s\n", 
                                     archxmmregs[-DREGBEG-op2],
                                     GetDeref(op1)); 
            #else
               ap->next = PrintAssln("\tmovlpd\t%s,%s\n", 
                                     archdregs[-DREGBEG-op2],
                                     GetDeref(op1));
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tstd\t%s,%s\n", archdregs[-DREGBEG-op2],
                                  GetDeref(op1));
         #elif defined(PPC)
            if (SToff[op1-1].sa[1] < 0)
               ap->next = PrintAssln("\tstfdx\t%s,%s\n",
                                     archdregs[-DREGBEG-op2], GetDeref(op1));
            else
               ap->next = PrintAssln("\tstfd\t%s,%s\n",archdregs[-DREGBEG-op2],
                                     GetDeref(op1));
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   *((double*)(%s)) = %s;\n", 
                                  GetDeref(op1), archdregs[-DREGBEG-op2]);
         #endif
         break;
      case FST:
         #ifdef X86
            #ifdef AVX
               /*vmovss rs, mrd*/
               ap->next = PrintAssln("\tvmovss\t%s,%s\n",
                                     archxmmregs[-FREGBEG-op2],GetDeref(op1));
            #else
               ap->next = PrintAssln("\tmovss\t%s,%s\n",archfregs[-FREGBEG-op2],
                                     GetDeref(op1));
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tst\t%s,%s\n", archfregs[-FREGBEG-op2],
                                  GetDeref(op1));
         #elif defined(PPC)
            if (SToff[op1-1].sa[1] < 0)
               ap->next = PrintAssln("\tstfsx\t%s,%s\n",
                                     archfregs[-FREGBEG-op2], GetDeref(op1));
            else
               ap->next = PrintAssln("\tstfs\t%s,%s\n", 
                                     archfregs[-FREGBEG-op2], GetDeref(op1));
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   *((float*)(%s)) = %s;\n", 
                                  GetDeref(op1), archfregs[-FREGBEG-op2]);
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
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s << %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
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
/*
 *       NOTE: ANSI C's shift may not be logical!
 */
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s >> %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
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
/*
 *       NOTE: ANSI C's shift may not be arithmetic!
 */
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s >> %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
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
         #ifdef FKO_ANSIC
            ap->next = PrintAssln("%s = ~%s;\n", archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tnot\t%s\n", archiregs[-IREGBEG-op1]);
         #endif
         break;
   #ifdef X86_64
      case XORS:
      case ORS:
         if (inst == XORS) sptr = "xorl";
         else sptr = "orl";
         assert(op1 == op2);
         ap->next = PrintAssln("\t%s\t%s, %s\n", sptr, GetSregOrConst(op3), 
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
      case XOR:
      case OR :
          #ifdef X86_64
             if (inst == XOR) sptr = "xorq";
             else sptr = "orq";
          #elif defined(X86)
             if (inst == XOR) sptr = "xorl";
             else sptr = "orl";
          #else
             if (inst == XOR) sptr = "xor";
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
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s %s %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  inst == XOR ? "^" : "|",
                                  GetIregOrConst(op3));
         #endif
         break;
      case AND:
      case ANDCC :
          #ifdef X86_64
             sptr = "andq";
          #elif defined(X86)
             sptr = "andl";
          #elif defined(SPARC)
             if (inst == AND) sptr = "and";
             else sptr = "andcc";
          #elif defined(PPC)
             if (inst == AND)
             {
                if (op3 > 0) sptr = "andi";
                else sptr = "and";
             }
             else
             {
                if (op3 > 0) sptr = "andi.";
                else sptr = "and.";
             }
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
            ap->next = PrintAssln("\t%s\t%s, %s, %s\n", sptr,
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2], 
                                  GetIregOrConst(op3));
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s %s %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  inst == XOR ? "^" : "|",
                                  GetIregOrConst(op3));
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
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s + %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
         #endif
         break;
   #ifdef X86_64
      case SUBS:
         assert(op1 == op2);
         ap->next = PrintAssln("\tsubl\t%s, %s\n", GetSregOrConst(op3), 
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
      #ifndef X86
      case SUBCC:
         #if defined(SPARC)
            ap->next = PrintAssln("\tsubcc\t%s, %s, %s\n", 
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tsubi.\t%s, %s, %s\n",
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2], 
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tsub.\t%s, %s, %s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
      #else
      case SUBCC:
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
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s - %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
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
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s * %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
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
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s * %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
         #endif
         break;
      case DIV:
/*
 * HERE HERE: need to remember that eax & edx are overwritten!!
 */
         #ifdef X86_32
            fprintf(stderr, "op1=%d, op2=%d (%d,%d)\n", op1, op2,
                    iName2Reg("@eax"), iName2Reg("@edx"));

            assert(op1 == -iName2Reg("@eax") && op2 == -iName2Reg("@edx"));
            ap->next = PrintAssln("\tidiv %s\n", GetIregOrDeref(op3));
         #elif defined (X86_64)
            assert(op1 == -iName2Reg("@rax") && op2 == -iName2Reg("@rdx"));
            ap->next = PrintAssln("\tidiv %s\n", GetIregOrDeref(op3));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tsdiv\t%s,%s,%s\n", archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3), archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            assert(op3 < 0);
            ap->next = PrintAssln("\tdivw\t%s,%s,%s\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  archiregs[-IREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s / %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
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
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s / %s;\n", 
                                  archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
         #endif
         break;
      case CMPAND :
         #ifdef X86
            ap->next = PrintAssln("\ttest\t%s,%s\n", GetIregOrConst(op3),
                                  GetIregOrDeref(op2));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tandcc\t%s,%s,%%g0\n", 
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tandi.\t%s,%s,%s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tand.\t%s,%s,%s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
      case CMP:
         #ifdef X86
            ap->next = PrintAssln("\tcmp\t%s,%s\n", 
                                  GetIregOrConst(op3),
                                  GetIregOrDeref(op2));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tsubcc\t%s,%s,%%g0\n", 
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tcmpwi\t%s,%s,%s\n", 
                                     ICCREGS[-ICC0-op1],
                                     archiregs[-IREGBEG-op2],
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tcmpw\t%s,%s,%s\n", 
                                     ICCREGS[-ICC0-op1],
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   icc = %s - %s;\n", 
                                  archiregs[-IREGBEG-op2],
                                  GetIregOrConst(op3));
         #endif
         break;
      case FCMP:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvcomiss\t%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-FREGBEG-op2]);
               else   
                  ap->next = PrintAssln("\tvcomiss\t%s,%s\n",
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op2]);
            #else
               ap->next = PrintAssln("\tcomiss\t%s,%s\n", GetDregOrDeref(op3),
	                             archfregs[-FREGBEG-op2]);
            #endif   
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfcmpes\t%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op3]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfcmpo\t%s,%s,%s\n", 
               FCCREGS[-FCC0-op1], archfregs[-FREGBEG-op2],
	       archfregs[-FREGBEG-op3]);
         #endif
         break;
      case FCMPD:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvcomisd\t%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-DREGBEG-op2]);
               else
                  ap->next = PrintAssln("\tvcomisd\t%s,%s\n", GetDregOrDeref(op3),
	                                archxmmregs[-DREGBEG-op2]);
            #else
               ap->next = PrintAssln("\tcomisd\t%s,%s\n", GetDregOrDeref(op3),
	                             archdregs[-DREGBEG-op2]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfcmped\t%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op3]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfcmpo\t%s,%s,%s\n", 
               FCCREGS[-FCC0-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
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
         #elif defined(FKO_ANSIC)
            if (op1 < 0)
               ap->next = PrintAssln("   %s = %s;\n", archiregs[-IREGBEG-op1],
                                     GetIregOrConst(op2));
            else
               ap->next = PrintAssln("   %s = %s;\n", 
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
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = -%s;\n", archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op1]);
         #endif
         break;
   #ifdef X86_64
      case CVTSI:
         k = -iName2Reg("@rax");
         assert(op1 == k && op2 == k);
         ap->next = PrintAssln("\tcltq\n");
/*         ap->next = PrintAssln("\tcdqe\n"); */
         break;
      case CVTIS:
         ap->next = PrintAssln("movl\t%s,%s\n", archiregs[-IREGBEG-op2],
                               archsregs[-IREGBEG-op1]);
         break;
   #endif
/*
 * These are x86-only instructions to handle SSE-to-icc conversions
 */
   #ifdef X86
/*    Majedul: Not in use right now. see h2l.c:800 */
      case FCMPW:  /* special cmp that overwrites an operand */
         assert(op3 > 0);
         k = SToff[op3-1].i;
         assert(k < 3 && k >= 0);

         #ifdef AVX
/* 
 *          Note: There is a significant difference between vcmpss and cmpss
 *          vcmpss rs2, rs1, rd # true? rd[0] = 0xFFFFFFFF : 0x00000000
 *          rd[1:3] = rs1[1:3]; rd[4:7] = 0
 *          Where as for cmpss rd[1:3] is unchanged. 
 *          Here, (rd==rs1) can be used.
 */
            ap->next = PrintAssln("\tvcmpss\t$%d,%s,%s,%s\n", k,
                                  archfregs[-FREGBEG-op2],
                                  archfregs[-FREGBEG-op1],
                                  archfregs[-FREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpss\t$%d,%s,%s\n", k, 
                          archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op1]);
         #endif
         break;
/*    Majedul: Not in use right now. see h2l.c:800*/
      case FCMPWD:  /* special cmp that overwrites an operand */
         assert(op3 > 0);
         k = SToff[op3-1].i;
         assert(k < 3 && k >= 0);
         #ifdef AVX
/*          k==[0,1,2]; [EQ,LT,LE]*/
            ap->next = PrintAssln("\tvcmpsd\t$%d,%s,%s,%s\n", k,
                                  archdregs[-DREGBEG-op2],
                                  archdregs[-DREGBEG-op1],
                                  archdregs[-DREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpsd\t$%d,%s,%s\n", k, 
                          archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op1]);
         #endif
         break;
/*    Majedul: Not in use currently. see h2l.c:800*/
      case CVTBFI:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovmskps\t%s,%s\n",
                                   archfregs[-FREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);      
         # else
            ap->next = PrintAssln("\tmovmskps\t%s,%s\n",
                                   archfregs[-FREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);
         #endif
         break;
 /*   Majedul: Not in use right now. see h2l.c:800*/   
      case CVTBDI:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovmskpd\t%s,%s\n", 
                                  archdregs[-DREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovmskpd\t%s,%s\n", 
                                  archdregs[-DREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
         #endif
         break;
   #endif
      case JMP:
         #ifdef X86
            ap->next = PrintAssln("\tjmp\t%s\n", STname[op2-1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tba\t%s\n\tnop\n", STname[op2-1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tb\t%s\n", STname[op2-1]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   goto %s;\n", STname[op2-1]);
         #endif
         break;
      case JEQ:
         #ifdef X86
            ap->next = PrintAssln("\tje\t%s\n", STname[op3-1]);
         #elif defined(SPARC)
            if (-op1 >= ICC0 && -op1 < ICC0+NICC)
               ap->next = PrintAssln("\tbe\t%s\n\tnop\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tfbe\t%s\n\tnop\n", STname[op3-1]);
         #elif defined(PPC)
            if (-op2 >= ICC0 && -op2 <= ICC0+NICC) sptr = ICCREGS[-ICC0-op2];
            else sptr = FCCREGS[-FCC0-op2];
            k = sptr[2] - '0';
            ap->next = PrintAssln("\tbt\t%d, %s\n", k*4+2, STname[op3-1]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   if (icc < 0) goto %s;\n",
                                  STname[op3-1]);
         #endif
         break;
      case JNE:
         #ifdef X86
            ap->next = PrintAssln("\tjne\t%s\n", STname[op3-1]);
         #elif defined(SPARC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC)
               ap->next = PrintAssln("\tbne\t%s\n\tnop\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tfbne\t%s\n\tnop\n", STname[op3-1]);
         #elif defined(PPC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC) sptr = ICCREGS[-ICC0-op2];
            else sptr = FCCREGS[-FCC0-op2];
            k = sptr[2] - '0';
            ap->next = PrintAssln("\tbf\t%d, %s\n", k*4+2, STname[op3-1]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   if (icc < 0) goto %s;\n",
                                  STname[op3-1]);
         #endif
         break;
      case JLT:
         #ifdef X86
            if (-op2 >= ICC0 && -op2 < ICC0+NICC)
               ap->next = PrintAssln("\tjl\t%s\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tjb\t%s\n", STname[op3-1]);
         #elif defined(SPARC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC)
               ap->next = PrintAssln("\tbl\t%s\n\tnop\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tfbl\t%s\n\tnop\n", STname[op3-1]);
         #elif defined(PPC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC) sptr = ICCREGS[-ICC0-op2];
            else sptr = FCCREGS[-FCC0-op2];
            k = sptr[2] - '0';
            ap->next = PrintAssln("\tbt\t%d, %s\n", k*4+0, STname[op3-1]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   if (icc < 0) goto %s;\n",
                                  STname[op2-1]);
         #endif
         break;
      case JGT:
         #ifdef X86
            if (-op2 >= ICC0 && -op2 < ICC0+NICC)
               ap->next = PrintAssln("\tjg\t%s\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tja\t%s\n", STname[op3-1]);
         #elif defined(SPARC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC)
               ap->next = PrintAssln("\tbg\t%s\n\tnop\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tfbg\t%s\n\tnop\n", STname[op3-1]);
         #elif defined(PPC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC) sptr = ICCREGS[-ICC0-op2];
            else sptr = FCCREGS[-FCC0-op2];
            k = sptr[2] - '0';
            ap->next = PrintAssln("\tbt\t%d, %s\n", k*4+1, STname[op3-1]);
         #endif
         break;
      case JLE:
         #ifdef X86
            if (-op2 >= ICC0 && -op2 < ICC0+NICC)
               ap->next = PrintAssln("\tjle\t%s\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tjna\t%s\n", STname[op3-1]);
         #elif defined(SPARC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC)
               ap->next = PrintAssln("\tble\t%s\n\tnop\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tfble\t%s\n\tnop\n", STname[op3-1]);
         #elif defined(PPC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC) sptr = ICCREGS[-ICC0-op2];
            else sptr = FCCREGS[-FCC0-op2];
            k = sptr[2] - '0';
            ap->next = PrintAssln("\tbf\t%d, %s\n", k*4+1, STname[op3-1]);
         #endif
         break;
      case JGE:
         #ifdef X86
            if (-op2 >= ICC0 && -op2 < ICC0+NICC)
               ap->next = PrintAssln("\tjge\t%s\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tjae\t%s\n", STname[op3-1]);
         #elif defined(SPARC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC)
               ap->next = PrintAssln("\tbge\t%s\n\tnop\n", STname[op3-1]);
            else
               ap->next = PrintAssln("\tfbge\t%s\n\tnop\n", STname[op3-1]);
         #elif defined(PPC)
            if (-op2 >= ICC0 && -op2 < ICC0+NICC) sptr = ICCREGS[-ICC0-op2];
            else sptr = FCCREGS[-FCC0-op2];
            k = sptr[2] - '0';
            ap->next = PrintAssln("\tbf\t%d, %s\n", k*4+0, STname[op3-1]);
         #endif
         break;
/*
 *    HERE HERE: need a global variable for ANSI C, saying whether
 *    we are returning a variable or not
 */
      case RET:
         #ifdef X86
            ap->next = PrintAssln("\tret\n");
         #elif defined(SPARC)
            ap->next = PrintAssln("\tret\n\trestore\n");
         #elif defined(PPC)
            ap->next = PrintAssln("\tblr\n");
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   return(%s);\n", "notsupported");
         #endif
         break;

      case FMACD:
         #ifdef X86
            #if defined(ArchHasMAC) && defined(FMA4)
               if (op3 < 0)   /* FMA4: only src2 can be mem */
                  ap->next = PrintAssln("\tvfmaddsd\t%s,%s,%s,%s\n", 
	                                archxmmregs[-DREGBEG-op1],
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-DREGBEG-op2],
                                        archxmmregs[-DREGBEG-op1]);
               else   
                  ap->next = PrintAssln("\tvfmaddsd\t%s,%s,%s,%s\n", 
	                                archxmmregs[-DREGBEG-op1],
                                        GetDregOrDeref(op3),
	                                archxmmregs[-DREGBEG-op2],
                                        archxmmregs[-DREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "FMACD Not found in this x86 arch!");
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tUNIMP\n");
	    fko_error(__LINE__, "FMACD found in SPARC code!");
         #elif defined(PPC)
            ap->next = PrintAssln("\tfmadd\t%s,%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3], archdregs[-DREGBEG-op1]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s += %s * %s;\n",
                archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
                archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FMAC:
         #ifdef X86
            #if defined(ArchHasMAC) && defined(FMA4) 
               if (op3 < 0) /*FMA4: only src2 can be used as mem */
                  ap->next = PrintAssln("\tvfmaddss\t%s,%s,%s,%s\n", 
	                                archxmmregs[-FREGBEG-op1],
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvfmaddss\t%s,%s,%s,%s\n", 
	                                archxmmregs[-FREGBEG-op1],
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "FMAC Not found in this x86 arch!");
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tUNIMP\n");
	    fko_error(__LINE__, "FMAC found in SPARC code!");
         #elif defined(PPC)
            ap->next = PrintAssln("\tfmadds\t%s,%s,%s,%s\n", 
	       archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
	       archfregs[-FREGBEG-op3], archfregs[-FREGBEG-op1]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s += %s * %s;\n",
                archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
                archfregs[-FREGBEG-op3]);
         #endif
         break;
      case FMULD:
         #ifdef X86
            #ifdef AVX
/*             vmulsd mr16s2,rs1,rd # rd[0]=rs1[0]*mrs2[0] */
               if (op3 < 0)
                  ap->next = PrintAssln("\tvmulsd\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-DREGBEG-op2],
                                        archxmmregs[-DREGBEG-op1]);
               else   
                  ap->next = PrintAssln("\tvmulsd\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-DREGBEG-op2],
                                        archxmmregs[-DREGBEG-op1]);
            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tmulsd\t%s,%s\n", GetDregOrDeref(op3),
	                             archdregs[-DREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfmuld\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op3],
	       archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfmul\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s * %s;\n",
                archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
                archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FMUL:
         #ifdef X86
            #ifdef AVX
/*             vmulss mr16s2,r16s1,r16d  #  */
               if (op3 < 0)
                  ap->next = PrintAssln("\tvmulss\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvmulss\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tmulss\t%s,%s\n", GetDregOrDeref(op3),
	                            archfregs[-FREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfmuls\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op3],
	       archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfmuls\t%s,%s,%s\n", 
	       archdregs[-FREGBEG-op1], archdregs[-FREGBEG-op2],
	       archdregs[-FREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s * %s;\n",
                archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
                archfregs[-FREGBEG-op3]);
         #endif
         break;
      case FDIVD:
         #ifdef X86
            #ifdef AVX
/*             vdivsd mr16s2,r16s1,r16d */
               if (op3 < 0)
                  ap->next = PrintAssln("\tvdivsd\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-DREGBEG-op2],  
	                                archxmmregs[-DREGBEG-op1]);  
               else
                  ap->next = PrintAssln("\tvdivsd\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-DREGBEG-op2],  
	                                archxmmregs[-DREGBEG-op1]);  
            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tdivsd\t%s,%s\n", GetDregOrDeref(op3),
	                             archdregs[-DREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfdivd\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op3],
	       archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfdiv\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s / %s;\n",
                archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
                archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FDIV:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvdivss\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-FREGBEG-op2], 
	                                archxmmregs[-FREGBEG-op1]); 
               else
                  ap->next = PrintAssln("\tvdivss\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op2], 
	                                archxmmregs[-FREGBEG-op1]); 

            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tdivss\t%s,%s\n", GetDregOrDeref(op3),
	                             archfregs[-FREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfdivs\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op3],
	       archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfdivs\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
	       archfregs[-FREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s / %s;\n",
                archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
                archfregs[-FREGBEG-op3]);
         #endif
         break;
      case FZERO:
         #ifdef X86
            #ifdef AVX
               ap->next = PrintAssln("\tvxorps\t%s,%s,%s\n", 
                                     archfregs[-FREGBEG-op1],
                                     archfregs[-FREGBEG-op1],
                                     archfregs[-FREGBEG-op1]);
               
            #else
               ap->next = PrintAssln("\txorps\t%s,%s\n", 
                                     archfregs[-FREGBEG-op1],
                                     archfregs[-FREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfzeros\t%s\n", archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            fko_error(__LINE__, "FZERO not valid inst for PPC!\n");
         #endif
         break;
      case FZEROD:
         #ifdef X86
            #ifdef AVX
               ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                     archdregs[-DREGBEG-op1],
                                     archdregs[-DREGBEG-op1],
                                     archdregs[-DREGBEG-op1]);

            #else
               ap->next = PrintAssln("\txorpd\t%s,%s\n", 
                                     archdregs[-DREGBEG-op1],
                                     archdregs[-DREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfzero\t%s\n", archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            fko_error(__LINE__, "FZERO not valid inst for PPC!\n");
         #endif
         break;
      case FADDD:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvaddsd\t%s,%s,%s\n",
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-DREGBEG-op2], 
	                                archxmmregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvaddsd\t%s,%s,%s\n",
                                        GetDregOrDeref(op3),
	                                archxmmregs[-DREGBEG-op2], 
	                                archxmmregs[-DREGBEG-op1]);
            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\taddsd\t%s,%s\n", GetDregOrDeref(op3),
	                          archdregs[-DREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfaddd\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op3],
	       archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfadd\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s + %s;\n",
                archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
                archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FADD:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0 )
                  ap->next = PrintAssln("\tvaddss\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-FREGBEG-op1],
	                                archxmmregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvaddss\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op1],
	                                archxmmregs[-FREGBEG-op1]);

            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\taddss\t%s,%s\n", GetDregOrDeref(op3),
	                             archfregs[-FREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfadds\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op3],
	       archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfadds\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
	       archfregs[-FREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s + %s;\n",
                archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
                archfregs[-FREGBEG-op3]);
         #endif
         break;
      case FSUBD:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvsubsd\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-DREGBEG-op2],
	                                archxmmregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvsubsd\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-DREGBEG-op2],
	                                archxmmregs[-DREGBEG-op1]);
            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tsubsd\t%s,%s\n", GetDregOrDeref(op3),
	                          archdregs[-DREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfsubd\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op3],
	       archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfsub\t%s,%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
	       archdregs[-DREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s - %s;\n",
                archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2],
                archdregs[-DREGBEG-op3]);
         #endif
         break;
      case FSUB:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvsubss\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvsubss\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);

            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tsubss\t%s,%s\n", GetDregOrDeref(op3),
	                          archfregs[-FREGBEG-op1]);
            #endif 
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfsubs\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op3],
	       archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfsubs\t%s,%s,%s\n", 
	       archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
	       archfregs[-FREGBEG-op3]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s - %s;\n",
                archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
                archfregs[-FREGBEG-op3]);
         #endif
         break;
      case FABS:
         #ifdef X86
            #ifdef AVX
               if (op3 >= 0)
                  ap->next = PrintAssln("\tvandps\t%s,%s,%s\n", 
                                        GetDeref(SToff[DTabss-1].sa[2]),
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvandps\t%s, %s, %s\n", 
	                                archfregs[-FREGBEG-op3],
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
            #else
               assert(op2 == op1);
               if (op3 >= 0)
                  ap->next = PrintAssln("\tandps\t%s,%s\n", 
                                        GetDeref(SToff[DTabss-1].sa[2]),
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tandps\t%s,%s\n", 
	                                archfregs[-FREGBEG-op3],
	                                archfregs[-FREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfabss\t%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfabs\t%s,%s\n", 
	       archdregs[-FREGBEG-op1], archdregs[-FREGBEG-op2]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   fcc = %s - %s;\n",
                archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2]);
         #endif
         break;
      case FABSD:
         #ifdef X86
	    assert(DTabsds);
            #ifdef AVX
               if (op3 >= 0) /* need to check the usage */
                  ap->next = PrintAssln("\tvandpd\t%s,%s,%s\n",
                                        GetDeref(SToff[DTabsds-1].sa[2]),
                                        archdregs[-DREGBEG-op2],
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvandpd\t%s,%s,%s\n",
                                        archdregs[-DREGBEG-op3],
                                        archdregs[-DREGBEG-op2],
	                                archdregs[-DREGBEG-op1]);
            #else
/*             Here assert is macro defined in fko.h */
               /*assert (op2 == op2)*/
               assert(op1==op2);
               if (op3 >= 0)
                  ap->next = PrintAssln("\tandpd\t%s,%s\n",
                                        GetDeref(SToff[DTabsds-1].sa[2]),
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tandpd\t%s,%s\n",
                                        archdregs[-DREGBEG-op3],
	                                archdregs[-DREGBEG-op1]);
               #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfabsd\t%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfabs\t%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = %s >= 0 ? %s : -%s;\n",
                archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2],
                archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op2]);
         #endif
         break;
      case FNEG:
         #ifdef X86
	    assert(DTnzeros);
            #ifdef AVX
               if (op1 == op2)
                  ap->next = PrintAssln("\tvxorps\t%s,%s,%s\n", 
                                        GetDeref(SToff[DTnzeros-1].sa[2]),
                                        archfregs[-FREGBEG-op1],
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvxorps\t%s,%s,%s\n", 
	                                archfregs[-FREGBEG-op2],
                                        archfregs[-FREGBEG-op1],
	                                archfregs[-FREGBEG-op1]);
            #else
               if (op1 == op2)
                  ap->next = PrintAssln("\txorps\t%s,%s\n", 
                                        GetDeref(SToff[DTnzeros-1].sa[2]),
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\txorps\t%s,%s\n", 
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfnegs\t%s,%s\n", 
	       archfregs[-FREGBEG-op2], archfregs[-FREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfneg\t%s,%s\n", 
	       archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = -%s;\n",
                archfregs[-FREGBEG-op1], archfregs[-FREGBEG-op2]);
         #endif
         break;
      case FNEGD:
         #ifdef X86
	    assert(DTnzerods);
            #ifdef AVX
	       if (op1 == op2)
                  ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                        GetDeref(SToff[DTnzerods-1].sa[2]),
                                        archdregs[-DREGBEG-op1],
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                        archdregs[-DREGBEG-op2],
                                        archdregs[-DREGBEG-op1],
                                        archdregs[-DREGBEG-op1]);
            #else
	       if (op1 == op2)
                  ap->next = PrintAssln("\txorpd\t%s,%s\n", 
                                        GetDeref(SToff[DTnzerods-1].sa[2]),
	                                archdregs[-DREGBEG-op1]);
               else
                   ap->next = PrintAssln("\txorpd\t%s,%s\n", 
                                         archdregs[-DREGBEG-op2],
                                         archdregs[-DREGBEG-op1]);
            #endif
         #elif defined(SPARC)
            ap->next = PrintAssln("\tfnegd\t%s,%s\n", 
	       archdregs[-DREGBEG-op2], archdregs[-DREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tfneg\t%s,%s\n", 
	       archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = -%s;\n",
                archdregs[-DREGBEG-op1], archdregs[-DREGBEG-op2]);
         #endif
         break;
      case CMPFLAG:
         switch (op1)
         {
         case CF_REGRESTORE:
            sptr = "REGISTER RESTORE FLAG";
            break;
         case CF_REGSAVE:
            sptr = "REGISTER SAVE FLAG";
            break;
         case CF_LOOP_INIT:
            sptr = "LOOP INIT FLAG";
            break;
         case CF_LOOP_BODY:
            sptr = "LOOP BODY FLAG";
            break;
         case CF_LOOP_UPDATE:
            sptr = "LOOP UPDATE FLAG";
            break;
         case CF_LOOP_END:
            sptr = "LOOP END FLAG";
            break;
         case CF_LOOP_PTRUPDATE:
            sptr = "LOOP PTR UPDATE FLAG";
            break;
         case CF_VRED_END:
            sptr = "VECTOR REDUCTION END";
            break;
         default:
            sptr = "UNKNOWN COMPFLAG";
         }
         #ifdef X86
            /*continue;*/ /*just to skip the FLAG comments*/ 
	    ap->next = PrintAssln("# CMPFLAG %d %d %d; %s\n",
                                  op1, op2, op3, sptr);
         #elif defined(SPARC)
	    ap->next = PrintAssln("! CMPFLAG %d %d %d; %s\n",
                                  op1, op2, op3, sptr);
         #elif defined(PPC)
	       ap->next = PrintAssln("# CMPFLAG %d %d %d; %s\n",
                                     op1, op2, op3, sptr);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("/* CMPFLAG %d %d %d; %s */\n",
                                  op1, op2, op3, sptr);
         #endif
         break;
      case FMOV:
/*
 *       Handle fp constant moves seperately
 */
         if (op2 > 0)
	 {
	    assert(IS_GLOB(STflag[op2-1]));
	    #ifdef X86
               #ifdef AVX
	          ap->next = PrintAssln("\tvmovss\t%s,%s\n", STname[op2-1],
	                                archxmmregs[-FREGBEG-op1]);
               #else 
	          ap->next = PrintAssln("\tmovss\t%s,%s\n", STname[op2-1],
	                                archfregs[-FREGBEG-op1]);
               #endif
	    #elif defined(SPARC)
	       ap->next = PrintAssln("\tsethi\t@hi(%s),%s\n", STname[op2-1],
	                             archiregs[-IREGBEG-op3]);
	       ap->next = PrintAssln("\tor\t%s, @lo(%s), %s\n", 
	                             archiregs[-IREGBEG-op3], STname[op2-1],
	                             archiregs[-IREGBEG-op3]);
	       ap->next = PrintAssln("\tld\t[%s], %s\n",
	                             archiregs[-IREGBEG-op3],
	                             archfregs[-FREGBEG-op1]);
            #elif defined(PPC)
	       ap->next = PrintAssln("\tlis\t%s, ha16(%s)\n", 
	                             archiregs[-IREGBEG-op3], STname[op2-1]);
	       ap->next = PrintAssln("\tori\t%s, lo16(%s)\n", 
	                             archiregs[-IREGBEG-op3], STname[op2-1]);
               ap->next = PrintAssln("\tlfd\t%s,%s\n",
		                     archfregs[-FREGBEG-op1],
				     archiregs[-IREGBEG-op3]);
            #elif defined(FKO_ANSIC)
               ap->next = PrintAssln("   %s = %s\n", archfregs[-FREGBEG-op1],
                                     STname[op2-1]);
	    #endif
	 }
	 else
	 {
            #ifdef X86
	       sptr = archfregs[-FREGBEG-op1];
/*
 *             If we are moving to x87 register, must go through memory
 */
	       if (sptr[1] == 's' && sptr[2] == 't')
               {
                  assert(DTx87);
                  #ifdef AVX
                     ap->next = PrintAssln("\tvmovss\t%s,%s\n",
                                           archxmmregs[-FREGBEG-op2],
                                           GetDeref(SToff[DTx87-1].sa[2]));
                  #else
                     ap->next = PrintAssln("\tmovss\t%s,%s\n",
                                           archfregs[-FREGBEG-op2],
                                           GetDeref(SToff[DTx87-1].sa[2]));
                  #endif
                  ap = ap->next;
                  ap->next = PrintAssln("\tfld\t%s\n",
                                        GetDeref(SToff[DTx87-1].sa[2]));
               }
	       else
                  #ifdef AVX
/*                   vmovss xmm1,xmm2,xmm3 #merge xmm1 and xmm2 into xmm3 */
                     ap->next = PrintAssln("\tvmovss\t%s,%s,%s\n",
	                                   archxmmregs[-FREGBEG-op2],
	                                   archxmmregs[-FREGBEG-op1],
	                                   archxmmregs[-FREGBEG-op1]);
                  #else
                     ap->next = PrintAssln("\tmovss\t%s,%s\n",
	                                   archfregs[-FREGBEG-op2], sptr);
                  #endif
            #elif defined(SPARC)
               ap->next = PrintAssln("\tfmovs\t%s,%s\n",archfregs[-FREGBEG-op2],
                                     archfregs[-FREGBEG-op1]);
            #elif defined(PPC)
               ap->next = PrintAssln("\tfmr\t%s,%s\n", archfregs[-FREGBEG-op1],
                                     archfregs[-FREGBEG-op2]);
            #elif defined(FKO_ANSIC)
               ap->next = PrintAssln("   %s = %s\n", archfregs[-FREGBEG-op1],
                                     archdregs[-FREGBEG-op2]);
            #endif
            }
	 break;
      case FMOVD:
/*
 *       Handle fp constant moves seperately
 */
         if (op2 > 0)
	 {
	    assert(IS_GLOB(STflag[op2-1]));
	    #ifdef X86
               #ifdef AVX /*need to check the usage!... using vmovsd */
	          ap->next = PrintAssln("\tvmovsd\t%s,%s\n", STname[op2-1],
	                                archxmmregs[-DREGBEG-op1]);
               #else
	          ap->next = PrintAssln("\tmovlpd\t%s,%s\n", STname[op2-1],
	                                archdregs[-DREGBEG-op1]);
               #endif
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
            #elif defined(FKO_ANSIC)
               ap->next = PrintAssln("   %s = %s\n", archdregs[-DREGBEG-op1],
                                     STname[op2-1]);
	    #endif
	 }
	 else
	 {
            #ifdef X86
	       sptr = archdregs[-DREGBEG-op1];
	       if (sptr[1] == 's' && sptr[2] == 't')
               {
                  assert(DTx87d);
                  #ifdef AVX
                     ap->next = PrintAssln("\tvmovsd\t%s,%s\n",
                                           archxmmregs[-DREGBEG-op2],
                                           GetDeref(SToff[DTx87d-1].sa[2]));
                  #else
                     ap->next = PrintAssln("\tmovsd\t%s,%s\n",
                                           archdregs[-DREGBEG-op2],
                                           GetDeref(SToff[DTx87d-1].sa[2]));
                  #endif
                  ap = ap->next;
                  ap->next = PrintAssln("\tfldl\t%s\n",
                                        GetDeref(SToff[DTx87d-1].sa[2]));
               }
	       else
                  #ifdef AVX
/*                   vmovsd xmm1,xmm2,xmm3 # for regs move, 3 operands */
                     ap->next = PrintAssln("\tvmovsd\t%s,%s,%s\n",
	                                   archxmmregs[-DREGBEG-op2], 
	                                   archxmmregs[-DREGBEG-op1], 
                                           archxmmregs[-DREGBEG-op1]);
                  #else
                     ap->next = PrintAssln("\tmovsd\t%s,%s\n",
	                                   archdregs[-DREGBEG-op2], sptr);     
                  #endif
            #elif defined(SPARC)
               ap->next = PrintAssln("\tfmovd\t%s,%s\n",archdregs[-DREGBEG-op2],
                                     archdregs[-DREGBEG-op1]);
            #elif defined(PPC)
               ap->next = PrintAssln("\tfmr\t%s,%s\n", archdregs[-DREGBEG-op1],
                                     archdregs[-DREGBEG-op2]);
            #elif defined(FKO_ANSIC)
               ap->next = PrintAssln("   %s = %s\n", archdregs[-DREGBEG-op1],
                                     archdregs[-DREGBEG-op2]);
            #endif
            }
	 break;
      case COMMENT:
         #ifdef X86
             continue;  /* skip comments temporary for testing with ATLAS*/
            ap->next = PrintAssln("#%s\n", op1 ? STname[op1-1] : "");
         #elif defined(SPARC)
            ap->next = PrintAssln("!%s\n", op1 ? STname[op1-1] : "");
	 #elif defined(PPC)
            ap->next = PrintAssln("#%s\n", op1 ? STname[op1-1] : "");
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("/* %s */\n", op1 ? STname[op1-1] : "");
         #endif
         break;
#ifdef X86
/*       
 *    Majedul: vpinsrw works on xmm register. 
 */        
      case VGR2VR16: /* pinsrw and vpinsrw works for xmms regs */
         op1 = -op1;
         if (op1 >= VDREGBEG && op1 < VDREGEND)
            op1 = op1 - VDREGBEG + DREGBEG;
         else if (op1 >= VFREGBEG && op1 < VFREGEND)
            op1 = op1 - VFREGBEG + DREGBEG;
         else if (op1 >= FREGBEG && op1 < FREGEND)
            op1 = op1 - FREGBEG + DREGBEG;
         op1 = -op1;
         #ifdef AVX
            /*ap->next = PrintAssln("\tpinsrw\t%s,%s,%s\n", GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op2],
                                  archxmmregs[-DREGBEG-op1]);*/
            ap->next = PrintAssln("\tvpinsrw\t%s,%s,%s,%s\n", 
                                  GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op2],
                                  archxmmregs[-DREGBEG-op1],
                                  archxmmregs[-DREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tpinsrw\t%s,%s,%s\n", GetIregOrConst(op3),
                               archiregs[-IREGBEG-op2],archdregs[-DREGBEG-op1]);
         #endif
         break;
#endif
/*
 * Only x86 has double precision SIMD inst
 */
   #ifdef X86
      case VDLD:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovapd\t%s,%s\n", GetDeref(op2),
                                  archvdregs[-VDREGBEG-op1]);       
         #else
            ap->next = PrintAssln("\tmovapd\t%s,%s\n", GetDeref(op2),
                                  archvdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDLDS:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovsd\t%s,%s\n", GetDeref(op2),
                                  archxmmregs[-VDREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovsd\t%s,%s\n", GetDeref(op2),
                                  archvdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDLDL:  /* NOTE: dest is also source */
         #ifdef AVX
/*          Not defined in AVX  */
            ap->next = PrintAssln("ERROR:\t%d %d %d %d\n", 
                                  ip->inst[0], op1, op2, op3);
            fko_warn(__LINE__, "Need to redefine for AVX when necessary");
         #else
            ap->next = PrintAssln("\tmovlpd\t%s, %s\n", GetDeref(op2),
                                  archvdregs[-VDREGBEG-op1]);  
         #endif
         break;
      case VDLDH:  /* NOTE: dest is also source */
         #ifdef AVX
/*          Not defined in AVX */
            ap->next = PrintAssln("ERROR:\t%d %d %d %d\n", 
                                  ip->inst[0], op1, op2, op3);
            fko_warn(__LINE__, "Need to redefine for AVX");
         #else
            ap->next = PrintAssln("\tmovhpd\t%s, %s\n", GetDeref(op2),
                                  archvdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDSTNT:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovntpd\t%s,%s\n", 
                                  archvdregs[-VDREGBEG-op2],
                                  GetDeref(op1));

         #else
            ap->next = PrintAssln("\tmovntpd\t%s, %s\n", 
                                  archvdregs[-VDREGBEG-op2],
                                  GetDeref(op1));
         #endif
         break;
      case VDST:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovapd\t%s, %s\n", 
                                  archvdregs[-VDREGBEG-op2],
                                  GetDeref(op1));
         #else
            ap->next = PrintAssln("\tmovapd\t%s, %s\n", 
                                  archvdregs[-VDREGBEG-op2],
                                  GetDeref(op1));
         #endif   
         break;
      case VDSTS: 
         #ifdef AVX 
            ap->next = PrintAssln("\tvmovsd\t%s, %s\n", 
                                  archxmmregs[-VDREGBEG-op2],
                                  GetDeref(op1));
         #else
            ap->next = PrintAssln("\tmovlpd\t%s, %s\n", 
                                  archvdregs[-VDREGBEG-op2],
                                  GetDeref(op1));
         #endif
         break;
      case VDMOV:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovapd\t%s, %s\n", 
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovapd\t%s, %s\n", 
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDMOVS:
/*
 *       Allow vector-to-fp regs for reg asg opt
 */
         assert (op1 < 0 && op2 < 0);
         op1 = -op1;
         op2 = -op2;
         if (op1 >= DREGBEG && op1 < DREGEND)
            op1 = op1 - DREGBEG + VDREGBEG;
         if (op2 >= DREGBEG && op2 < DREGEND)
            op2 = op2 - DREGBEG + VDREGBEG;
         op1 = -op1;
         op2 = -op2;
         
/*         if (op1 != op2)
            #ifdef AVX
               sptr = "vmovsd";
               
            #else
               sptr = "movsd";
            #endif
         else
            #ifdef AVX
               sptr = "vmovapd";
            #else
               sptr = "movapd";
            #endif
*/  
         if (op1 != op2)
            #ifdef AVX
             ap->next = PrintAssln("\tvmovsd\t%s, %s,%s\n", 
                                   archxmmregs[-VDREGBEG-op2],
                                   archxmmregs[-VDREGBEG-op1],
                                   archxmmregs[-VDREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tmovsd\t%s, %s\n",  
                                     archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            #endif
         else
            #ifdef AVX
               ap->next = PrintAssln("\tvmovapd\t%s, %s\n", 
                                     archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tmovapd\t%s, %s\n", 
                                     archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            #endif

         break;
      case VDMUL:
         #ifdef AVX
            ap->next = PrintAssln("\tvmulpd\t%s, %s, %s\n", 
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tmulpd\t%s, %s\n", GetDregOrDeref(op3),
                               archvdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDMAC:
         #ifdef X86
            #if defined(ArchHasMAC) && defined(FMA4) 
               assert( (op1 < 0) && (op2 < 0));
               if (op3 < 0) /*FMA4: only src2(here op3) can be mem */
                  ap->next = PrintAssln("\tvfmaddpd\t%s,%s,%s,%s\n", 
	                                archdregs[-VDREGBEG-op1],
                                        archdregs[GetDregID(op3)],
	                                archdregs[-VDREGBEG-op2],
	                                archdregs[-VDREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvfmaddpd\t%s,%s,%s,%s\n", 
	                                archdregs[-VDREGBEG-op1],
                                        GetDregOrDeref(op3),
	                                archdregs[-VDREGBEG-op2],
	                                archdregs[-VDREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "VDMAC Not found in this x86 arch!");
            #endif
         #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "VDMAC Not supported in this arch!");
         #endif
         break;
      case VDADD:
         #ifdef AVX
            ap->next = PrintAssln("\tvaddpd\t%s, %s, %s\n", 
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op1], 
                                  archvdregs[-VDREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\taddpd\t%s, %s\n", GetDregOrDeref(op3),
                               archvdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDABS:
         #ifdef AVX
/*         
 *          Assuming required const 0x7FFFFFFFFFFFFFFFF is loaded on all the
 *          the doubles in source. check VGR2VR16,VsConstGen,FpConstStore
 */        
	    assert(DTabsd);
            if (op3 >= 0)
               ap->next = PrintAssln("\tvandpd\t%s, %s, %s\n",
                                     GetDeref(SToff[DTabsd-1].sa[2]),
	                             archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvandpd\t%s, %s, %s\n",
	                             archvdregs[-VDREGBEG-op3],
	                             archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
         #else
	    assert(op1 == op2);
	    assert(DTabsd);
            if (op3 >= 0)
               ap->next = PrintAssln("\tandpd\t%s,%s\n",
                                     GetDeref(SToff[DTabsd-1].sa[2]),
	                             archvdregs[-VDREGBEG-op1]);
            else
               ap->next = PrintAssln("\tandpd\t%s,%s\n",
	                             archvdregs[-VDREGBEG-op3],
	                             archvdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDZERO:
         #ifdef AVX
            ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                  archvdregs[-VDREGBEG-op1],
                                  archvdregs[-VDREGBEG-op1],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            ap->next = PrintAssln("\txorpd\t%s,%s\n", archvdregs[-VDREGBEG-op1],
                                  archvdregs[-VDREGBEG-op1]);
         #endif
         break;
/*
 * NOTE: can use PSHUFD for case where dest is output only
 */
      case VDSHUF:
         #ifdef AVX
            cp = imap2cmap(SToff[op3-1].i);
/*
 *          Generalized combination :
 *          cp represents the position of destination:
 *          cp3,cp2,cp1,cp0   rd[3,2,1,0]
 *          value of cp can be 0~7: 0~3 represents double of rd; 
 *          4~7 represents double of rs.
 *          Right now we need following combination:
 *             0x3276  => rd[127:0] = rs[255:128], rd[255:128]=rd[255:128]
 *             0x3215  => rd[63:0] = rs[127:64], rd[255:64]=rd[255:64]
 *             0x0000  => rd[255:192,191:128,127:64,63:0] = rd[63:0] 
 *             0x2200  => rd[127:64,63:0]=rd[63:0]; 
 *                        rd[255:192,191:128]=rd[191:128] 
 */          
            if (cp[3]==0 && cp[2]==0 &&cp[1]==0 && cp[0]==0) 
            {
               assert(op1 == op2);
/*           
 *             need 2 instructions for that:
 *                vperm2f128 0,op1,op1,op1
 *                vshufpd 0,op1,op1,op1
 *             Note: vmovddup op1,op1 also can be used instead of vshufpd   
 */           
               ap->next = PrintAssln("\tvperm2f128\t$%d,%s,%s,%s\n", 0,
                                     archvdregs[-VDREGBEG-op1], 
                                     archvdregs[-VDREGBEG-op1], 
                                     archvdregs[-VDREGBEG-op1]);
               ap=ap->next;
               ap->next = PrintAssln("\tvshufpd\t$%d,%s,%s,%s\n", 0,
                                     archvdregs[-VDREGBEG-op1], 
                                     archvdregs[-VDREGBEG-op1], 
                                     archvdregs[-VDREGBEG-op1]);
            }  
            else if (cp[3] == 2 && cp[2] == 2 && cp[1] == 0 && cp[0] ==0)
            {
               assert(op1==op2);
               ap->next = PrintAssln("\tvmovddup\t%s,%s\n",
                                     archvdregs[-VDREGBEG-op1], 
                                     archvdregs[-VDREGBEG-op1]);
            }
            else if (cp[3] == 3 && cp[2] == 2 && cp[1] == 7 && cp[0] == 6)
               ap->next = PrintAssln("\tvperm2f128\t$0x31,%s,%s,%s\n",
                                     archvdregs[-VDREGBEG-op1], /* src2*/ 
                                     archvdregs[-VDREGBEG-op2], 
                                     archvdregs[-VDREGBEG-op1]);
            else if (cp[3] == 3 && cp[2] == 2 && cp[1] == 1 && cp[0] == 5)
            {
               ap->next = PrintAssln("ERROR:\t%d %d %d %d\n", 
                                     ip->inst[0], op1, op2, op3);
               fko_warn(__LINE__, "Not implemented this VDSHUF for AVX yet!");         
            }
            else if (cp[3] == 3 && cp[2] == 7 && cp[1] == 1 && cp[0] == 5)
               /*ap->next = PrintAssln("\tvshufpd\t$0x0F,%s,%s,%s\n",
                                       archvdregs[-VDREGBEG-op1],  
                                       archvdregs[-VDREGBEG-op2], 
                                       archvdregs[-VDREGBEG-op1]);
               */
               ap->next = PrintAssln("\tvunpckhpd\t%s,%s,%s\n", 
                                     archvdregs[-VDREGBEG-op1],
                                     archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            else if (cp[3] == 6 && cp[2] == 2 && cp[1] == 4 && cp[0] == 0)
               ap->next = PrintAssln("\tvunpcklpd\t%s,%s,%s\n", 
                                     archvdregs[-VDREGBEG-op2], /* src2 */
                                     archvdregs[-VDREGBEG-op1],
                                     archvdregs[-VDREGBEG-op1]);
            else 
            {
               ap->next = PrintAssln("ERROR:\t%d %d %d %d\n", 
                                     ip->inst[0], op1, op2, op3);
               fko_warn(__LINE__, "Not implemented the VDSHUF for AVX yet!");    
            }

         #else

         cp = imap2cmap(SToff[op3-1].i);
         if (cp[0] == 0 && cp[1] == 2)
            ap->next = PrintAssln("\tunpcklpd\t%s,%s\n", 
               archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         else if (cp[0] == 0 && cp[1] == 1)
            fko_warn(__LINE__, "Useless VDSHUF");
         else if (cp[0] == 2 && cp[1] == 3)
            ap->next = PrintAssln("\tmovapd\t%s,%s\n", 
               archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         else if (cp[0] == 1 && cp[1] == 3)
            ap->next = PrintAssln("\tunpckhpd\t%s,%s\n", 
               archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         else if (cp[0] < 2 && cp[1] > 1 && cp[1] < 4) /* shufpd */
         {
            i = cp[0];
            if (cp[1] == 3)
               i |= 2;
            ap->next = PrintAssln("\tshufpd\t$%d,%s,%s\n", i,
               archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         }
         else
         {
            if (op1 == op2)  /* any shuffling is possible if both regs same */
            {
               for (i=0; i < 2; i++)
                  if (cp[i] > 1) cp[i] -= 2;
               if (cp[0] == 0)
               {
                  if (cp[1] == 0)
                     ap->next = PrintAssln("\tunpcklpd\t%s,%s\n", 
                        archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
                  else
                     fko_warn(__LINE__, "Useless VDSHUF");
               }
               else /* cp[0] == 1 */
               {
                  if (cp[1] == 0)  /* cp[0] == 1 && cp[1] == 0 */
                     ap->next = PrintAssln("\tshufpd\t$%d,%s,%s\n", 1,
                        archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);

                  else /* cp[0] == 1 && cp[1] == 1 */
                     ap->next = PrintAssln("\tunpckhpd\t%s,%s\n", 
                        archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);

               }
            }
/*
 *          Can use pshufd if all ops come from source
 */
            else if (cp[0] > 1 && cp[1] > 1)
            {
               if (cp[0] == 2)
                  j = 0x2;
               else if (cp[0] = 3)
                  j = 0xE;
               if (cp[1] == 2)
                  j |= 0x00;
               else if (cp[1] == 3)
                  j |= 0xE0;
               ap->next = PrintAssln("\tpshufd\t$0x%x,%s,%s\n",
                  j, archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
            }
            else
               fko_error(__LINE__, "No such shuffle inst, imap=%d,%d!\n",
                         cp[0], cp[1]);
         }
         #endif
         break;
   #endif
/*
 * Only x86 and PowerPC have single prec vector instructions
 */
   #if defined(X86)
      case VFLD:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovaps\t%s, %s\n", GetDeref(op2),
                                  archvfregs[-VFREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovaps\t%s, %s\n", GetDeref(op2),
                                  archvfregs[-VFREGBEG-op1]);
         #endif
         break;
      case VFLDS:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovss\t%s, %s\n", GetDeref(op2),
                                  archxmmregs[-VFREGBEG-op1]); 
         #else
            ap->next = PrintAssln("\tmovss\t%s, %s\n", GetDeref(op2),
                                  archvfregs[-VFREGBEG-op1]);
         #endif
         break;
      case VFLDL:  /* NOTE: dest is also source */
         #ifdef AVX
            ap->next = PrintAssln("ERROR:\t%d %d %d %d\n", 
                                  ip->inst[0], op1, op2, op3);
            fko_error(__LINE__, "VFLDL cann't be used in AVX right now");
         #else
            ap->next = PrintAssln("\tmovlps\t%s, %s\n", GetDeref(op2),
                                  archvfregs[-VFREGBEG-op1]);
         #endif   
         break;
      case VFLDH:  /* NOTE: dest is also source */
         #ifdef AVX
            ap->next = PrintAssln("ERROR:\t%d %d %d %d\n", 
                                  ip->inst[0], op1, op2, op3);
            fko_error(__LINE__, "VFLDH cann't be used in AVX right now");
         #else
            ap->next = PrintAssln("\tmovhps\t%s, %s\n", GetDeref(op2),
                                  archvfregs[-VFREGBEG-op1]);
         #endif
         break;
      case VFSTNT:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovntps\t%s, %s\n", 
                                  archvfregs[-VFREGBEG-op2],
                                  GetDeref(op1));
         #else
            ap->next = PrintAssln("\tmovntps\t%s, %s\n", 
                                  archvfregs[-VFREGBEG-op2],
                                  GetDeref(op1));
         #endif
         break;
      case VFST:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovaps\t%s, %s\n", 
                                  archvfregs[-VFREGBEG-op2],
                                  GetDeref(op1));
         #else
            ap->next = PrintAssln("\tmovaps\t%s, %s\n", 
                                  archvfregs[-VFREGBEG-op2],
                                  GetDeref(op1));
         #endif
         break;
      case VFSTS:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovss\t%s, %s\n", 
                                  archxmmregs[-VFREGBEG-op2],
                                  GetDeref(op1));
         #else
            ap->next = PrintAssln("\tmovss\t%s, %s\n", 
                                  archvfregs[-VFREGBEG-op2],
                                  GetDeref(op1));
         #endif
         break;
      case VFMOV:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovaps\t%s, %s\n", 
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovaps\t%s, %s\n", 
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #endif
         break;
      case VFMOVS:
/*
 *       Allow vector-to-fp regs for reg asg opt
 */
         assert (op1 < 0 && op2 < 0);
         op1 = -op1;
         op2 = -op2;
         if (op1 >= FREGBEG && op1 < FREGEND)
            op1 = op1 - FREGBEG + VFREGBEG;
         if (op2 >= FREGBEG && op2 < FREGEND)
            op2 = op2 - FREGBEG + VFREGBEG;
         op1 = -op1;
         op2 = -op2;
/*         
         if (op1 != op2)
            #ifdef AVX
               sptr = "vmovss";
            #else
               sptr = "movss";
            #endif
         else
            #ifdef AVX
               sptr = "vmovaps";
            #else
               sptr = "movaps";
            #endif       
         ap->next = PrintAssln("\t%s\t%s, %s\n", sptr, 
                               archvfregs[-VFREGBEG-op2],
                               archvfregs[-VFREGBEG-op1]);
*/             
         if (op1 != op2)
            #ifdef AVX
               ap->next = PrintAssln("\tvmovss\t%s, %s,%s\n", 
                                     archxmmregs[-VFREGBEG-op2],
                                     archxmmregs[-VFREGBEG-op1],
                                     archxmmregs[-VFREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tmovss\t%s, %s\n",  
                                     archvdregs[-VFREGBEG-op2],
                                     archvdregs[-VFREGBEG-op1]);
            #endif
         else
            #ifdef AVX
               ap->next = PrintAssln("\tvmovaps\t%s, %s\n", 
                                     archvfregs[-VFREGBEG-op2],
                                     archvfregs[-VFREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tmovaps\t%s, %s\n", 
                                     archvfregs[-VFREGBEG-op2],
                                     archvfregs[-VFREGBEG-op1]);
            #endif
         break;
      case VFMUL:
         #ifdef AVX
            ap->next = PrintAssln("\tvmulps\t%s, %s, %s\n", 
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tmulps\t%s, %s\n", GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFMAC:
         #ifdef X86
            #if defined(ArchHasMAC) && defined(FMA4) 
               assert( (op1 < 0) && (op2 < 0));
               if (op3 < 0) /*FMA4: only src2(here op3) can be mem */
                  ap->next = PrintAssln("\tvfmaddps\t%s,%s,%s,%s\n", 
	                                archfregs[-VFREGBEG-op1],
                                        archfregs[GetDregID(op3)],
	                                archfregs[-VFREGBEG-op2],
	                                archfregs[-VFREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvfmaddps\t%s,%s,%s,%s\n", 
	                                archfregs[-VFREGBEG-op1],
                                        GetDregOrDeref(op3),
	                                archfregs[-VFREGBEG-op2],
	                                archfregs[-VFREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "VFMAC Not found in this x86 arch!");
            #endif
         #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "VFMAC Not supported in this arch!");
         #endif
         break;
      case VFADD:
         #ifdef AVX
            ap->next = PrintAssln("\tvaddps\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2], 
                                  archvfregs[-VFREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\taddps\t%s, %s\n", GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op1]);
         #endif
         break;
      case VFABS:
	 assert(DTabs);
         #ifdef AVX
            if (op3 >= 0)
               ap->next = PrintAssln("\tvandps\t%s,%s,%s\n",
                                     GetDeref(SToff[DTabs-1].sa[2]),
	                             archvfregs[-VFREGBEG-op2],
	                             archvfregs[-VFREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvandps\t%s,%s,%s\n",
	                             archvfregs[-VFREGBEG-op3],
	                             archvfregs[-VFREGBEG-op2],
	                             archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
            if (op3 >= 0)
               ap->next = PrintAssln("\tandps\t%s,%s\n",
                                     GetDeref(SToff[DTabs-1].sa[2]),
	                             archvfregs[-VFREGBEG-op1]);
            else
               ap->next = PrintAssln("\tandps\t%s,%s\n",
	                             archvfregs[-VFREGBEG-op3],
	                             archvfregs[-VFREGBEG-op1]);
         #endif
         break;
      case VFZERO:
         #ifdef AVX
            ap->next = PrintAssln("\tvxorps\t%s,%s, %s\n", 
                                  archvfregs[-VFREGBEG-op1],
                                  archvfregs[-VFREGBEG-op1],
                                  archvfregs[-VFREGBEG-op1]); 
         #else
            ap->next = PrintAssln("\txorps\t%s,%s\n", archvfregs[-VFREGBEG-op1],
                                  archvfregs[-VFREGBEG-op1]);
         #endif   
         break;
/*
 *    Majedul: adding new vector-compare instructions. they overwrite the 
 *    destination register.... I use 4 operand instruction here without 
 *    predicates ...
 *    NOTE: in avx implementation, the inst supports 4 operand. dest is diff
 *    than any other srcs. For SSE, as it supports 3 operands, need to change
 *    the LIL for it!
 */
      case VFCMPEQW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$0,%s,%s\n",  
                          archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPNEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$4,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$4,%s,%s\n",  
                          archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPLTW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$1,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$1,%s,%s\n",  
                          archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPLEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$2,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$2,%s,%s\n",  
                          archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPNLTW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$5,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$5,%s,%s\n",  
                          archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPNLEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$6,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$6,%s,%s\n",  
                          archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
/* 
 *    Majedul: Following vector-cmps are not supported in SSE, only 
 *    supported by AVX
 */         
      case VFCMPGTW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0E,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VFCMPGTW only supported in AVX!");
         #endif 
         break;
      case VFCMPGEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0D,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VFCMPGEW only supported in AVX!");
         #endif 
         break;
      case VFCMPNGTW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0A,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VFCMPNGTW only supported in AVX!");
         #endif 
         break;
      case VFCMPNGEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$9,%s,%s,%s\n",
                                  archvfregs[-VFREGBEG-op3],
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VFCMPNGEW only supported in AVX!");
         #endif 
         break;
/*
 *    Mov masks the sign bits of all floats to low 4/8 bits of ireg 
 */
      case VFSBTI:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovmskps\t%s,%s\n",
                                   archvfregs[-VFREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);      
         # else
            ap->next = PrintAssln("\tmovmskps\t%s,%s\n",
                                   archvfregs[-VFREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);
         #endif
         break;
      
      case VDCMPEQW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$0,%s,%s\n",  
                          archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPNEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$4,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$4,%s,%s\n",  
                          archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPLTW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$1,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$1,%s,%s\n",  
                          archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPLEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$2,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$2,%s,%s\n",  
                          archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPNLTW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$5,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$5,%s,%s\n",  
                          archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPNLEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$6,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tcmpps\t$6,%s,%s\n",  
                          archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
/* 
 *    Majedul: Dollowing vector-cmps are not supported in SSE, only 
 *    supported by AVX
 */         
      case VDCMPGTW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0E,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VDCMPGTW only supported in AVX!");
         #endif 
         break;
      case VDCMPGEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0D,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VDCMPGEW only supported in AVX!");
         #endif 
         break;
      case VDCMPNGTW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0A,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VDCMPNGTW only supported in AVX!");
         #endif 
         break;
      case VDCMPNGEW:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$9,%s,%s,%s\n",
                                  archvdregs[-VDREGBEG-op3],
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VDCMPNGEW only supported in AVX!");
         #endif 
         break;
/*
 *    Mov masks the sign bits of all doubles to low 2/4 bits of ireg 
 */
      case VDSBTI:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovmskps\t%s,%s\n",
                                   archvdregs[-VDREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);      
         # else
            ap->next = PrintAssln("\tmovmskps\t%s,%s\n",
                                   archvdregs[-VDREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);
         #endif
         break;

      case VFSHUF:
         cp = imap2cmap(SToff[op3-1].i); /* return hex char */ 
/*       
 *       Redefining cp for AVX
 *       cp represents the position of destination:
 *       cp7,cp6,cp5,cp4,cp3,cp2,cp1,cp0   rd[7,6,5,4,3,2,1,0]
 *       value of cp can be 0~15: 0~7 represents float of rd; 
 *       7~15 represents float of rs.
 *       Right now we need following combination:
 */      
         #ifdef AVX
            if (op1 == op2)
            {
               j=0;
               for (i=0; i<8; i++)
                  if(cp[i])
                  {
                     j=1;
                     break;
                  }
               if (!j)
               {
                  ap->next = PrintAssln("\tvperm2f128\t$%d,%s,%s,%s\n", 0,
                                        archvfregs[-VFREGBEG-op1], 
                                        archvfregs[-VFREGBEG-op1], 
                                        archvfregs[-VFREGBEG-op1]);
                  ap=ap->next;
                  ap->next = PrintAssln("\tvshufps\t$%d,%s,%s,%s\n", 0,
                                        archvfregs[-VFREGBEG-op1], 
                                        archvfregs[-VFREGBEG-op1], 
                                        archvfregs[-VFREGBEG-op1]);
               }
            }
            else
            {
/*           
 *            Implemented combination:
 *                7654FEDC
 *                765432BA
 *                76CD3289
 */             
               if (cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4)    
                  if (cp[3] == 15 && cp[2] == 14 && cp[1] == 13 && cp[0] == 12)
                     ap->next = PrintAssln("\tvperm2f128\t$0x31,%s,%s,%s\n",
                                           archvfregs[-VFREGBEG-op1], /* src2*/ 
                                           archvfregs[-VFREGBEG-op2], 
                                           archvfregs[-VFREGBEG-op1]);
                  else if (cp[3] == 3 && cp[2] == 2 && cp[1] == 11 
                           && cp[0] == 10)  
                     ap->next = PrintAssln("\tvmovhlps\t%s,%s,%s\n",
                                           archxmmregs[-VFREGBEG-op2], 
                                           archxmmregs[-VFREGBEG-op1], 
                                           archxmmregs[-VFREGBEG-op1]);
                  else
                     fko_warn(__LINE__, "Not implemented this VFSHUF yet");
/*             Implementing 76CD3289*/
               else if (cp[7] == 7 && cp[6] == 6 && cp[3] == 3 && cp[2] == 2 &&
                        cp[5] == 12 && cp[4] == 13 && cp[1] == 8 && cp[0] == 9)
                  ap->next = PrintAssln("\tvshufps\t$0xE1,%s,%s,%s\n",
                                         archvfregs[-VFREGBEG-op1], /* src2*/
                                         archvfregs[-VFREGBEG-op2],
                                         archvfregs[-VFREGBEG-op1]);
               else
                fko_warn(__LINE__, "Not implemented this VFSHUF yet");
            }
         #else
            if (op1 == op2)  /* any shuffling legal if they are the same reg */
            {
               for (i=0; i < 4; i++)
                  if (cp[i] > 3) cp[i] -= 4;
               if (cp[0] == 0 && cp[1] == 0 && cp[2] == 1 && cp[3] == 1)
                  ap->next = PrintAssln("\tunpcklps\t%s,%s\n", 
                     archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
               else if (cp[0] == 2 && cp[1] == 2 && cp[2] == 3 && cp[3] == 3)
                  ap->next = PrintAssln("\tunpckhps\t%s,%s\n", 
                     archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
               else if (cp[0] == 0 && cp[1] == 1 && cp[2] == 0 && cp[3] == 1)
                  ap->next = PrintAssln("\tmovlhps\t%s,%s\n", 
                     archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
               else if (cp[0] == 2 && cp[1] == 3 && cp[2] == 2 && cp[3] == 3)
                  ap->next = PrintAssln("\tmovhlps\t%s,%s\n", 
                     archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
               else if (cp[0] == 0 && cp[1] == 1 && cp[2] == 2 && cp[3] == 3)
                  fko_warn(__LINE__, "Useless VFSHUF");
               else
               {
                  i = cp[0] | ((cp[1])<<2) | (cp[2]<<4) | (cp[3]<<6);
                  ap->next = PrintAssln("\tshufps\t$%d,%s,%s\n", i,
                     archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
               }
            }
            else if (cp[0] == 0 && cp[1] == 4 && cp[2] == 1 && cp[3] == 5)
               ap->next = PrintAssln("\tunpcklps\t%s,%s\n", 
                  archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
            else if (cp[0] == 2 && cp[1] == 6 && cp[2] == 3 && cp[3] == 7)
               ap->next = PrintAssln("\tunpckhps\t%s,%s\n", 
                  archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
            else if (cp[0] == 0 && cp[1] == 1 && cp[2] == 4 && cp[3] == 5)
               ap->next = PrintAssln("\tmovlhps\t%s,%s\n", 
                  archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
            else if (cp[0] == 6 && cp[1] == 7 && cp[2] == 2 && cp[3] == 3)
               ap->next = PrintAssln("\tmovhlps\t%s,%s\n", 
                  archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
            else if (cp[0] < 4 && cp[1] < 4 && cp[2] > 3 && cp[3] > 3)
            {
               i = cp[0] | ((cp[1])<<2) | ((cp[2]-4)<<4) | ((cp[3]-4)<<6);
               ap->next = PrintAssln("\tshufps\t$%d,%s,%s\n", i,
                  archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
            }
/*
 *          Can use pshufd if all ops come from source
 */
            else if (cp[0] > 3 && cp[1] > 3 && cp[2] > 3 && cp[3] > 3)
            {
               j = 0;
               for (i=0; i < 4; i++)
               {
                  if (cp[i] == 4)
                     j |= 0x0 << (i*2);
                  else if (cp[i] == 5)
                     j |= 0x1 << (i*2);
                  else if (cp[i] == 6)
                     j |= 0x2 << (i*2);
                  else 
                     j |= 0x3 << (i*2);
               }
               ap->next = PrintAssln("\tpshufd\t$0x%x,%s,%s\n",
                  j, archvfregs[-VFREGBEG-op2], archvfregs[-VFREGBEG-op1]);
            }
            else
               fko_error(__LINE__, "No such Sshuffle inst, imap=%d,%d,%d,%d!\n",
                         cp[0], cp[1],cp[2],cp[3]);
         #endif
         break;
   #endif
      case PREFR:
         #ifdef X86
            i = SToff[op3-1].i;
            if (i > 0)
               ap->next = PrintAssln("\tprefetcht%d\t%s\n",i, GetDeref(op2));
            else
            {
               if (FKO_FLAG & IFF_3DNOWR)
                  ap->next = PrintAssln("\tprefetch\t%s\n", GetDeref(op2));
               else if (FKO_FLAG & IFF_TAR)
                  ap->next = PrintAssln("\tprefetcht0\t%s\n", GetDeref(op2));
               else
                  ap->next = PrintAssln("\tprefetchnta\t%s\n", GetDeref(op2));
            }
         #endif
         break;
      case PREFW:
         #ifdef X86
            i = SToff[op3-1].i;
            if (i > 0)
               ap->next = PrintAssln("\tprefetcht%d\t%s\n",i, GetDeref(op2));
            else
            {
               if (FKO_FLAG & IFF_3DNOWW)
                  ap->next = PrintAssln("\tprefetchw\t%s\n", GetDeref(op2));
               else if (FKO_FLAG & IFF_TAW)
                  ap->next = PrintAssln("\tprefetcht0\t%s\n", GetDeref(op2));
               else
                  ap->next = PrintAssln("\tprefetchnta\t%s\n", GetDeref(op2));
            }
         #endif
         break;
/*
 *  HERE HERE HERE:
 */
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
 * These short inst not defined, since they are not used in preamble
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
/*      fprintf(stderr, "%s", ap->ln); */
      ap = ap->next;
   }
   }
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
