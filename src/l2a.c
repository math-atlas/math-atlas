#include "ifko.h"
#include "fko_arch.h"
#include <stdarg.h>

#define MyAssert(arg_) \
{ \
   if (!(arg_)) while(1); \
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
            sprintf(ln, "%d(%s,%s,%s)", con, archiregs[-IREGBEG-ptr],
                    archiregs[-IREGBEG-reg], SToff[mul-1]);
         else
            sprintf(ln, "(%s,%s,%s)", archiregs[-IREGBEG-ptr],
                    archiregs[-IREGBEG-reg], SToff[mul-1]);
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
         if (IS_INT(flag) || IS_LONG(flag)) sprintf(ln, "$%d", SToff[id].i);
/*         else if (IS_LONG(flag)) sprintf(ln, "$%ld", SToff[id].l); */
      #else
         if (IS_INT(flag) || IS_LONG(flag)) sprintf(ln, "%d", SToff[id].i);
/*         else if (IS_LONG(flag)) sprintf(ln, "%ld", SToff[id].l); */
      #endif
         else fko_error(__LINE__, "Integer constant expected!\n");
   }
   return(ln);
}

struct assmln *lil2ass(INSTQ *head)
{
   INSTQ *ip=head;
   struct assmln *ahead=NULL, *ap;
   short op1, op2, op3, k;
   #ifdef SPARC
      int SeenSave=0;
   #endif
   char ln[1024];

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
         if (IS_GLOB(STflag[--op1]))
            ap->next = PrintAssln(".globl\t%s\n", STname[op1]);
         else
            ap->next = PrintAssln(".local\t%s\n", STname[op1]);
         ap = ap->next;
         ap->next = PrintAssln("%s:\n", STname[op1]);
         break;
/*
 *    32 bit integer
 */
      case LD:
         #ifdef X86
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
      case ST:
         #ifdef X86
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
         #elif defined(PPC)
            k = (op2-1)<<2;
            if (DT[k+1])
               ap->next = PrintAssln("\tstwx\t%s,%s\n", archiregs[-IREGBEG-op1],
                                     GetDeref(op2));
            else
               ap->next = PrintAssln("\tstw\t%s,%s\n", archiregs[-IREGBEG-op1], 
                                     GetDeref(op2));
         #endif
         break;
      case SHL:
         #ifdef X86
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
      case SHR:
         #ifdef X86
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
      case SAR:
         #ifdef X86
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
      case ADD:
         #ifdef X86
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
      case SUB:
         #ifdef X86
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
            if (op3 > 0)
            {
               ap->next = PrintAssln("\timull\t%s,%s,%s\n", 
                                     GetIregOrConst(op3),
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op1]);
            }
            else
            {
               assert(op1 == op2);
               ap->next = PrintAssln("\timull\t%s,%s\n", 
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
                                     archiregs[-IREGBEG-op2]
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tmullw\t%s,%s,%s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2]
                                     archiregs[-IREGBEG-op3]);
         #endif
         break;
      case UMUL:
         #ifdef X86
/*
 * NOTE: we use signed mul for umul on the x86, so we can usual the
 * unrestricted form.  This means our total size may lose a bit
 */
            if (op3 > 0)
            {
               ap->next = PrintAssln("\timull\t%s,%s,%s\n", 
                                     GetIregOrConst(op3),
                                     archiregs[-IREGBEG-op2],
                                     archiregs[-IREGBEG-op1]);
            }
            else
            {
               assert(op1 == op2);
               ap->next = PrintAssln("\timull\t%s,%s\n", 
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
                                     archiregs[-IREGBEG-op2]
                                     GetIregOrConst(op3));
            else
               ap->next = PrintAssln("\tmullw\t%s,%s,%s\n", 
                                     archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2]
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
                                  archiregs[-IREGBEG-op2]
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
                                  archiregs[-IREGBEG-op2]
                                  archiregs[-IREGBEG-op3]);
         #endif
         break;
      case CMP:
         #ifdef X86
            ap->next = PrintAssln("\tcmp\t%s,%s\n", GetIregOrDeref(op2),
                                  GetIregOrConst(op3));
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
      case MOV:
         #ifdef X86
            ap->next = PrintAssln("\tmov\t%s,%s\n", GetIregOrConst(op2),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tmov\t%s,%s\n", GetIregOrConst(op2),
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            if (op3 > 0)
               ap->next = PrintAssln("\tli\t%s,%s\n", archiregs[-IREGBEG-op1],
                                     GetIregOrConst(op2));
            else
               ap->next = PrintAssln("\tmr\t%s,%s\n", archiregs[-IREGBEG-op1],
                                     archiregs[-IREGBEG-op2]);
         #endif
         break;
      case NEG:
         #ifdef X86
            assert(op1 == op2);
            ap->next = PrintAssln("\tneg\t%s", GetIregOrDeref(op1));
         #elif defined(SPARC)
            ap->next = PrintAssln("\tneg\t%s, %s", archiregs[-IREGBEG-op2],
                                  archiregs[-IREGBEG-op1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tneg\t%s, %s", archiregs[-IREGBEG-op1],
                                  archiregs[-IREGBEG-op2]);
         #endif
         break;
      case JMP:
         #ifdef X86
            ap->next = PrintAssln("\tjmp\t%s", STname[op1-1]);
         #elif defined(SPARC)
            ap->next = PrintAssln("\tba\t%s\n\tnop\n", STname[op1-1]);
         #elif defined(PPC)
            ap->next = PrintAssln("\tb\t%s", STname[op1-1]);
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
