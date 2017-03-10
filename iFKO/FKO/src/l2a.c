/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
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
   short ptr, reg, mul; 
   int con;
   id--;
/*
 * NOTE: fpconst must be initialize in CONST_INIT before using in expression,
 * though there is no checking for that in parser. Using fpconst without 
 * initialized it before may result in id>0 but not IS_DEREF (rather const) here 
 * and will caught by this assertion.
 */
#if 1
   if (id < 0 || !IS_DEREF(STflag[id]))
   {
      PrintST(stderr);
      fprintf(stderr,"id=%d flag=%d\n", id, STflag[id] );
      assert (id >= 0 && IS_DEREF(STflag[id]));
   }
#else
   assert (id >= 0 && IS_DEREF(STflag[id]));
#endif   
   ptr = SToff[id].sa[0];
   reg = SToff[id].sa[1];
   mul = SToff[id].sa[2];
   con = SToff[id].sa[3];
   con = GetDTcon(con); /* it can be an index of DTcon table */
   if (reg > 0) reg = 0;
#if 0
   fprintf(stderr, "[%d, %d, %d, %d]\n",ptr,reg,mul,con);
   fflush(stderr);
#endif
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
#if 0   
   if (id < 0)
   {
      return(archsregs[-IREGBEG-id]);
   }
   else if (id)
      return(GetDeref(id));
#endif
   assert(id);
   if (id < 0)
      return(archsregs[-IREGBEG-id]);
   return(GetDeref(id));
}
#endif

static char *GetIregOrDeref(short id)
/*
 * given an id, return string containing register name if less than zero,
 * or a properly formated deref entry if greater than zero
 */
{
#if 0   
   if (id < 0)
      return(archiregs[-IREGBEG-id]);
   else if (id)
   {
      assert(IS_DEREF(STflag[id]));
      return(GetDeref(id));
   }
#endif
   assert(id);
   if (id < 0)
      return(archiregs[-IREGBEG-id]);
   assert(IS_DEREF(STflag[id]));
   return(GetDeref(id));
}

static int GetDregID(short id)
{
/*
 * given an reg id, return appropriate positive index, otherwise -1
 */
   if (id < 0)
   {
      #ifdef VIREGBEG
         if(-id >= VIREGBEG)
            return(-VIREGBEG-id);
      #endif
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
#if 0
   if (id < 0)
   {
      #ifdef VIREGBEG
         if(-id >= VIREGBEG)
            return(archviregs[-VIREGBEG-id]);
      #endif
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
#endif
   assert(id);
   if (id < 0)
   {
      #ifdef VIREGBEG
         if(-id >= VIREGBEG)
            return(archviregs[-VIREGBEG-id]);
      #endif
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
   short inst, op1, op2, op3;
   #ifdef SPARC
      int SeenSave=0;
   #endif
   #ifdef PPC
      int k;
   #endif
   int i, j;
   char *sptr;
   extern int DTabsd, DTnzerod, DTabs, DTnzero, DTx87, DTx87d;
   /*extern int DTabsds, DTnzerods, DTabss, DTnzeros;*/
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
	               ip->inst[0], ip->inst[1], ip->inst[2], ip->inst[3], 
                       ap->ln);
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
            /*fprintf(stderr, "op1=%d, op2=%d (%d,%d)\n", op1, op2,
                    iName2Reg("@eax"), iName2Reg("@edx"));*/

            assert(op1 == -iName2Reg("@eax") && op2 == -iName2Reg("@edx"));
            ap->next = PrintAssln("\tidiv %s\n", GetIregOrDeref(op3));
         #elif defined (X86_64)
            /*fprintf(stderr, "op1=%d, op2=%d (%d,%d)\n", op1, op2,
                    iName2Reg("@rax"), iName2Reg("@rdx"));*/
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
                  ap->next = PrintAssln("\tvcomisd\t%s,%s\n", 
                                        GetDregOrDeref(op3),
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
         ap->next = PrintAssln("\tnegl\t%s\n", GetIregOrDeref(op1));
         break;
   #endif
      case NEG:
         #ifdef X86_64
            assert(op1 == op2);
            ap->next = PrintAssln("\tnegq\t%s\n", GetIregOrDeref(op1));
         #elif defined(X86)
            assert(op1 == op2);
            ap->next = PrintAssln("\tnegl\t%s\n", GetIregOrDeref(op1));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tneg\t%s, %s\n", archiregs[-IREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tneg\t%s, %s\n", archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2]);
         #elif defined(FKO_ANSIC)
            ap->next = PrintAssln("   %s = -%s;\n", archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op1]);
         #endif
         break;
   #ifdef X86_64
      case LEAS2: LEAS4: LEAS8:
         fko_error(__LINE__, "Not implemented LEA for short!!");
         break;
   #endif
   #ifdef X86  
/*
 *    HERE HERE, we implement LEA for only following specific cases where we can
 *    reduce register usage using this special instruction:
 *       1) pA += incA;
 *       2) pA = pA0 + incA;
 *    NOTE: it's not a general implementation of lea in x86
 */
      case LEA2:
         assert(op1 < 0 && op2 < 0 && op3 < 0);
         ap->next = PrintAssln("\tlea\t(%s, %s, 2), %s\n", 
                                  archiregs[-IREGBEG-op2], 
                                  archiregs[-IREGBEG-op3], 
                                  archiregs[-IREGBEG-op1]);
         break;
      case LEA4:
         assert(op1 < 0 && op2 < 0 && op3 < 0);
         ap->next = PrintAssln("\tlea\t(%s, %s, 4), %s\n", 
                                  archiregs[-IREGBEG-op2], 
                                  archiregs[-IREGBEG-op3], 
                                  archiregs[-IREGBEG-op1]);
         break;
      case LEA8:
         assert(op1 < 0 && op2 < 0 && op3 < 0);
         ap->next = PrintAssln("\tlea\t(%s, %s, 8), %s\n", 
                                  archiregs[-IREGBEG-op2], 
                                  archiregs[-IREGBEG-op3], 
                                  archiregs[-IREGBEG-op1]);
         break;
   #endif
   #ifdef X86_64
      case CVTSI: /* convert 32bit to 64 bit int*/
/*
 *    Majedul: we are using  movslq for this 
 */
      #if 0         
         k = -iName2Reg("@rax");
         assert(op1 == k && op2 == k);
         ap->next = PrintAssln("\tcltq\n");
/*         ap->next = PrintAssln("\tcdqe\n"); */
      #else
         if (op1 < 0)
            ap->next = PrintAssln("\tmovslq\t%s, %s\n", 
                                  GetSregOrDeref(op2),  
                                  archiregs[-IREGBEG-op1]);
         else
            ap->next = PrintAssln("\tmovslq\t%s, %s\n", 
                                   archsregs[-IREGBEG-op2], 
                                   GetIregOrDeref(op1));  

      #endif
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

#if 0 
/*
 * NOTE: these two instructions are replaced with FCMPWXX and FCMPDWXX
 * see below.
 */
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
#endif         
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
         assert(op1<0 && op2<0);
         #ifdef X86
            #if defined(ArchHasMAC) && defined(FMA4)
               #ifdef AVX
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
               if (op3 < 0)   /* FMA4: only src2 can be mem */
                  ap->next = PrintAssln("\tvfmaddsd\t%s,%s,%s,%s\n", 
	                                archdregs[-DREGBEG-op1],
                                        archdregs[GetDregID(op3)],
	                                archdregs[-DREGBEG-op2],
                                        archdregs[-DREGBEG-op1]);
               else   
                  ap->next = PrintAssln("\tvfmaddsd\t%s,%s,%s,%s\n", 
	                                archdregs[-DREGBEG-op1],
                                        GetDregOrDeref(op3),
	                                archdregs[-DREGBEG-op2],
                                        archdregs[-DREGBEG-op1]);
               #endif
            #elif defined(ArchHasMAC) && defined(FMA3)
/*
 *          Note: we only implement FMA with op1 += op2 * op3 style 
 */
               #ifdef AVX
               if (op3 < 0)   /* FMA4: only src2 can be mem */
                  ap->next = PrintAssln("\tvfmadd231sd\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-DREGBEG-op2],
                                        archxmmregs[-DREGBEG-op1]);
               else   
                  ap->next = PrintAssln("\tvfmadd231sd\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-DREGBEG-op2],
                                        archxmmregs[-DREGBEG-op1]);
               #else
               if (op3 < 0)   /* FMA4: only src2 can be mem */
                  ap->next = PrintAssln("\tvfmadd231sd\t%s,%s,%s\n", 
                                        archdregs[GetDregID(op3)],
	                                archdregs[-DREGBEG-op2],
                                        archdregs[-DREGBEG-op1]);
               else   
                  ap->next = PrintAssln("\tvfmadd231sd\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archdregs[-DREGBEG-op2],
                                        archdregs[-DREGBEG-op1]);
               #endif
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
               #ifdef AVX
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
               if (op3 < 0) /*FMA4: only src2 can be used as mem */
                  ap->next = PrintAssln("\tvfmaddss\t%s,%s,%s,%s\n", 
	                                archfregs[-FREGBEG-op1],
                                        archfregs[GetDregID(op3)],
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvfmaddss\t%s,%s,%s,%s\n", 
	                                archfregs[-FREGBEG-op1],
                                        GetDregOrDeref(op3),
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
               #endif
            #elif defined(ArchHasMAC) && defined(FMA3)
               #ifdef AVX
               if (op3 < 0) 
                  ap->next = PrintAssln("\tvfmadd231ss\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvfmadd231ss\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
               #else
               if (op3 < 0) 
                  ap->next = PrintAssln("\tvfmadd231ss\t%s,%s,%s\n", 
                                        archfregs[GetDregID(op3)],
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvfmadd231ss\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
               #endif
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
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvaddss\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op2],
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
   #ifdef X86
/*
 *    hybrid inst: fr1 = fr2 & vfr3/mem
 */
      case VFSABS:
            #ifdef AVX
               if (op3 >= 0)
                  ap->next = PrintAssln("\tvandps\t%s,%s,%s\n", 
                                        /*GetDeref(SToff[DTabs-1].sa[2]),*/
                                        GetDeref(op3),
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvandps\t%s, %s, %s\n", 
	                                archvfregs[-VFREGBEG-op3],
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
            #else
               assert(op2 == op1);
               if (op3 >= 0)
                  ap->next = PrintAssln("\tandps\t%s,%s\n", 
                                        /*GetDeref(SToff[DTabs-1].sa[2]),*/
                                        GetDeref(op3),
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tandps\t%s,%s\n", 
	                                archvfregs[-VFREGBEG-op3],
	                                archfregs[-FREGBEG-op1]);
            #endif

         break;
   #endif
      case FABS:
         #ifdef X86
/*
 *          FIXED: FABS is not valid in x86 anymore, since we don't have 
 *          scalar 'and' operation in fp
 */
            fko_error(__LINE__, "we don't support FABS inst in x86 anymore\n");
            #if 0
            #ifdef AVX
               if (op3 >= 0)
                  ap->next = PrintAssln("\tvandps\t%s,%s,%s\n", 
                                        /*GetDeref(SToff[DTabss-1].sa[2]),*/
                                        GetDeref(op3),
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
                                        /*GetDeref(SToff[DTabss-1].sa[2]),*/
                                        GetDeref(op3),
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tandps\t%s,%s\n", 
	                                archfregs[-FREGBEG-op3],
	                                archfregs[-FREGBEG-op1]);
            #endif
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
   #ifdef X86
/*
 *    hybrid inst: fr1 = fr2 & vfr3/mem
 */
      case VDSABS:
            #ifdef AVX
               if (op3 >= 0) /* need to check the usage */
                  ap->next = PrintAssln("\tvandpd\t%s,%s,%s\n",
                                        /*GetDeref(SToff[DTabsd-1].sa[2]),*/
                                        GetDeref(op3),
                                        archdregs[-DREGBEG-op2],
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvandpd\t%s,%s,%s\n",
                                        archvdregs[-VDREGBEG-op3],
                                        archdregs[-DREGBEG-op2],
	                                archdregs[-DREGBEG-op1]);
            #else
               assert(op1==op2);
               if (op3 >= 0)
                  ap->next = PrintAssln("\tandpd\t%s,%s\n",
                                        /*GetDeref(SToff[DTabsd-1].sa[2]),*/
                                        GetDeref(op3),
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tandpd\t%s,%s\n",
                                        archvdregs[-VDREGBEG-op3],
	                                archdregs[-DREGBEG-op1]);
               #endif
         break;
   #endif
      case FABSD:
         #ifdef X86
            fko_error(__LINE__, "we don't support FABSD inst in x86 anymore\n");
            #if 0
            #ifdef AVX
               if (op3 >= 0) /* need to check the usage */
                  ap->next = PrintAssln("\tvandpd\t%s,%s,%s\n",
                                        /*GetDeref(SToff[DTabsds-1].sa[2]),*/
                                        GetDeref(op3),
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
                                        /*GetDeref(SToff[DTabsds-1].sa[2]),*/
                                        GetDeref(op3),
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tandpd\t%s,%s\n",
                                        archdregs[-DREGBEG-op3],
	                                archdregs[-DREGBEG-op1]);
            #endif
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
   #ifdef X86
/*
 *    hybrid inst: fr1 = fr2 xor vfr3/mem
 */
      case VFSNEG:
            #ifdef AVX
               if (op3 >= 0)
                  ap->next = PrintAssln("\tvxorps\t%s,%s,%s\n", 
                                        /*GetDeref(SToff[DTnzero-1].sa[2]),*/
                                        GetDeref(op3),
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvxorps\t%s,%s,%s\n", 
	                                archvfregs[-VFREGBEG-op3],
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
            #else
               assert(op2 == op1);
               if (op3 >= 0)
                  ap->next = PrintAssln("\txorps\t%s,%s\n", 
                                        /*GetDeref(SToff[DTnzero-1].sa[2]),*/
                                        GetDeref(op3),
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\txorps\t%s,%s\n", 
	                                archvfregs[-VFREGBEG-op3],
	                                archfregs[-FREGBEG-op1]);
            #endif
            break;
   #endif
      case FNEG:
         #ifdef X86
            fko_error(__LINE__, "we don't support FNEG inst in x86 anymore\n");
            #if 0
            #ifdef AVX
               if (op3 >= 0)
                  ap->next = PrintAssln("\tvxorps\t%s,%s,%s\n", 
                                        /*GetDeref(SToff[DTnzeros-1].sa[2]),*/
                                        GetDeref(op3),
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvxorps\t%s,%s,%s\n", 
	                                archfregs[-FREGBEG-op3],
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
            #else
               assert(op2 == op1);
               if (op3 >= 0)
                  ap->next = PrintAssln("\txorps\t%s,%s\n", 
                                        /*GetDeref(SToff[DTnzeros-1].sa[2]),*/
                                        GetDeref(op3),
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\txorps\t%s,%s\n", 
	                                archfregs[-FREGBEG-op3],
	                                archfregs[-FREGBEG-op1]);
            #endif
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
   #ifdef X86
/*
 *    hybrid inst: fr1 = fr2 xor vfr3/mem
 */
      case VDSNEG:
            #ifdef AVX
               if (op3 >= 0) 
                  ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                        /*GetDeref(SToff[DTnzerod-1].sa[2]),*/
                                        GetDeref(op3),
                                        archdregs[-DREGBEG-op2],
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                        archvdregs[-VDREGBEG-op3],
                                        archdregs[-DREGBEG-op2],
	                                archdregs[-DREGBEG-op1]);
            #else
               assert(op1==op2);
               if (op3 >= 0)
                  ap->next = PrintAssln("\txorpd\t%s,%s\n", 
                                        /*GetDeref(SToff[DTnzerod-1].sa[2]),*/
                                        GetDeref(op3),
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\txorpd\t%s,%s\n", 
                                        archvdregs[-VDREGBEG-op3],
	                                archdregs[-DREGBEG-op1]);
            #endif
         break;
   #endif
      case FNEGD:
         #ifdef X86
            fko_error(__LINE__, "we don't support FNEG inst in x86 anymore\n");
            #if 0
            #ifdef AVX
               if (op3 >= 0) 
                  ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                        //GetDeref(SToff[DTnzerods-1].sa[2]),
                                        GetDeref(op3),
                                        archdregs[-DREGBEG-op2],
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                        archdregs[-DREGBEG-op3],
                                        archdregs[-DREGBEG-op2],
	                                archdregs[-DREGBEG-op1]);
            #else
               assert(op1==op2);
               if (op3 >= 0)
                  ap->next = PrintAssln("\txorpd\t%s,%s\n", 
                                        //GetDeref(SToff[DTnzerods-1].sa[2]),
                                        GetDeref(op3),
	                                archdregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\txorpd\t%s,%s\n", 
                                        archdregs[-DREGBEG-op3],
	                                archdregs[-DREGBEG-op1]);
            #endif
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
/*
 *    Majedul: will introduce synthetic instruction for other architecture 
 *    later
 */
      case FMAXD:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvmaxsd\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-DREGBEG-op2],
	                                archxmmregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvmaxsd\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-DREGBEG-op2],
	                                archxmmregs[-DREGBEG-op1]);
            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tmaxsd\t%s,%s\n", GetDregOrDeref(op3),
	                          archdregs[-DREGBEG-op1]);
            #endif
         #endif
         break;
      case FMIND:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvminsd\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-DREGBEG-op2],
	                                archxmmregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvminsd\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-DREGBEG-op2],
	                                archxmmregs[-DREGBEG-op1]);
            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tminsd\t%s,%s\n", GetDregOrDeref(op3),
	                          archdregs[-DREGBEG-op1]);
            #endif
         #endif
         break;
      case FMAX:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvmaxss\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvmaxss\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);

            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tmaxss\t%s,%s\n", GetDregOrDeref(op3),
	                          archfregs[-FREGBEG-op1]);
            #endif 
         #endif
         break;
      case FMIN:
         #ifdef X86
            #ifdef AVX
               if (op3 < 0)
                  ap->next = PrintAssln("\tvminss\t%s,%s,%s\n", 
                                        archxmmregs[GetDregID(op3)],
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvminss\t%s,%s,%s\n", 
                                        GetDregOrDeref(op3),
	                                archxmmregs[-FREGBEG-op2],
	                                archxmmregs[-FREGBEG-op1]);

            #else
	       assert (op1 == op2);
               ap->next = PrintAssln("\tminss\t%s,%s\n", GetDregOrDeref(op3),
	                          archfregs[-FREGBEG-op1]);
            #endif 
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
         case CF_SCAL_RES:
            sptr = "SCALAR RESTART START";
            break;
         case CF_SSV_VUPDATE:
            sptr = "SSV VECTOR UPDATE";
            break;
         case CF_SSV_VRECOVERY:
            sptr = "SSV VECTOR RECOVERY";
            break;
         default:
            sptr = "UNKNOWN COMPFLAG";
         }
         #ifdef X86
            continue; /*just to skip the FLAG comments*/ 
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
/*
 *          NOTE: vmovss for reg to reg move seems extremly slow!!!!                
 */
#if 0                  
                     ap->next = PrintAssln("\tvmovss\t%s,%s,%s\n",
	                                   archxmmregs[-FREGBEG-op2],
	                                   archxmmregs[-FREGBEG-op1],
	                                   archxmmregs[-FREGBEG-op1]);
#else
                     ap->next = PrintAssln("\tvmovaps\t%s,%s\n",
	                                   archfregs[-FREGBEG-op2],
	                                   archfregs[-FREGBEG-op1]);
#endif
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
#if 0                  
                     ap->next = PrintAssln("\tvmovsd\t%s,%s,%s\n",
	                                   archxmmregs[-DREGBEG-op2], 
	                                   archxmmregs[-DREGBEG-op1], 
                                           archxmmregs[-DREGBEG-op1]);
#else
                     ap->next = PrintAssln("\tvmovapd\t%s,%s\n",
	                                   archdregs[-DREGBEG-op2], 
                                           archdregs[-DREGBEG-op1]);
#endif
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
#ifdef X86         
/*
 *    CMOV1 and CMOV2
 *    Right Now, We will consider only X86 arch for this conditional move
 *    later, we will implment the select for AltiVec.
 *    NOTE: AVX/SSE doesn't have scalar blend. So, we use vector blend 
 *
 *    NOTE: 
 *         convention: 
 *         dest = mask==0? scr1: src2
 *         dest = mask? src2: src1
 *         if mask is zero, then dest is set from src1; otherwise from scr2
 *
 *   LIL:
 *        FCMOV1 means dest is alias with SRC1
 *        FCMOV2 means dest is alias with SRC2
 */
      case FCMOV1:
            #if defined(AVX) 
               assert( (op1 < 0) && (op3 < 0));
               if (op2 < 0) /* only src2(here op2) can be mem */
                  ap->next = PrintAssln("\tvblendvps\t%s,%s,%s,%s\n", 
	                                archfregs[-FREGBEG-op3],
                                        archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1],
	                                archfregs[-FREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvblendvps\t%s,%s,%s,%s\n", 
	                                archfregs[-FREGBEG-op3],
                                        GetDregOrDeref(op2),
	                                archfregs[-FREGBEG-op1],
	                                archfregs[-FREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV is Not found in this x86 arch!");
            #endif
         break;
      case FCMOV2:
            #if defined(AVX) 
               assert( (op1 < 0) && (op3 < 0) && (op2 < 0));
                  ap->next = PrintAssln("\tvblendvps\t%s,%s,%s,%s\n", 
	                                archfregs[-FREGBEG-op3],
                                        archfregs[-FREGBEG-op1],
	                                archfregs[-FREGBEG-op2],
	                                archfregs[-FREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV is Not found in this x86 arch!");
            #endif
         break;
      case FCMOVD1:
            #if defined(AVX) 
               assert( (op1 < 0) && (op3 < 0));
               if (op2 < 0) /* only src2(here op2) can be mem */
                  ap->next = PrintAssln("\tvblendvpd\t%s,%s,%s,%s\n", 
	                                archfregs[-DREGBEG-op3],
                                        archfregs[-DREGBEG-op2],
	                                archfregs[-DREGBEG-op1],
	                                archfregs[-DREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvblendvpd\t%s,%s,%s,%s\n", 
	                                archfregs[-DREGBEG-op3],
                                        GetDregOrDeref(op2),
	                                archfregs[-DREGBEG-op1],
	                                archfregs[-DREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV is Not found in this x86 arch!");
            #endif
         break;
      case FCMOVD2:
            #if defined(AVX) 
               assert( (op1 < 0) && (op3 < 0) && (op2 < 0));
                  ap->next = PrintAssln("\tvblendvpd\t%s,%s,%s,%s\n", 
	                                archfregs[-DREGBEG-op3],
                                        archfregs[-DREGBEG-op1],
	                                archfregs[-DREGBEG-op2],
	                                archfregs[-DREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV is Not found in this x86 arch!");
            #endif
         break;
#endif

/*
 *    Majedul: Right now, these special purpose FCMPW insts are considered 
 *    only for X86. 
 *    We have 6 conditional branches: JEQ, JNE, JLT, JLE, JGT, JGE
 *    So, we will have six variants of those.
 *    Format:  FCMPWXX and FCMPDWXX
 *    op3 can be mem .
 *    NOTE: SSE doesn't support GT and GE, we will implement those with 
 *    NLE, NLT
 */
#ifdef X86
      case FCMPWEQ:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpss\t$0,%s,%s,%s\n",
                                     archxmmregs[-FREGBEG-op3],
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpss\t$0,%s,%s,%s\n",
                                     GetDeref(op3),
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
         #else
            assert(op1 == op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpss\t$0,%s,%s\n", 
                                     archvfregs[-FREGBEG-op3], 
                                     archvfregs[-FREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpss\t$0,%s,%s\n", 
                                     GetDeref(op3), 
                                     archvfregs[-FREGBEG-op1]);
         #endif 
         break;
      case FCMPWNE:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpss\t$4,%s,%s,%s\n",
                                     archxmmregs[-FREGBEG-op3],
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpss\t$4,%s,%s,%s\n",
                                     GetDeref(op3),
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
         #else
            assert(op1 == op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpss\t$4,%s,%s\n",  
                                     archvfregs[-FREGBEG-op3], 
                                     archvfregs[-FREGBEG-op1]);
            else 
               ap->next = PrintAssln("\tcmpss\t$4,%s,%s\n",  
                                     GetDeref(op3), 
                                     archvfregs[-FREGBEG-op1]);
         #endif 
         break;
      case FCMPWLT:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpss\t$1,%s,%s,%s\n",
                                     archxmmregs[-FREGBEG-op3],
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpss\t$1,%s,%s,%s\n",
                                     GetDeref(op3),
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
         #else
            assert(op1 == op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpss\t$1,%s,%s\n",  
                                      archvfregs[-FREGBEG-op3], 
                                      archvfregs[-FREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpss\t$1,%s,%s\n",  
                                      GetDeref(op3), 
                                      archvfregs[-FREGBEG-op1]);
         #endif 
         break;
      case FCMPWLE:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpss\t$2,%s,%s,%s\n",
                                     archxmmregs[-FREGBEG-op3],
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpss\t$2,%s,%s,%s\n",
                                     GetDeref(op3),
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmpss\t$2,%s,%s\n",  
                                  archvfregs[-FREGBEG-op3], 
                                  archvfregs[-FREGBEG-op1]);
         #endif 
         break;
      case FCMPWGT:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpss\t$0x0E,%s,%s,%s\n",
                                     archxmmregs[-FREGBEG-op3],
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
            else   
               ap->next = PrintAssln("\tvcmpss\t$0x0E,%s,%s,%s\n",
                                     GetDeref(op3),
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
         #else
/*             SSE: GT is replaced with NLE */            
            assert(op1 == op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpss\t$6,%s,%s\n",  
                                     archvfregs[-FREGBEG-op3], 
                                     archvfregs[-FREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpss\t$6,%s,%s\n",  
                                     GetDeref(op3), 
                                     archvfregs[-FREGBEG-op1]);
         #endif 
         break;
      case FCMPWGE:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpss\t$0x0D,%s,%s,%s\n",
                                     archxmmregs[-FREGBEG-op3],
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpss\t$0x0D,%s,%s,%s\n",
                                     GetDeref(op3),
                                     archxmmregs[-FREGBEG-op2],
                                     archxmmregs[-FREGBEG-op1]);
         #else
/*          GE is replaced with NLT*/            
            assert(op1 == op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpss\t$5,%s,%s\n",  
                                     archvfregs[-FREGBEG-op3], 
                                     archvfregs[-FREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpss\t$5,%s,%s\n",  
                                     GetDeref(op3), 
                                     archvfregs[-FREGBEG-op1]);
         #endif 
         break;
         
      case FCMPDWEQ:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpsd\t$0,%s,%s,%s\n",
                                     archxmmregs[-DREGBEG-op3],
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpsd\t$0,%s,%s,%s\n",
                                     GetDeref(op3),
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
         #else
            assert(op1 ==op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpsd\t$0,%s,%s\n",  
                                     archvdregs[-DREGBEG-op3], 
                                     archvdregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpsd\t$0,%s,%s\n",  
                                     GetDeref(op3), 
                                     archvdregs[-DREGBEG-op1]);
         #endif 
         break;
      case FCMPDWNE:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpsd\t$4,%s,%s,%s\n",
                                     archxmmregs[-DREGBEG-op3],
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpsd\t$4,%s,%s,%s\n",
                                     GetDeref(op3), 
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
         #else
            assert(op1 ==op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpsd\t$4,%s,%s\n",  
                                     archvdregs[-DREGBEG-op3], 
                                     archvdregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpsd\t$4,%s,%s\n",  
                                     GetDeref(op3), 
                                     archvdregs[-DREGBEG-op1]);
         #endif
         break;
      case FCMPDWLT:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpsd\t$1,%s,%s,%s\n",
                                     archxmmregs[-DREGBEG-op3],
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpsd\t$1,%s,%s,%s\n",
                                     GetDeref(op3), 
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
         #else
            assert(op1 ==op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpsd\t$1,%s,%s\n",  
                                     archvdregs[-DREGBEG-op3], 
                                     archvdregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpsd\t$1,%s,%s\n",  
                                     GetDeref(op3), 
                                     archvdregs[-DREGBEG-op1]);
         #endif
         break;
      case FCMPDWLE:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpsd\t$2,%s,%s,%s\n",
                                     archxmmregs[-DREGBEG-op3],
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpsd\t$2,%s,%s,%s\n",
                                     GetDeref(op3), 
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
         #else
            assert(op1 ==op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpsd\t$2,%s,%s\n",  
                                     archvdregs[-DREGBEG-op3], 
                                     archvdregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpsd\t$2,%s,%s\n",  
                                     GetDeref(op3), 
                                     archvdregs[-DREGBEG-op1]);
         #endif
         break;
      case FCMPDWGT:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpsd\t$0x0E,%s,%s,%s\n",
                                     archxmmregs[-DREGBEG-op3],
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpsd\t$0x0E,%s,%s,%s\n",
                                     GetDeref(op3), 
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
         #else
/*             SSE: GT is replaced with NLE */            
            assert(op1 ==op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpsd\t$6,%s,%s\n",  
                                     archvdregs[-DREGBEG-op3], 
                                     archvdregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpsd\t$6,%s,%s\n",  
                                     GetDeref(op3), 
                                     archvdregs[-DREGBEG-op1]);
         #endif
         break;
      case FCMPDWGE:
         #ifdef AVX
            if (op3 < 0)
               ap->next = PrintAssln("\tvcmpsd\t$0x0D,%s,%s,%s\n",
                                     archxmmregs[-DREGBEG-op3],
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvcmpsd\t$0x0D,%s,%s,%s\n",
                                     GetDeref(op3), 
                                     archxmmregs[-DREGBEG-op2],
                                     archxmmregs[-DREGBEG-op1]);
         #else
/*             SSE: GE is replaced with NLT */            
            assert(op1 ==op2);
            if (op3 < 0)
               ap->next = PrintAssln("\tcmpsd\t$5,%s,%s\n",  
                                     archvdregs[-DREGBEG-op3], 
                                     archvdregs[-DREGBEG-op1]);
            else
               ap->next = PrintAssln("\tcmpsd\t$5,%s,%s\n",  
                                     GetDeref(op3), 
                                     archvdregs[-DREGBEG-op1]);
         #endif
         break;         
#endif         

      case COMMENT:
         #ifdef X86
            /*continue;*/ /* skip comments temporary for testing with ATLAS*/
/*
 *          NOTE: I skip the repeated lines of Inserted LD from ... comments,
 *          can't get rid of that right now since it is used to keep track the 
 *          fpconst load. see DoRegAsgTransforms in optreg.c  
 */
            #if 1 
            if (op1 && STname[op1-1] 
                  && !strstr(STname[op1-1],"Inserted LD from"))
               ap->next = PrintAssln("#%s\n", op1 ? STname[op1-1] : "");
            else continue;
            #else
               ap->next = PrintAssln("#%s\n", op1 ? STname[op1-1] : "");
            #endif
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
/*
 *    FIXME: vpinsrd works on 32 bit ireg like: eax instead of rax...
 *    Only 8 of the 16 IREG has 32 bit version. If we don't make SREG visible,
 *    it will eventually cause problem. 
 *    sirk3amax => -rc -V -Ps b A 0 4 -P all 0 128 -U 2
 *    NOTE: added an assertion here
 */
      case VGR2VR32:
         op1 = -op1;
         if (op1 >= VDREGBEG && op1 < VDREGEND)
            op1 = op1 - VDREGBEG + DREGBEG;
         else if (op1 >= VFREGBEG && op1 < VFREGEND)
            op1 = op1 - VFREGBEG + DREGBEG;
         else if (op1 >= FREGBEG && op1 < FREGEND)
            op1 = op1 - FREGBEG + DREGBEG;
         else if (op1 >= VIREGBEG && op1 < VIREGEND)
            op1 = op1 - VIREGBEG + DREGBEG;
         op1 = -op1;
         #ifdef X86_64
            assert((-op2) >= IREGBEG && (-op2) < (IREGBEG + NSR) );
         #endif
         #ifdef AVX
            /*ap->next = PrintAssln("\tpinsrw\t%s,%s,%s\n", GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op2],
                                  archxmmregs[-DREGBEG-op1]);*/
            ap->next = PrintAssln("\tvpinsrd\t%s,%s,%s,%s\n", 
                                  GetIregOrConst(op3),
                              #ifdef X86_64
                                  archsregs[-IREGBEG-op2],
                              #else
                                  archiregs[-IREGBEG-op2],
                              #endif
                                  archxmmregs[-DREGBEG-op1],
                                  archxmmregs[-DREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tpinsrd\t%s,%s,%s\n", GetIregOrConst(op3),
                           #ifdef X86_64
                               archsregs[-IREGBEG-op2],
                           #else
                               archiregs[-IREGBEG-op2],
                           #endif
                               archdregs[-DREGBEG-op1]);
         #endif
         break;
/*
 *    vpinsrq works on 32 bit ireg like: eax instead of rax...
 */
      case VGR2VR64:
         op1 = -op1;
         if (op1 >= VDREGBEG && op1 < VDREGEND)
            op1 = op1 - VDREGBEG + DREGBEG;
         else if (op1 >= VFREGBEG && op1 < VFREGEND)
            op1 = op1 - VFREGBEG + DREGBEG;
         else if (op1 >= FREGBEG && op1 < FREGEND)
            op1 = op1 - FREGBEG + DREGBEG;
         else if (op1 >= VIREGBEG && op1 < VIREGEND)
            op1 = op1 - VIREGBEG + DREGBEG;
         op1 = -op1;
         #ifdef AVX
            /*ap->next = PrintAssln("\tpinsrw\t%s,%s,%s\n", GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op2],
                                  archxmmregs[-DREGBEG-op1]);*/
            ap->next = PrintAssln("\tvpinsrq\t%s,%s,%s,%s\n", 
                                  GetIregOrConst(op3),
                                  archiregs[-IREGBEG-op2],
                                  archxmmregs[-DREGBEG-op1],
                                  archxmmregs[-DREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tpinsrq\t%s,%s,%s\n", GetIregOrConst(op3),
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
      case VDLDU:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovupd\t%s,%s\n", GetDeref(op2),
                                  archvdregs[-VDREGBEG-op1]);       
         #else
            ap->next = PrintAssln("\tmovupd\t%s,%s\n", GetDeref(op2),
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
            fko_error(__LINE__, "Need to redefine for AVX when necessary");
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
            fko_error(__LINE__, "Need to redefine for AVX");
         #else
            ap->next = PrintAssln("\tmovhpd\t%s, %s\n", GetDeref(op2),
                                  archvdregs[-VDREGBEG-op1]);
         #endif
         break;
/*
 *    special load which broadcast the value
 */
      case VDLDSB:
         #ifdef AVX
            ap->next = PrintAssln("\tvbroadcastsd\t%s,%s\n", GetDeref(op2),
                                  archvdregs[-VDREGBEG-op1]);
         #else
/* 
 *          movddup only supported in SSE3... !!!
 */
            ap->next = PrintAssln("\tmovddup\t%s,%s\n", GetDeref(op2),
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
      case VDSTU:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovupd\t%s, %s\n", 
                                  archvdregs[-VDREGBEG-op2],
                                  GetDeref(op1));
         #else
            ap->next = PrintAssln("\tmovupd\t%s, %s\n", 
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
 *       Allow scalar to vector reg to reg move
 *       two variation :
 * 1. VDMOVS vr0, sr, vr1  // vr0[0] = sr, vr0[vlen-1: 1] = vr1[vlen-1: 1]
 * 2. VDMOVS sr, vr, 0     // sr = vr[0]
 *       here vr0 and vr1 should be vector register
 */
         op1 = -op1;
         op2 = -op2;
         op3 = op3 ? -op3: op3;
/*
 *       sr-to-vr: VDMOVS vr0, sr, vr1 
 */
         if (op1 >= VDREGBEG && op1 < VDREGEND)
         {
            assert(op3);
            assert(op2 >= DREGBEG && op2 < DREGEND);
            op2 = op2 - DREGBEG + VDREGBEG;
         
            if (op1 == op2 == op3) /* nothing to do */
               continue; 
         
            op1 = -op1;
            op2 = -op2;
            op3 = -op3;
            #ifdef AVX
               ap->next = PrintAssln("\tvmovsd\t%s, %s,%s\n", 
                                     archxmmregs[-VDREGBEG-op2],
                                     archxmmregs[-VDREGBEG-op3],
                                     archxmmregs[-VDREGBEG-op1]); 
            #else
               assert(op1==op3);
               ap->next = PrintAssln("\tmovsd\t%s, %s\n",  
                                     archvfregs[-VDREGBEG-op2],
                                     archvfregs[-VDREGBEG-op1]);
            #endif
         }
/*
 *       vr-to-sr: VDMOVS sr, vr0, 0
 */
         else if (op1 >= DREGBEG && op1 < DREGEND)
         {
            assert(op2 >= VDREGBEG && op2 < VDREGEND);
            assert(!op3);
/*
 *          sreg and vsreg are aliased in X86
 */
            op1 = op1 - DREGBEG + VDREGBEG;
            if (op1 == op2) /* same reg, no need to move */
               continue;
         
            op1 = -op1;
            op2 = -op2;
            /*fprintf(stderr, "********* VDMOVS %d %d %d\n", -VDREGBEG-op1, 
                  -VDREGBEG-op2, op3);*/
            #ifdef AVX
/*
 *             vmovsd is expensive. as dreg and vdreg are aliased, we use VDMOV
 */
               ap->next = PrintAssln("\tvmovapd\t%s, %s\n", 
                                     archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tmovsd\t%s, %s\n",  
                                     archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            #endif
         }
#if 0         
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
#if 0            
             ap->next = PrintAssln("\tvmovsd\t%s, %s,%s\n", 
                                   archxmmregs[-VDREGBEG-op2],
                                   archxmmregs[-VDREGBEG-op1],
                                   archxmmregs[-VDREGBEG-op1]);
#else
             ap->next = PrintAssln("\tvmovapd\t%s, %s\n", 
                                   archvdregs[-VDREGBEG-op2],
                                   archvdregs[-VDREGBEG-op1]);
#endif
            #else
               ap->next = PrintAssln("\tmovsd\t%s, %s\n",  
                                     archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            #endif
/*
 *          In X86, scalar and vector registers are aliased. 
 *          So, we don't need to emit the move instruction with same
 *          source and destination.
 */
         else
#if 0            
            #ifdef AVX
               ap->next = PrintAssln("\tvmovapd\t%s, %s\n", 
                                     archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tmovapd\t%s, %s\n", 
                                     archvdregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1]);
            #endif
#else
               continue;
#endif
               
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
      case VDDIV:
         #ifdef AVX
            ap->next = PrintAssln("\tvdivpd\t%s, %s, %s\n", 
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tdivpd\t%s, %s\n", GetDregOrDeref(op3),
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
            #elif defined(ArchHasMAC) && defined(FMA3)
               assert( (op1 < 0) && (op2 < 0));
               if (op3 < 0) /*FMA4: only src2(here op3) can be mem */
                  ap->next = PrintAssln("\tvfmadd231pd\t%s,%s,%s\n", 
                                        archdregs[GetDregID(op3)],
	                                archdregs[-VDREGBEG-op2],
	                                archdregs[-VDREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvfmadd231pd\t%s,%s,%s\n", 
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
                                  archvdregs[-VDREGBEG-op2], 
                                  archvdregs[-VDREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\taddpd\t%s, %s\n", GetDregOrDeref(op3),
                               archvdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDSUB:
         #ifdef AVX
            ap->next = PrintAssln("\tvsubpd\t%s, %s, %s\n", 
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2], 
                                  archvdregs[-VDREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tsubpd\t%s, %s\n", GetDregOrDeref(op3),
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
      case VDNEG:
	 assert(DTnzerod);
         #ifdef AVX
            if (op3 >= 0) /* need to check the usage */
               ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                     GetDeref(SToff[DTnzerod-1].sa[2]),
                                     archdregs[-VDREGBEG-op2],
	                             archdregs[-VDREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvxorpd\t%s,%s,%s\n", 
                                     archdregs[-VDREGBEG-op3],
                                     archdregs[-VDREGBEG-op2],
	                             archdregs[-VDREGBEG-op1]);
         #else
            assert(op1==op2);
            if (op3 >= 0)
               ap->next = PrintAssln("\txorpd\t%s,%s\n", 
                                     GetDeref(SToff[DTnzerod-1].sa[2]),
	                             archdregs[-VDREGBEG-op1]);
            else
               ap->next = PrintAssln("\txorpd\t%s,%s\n", 
                                     archdregs[-VDREGBEG-op3],
	                             archdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDMAX:
         #ifdef AVX
            ap->next = PrintAssln("\tvmaxpd\t%s, %s, %s\n", 
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2], 
                                  archvdregs[-VDREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tmaxpd\t%s, %s\n", GetDregOrDeref(op3),
                               archvdregs[-VDREGBEG-op1]);
         #endif
         break;
      case VDMIN:
         #ifdef AVX
            ap->next = PrintAssln("\tvminpd\t%s, %s, %s\n", 
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2], 
                                  archvdregs[-VDREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tminpd\t%s, %s\n", GetDregOrDeref(op3),
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
      case VDHADD:
/*
 *       behavior diff for avx and sse??
 */
         #ifdef AVX
            ap->next = PrintAssln("\tvhaddpd\t%s,%s,%s\n",
                                     GetDregOrDeref(op3), 
                                     archvdregs[-VDREGBEG-op2], 
                                     archvdregs[-VDREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\thaddpd\t%s, %s\n", GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op1]);
         #endif
         break;
/*
 *    Conditional MOV or, Select operation.
 *    NOTE: these instructions are only supported in SSE4.1 (or, above) and AVX
 *    AVX: 
 *             vblendvps ymm1, ymm2, ymm3/m256, ymm4
 *    SSE4.1: 
 *             blendvps xmm1, xmm2/m128, <XMM0>
 *    
 *    NOTE: There are two problems with SSE's inst:
 *          1) XMM0 must be used as the mask which may complicate the ra.
 *          2) dest is always set as src1, can't implement 2 version without
 *             changing the mask (need to compute mask' )
 *          
 *    VFCMOV1:  vreg0, vreg1/mem, vreg2    # vreg0 = vreg2? vreg0 : vreg1         
 *    VFCMOV2:  vreg0, vreg1, vreg2        # vreg0 = vreg2? vreg1 : vreg0
 *
 *    NOTE: for 2nd version, no mem can be used. in AVX, only src2 can be mem.
 *    but here, we use dest as src2 and it can't be mem.
 */
      case VDCMOV1:
         #ifdef X86
            #if defined(AVX) 
               assert( (op1 < 0) && (op3 < 0));
               if (op2 < 0) /* only src2(here op2) can be mem */
                  ap->next = PrintAssln("\tvblendvpd\t%s,%s,%s,%s\n", 
	                                archvdregs[-VDREGBEG-op3],
                                        archvdregs[-VDREGBEG-op2],
	                                archvdregs[-VDREGBEG-op1],
	                                archvdregs[-VDREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvblendvpd\t%s,%s,%s,%s\n", 
	                                archvdregs[-VDREGBEG-op3],
                                        GetDregOrDeref(op2),
	                                archvdregs[-VDREGBEG-op1],
	                                archvdregs[-VDREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV is Not found in this x86 arch!");
            #endif
         #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV Not supported in this arch!");
         #endif
         break;

      case VDCMOV2:
         #ifdef X86
            #if defined(AVX) 
               assert( (op1 < 0) && (op3 < 0) && (op2 < 0));
                  ap->next = PrintAssln("\tvblendvpd\t%s,%s,%s,%s\n", 
	                                archvdregs[-VDREGBEG-op3],
                                        archvdregs[-VDREGBEG-op1],
	                                archvdregs[-VDREGBEG-op2],
	                                archvdregs[-VDREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV is Not found in this x86 arch!");
            #endif
         #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV Not supported in this arch!");
         #endif
         break;
         
/*
 * NOTE: can use PSHUFD for case where dest is output only
 */
      case VDSHUF:
         cp = imap2cmap(SToff[op3-1].i);
#if defined(AVX) && 1
/*
 *          handle as special case first.... later will check whether prior 
 *          implementation can handle it
 */
           if (cp[3] == 3 && cp[2] == 2 && cp[1] == 4 && cp[0] == 0)
           {
               ap->next = PrintAssln("\tunpcklpd\t%s,%s\n", 
                                     archxmmregs[-VDREGBEG-op2],
                                     archxmmregs[-VDREGBEG-op1]);
           }
           else if (cp[3] == 5 && cp[2] == 4 && cp[1] == 1 && cp[0] == 0)
           {
               ap->next = PrintAssln("\tvinsertf128\t$1, %s, %s, %s\n", 
                                     archxmmregs[-VDREGBEG-op2],
                                     archvdregs[-VDREGBEG-op1],
                                     archvdregs[-VDREGBEG-op1]);
           }
           else if ( op1 == op2 && 
                     cp[3] == 3 && cp[2] == 2 && cp[1] == 1 && cp[0] == 1)
           {
               ap->next = PrintAssln("\tunpckhpd\t%s,%s\n", 
                                     archxmmregs[-VDREGBEG-op2],
                                     archxmmregs[-VDREGBEG-op1]); 
           }
           else if ( op1 == op2 && 
                     cp[3] == 3 && cp[2] == 2 && cp[1] == 3 && cp[0] == 2)
           {
               ap->next = PrintAssln("\tvextractf128\t$1, %s, %s\n", 
                                     archvdregs[-VDREGBEG-op1],
                                     archxmmregs[-VDREGBEG-op2]);
           }
           else
#endif
         #ifdef AVX2
         if (op1 == op2 || (cp[3] > 3 && cp[2] > 3 && cp[1] > 3 && cp[0] > 3)) 
         {
            for (i=0; i < 4; i++) 
               if (cp[i] > 3) cp[i] -= 4;
            j = 0;
            for (i=0; i < 4; i++)
               j |= cp[i] << (i*2);
            ap->next = PrintAssln("\tvpermpd\t$0x%x,%s,%s\n",
                    j, archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         }
#if 0
         else if (op1 == op2)
         {
            for (i=0; i < 4; i++)
               if (cp[i] > 3) cp[i] -= 4;
            j = 0;
            for (i=0; i < 4; i++)
               j |= cp[i] << (i*2);
            ap->next = PrintAssln("\tvpermpd\t$0x%x,%s,%s\n",
                    j, archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         }
#endif         
         else 
         #endif
         #ifdef AVX
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
            else if (cp[3] == 3 && cp[2] == 2 && cp[1] == 4 && cp[0] == 0)
            {
               ap->next = PrintAssln("\tunpcklpd\t%s,%s\n", 
                                     archxmmregs[-VDREGBEG-op2],
                                     archxmmregs[-VDREGBEG-op1]);
            
            }
            else if (cp[3] == 3 && cp[2] == 2 && cp[1] == 1 && cp[0] == 5)
            {
               ap->next = PrintAssln("ERROR:\t%s %d %d %d\n", 
                                     instmnem[ip->inst[0]], op1, op2, op3);
               fko_error(__LINE__, "Not implemented this VDSHUF for AVX yet!");         
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
#if 1
/*
 *          if op1 == op2 and the upper half are same as destination
 *          FIXME: Although in HIL, we use same operand: op1==op2
 *          repeateable optimization may change it to a diff src than dest
 *          NOTE: V[F/D]SHUF instruciton is a inst of dest_inuse_implecitly
 *          If 2nd operand is also dest, it is kind of redundant... no way to
 *          manage that !!!
 */
            else if ( (op1 == op2) && cp[3] == 3 && cp[2] == 2)
            {
               for (i=0; i < 2; i++)   /* dest & src are same*/
                  if (cp[0] > 4) cp[0] -= 4;
               if (cp[0] > 1 && cp[1] > 1) /* shift the upper half to lower*/
               {
                  ap->next = PrintAssln("\tvperm2f128\t$%d,%s,%s,%s\n", 0x11,
                                        archvdregs[-VDREGBEG-op1], 
                                        archvdregs[-VDREGBEG-op1], 
                                        archvdregs[-VDREGBEG-op1]);
                  ap=ap->next;
                  cp[0] -= 2;
                  cp[1] -= 2;
               }
               
               if (cp[0] > 1 || cp[1] > 1)
                  fko_error(__LINE__, "Not implemented VDHSUF: %x",
                           SToff[op3-1].i); 

               if (cp[0] == 0)
               {
                  if (cp[1] == 0)
                     ap->next = PrintAssln("\tunpcklpd\t%s,%s\n",
                                           archxmmregs[-VDREGBEG-op1], 
                                           archxmmregs[-VDREGBEG-op1]);
                  else
                     fko_error(__LINE__, "Useless VDSHUF");
               }
               else /* cp[0] == 1*/
               {
                  if (cp[1] == 0)
                     ap->next = PrintAssln("\tshufpd\t%d,%s,%s\n",1,
                                           archxmmregs[-VDREGBEG-op1], 
                                           archxmmregs[-VDREGBEG-op1]);
                  else /* cp[0] == 1 && cp[1] == 1*/
                  {
                     ap->next = PrintAssln("\tunpckhpd\t%s,%s\n",
                                           archxmmregs[-VDREGBEG-op1], 
                                           archxmmregs[-VDREGBEG-op1]);
                  }
               }    
            }

#endif
            else 
            {
               /*ap->next = PrintAssln("ERROR:\t%d %d %d %d\n", 
                                     ip->inst[0], op1, op2, op3);*/
               fko_error(__LINE__, "Not implemented VDSHUF %d, %d, 0x%x for AVX!",
                         op1, op2, SToff[op3-1].i);    
            }

         #else

         /*cp = imap2cmap(SToff[op3-1].i);*/
         if (cp[0] == 0 && cp[1] == 2)
            ap->next = PrintAssln("\tunpcklpd\t%s,%s\n", 
               archvdregs[-VDREGBEG-op2], archvdregs[-VDREGBEG-op1]);
         else if (cp[0] == 0 && cp[1] == 1)
            fko_error(__LINE__, "Useless VDSHUF");
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
                     fko_error(__LINE__, "Useless VDSHUF");
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
/*
 *    Two special shuffle, specially needed in avx
 */
      case VDHIHALF:
/*
 *       vr0[1,0], vr1[1,0], vr2[1,0]: vr0[0] = vr1[1]; vr0[1] = vr2[1]; 
 */
         #ifdef AVX
            if ( op3 > 0)
               ap->next = PrintAssln("\tvperm2f128\t$0x31,%s,%s,%s\n",
                                     GetDeref(op3), 
                                     archvdregs[-VDREGBEG-op2], 
                                     archvdregs[-VDREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvperm2f128\t$0x31,%s,%s,%s\n",
                                     archvdregs[-VDREGBEG-op3], 
                                     archvdregs[-VDREGBEG-op2], 
                                     archvdregs[-VDREGBEG-op1]);
            
         #else
            fko_error(__LINE__, "Not implemented this instruction in SSE yet!");
         #endif
         break;

      case VDLOHALF:
/*
 *       vr0[1,0], vr1[1,0], vr2[1,0]: vr0[0] = vr1[0]; vr0[1] = vr2[0]; 
 */
         #ifdef AVX
            if ( op3 > 0)
               ap->next = PrintAssln("\tvperm2f128\t$0x20,%s,%s,%s\n",
                                     GetDeref(op3), 
                                     archvdregs[-VDREGBEG-op2], 
                                     archvdregs[-VDREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvperm2f128\t$0x20,%s,%s,%s\n",
                                     archvdregs[-VDREGBEG-op3], 
                                     archvdregs[-VDREGBEG-op2], 
                                     archvdregs[-VDREGBEG-op1]);
            
         #else
            fko_error(__LINE__, "Not implemented this instruction in SSE yet!");
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
      case VFLDU:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovups\t%s, %s\n", GetDeref(op2),
                                  archvfregs[-VFREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovups\t%s, %s\n", GetDeref(op2),
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
/*
 *    special load which can broadcast
 */
      case VFLDSB:
         #ifdef AVX
            ap->next = PrintAssln("\tvbroadcastss\t%s, %s\n", GetDeref(op2),
                                  archvfregs[-VFREGBEG-op1]);
         #else
/*
 *          SSE doesn't have any broadcast instruction. So, we are using
 *          multiple instruction to implement that
 */
            ap->next = PrintAssln("\tmovss\t%s, %s\n", GetDeref(op2),
                                  archvfregs[-VFREGBEG-op1]);
            ap = ap->next;                           
            ap->next = PrintAssln("\tshufps\t$%d, %s, %s\n", 0, 
                                  archvfregs[-VFREGBEG-op1], 
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
      case VFSTU:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovups\t%s, %s\n", 
                                  archvfregs[-VFREGBEG-op2],
                                  GetDeref(op1));
         #else
            ap->next = PrintAssln("\tmovups\t%s, %s\n", 
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
 *       Allow scalar reg to vector reg move and vector to scalar move
 *       two variation of these inst
 * 1. VFMOVS vr0, sr, vr1  // vr0[0] = vr2[0], vr0[vlen-1: 1] = vr1[vlen-1: 1]
 * 2. VFMOVS sr, vr, 0     // sr = vr[0]
 *       here vr0 and vr1 should be vector register
 */
         op1 = -op1;
         op2 = -op2;
         op3 = op3 ? -op3: op3;
/*
 *       sr-to-vr: VFMOVS vr0, sr, vr1 
 */
         if (op1 >= VFREGBEG && op1 < VFREGEND)
         {
            assert(op3); 
            assert(op2 >= FREGBEG && op2 < FREGEND);
            op2 = op2 - FREGBEG + VFREGBEG;
           
            if ( (op1 == op2) && (op2 == op3) ) /* nothing to do */
               continue;
         
            op1 = -op1;
            op2 = -op2;
            op3 = -op3;
            
            #ifdef AVX
               ap->next = PrintAssln("\tvmovss\t%s, %s,%s\n", 
                                     archxmmregs[-VFREGBEG-op2],
                                     archxmmregs[-VFREGBEG-op3],
                                     archxmmregs[-VFREGBEG-op1]); 
            #else
               assert(op1==op3);
               ap->next = PrintAssln("\tmovss\t%s, %s\n",  
                                     archvfregs[-VFREGBEG-op2],
                                     archvfregs[-VFREGBEG-op1]);
            #endif
         }
/*
 *       vr-to-sr: VFMOVS sr, vr0, 0
 */
         else if (op1 >= FREGBEG && op1 < FREGEND)
         {
            assert(op2 >= VFREGBEG && op2 < VFREGEND);
            assert(!op3);
/*
 *          freg and vfreg are aliased in x86
 */
            op1 = op1 - FREGBEG + VFREGBEG;
            if (op1 == op2) /* same reg, no need for this mov */
               continue;
         
            op1 = -op1;
            op2 = -op2;

            #ifdef AVX
/*
 *             vmovss is expansive and 3 operand inst in avx
 */
               ap->next = PrintAssln("\tvmovaps\t%s, %s\n", 
                                     archvfregs[-VFREGBEG-op2],
                                     archvfregs[-VFREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tmovss\t%s, %s\n",  
                                     archvfregs[-VFREGBEG-op2],
                                     archvfregs[-VFREGBEG-op1]);
            #endif
         }
         else
            assert(0); /* unknown case !! */
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
      case VFDIV:
         #ifdef AVX
            ap->next = PrintAssln("\tvdivps\t%s, %s, %s\n", 
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tdivps\t%s, %s\n", GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
/*
 *    NOTE: we can use GetDregOrDeref() directly .... ... 
 */
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
            #elif defined(ArchHasMAC) && defined(FMA3)
               assert( (op1 < 0) && (op2 < 0));
               if (op3 < 0) /*FMA4: only src2(here op3) can be mem */
                  ap->next = PrintAssln("\tvfmadd231ps\t%s,%s,%s\n", 
                                        archfregs[GetDregID(op3)],
	                                archfregs[-VFREGBEG-op2],
	                                archfregs[-VFREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvfmadd231ps\t%s,%s,%s\n", 
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
      case VFSUB:
         #ifdef AVX
            ap->next = PrintAssln("\tvsubps\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2], 
                                  archvfregs[-VFREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tsubps\t%s, %s\n", GetDregOrDeref(op3),
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
      case VFMAX:
         #ifdef AVX
            ap->next = PrintAssln("\tvmaxps\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2], 
                                  archvfregs[-VFREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tmaxps\t%s, %s\n", GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op1]);
         #endif
         break;
      case VFMIN:
         #ifdef AVX
            ap->next = PrintAssln("\tvminps\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2], 
                                  archvfregs[-VFREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tminps\t%s, %s\n", GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op1]);
         #endif
         break;
      case VFNEG:
	 assert(DTnzero);
         #ifdef AVX
            if (op3 >= 0)
               ap->next = PrintAssln("\tvxorps\t%s,%s,%s\n", 
                                     GetDeref(SToff[DTnzero-1].sa[2]),
	                             archfregs[-VFREGBEG-op2],
	                             archfregs[-VFREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvxorps\t%s,%s,%s\n", 
	                             archfregs[-VFREGBEG-op3],
	                             archfregs[-VFREGBEG-op2],
	                             archfregs[-VFREGBEG-op1]);
         #else
            assert(op2 == op1);
            if (op3 >= 0)
               ap->next = PrintAssln("\txorps\t%s,%s\n", 
                                     GetDeref(SToff[DTnzero-1].sa[2]),
	                             archfregs[-VFREGBEG-op1]);
            else
               ap->next = PrintAssln("\txorps\t%s,%s\n", 
	                             archfregs[-VFREGBEG-op3],
	                             archfregs[-VFREGBEG-op1]);
         #endif
         break;
      case VFZERO:
         assert(ap);
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
      case VFHADD:
/*
 *    NOTE: behavior of this inst for AVX and SSE is different!!!
 *    But since we only use these instrucitons now for reduction (not for 
 *    translating HIL to LIL), we ignore this issue fro now. 
 */
         #ifdef AVX
            ap->next = PrintAssln("\tvhaddps\t%s,%s,%s\n",
                                     GetDregOrDeref(op3), 
                                     archvfregs[-VFREGBEG-op2], 
                                     archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\thaddps\t%s, %s\n", GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op1]);
         #endif
         break;
/*
 *    Majedul: adding new vector-compare instructions. they overwrite the 
 *    destination register.... I use 4 operand instruction here without 
 *    predicates ...
 *    NOTE: in avx implementation, the inst supports 4 operand. dest is diff
 *    than any other srcs. For SSE, as it supports 3 operands, need to change
 *    the LIL for it! and also the transformation it uses!!!
 *    
 *    FIXED: if any instruction doesn't support memory ref, assert this out!
 *    Here, VCMPXX instruction does support mem ref. need to check later 
 *    whether an assertion is needed for op1 && op2 as negative!!!
 */
      case VFCMPWEQ:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2); /* 1st src and dest are same */
            ap->next = PrintAssln("\tcmpps\t$0,%s,%s\n",  
                          GetDregOrDeref(op3), archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPWNE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$4,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmpps\t$4,%s,%s\n",  
                        GetDregOrDeref(op3), archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPWLT:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$1,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmpps\t$1,%s,%s\n",  
                        GetDregOrDeref(op3), archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPWLE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$2,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmpps\t$2,%s,%s\n",  
                        GetDregOrDeref(op3), archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPWGT:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0E,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
/*             SSE: GT is replaced with NLE */            
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmpps\t$6,%s,%s\n",  
                        GetDregOrDeref(op3), archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPWGE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0D,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
/*             SSE: GE is replaced with NLT */            
            ap->next = PrintAssln("\tcmpps\t$5,%s,%s\n",  
                        GetDregOrDeref(op3), archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
#if 0
      case VFCMPWNLT:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$5,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmpps\t$5,%s,%s\n",  
                        GetDregOrDeref(op3), archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
      case VFCMPWNLE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$6,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmpps\t$6,%s,%s\n",  
                        GetDregOrDeref(op3), archvfregs[-VFREGBEG-op1]);
         #endif 
         break;
/* 
 *    Majedul: Following vector-cmps are not supported in SSE, only 
 *    supported by AVX
 */         
      case VFCMPWGT:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0E,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VFCMPWGT only supported in AVX!");
         #endif 
         break;
      case VFCMPWGE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0D,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VFCMPWGE only supported in AVX!");
         #endif 
         break;
      case VFCMPWNGT:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$0x0A,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VFCMPWNGT only supported in AVX!");
         #endif 
         break;
      case VFCMPWNGE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmpps\t$9,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvfregs[-VFREGBEG-op2],
                                  archvfregs[-VFREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VFCMPWNGE only supported in AVX!");
         #endif 
         break;
#endif

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
      
      case VDCMPWEQ:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$0,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            assert(op1 ==op2);
            ap->next = PrintAssln("\tcmppd\t$0,%s,%s\n",  
                          GetDregOrDeref(op3), archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPWNE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$4,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmppd\t$4,%s,%s\n",  
                          GetDregOrDeref(op3), archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPWLT:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$1,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmppd\t$1,%s,%s\n",  
                          GetDregOrDeref(op3), archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPWLE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$2,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmppd\t$2,%s,%s\n",  
                          GetDregOrDeref(op3), archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
/* 
 *    Majedul: Vector-cmps are not supported in SSE, only 
 *    supported by AVX
 */         
      case VDCMPWGT:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$0x0E,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
/*             SSE: GT is replaced with NLE */            
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmppd\t$6,%s,%s\n",  
                          GetDregOrDeref(op3), archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPWGE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$0x0D,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
/*             SSE: GE is replaced with NLT */            
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmppd\t$5,%s,%s\n",  
                          GetDregOrDeref(op3), archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
#if 0         
      case VDCMPWNLT:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$5,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmppd\t$5,%s,%s\n",  
                          GetDregOrDeref(op3), archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPWNLE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$6,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tcmppd\t$6,%s,%s\n",  
                          GetDregOrDeref(op3), archvdregs[-VDREGBEG-op1]);
         #endif 
         break;
      case VDCMPWNGT:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$0x0A,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VDCMPWNGT only supported in AVX!");
         #endif 
         break;
      case VDCMPWNGE:
         #ifdef AVX
            ap->next = PrintAssln("\tvcmppd\t$9,%s,%s,%s\n",
                                  GetDregOrDeref(op3),
                                  archvdregs[-VDREGBEG-op2],
                                  archvdregs[-VDREGBEG-op1]);
         #else
	    fko_error(__LINE__, "VDCMPWNGE only supported in AVX!");
         #endif 
         break;
#endif         
/*
 *    Mov masks the sign bits of all doubles to low 2/4 bits of ireg 
 */
      case VDSBTI:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovmskpd\t%s,%s\n",
                                   archvdregs[-VDREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);      
         # else
            ap->next = PrintAssln("\tmovmskpd\t%s,%s\n",
                                   archvdregs[-VDREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);
         #endif
         break;
/*
 *    Conditional MOV or, Select operation.
 *    NOTE: these instructions are only supported in SSE4.1 (or, above) and AVX
 *    AVX: 
 *             vblendvps ymm1, ymm2, ymm3/m256, ymm4
 *    SSE4.1: 
 *             blendvps xmm1, xmm2/m128, <XMM0>
 *    
 *    NOTE: There are two problems with SSE's inst:
 *          1) XMM0 must be used as the mask which may complicate the ra.
 *          2) dest is always set as src1, can't implement 2 version without
 *             changing the mask (need to compute mask' )
 *          
 *    VFCMOV1:  vreg0, vreg1/mem, vreg2    # vreg0 = vreg2? vreg0 : vreg1         
 *    VFCMOV2:  vreg0, vreg1, vreg2        # vreg0 = vreg2? vreg1 : vreg0
 *
 *    NOTE: for 2nd version, no mem can be used. in AVX, only src2 can be mem.
 *    but here, we use dest as src2 and it can't be mem.
 */
      case VFCMOV1:
         #ifdef X86
            #if defined(AVX) 
               assert( (op1 < 0) && (op3 < 0));
               if (op2 < 0) /* only src2(here op2) can be mem */
                  ap->next = PrintAssln("\tvblendvps\t%s,%s,%s,%s\n", 
	                                archvfregs[-VFREGBEG-op3],
                                        archvfregs[-VFREGBEG-op2],
	                                archvfregs[-VFREGBEG-op1],
	                                archvfregs[-VFREGBEG-op1]);
               else
                  ap->next = PrintAssln("\tvblendvps\t%s,%s,%s,%s\n", 
	                                archvfregs[-VFREGBEG-op3],
                                        GetDregOrDeref(op2),
	                                archvfregs[-VFREGBEG-op1],
	                                archvfregs[-VFREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV is Not found in this x86 arch!");
            #endif
         #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV Not supported in this arch!");
         #endif
         break;
      case VFCMOV2:
         #ifdef X86
            #if defined(AVX) 
               assert( (op1 < 0) && (op3 < 0) && (op2 < 0));
                  ap->next = PrintAssln("\tvblendvps\t%s,%s,%s,%s\n", 
	                                archvfregs[-VFREGBEG-op3],
                                        archvfregs[-VFREGBEG-op1],
	                                archvfregs[-VFREGBEG-op2],
	                                archvfregs[-VFREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV is Not found in this x86 arch!");
            #endif
         #else
               ap->next = PrintAssln("\tUNIMP\n");
	       fko_error(__LINE__, "This CMOV Not supported in this arch!");
         #endif
         break;

      case VFSHUF:
         cp = imap2cmap(SToff[op3-1].i); /* return hex char */ 
/*       
 *       Redefining cp for AVX
 *       cp represents the position of destination:
 *       cp7,cp6,cp5,cp4,cp3,cp2,cp1,cp0   rd[7,6,5,4,3,2,1,0]
 *       value of cp can be 0~15: 0~7 represents float of rd; 
 *       8~15 represents float of rs.
 *       Right now we need following combination:
 */     
         #ifdef AVX
#if 1         
/*
 *          Optimizing following specific combination as a special case...
 *          when ever we find 7654xxxx we can use sse instruction
 *          7654 7654
 *          7654 9180
 *          7654 9810
 *          7654 3211
 *          7654 3322
 *          7654 3232
 *          
 *          FIXME: legacy SSE may get converted into VEX.128 instruction using
 *          -mavx. But all VEX.128 zeros the upper-half of the YMM register
 */
            /* 7654 7654 */
            if (cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4 &&
                cp[3] == 7 && cp[2] == 6 && cp[1] == 5 && cp[0] == 4 )
               ap->next = PrintAssln("\tvextractf128\t$1,%s,%s\n",
                                     archvfregs[-VFREGBEG-op1], 
                                     archxmmregs[-VFREGBEG-op1]);
            /* 7654 9180 */
            else if (cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4 &&
                     cp[3] == 9 && cp[2] == 1 && cp[1] == 8 && cp[0] == 0)
               ap->next = PrintAssln("\tunpcklps\t%s,%s\n",
                                     archxmmregs[-VFREGBEG-op2], 
                                     archxmmregs[-VFREGBEG-op1]);
            /* 7654 9810 */
            else if (cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4 &&
                     cp[3] == 9 && cp[2] == 8 && cp[1] == 1 && cp[0] == 0)
               ap->next = PrintAssln("\tmovlhps\t%s,%s\n",
                                     archxmmregs[-VFREGBEG-op2], 
                                     archxmmregs[-VFREGBEG-op1]);
            /* 7654 3211 */
            else if (cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4 &&
                     cp[3] == 3 && cp[2] == 2 && cp[1] == 1 && cp[0] == 1)
            {
               assert(op1 == op2);
               ap->next = PrintAssln("\tshufps\t$%d,%s,%s\n", 0xe5,
                                     archxmmregs[-VFREGBEG-op1], 
                                     archxmmregs[-VFREGBEG-op1]);
            }
            /* 7654 3322 */
            else if (cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4 &&
                     cp[3] == 3 && cp[2] == 3 && cp[1] == 2 && cp[0] == 2)
            {
               assert(op1 == op2);
               ap->next = PrintAssln("\tunpckhps\t%s,%s\n",
                                     archxmmregs[-VFREGBEG-op1], 
                                     archxmmregs[-VFREGBEG-op1]);
            }
            /* 7654 3232 */
            else if (cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4 &&
                     cp[3] == 3 && cp[2] == 2 && cp[1] == 3 && cp[0] == 2)
            {
               assert(op1 == op2);
               ap->next = PrintAssln("\tmovhlps\t%s,%s\n",
                                     archxmmregs[-VFREGBEG-op1], 
                                     archxmmregs[-VFREGBEG-op1]);

            }
            /* BA983210*/
            else if (cp[7] == 11 && cp[6] == 10 && cp[5] == 9 && cp[4] == 8 &&
                     cp[3] == 3 && cp[2] == 2 && cp[1] == 1 && cp[0] == 0)
               ap->next = PrintAssln("\tvperm2f128\t$0x02,%s,%s,%s\n",
                                     archvfregs[-VFREGBEG-op1], /* src2*/ 
                                     archvfregs[-VFREGBEG-op2], 
                                     archvfregs[-VFREGBEG-op1]);
            else
#endif
            if (op1 == op2)
            {
               j=0;
               for (i=0; i<8; i++)
               { 
                  if (cp[i])
                  {
                     j=1;
                     if(cp[i] > 7) /* no need to consider src */
                        cp[i] -= 8;
                  }
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
               else if (cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4)
               {
                  if (cp[3] > 3 && cp[2] > 3 && cp[1] > 3 && cp[0] > 3)  
                  {
                     ap->next = PrintAssln("\tvperm2f128\t$%d,%s,%s,%s\n", 
                                           0x11,
                                           archvfregs[-VFREGBEG-op1], 
                                           archvfregs[-VFREGBEG-op1], 
                                           archvfregs[-VFREGBEG-op1]);
                     ap=ap->next;
                     cp[0] -= 4;
                     cp[1] -= 4;
                     cp[2] -= 4;
                     cp[3] -= 4;
                  }
                  if (cp[3] > 3 || cp[2] > 3 || cp[1] > 3 || cp[0] > 3) 
                     fko_error(__LINE__, "not implemented VFSHUF: %x", 
                                  SToff[op3-1].i);
                     
                  assert(cp[0]>=0 && cp[1]>=0 && cp[2]>=0 && cp[3] >=0);
                  i = cp[0] | (cp[1]<<2) | (cp[2]<<4) | (cp[3]<<6); 
                  ap->next = PrintAssln("\tshufps\t$%d,%s,%s\n", i,
                                        archxmmregs[-VFREGBEG-op1], 
                                        archxmmregs[-VFREGBEG-op1]);
               }
               else
               {
                  fko_error(__LINE__, "not implemented VFSHUF (op1==op2): %x", 
                            SToff[op3-1].i);
               }
            }
            else /* op1 != op2 */
            {
/*            NOTE: we have only implemented those combination which we need! 
 *            Implemented combination:
 *                7654FEDC
 *                765432BA
 *                76CD3289
 */             
               if (cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4)   
               {
                  if (cp[3] == 15 && cp[2] == 14 && cp[1] == 13 && cp[0] == 12)
                     ap->next = PrintAssln("\tvperm2f128\t$0x31,%s,%s,%s\n",
                                           archvfregs[-VFREGBEG-op1], /* src2*/ 
                                           archvfregs[-VFREGBEG-op2], 
                                           archvfregs[-VFREGBEG-op1]);
                  else if (cp[3] == 9 && cp[2] == 1 && cp[1] == 8 && cp[0] == 0)
                     ap->next = PrintAssln("\tunpcklps\t%s,%s\n",
                                           archxmmregs[-VFREGBEG-op2], 
                                           archxmmregs[-VFREGBEG-op1]);
                  else if (cp[3] == 9 && cp[2] == 8 && cp[1] == 1 && cp[0] == 0)
                     ap->next = PrintAssln("\tmovlhps\t%s,%s\n",
                                           archxmmregs[-VFREGBEG-op2], 
                                           archxmmregs[-VFREGBEG-op1]);
/*
 *                FIXED: op1[255:128] may become zero using vmovhlps. 
 *                use movhlps to keep the upper value unchanged!
 */
                  else if (cp[3] == 3 && cp[2] == 2 && cp[1] == 11 
                           && cp[0] == 10)  
                     /*ap->next = PrintAssln("\tvmovhlps\t%s,%s,%s\n",
                                           archxmmregs[-VFREGBEG-op2], 
                                           archxmmregs[-VFREGBEG-op1], 
                                           archxmmregs[-VFREGBEG-op1]);*/
                     ap->next = PrintAssln("\tmovhlps\t%s,%s\n",
                                           archxmmregs[-VFREGBEG-op2], 
                                           archxmmregs[-VFREGBEG-op1]);
                  else
                     fko_error(__LINE__, "Not implemented this VFSHUF yet: %x",
                               SToff[op3-1].i);
               }
/*             Implementing 3210BA98*/
               else if (cp[7] == 3 && cp[6] == 2 && cp[5] == 1 && cp[4] == 0 &&
                        cp[3] == 11 && cp[2] == 10 && cp[1] == 9 && cp[0] == 8)
                     ap->next = PrintAssln("\tvperm2f128\t$0x20,%s,%s,%s\n",
                                           archvfregs[-VFREGBEG-op1], /* src2*/ 
                                           archvfregs[-VFREGBEG-op2], 
                                           archvfregs[-VFREGBEG-op1]);
/*             Implementing 76CD3289*/
               else if (cp[7] == 7 && cp[6] == 6 && cp[5] == 12 && cp[4] == 13 &&
                        cp[3] == 3 && cp[2] == 2 && cp[1] == 8 && cp[0] == 9)
                  ap->next = PrintAssln("\tvshufps\t$0xE1,%s,%s,%s\n",
                                         archvfregs[-VFREGBEG-op1], /* src2*/
                                         archvfregs[-VFREGBEG-op2],
                                         archvfregs[-VFREGBEG-op1]);
               else
                fko_error(__LINE__, "Not implemented this VFSHUF yet");
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
                  fko_error(__LINE__, "Useless VFSHUF");
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
/*
 *    Two special shuffle, specially needed in avx
 */
      case VFHIHALF:
/*
 *       vr0[1,0], vr1[1,0], vr2[1,0]: vr0[0] = vr1[1]; vr0[1] = vr2[1]; 
 */
         #ifdef AVX
            if ( op3 > 0)
               ap->next = PrintAssln("\tvperm2f128\t$0x31,%s,%s,%s\n",
                                     GetDeref(op3), 
                                     archvfregs[-VFREGBEG-op2], 
                                     archvfregs[-VFREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvperm2f128\t$0x31,%s,%s,%s\n",
                                     archvfregs[-VFREGBEG-op3], 
                                     archvfregs[-VFREGBEG-op2], 
                                     archvfregs[-VFREGBEG-op1]);
            
         #else
            fko_error(__LINE__, "Not implemented this instruction in SSE yet!");
         #endif
         break;

      case VFLOHALF:
/*
 *       vr0[1,0], vr1[1,0], vr2[1,0]: vr0[0] = vr1[0]; vr0[1] = vr2[0]; 
 */
         #ifdef AVX
            if ( op3 > 0)
               ap->next = PrintAssln("\tvperm2f128\t$0x20,%s,%s,%s\n",
                                     GetDeref(op3), 
                                     archvfregs[-VFREGBEG-op2], 
                                     archvfregs[-VFREGBEG-op1]);
            else
               ap->next = PrintAssln("\tvperm2f128\t$0x20,%s,%s,%s\n",
                                     archvfregs[-VFREGBEG-op3], 
                                     archvfregs[-VFREGBEG-op2], 
                                     archvfregs[-VFREGBEG-op1]);
            
         #else
            fko_error(__LINE__, "Not implemented this instruction in SSE yet!");
         #endif
         break;

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
 * Majedul: Instruction to convert the mask value. 
 * NOTE: scalar version of VFSBTI and VDSBTI. Don't find the scalar inst... 
 * so, implement with composit inst. LSB bit of ireg has the result
 */
#ifdef X86
      case CVTMASKFI:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovmskps\t%s,%s\n",
                                   archvfregs[-VFREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);    
         # else
            ap->next = PrintAssln("\tmovmskps\t%s,%s\n",
                                   archvfregs[-VFREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);
         #endif

         #ifdef X86_64
            ap->next = PrintAssln("\tandq\t$1, %s\n",archiregs[-IREGBEG-op1]);
         #elif defined(X86)
            ap->next = PrintAssln("\tandl\t$1, %s\n",archiregs[-IREGBEG-op1]);
         #endif
         break;
      case CVTMASKDI:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovmskpd\t%s,%s\n",
                                   archvdregs[-VDREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);      
         # else
            ap->next = PrintAssln("\tmovmskpd\t%s,%s\n",
                                   archvdregs[-VDREGBEG-op2],
                                   archiregs[-IREGBEG-op1]);
         #endif
         #ifdef X86_64
            ap->next = PrintAssln("\tandq\t$1, %s\n",archiregs[-IREGBEG-op1]);
         #elif defined(X86)
            ap->next = PrintAssln("\tandl\t$1, %s\n",archiregs[-IREGBEG-op1]);
         #endif
         break;
/*
 * Majedul: some instruction for V_INT type operation
 * NOTE: implementing two versions of int vector instructions 
 */
/* NOTE: affectively VSMOV and VIMOV are same as they mov the whole vector
 * I use VMOV for that
 */        
      /*case VSMOV: case VIMOV:*/
   case VMOV:
/*
 *       Majedul: although double quadword means 128 bit, vmovdqa can mov 256bit
 *       packed integer. unaligned version of this instrucion is vmovdqu 
 */
         #ifdef AVX
            ap->next = PrintAssln("\tvmovdqa\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archviregs[-VIREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovdqa\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VSMOVS: /* 32 bit int */
/*
 *       Allow vector-to-fp regs for reg asg opt
 *       two variation of these inst
 * 1. VFMOVS vr0, sr, vr1  // vr0[0] = vr2[0], vr0[vlen-1: 1] = vr1[vlen-1: 1]
 * 2. VFMOVS sr, vr, 0     // sr = vr[0]
 *       here vr0 and vr1 should be vector register
 */
         op1 = -op1;
         op2 = -op2;
         op3 = op3 ? -op3: op3;
         
         if (op1 >= VIREGBEG && op1 < VIREGEND)
         {
            assert(op2 >= IREGBEG && op2 < IREGEND); /* ireg to vireg */
         
            op1 = -op1;
            op2 = -op2;
            #ifdef AVX
               #ifdef X86_64
                  ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                        archsregs[-IREGBEG-op2],
                                        archxmmregs[-VIREGBEG-op1]);
               #else
                  ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                        archiregs[-IREGBEG-op2],
                                        archxmmregs[-VIREGBEG-op1]);
               #endif
            #else
               #ifdef X86_64
                  ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                        archsregs[-IREGBEG-op2],
                                        archviregs[-VIREGBEG-op1]);
               #else
                  ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                     archiregs[-IREGBEG-op2],
                                     archviregs[-VIREGBEG-op1]);
               #endif
            #endif
         }
/*
 *       vr-to-sr: VFMOVS sr, vr0, 0
 */
         else if (op1 >= IREGBEG && op1 < IREGEND)
         {
            assert(op2 >= VIREGBEG && op2 < VIREGEND);
            assert(!op3);
         
            op1 = -op1;
            op2 = -op2;
         #ifdef AVX
            #ifdef X86_64
            assert((-op1) <= (IREGBEG+NSR));
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                  archxmmregs[-VIREGBEG-op2],
                                  archsregs[-IREGBEG-op1]);
            #else
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                  archxmmregs[-VIREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
            #endif
         #else
            #ifdef X86_64
            ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archsregs[-IREGBEG-op1]);
            #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
            #endif
         #endif
         }
#if 0
         assert (op1 < 0 && op2 < 0);
         op1 = -op1;
         op2 = -op2;
         if ( (op1 >= IREGBEG && op1 < IREGEND) 
               && (op2 >= VIREGBEG && op2 < VIREGEND) ) /* vireg to ireg */
         {
            op1 = -op1;
            op2 = -op2;
         #ifdef AVX
            #if X86_64
            assert((-op1) <= (IREGBEG+NSR));
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                  archxmmregs[-VIREGBEG-op2],
                                  archsregs[-IREGBEG-op1]);
            #else
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                  archxmmregs[-VIREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
            #endif
         #else
            #ifdef X86_64
            ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archsregs[-IREGBEG-op1]);
            #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
            #endif
         #endif
         }
         else if ( (op1 >= VIREGBEG && op1 < VIREGEND) 
               && (op2 >= IREGBEG && op2 < IREGEND) ) /* ireg to vireg, why? */
         {
            op1 = -op1;
            op2 = -op2;
         #ifdef AVX
            #ifdef X86_64
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                  archsregs[-IREGBEG-op2],
                                  archxmmregs[-VIREGBEG-op1]);
            #else
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                  archiregs[-IREGBEG-op2],
                                  archxmmregs[-VIREGBEG-op1]);
            #endif
         #else
            #ifdef X86_64
            ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                  archsregs[-IREGBEG-op2],
                                  archviregs[-VIREGBEG-op1]);
            #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                  archiregs[-IREGBEG-op2],
                                  archviregs[-VIREGBEG-op1]);
            #endif
         #endif
         }
         else 
            fko_error(__LINE__, "Undefined operands for VMOVS inst!!\n");

#endif         
         break;
   case VIMOVS:
/*
 *       Allow scalar to vector register move 
 *       two variation of these inst
 * 1. VIMOVS vr0, sr, vr1  // vr0[0] = vr2[0], vr0[vlen-1: 1] = vr1[vlen-1: 1]
 * 2. VIMOVS sr, vr, 0     // sr = vr[0]
 *       here vr0 and vr1 should be vector register
 */
         op1 = -op1;
         op2 = -op2;
         op3 = op3 ? -op3: op3;
/*
 *       sr-to-vr: VFMOVS vr0, sr, vr1 
 */
         if (op1 >= VIREGBEG && op1 < VIREGEND) 
         {
            assert(op1 == op3);
            assert(op2 >= IREGBEG && op2 < IREGEND); /* ireg to vireg */
         
            op1 = -op1;
            op2 = -op2;
            #ifdef AVX
               ap->next = PrintAssln("\tvmovq\t%s, %s\n", 
                                     archiregs[-IREGBEG-op2],
                                     archxmmregs[-VIREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                     archiregs[-IREGBEG-op2],
                                     archviregs[-VIREGBEG-op1]);
            #endif
         }
/*
 *       vr-to-sr: VFMOVS sr, vr0, 0
 */
         else if (op1 >= IREGBEG && op1 < IREGEND)
         {
            assert(op2 >= VIREGBEG && op2 < VIREGEND);
            assert(!op3);
         
            op1 = -op1;
            op2 = -op2;
            #ifdef AVX
               ap->next = PrintAssln("\tvmovq\t%s, %s\n", 
                                     archxmmregs[-VIREGBEG-op2],
                                     archiregs[-IREGBEG-op1]);
            #else
               ap->next = PrintAssln("\tmovq\t%s, %s\n", 
                                     archviregs[-VIREGBEG-op2],
                                     archiregs[-IREGBEG-op1]);
            #endif
         }
#if 0         
         assert (op1 < 0 && op2 < 0);
         op1 = -op1;
         op2 = -op2;
         if ( (op1 >= IREGBEG && op1 < IREGEND) 
               && (op2 >= VIREGBEG && op2 < VIREGEND) ) /* vireg to ireg */
         {
            op1 = -op1;
            op2 = -op2;
         #ifdef AVX
            ap->next = PrintAssln("\tvmovq\t%s, %s\n", 
                                  archxmmregs[-VIREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovq\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
         #endif
         }
         else if ( (op1 >= VIREGBEG && op1 < VIREGEND) 
               && (op2 >= IREGBEG && op2 < IREGEND) ) /* ireg to vireg */
         {
            op1 = -op1;
            op2 = -op2;
         #ifdef AVX
            ap->next = PrintAssln("\tvmovq\t%s, %s\n", 
                                  archiregs[-IREGBEG-op2],
                                  archxmmregs[-VIREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                  archiregs[-IREGBEG-op2],
                                  archviregs[-VIREGBEG-op1]);
         #endif
         }
         else 
            fko_error(__LINE__, "Undefined operands for VMOVS inst!!\n");
#endif
         break;
/*
 * VSLD and VILD affectively same for avx2. so, we use one instruction
 */
   /*case VSLD: case VILD:*/
   case VLD:
         /*fprintf(stderr, "op1=%d, op2=%d\n", op1, op2);*/
         assert(op1 < 0 && op2 >= 0);
         #ifdef AVX
            ap->next = PrintAssln("\tvmovdqa\t%s, %s\n", GetDeref(op2),
                                  archviregs[-VIREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovdqa\t%s, %s\n", GetDeref(op2),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VLDU:
         /*fprintf(stderr, "op1=%d, op2=%d\n", op1, op2);*/
         assert(op1 < 0 && op2 >= 0);
         #ifdef AVX
            ap->next = PrintAssln("\tvmovdqu\t%s, %s\n", GetDeref(op2),
                                  archviregs[-VIREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovdqu\t%s, %s\n", GetDeref(op2),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VSLDS:
         assert(op1 < 0 && op2 >= 0);
         #ifdef AVX
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", GetDeref(op2),
                                  archxmmregs[-VIREGBEG-op1]); 
         #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n", GetDeref(op2),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VILDS:
         assert(op1 < 0 && op2 >= 0);
         #ifdef AVX
            ap->next = PrintAssln("\tvmovq\t%s, %s\n", GetDeref(op2),
                                  archxmmregs[-VIREGBEG-op1]); 
         #else
            ap->next = PrintAssln("\tmovq\t%s, %s\n", GetDeref(op2),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   /*case VSST: case VIST:*/
   case VST:
         assert(op2 < 0 && op1 >= 0);
         #ifdef AVX
            ap->next = PrintAssln("\tvmovdqa\t%s, %s\n", 
                  archviregs[-VIREGBEG-op2], GetDeref(op1) );
         #else
            ap->next = PrintAssln("\tmovdqa\t%s, %s\n", 
                  archviregs[-VIREGBEG-op2], GetDeref(op1) );
         #endif
         break;
   case VSTU:
         assert(op2 < 0 && op1 >= 0);
         #ifdef AVX
            ap->next = PrintAssln("\tvmovdqu\t%s, %s\n", 
                  archviregs[-VIREGBEG-op2], GetDeref(op1) );
         #else
            ap->next = PrintAssln("\tmovdqu\t%s, %s\n", 
                  archviregs[-VIREGBEG-op2], GetDeref(op1) );
         #endif
         break;
   case VSSTS:
         assert(op2 < 0 && op1 >= 0);
         #ifdef AVX
            ap->next = PrintAssln("\tvmovd\t%s, %s\n",
                  archxmmregs[-VIREGBEG-op2], GetDeref(op1)); 
         #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n",
                  archviregs[-VIREGBEG-op2], GetDeref(op1) );
         #endif
         break;
   case VISTS:
         assert(op2 < 0 && op1 >= 0);
         #ifdef AVX
            ap->next = PrintAssln("\tvmovq\t%s, %s\n",
                  archxmmregs[-VIREGBEG-op2], GetDeref(op1)); 
         #else
            ap->next = PrintAssln("\tmovq\t%s, %s\n",
                  archviregs[-VIREGBEG-op2], GetDeref(op1) );
         #endif
         break;
   case VSZERO : case VIZERO:
/*
 *       For AVX: intel manual said, it would zerod the upper 128 bit of 
 *       destination!!!
 *       ref: inter(R) 64 and IA-32 Architecures s/w developer's manual, 
 *       june 2011, page 1652 ... ... ... ... ..... 
 *       But supported in AVX2 .................. 
 */
         #ifdef AVX
            #ifdef AVX2 /* need AVX2 */
            ap->next = PrintAssln("\tvpxor\t%s,%s, %s\n", 
                                  archviregs[-VIREGBEG-op1],
                                  archviregs[-VIREGBEG-op1],
                                  archviregs[-VIREGBEG-op1]); 
            #else
            ap->next = PrintAssln("\tvpxor\t%s,%s, %s\n", 
                                  archxmmregs[-VIREGBEG-op1],
                                  archxmmregs[-VIREGBEG-op1],
                                  archxmmregs[-VIREGBEG-op1]); 
            #endif
         #else
            ap->next = PrintAssln("\tpxor\t%s,%s\n", archviregs[-VIREGBEG-op1],
                                  archviregs[-VIREGBEG-op1]);
         #endif   
         break;
   case VSADD:
/*
 *       Note: need to use AVX2 for this functionality to operate on 256 bit 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vadd is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpaddd\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpaddd\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VIADD:
/*
 *       Note: need to use AVX2 for this functionality to operate on 256 bit 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vadd is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpaddq\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpaddq\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VSSUB:
/*
 *       Note: need to use AVX2 for this functionality to operate on 256 bit 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vsub is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpsubd\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpsubd\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VISUB:
/*
 *       Note: need to use AVX2 for this functionality to operate on 256 bit 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vsub is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpsubq\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpsubq\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VSMAX: /* signed max */
/*
 *       condidering max on packed signed integers 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vmax is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpmaxsd\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpmaxsd\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VIMAX: /*???? AVX2 doesn't have max inst for 64bit?????? */
         assert(0);
         break;
   case VSMIN:
/*
 *       condidering max on packed signed integers 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vmax is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpminsd\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpminsd\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
   case VIMIN: /*???? AVX2 doesn't have max inst for 64bit?????? */
         assert(0);
         break;
   case VSSHUF:
/*
 *       Need to implement following options:
 *       
 *          AVX: 0, 7654FEDC, 765432BA/7654BABA, 76CD3289/7654BA99, B3A29180, 
 *          SSE: 0, 3276, 5555, 5140
 *          FIXME
 */
         cp = imap2cmap(SToff[op3-1].i); /* return hex char */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, 
                      "vishuf is not implemented in AVX. AVX2 needed!\n"); 
         
         #elif defined(AVX2)   
            if (!SToff[op3-1].i)
            {
               assert(op1==op2);
               ap->next = PrintAssln("\tvperm2i128\t$%d,%s,%s,%s\n", 0,
                                     archviregs[-VIREGBEG-op1], 
                                     archviregs[-VIREGBEG-op1], 
                                     archviregs[-VIREGBEG-op1]);
               ap=ap->next;
                  ap->next = PrintAssln("\tvpshufd\t$%d,%s,%s\n", 0,
                                        archviregs[-VIREGBEG-op1], 
                                        archviregs[-VIREGBEG-op1]);
            }
/*
 *          0x7654 FEDC
 *          0x7654 BABA
 *          0x7654 BA99
 */
            else if(cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4)    
            {
               if (cp[3] == 15 && cp[2] == 14 && cp[1] == 13 && cp[0] == 12)
               {
                  ap->next = PrintAssln("\tvperm2i128\t$0x31,%s,%s,%s\n",
                                        archviregs[-VIREGBEG-op1], /* src2*/ 
                                        archviregs[-VIREGBEG-op2], 
                                        archviregs[-VIREGBEG-op1]);
               }
               else if (cp[3] == 11 && cp[2] == 10 && cp[1] == 11 && cp[0] == 10)
               {
                  ap->next = PrintAssln("\tpshufd\t$%d,%s,%s\n", 0xEE,
                                        archxmmregs[-VIREGBEG-op2], 
                                        archxmmregs[-VIREGBEG-op1]);
               }
               else if (cp[3] == 11 && cp[2] == 10 && cp[1] == 9 && cp[0] == 9)
               {
                  ap->next = PrintAssln("\tpshufd\t$%d,%s,%s\n", 0xE5,
                                        archxmmregs[-VIREGBEG-op2], 
                                        archxmmregs[-VIREGBEG-op1]);
               }
               else
                fko_error(__LINE__, "Not implemented this VISHUF yet");
            }
/*
 *          0xBA98 3210
 *          0xD5C4 9180
 */
            else if(cp[7] == 11 && cp[6] == 10 && cp[5] == 9 && cp[4] == 8)    
            {
               if (cp[3] == 3 && cp[2] == 2 && cp[1] == 1 && cp[0] == 0)
               {
                  ap->next = PrintAssln("\tvperm2i128\t$0x20,%s,%s,%s\n",
                                        archviregs[-VIREGBEG-op2], /*src2==dest*/ 
                                        archviregs[-VIREGBEG-op1], 
                                        archviregs[-VIREGBEG-op1]);
               }
            }
            else if(cp[7] == 13 && cp[6] == 5 && cp[5] == 12 && cp[4] == 4)    
            {
               if (cp[3] == 9 && cp[2] == 1 && cp[1] == 8 && cp[0] == 0)
               {
                  ap->next = PrintAssln("\tvpunpckldq\t%s,%s,%s\n",
                                        archviregs[-VIREGBEG-op2], /* src2*/ 
                                        archviregs[-VIREGBEG-op1], 
                                        archviregs[-VIREGBEG-op1]);
               }
            }
            else
            fko_error(__LINE__, "Not implemented this vishuf yet!\n"); 

         #else
/*
 *       SSE: 0, 3276, 5555, 5140
 *       NOTE: not implemented for SSE... need to explore later
 */
            fko_error(__LINE__, "Not implemented VISHUF for SSE yet");
         #endif
         break;
      
      case VISHUF:
         cp = imap2cmap(SToff[op3-1].i);
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "AVX2 is needed !!!\n"); 
/*
 *          Generalized combination :
 *          cp represents the position of destination:
 *          cp3,cp2,cp1,cp0   rd[3,2,1,0]
 *          value of cp can be 0~7: 0~3 represents int64 of rd; 
 *          4~7 represents double of rs.
 *          NOTE: we are using permute operation which permutes source and 
 *          stores it in destination.
 *
 *          Right now we need following combination (X means don't care):
 *             0x 0000
 *             0x XX76
 *             0x XXX5
 *          NOTE: we don't have shuffle instruction for 64 bit int in avx2. So,
 *          we will use vpermq instruction. We can implement following 
 *          combination: 
 *             0x 7676
 *             0x 7655
 *
 */         
         #elif defined(AVX2)   
            if (!SToff[op3-1].i) /* ox0000 */
            {
               assert(op1==op2);
               /*
               ap->next = PrintAssln("\tvperm2i128\t$%d,%s,%s,%s\n", 0,
                                     archviregs[-VIREGBEG-op1], 
                                     archviregs[-VIREGBEG-op1], 
                                     archviregs[-VIREGBEG-op1]);
               ap=ap->next;
                  ap->next = PrintAssln("\tvpshufq\t$%d,%s,%s\n", 0,
                                        archviregs[-VIREGBEG-op1], 
                                        archviregs[-VIREGBEG-op1]);
               */
               ap->next = PrintAssln("\tvpermq\t$%d,%s,%s\n", 0,
                                     archviregs[-VIREGBEG-op2], 
                                     archviregs[-VIREGBEG-op1]);

            }
            else if (cp[3] == 7 && cp[2] == 6 && cp[1] == 7 && cp[0] == 6)
            {
               ap->next = PrintAssln("\tvpermq\t$%d,%s,%s\n", 0xEE,
                                     archviregs[-VIREGBEG-op2], 
                                     archviregs[-VIREGBEG-op1]);
            }
            else if (cp[3] == 7 && cp[2] == 6 && cp[1] == 5 && cp[0] == 5)
            {      
               ap->next = PrintAssln("\tvpermq\t$%d,%s,%s\n", 0xE5,
                                     archviregs[-VIREGBEG-op2], 
                                     archviregs[-VIREGBEG-op1]);
            }
            else if (cp[3] == 5 && cp[2] == 4 && cp[1] == 1 && cp[0] == 0)
            {
               ap->next = PrintAssln("\tvperm2i128\t$%d,%s,%s,%s\n", 0x20,
                                     archviregs[-VIREGBEG-op2], 
                                     archviregs[-VIREGBEG-op1], 
                                     archviregs[-VIREGBEG-op1]);
            }
            else
               fko_error(__LINE__, "No such shuffle inst, imap=%d,%d,%d,%d!\n",
                         cp[0], cp[1], cp[2], cp[3]);
         #endif
         break;
/*
 *    special instruction to set conditional codes in X86
 *    BTC : bit test and update CF flag
 */
      case BTC:   
         op1 = -op1;
         assert( op1>=ICCBEG && op1<ICCEND);
         ap->next = PrintAssln("\tbt\t%s,%s\n", 
                                 GetIregOrConst(op3),
	                         archiregs[-IREGBEG-op2]);
          
         break;
/*
 *    Majedul: CMOV1 / CMOV2 ... conditional mov for integer based on 
 *    conditional codes. we need to apply BT before it.
 */
      case CMOV1:  /* dest = (mask==0)? dest: src */
         op3 = -op3;
         assert(op3 >= ICCBEG && op3 < ICCEND);
         ap->next = PrintAssln("\tcmovc\t%s, %s\n", GetIregOrDeref(op2), 
                              archiregs[-IREGBEG-op1]);
         break;
      case CMOV2:  /* dest = (mask==0)? src: dest */
         op3 = -op3;
         assert(op3 >= ICCBEG && op3 < ICCEND);
         ap->next = PrintAssln("\tcmovnc\t%s, %s\n", GetIregOrDeref(op2), 
                              archiregs[-IREGBEG-op1]);
         break;
/* --------------------------------------------------------------------*/      
   #ifdef AVX2  
/*
 * Read this note before using following two instructions
 * =========================== NOTE ===========================================
 *    Majedul: hybrid inst using mask from floating point comparison! 
 *    in AVX/SSE, we can do it as the mask value has all 1 or zero for each 
 *    element. 
 *    Need to consider following issue carefully:
 *    1. For single precision float, we will consider mask 32 bit 
 *    2. for double precision, we will consider mask 64 bit
 *    3. as long as we prepare right mask value, vpblendvb works for both
 * ============================================================================
 */
      case VSCMOV1: case VICMOV1:  /* Hybrid .... dest = (mask==0)? dest: src */
         op3 = -op3;
/*
 *       NOTE: we are implementing this using vpblendvb (AVX2) but use imm!!
 *       let's use vpblendvb... should work with the mask
 *       this inst selects byte values from mask specified in the high bit of 
 *       each byte. Here, we use mask with all 1's or 0's in each 32bit/64bit. 
 *       so, it should work.
 *       NOTE: we support no memory operation now.
 */
         if (op3 >= VFREGBEG && op3 < VFREGEND)
         {
            op3 = -op3;
            ap->next = PrintAssln("\tvpblendvb\t%s,%s,%s,%s\n", 
	                          archvfregs[-VFREGBEG-op3],
                                  archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1],
	                          archviregs[-VIREGBEG-op1]); 
         }
         else if (op3 >= VDREGBEG && op3 < VDREGEND)
         {
            op3 = -op3;
            ap->next = PrintAssln("\tvpblendvb\t%s,%s,%s,%s\n", 
	                          archvdregs[-VDREGBEG-op3],
                                  archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1],
	                          archviregs[-VIREGBEG-op1]); 
         }
/*
 *       NOTE: we don't have vmax/vmin instruction, not even in avx2. So, we
 *       need to use vcmov to simulate the vmax instruction. In that case, 
 *       mask can be T_VINT type!!!
 *
 */
         else if (op3 >= VIREGBEG && op3 < VIREGEND)
         {
            /*fko_error(__LINE__,"Expecting Mask on floating point regs:%d"
                      " VFREG[%d,%d], VDREG[%d,%d], VIREG[%d,%d]\n",
                      op3, VFREGBEG, VFREGEND, VDREGBEG, VDREGEND, VIREGBEG,
                      VIREGEND);*/
            op3 = -op3;
            ap->next = PrintAssln("\tvpblendvb\t%s,%s,%s,%s\n", 
	                          archvdregs[-VIREGBEG-op3],
                                  archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1],
	                          archviregs[-VIREGBEG-op1]); 
            
         }
         else
            fko_error(__LINE__,"Expecting Mask on floating point regs:%d"
                      " VFREG[%d,%d], VDREG[%d,%d], VIREG[%d,%d]\n",
                      op3, VFREGBEG, VFREGEND, VDREGBEG, VDREGEND, VIREGBEG,
                      VIREGEND);
         
         break;
      case VSCMOV2: case VICMOV2:  /* dest = (mask==0)? src: dest */
         op3 = -op3;
         if (op3 >= VFREGBEG && op3 < VFREGEND)
         {
            op3 = -op3;
            ap->next = PrintAssln("\tvpblendvb\t%s,%s,%s,%s\n", 
	                          archvfregs[-VFREGBEG-op3],
                                  archviregs[-VIREGBEG-op1],
	                          archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1]); 
         }
         else if (op3 >= VDREGBEG && op3 < VDREGEND)
         {
            op3 = -op3;
            ap->next = PrintAssln("\tvpblendvb\t%s,%s,%s,%s\n", 
	                          archvdregs[-VDREGBEG-op3],
                                  archviregs[-VIREGBEG-op1],
	                          archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1]); 
         }
         else
            fko_error(__LINE__, "Expecting Mask value on floating point regs\n");
         
         break;

      case VICMPWGT:
            ap->next = PrintAssln("\tvpcmpgtq\t%s,%s,%s\n", 
	                          archviregs[-VIREGBEG-op3],
                                  archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1]); 
         break;
#endif         

#if 0         
      case VMOV:
/*
 *       Majedul: although double quadword means 128 bit, vmovdqa can mov 256bit
 *       packed integer. unaligned version of this instrucion is vmovdqu 
 */
         #ifdef AVX
            ap->next = PrintAssln("\tvmovdqa\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archviregs[-VIREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovdqa\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;

      case VMOVS: /* this is for 32 bit int */
/*
 *       Majedul: I implement it for both ways: vireg-to-ireg and ireg-to-vireg
 *       FIXME: vmovd works only on 32 bit register
 */
         assert (op1 < 0 && op2 < 0);
         op1 = -op1;
         op2 = -op2;
         if ( (op1 >= IREGBEG && op1 < IREGEND) 
               && (op2 >= VIREGBEG && op2 < VIREGEND) ) /* vireg to ireg */
         {
            op1 = -op1;
            op2 = -op2;
         #ifdef AVX
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                  archxmmregs[-VIREGBEG-op2],
                                  archsregs[-IREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                  archviregs[-VIREGBEG-op2],
                                  archsregs[-IREGBEG-op1]);
         #endif
         }
         else if ( (op1 >= VIREGBEG && op1 < VIREGEND) 
               && (op2 >= IREGBEG && op2 < IREGEND) ) /* ireg to vireg */
         {
            op1 = -op1;
            op2 = -op2;
         #ifdef AVX
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", 
                                  archsregs[-IREGBEG-op2],
                                  archxmmregs[-VIREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n", 
                                  archsregs[-IREGBEG-op2],
                                  archviregs[-VIREGBEG-op1]);
         #endif
         }
         else 
            fko_error(__LINE__, "Undefined operands for VMOVS inst!!\n");

         break;
      case VLD:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovdqa\t%s, %s\n", GetDeref(op2),
                                  archviregs[-VIREGBEG-op1]);
         #else
            ap->next = PrintAssln("\tmovdqa\t%s, %s\n", GetDeref(op2),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
      case VLDS:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovd\t%s, %s\n", GetDeref(op2),
                                  archxmmregs[-VIREGBEG-op1]); 
         #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n", GetDeref(op2),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
      case VST:
         #ifdef AVX
            ap->next = PrintAssln("\tvmovdqa\t%s, %s\n", 
                  archviregs[-VIREGBEG-op2], GetDeref(op1) );
         #else
            ap->next = PrintAssln("\tmovdqa\t%s, %s\n", 
                  archviregs[-VIREGBEG-op2], GetDeref(op1) );
         #endif
         break;
      case VSTS:
/*
 *       FIXME: vmovd works only on 32 bit int but 64 bit in FKO
 */
         #ifdef AVX
            ap->next = PrintAssln("\tvmovd\t%s, %s\n",
                  archxmmregs[-VIREGBEG-op2], GetDeref(op1)); 
         #else
            ap->next = PrintAssln("\tmovd\t%s, %s\n",
                  archviregs[-VIREGBEG-op2], GetDeref(op1) );
         #endif
         break;
      case VIZERO:
/*
 *       For AVX: intel manual said, it would zerod the upper 128 bit of 
 *       destination!!!
 *       ref: inter(R) 64 and IA-32 Architecures s/w developer's manual, 
 *       june 2011, page 1652 ... ... ... ... ..... 
 *       But supported in AVX2 .................. 
 */
         #ifdef AVX
            #ifdef AVX2 /* need AVX2 */
            ap->next = PrintAssln("\tvpxor\t%s,%s, %s\n", 
                                  archviregs[-VIREGBEG-op1],
                                  archviregs[-VIREGBEG-op1],
                                  archviregs[-VIREGBEG-op1]); 
            #else
            ap->next = PrintAssln("\tvpxor\t%s,%s, %s\n", 
                                  archxmmregs[-VIREGBEG-op1],
                                  archxmmregs[-VIREGBEG-op1],
                                  archxmmregs[-VIREGBEG-op1]); 
            #endif
         #else
            ap->next = PrintAssln("\tpxor\t%s,%s\n", archviregs[-VIREGBEG-op1],
                                  archviregs[-VIREGBEG-op1]);
         #endif   
         break;

      case VADD:
/*
 *       Note: need to use AVX2 for this functionality to operate on 256 bit 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vadd is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpaddd\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpaddd\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
      case VSUB:
/*
 *       Note: need to use AVX2 for this functionality to operate on 256 bit 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vsub is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpsubd\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpsubd\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
      case VMAX:
/*
 *       condidering max on packed signed integers 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vmax is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpmaxsd\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpmaxsd\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;
      case VMIN:
/*
 *       condidering max on packed signed integers 
 */
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vmax is not supported in AVX, AVX2 needed!!\n"); 
         #elif defined (AVX2)
            ap->next = PrintAssln("\tvpminsd\t%s,%s,%s\n", 
                                  GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op2], 
                                  archviregs[-VIREGBEG-op1]); 
         #else
            assert(op1 == op2);
            ap->next = PrintAssln("\tpminsd\t%s, %s\n", GetDregOrDeref(op3),
                                  archviregs[-VIREGBEG-op1]);
         #endif
         break;

      case VISHUF:
/*
 *       Need to implement following options:
 *       
 *          AVX: 0, 7654FEDC, 765432BA/7654BABA, 76CD3289/7654BA99, B3A29180, 
 */
         cp = imap2cmap(SToff[op3-1].i); /* return hex char */
#if 0
         fprintf(stderr, "%x%x%x%x%x%x%x%x\n",cp[7],cp[6],cp[5],cp[4],cp[3],
                 cp[2],cp[1],cp[0]);
         //fflush(stderr);
#endif
         #if defined(AVX) && !defined(AVX2)
            fko_error(__LINE__, "vishuf is not implemented in AVX,AVX2 needed!\n"); 
         
         #elif defined(AVX2)   
            if (!SToff[op3-1].i)
            {
               assert(op1==op2);
               ap->next = PrintAssln("\tvperm2i128\t$%d,%s,%s,%s\n", 0,
                                     archviregs[-VIREGBEG-op1], 
                                     archviregs[-VIREGBEG-op1], 
                                     archviregs[-VIREGBEG-op1]);
               ap=ap->next;
                  ap->next = PrintAssln("\tvpshufd\t$%d,%s,%s\n", 0,
                                        archviregs[-VIREGBEG-op1], 
                                        archviregs[-VIREGBEG-op1]);
            }
/*
 *          0x7654 FEDC
 *          0x7654 BABA
 *          0x7654 BA99
 */
            else if(cp[7] == 7 && cp[6] == 6 && cp[5] == 5 && cp[4] == 4)    
            {
               if (cp[3] == 15 && cp[2] == 14 && cp[1] == 13 && cp[0] == 12)
               {
                  ap->next = PrintAssln("\tvperm2i128\t$0x31,%s,%s,%s\n",
                                        archviregs[-VIREGBEG-op1], /* src2*/ 
                                        archviregs[-VIREGBEG-op2], 
                                        archviregs[-VIREGBEG-op1]);
               }
               else if (cp[3] == 11 && cp[2] == 10 && cp[1] == 11 && cp[0] == 10)
               {
                  ap->next = PrintAssln("\tpshufd\t$%d,%s,%s\n", 0xEE,
                                        archxmmregs[-VIREGBEG-op2], 
                                        archxmmregs[-VIREGBEG-op1]);
               }
               else if (cp[3] == 11 && cp[2] == 10 && cp[1] == 9 && cp[0] == 9)
               {
                  ap->next = PrintAssln("\tpshufd\t$%d,%s,%s\n", 0xE5,
                                        archxmmregs[-VIREGBEG-op2], 
                                        archxmmregs[-VIREGBEG-op1]);
               }
               else
                fko_error(__LINE__, "Not implemented this VISHUF yet");
            }
/*
 *          0xBA98 3210
 *          0xD5C4 9180
 */
            else if(cp[7] == 11 && cp[6] == 10 && cp[5] == 9 && cp[4] == 8)    
            {
               if (cp[3] == 3 && cp[2] == 2 && cp[1] == 1 && cp[0] == 0)
               {
                  ap->next = PrintAssln("\tvperm2i128\t$0x20,%s,%s,%s\n",
                                        archviregs[-VIREGBEG-op2], /*src2==dest*/ 
                                        archviregs[-VIREGBEG-op1], 
                                        archviregs[-VIREGBEG-op1]);
               }
            }
            else if(cp[7] == 13 && cp[6] == 5 && cp[5] == 12 && cp[4] == 4)    
            {
               if (cp[3] == 9 && cp[2] == 1 && cp[1] == 8 && cp[0] == 0)
               {
                  ap->next = PrintAssln("\tvpunpckldq\t%s,%s,%s\n",
                                        archviregs[-VIREGBEG-op2], /* src2*/ 
                                        archviregs[-VIREGBEG-op1], 
                                        archviregs[-VIREGBEG-op1]);
               }
            }
            else
            fko_error(__LINE__, "Not implemented this vishuf yet!\n"); 

         #else
/*
 *       SSE: 0, 3276, 5555, 5140
 *       NOTE: not implemented for SSE... need to explore later
 */
            fko_error(__LINE__, "Not implemented VISHUF for SSE yet");
         #endif
         break;
      
      case MASKTEST:  /* using only in scalar code, will be deleted soon */
            fko_error(__LINE__, "MASKTEST Not implemented !!!");   
         break;
/*
 *    Majedul: CMOV1 / CMOV2 ... conditional mov for integer based on 
 *    conditional codes. 
 */
      case CMOV1:  /* dest = (mask==0)? dest: src */
         op3 = -op3;
         assert(op3 >= ICCBEG && op3 < ICCEND);
         ap->next = PrintAssln("\tcmovc\t%s, %s\n", GetIregOrDeref(op2), 
                              archiregs[-IREGBEG-op1]);
         break;
      case CMOV2:  /* dest = (mask==0)? src: dest */
         op3 = -op3;
         assert(op3 >= ICCBEG && op3 < ICCEND);
         ap->next = PrintAssln("\tcmovnc\t%s, %s\n", GetIregOrDeref(op2), 
                              archiregs[-IREGBEG-op1]);
         break;
#ifdef AVX2  
/*
 * Read this note before using following two instructions
 * =========================== NOTE ===========================================
 *    Majedul: hybrid inst using mask from floating point comparison! 
 *    in AVX/SSE, we can do it as the mask value has all 1 or zero for each 
 *    element. 
 *    Need to consider following issue carefully:
 *       1. Here we consider 32 bit INT, so for mask of single precision 
 *       floating point, we can safely use them.
 *       2. For double precision floating point, we always consider pair of 
 *       32 bit INT where the valid value resides on one of the pair. Under this
 *       asumption, we can use this mask value directly.
 * ============================================================================
 */
      case VCMOV1:  /* Hybrid ....  dest = (mask==0)? dest: src */
         op3 = -op3;
/*
 *       NOTE: we are implementing this using vpblendvb (AVX2) but use imm!!
 *       let's use vpblendvb... should work with the mask
 *       this inst selecls byte values from mask specified in the high bit of 
 *       each byte. Here, we use mask with all 1's or 0's in each 32bit/64bit. 
 *       so, it should work.
 *       NOTE: we support no memory operation now.
 */
         if (op3 >= VFREGBEG && op3 < VFREGEND)
         {
            op3 = -op3;
            ap->next = PrintAssln("\tvpblendvb\t%s,%s,%s,%s\n", 
	                          archvfregs[-VFREGBEG-op3],
                                  archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1],
	                          archviregs[-VIREGBEG-op1]); 
         }
         else if (op3 >= VDREGBEG && op3 < VDREGEND)
         {
            op3 = -op3;
            ap->next = PrintAssln("\tvpblendvb\t%s,%s,%s,%s\n", 
	                          archvdregs[-VDREGBEG-op3],
                                  archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1],
	                          archviregs[-VIREGBEG-op1]); 
         }
         else
            fko_error(__LINE__,"Expecting Mask on floating point regs:%d\n",
                      op3);
         
         break;
      
      case VCMOV2:  /* dest = (mask==0)? src: dest */
         op3 = -op3;
         if (op3 >= VFREGBEG && op3 < VFREGEND)
         {
            op3 = -op3;
            ap->next = PrintAssln("\tvpblendvb\t%s,%s,%s,%s\n", 
	                          archvfregs[-VFREGBEG-op3],
                                  archviregs[-VIREGBEG-op1],
	                          archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1]); 
         }
         else if (op3 >= VDREGBEG && op3 < VDREGEND)
         {
            op3 = -op3;
            ap->next = PrintAssln("\tvpblendvb\t%s,%s,%s,%s\n", 
	                          archvdregs[-VDREGBEG-op3],
                                  archviregs[-VIREGBEG-op1],
	                          archviregs[-VIREGBEG-op2],
	                          archviregs[-VIREGBEG-op1]); 
         }
         else
            fko_error(__LINE__, "Expecting Mask value on floating point regs\n");
         
         break;
/*
 *    special instruction to set conditional codes in X86
 *    BTC : bit test and update CF flag
 */
      case BTC:   
         op1 = -op1;
         assert( op1>=ICCBEG && op1<ICCEND);
         ap->next = PrintAssln("\tbt\t%s,%s\n", 
                                 GetIregOrConst(op3),
	                         archiregs[-IREGBEG-op2]);
          
         break;
#endif         
   #endif
#endif
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
        fko_error(__LINE__, "Unknown instruction %d[%s] after %s\n", 
                  ip->inst[0],instmnem[ip->inst[0]], ap->ln);
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
