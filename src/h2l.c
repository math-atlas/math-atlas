#include "ifko.h"
#include "fko_arch.h"

static short type2len(int type)
{
   short len=4;
   if (type == T_LONG || type == T_DOUBLE) len = 8;
   return(len);
}
static short type2shift(int type)
{
   short len=2;
   if (type == T_LONG || type == T_DOUBLE) len = 3;
   return(len);
}

#ifdef ArchPtrIsLong
   #define ASHL SHLL
   #define AADD ADDL
   #define ASUB SUBL
#else
   #define ASHL SHL
   #define AADD ADD
   #define ASUB SUB
#endif

static int LocalLoad(short id)
/*
 * Loads the value stored in local id
 * RETURNS: register loaded to
 */
{
   short inst=5555, reg, typ;

   id--;
   if (IS_PTR(STflag[id]))
   {
      #ifdef ArchPtrIsLong
         inst = LDL;
         reg = GetReg(T_INT);
      #else
         inst = LD;
         reg = GetReg(T_LONG);
      #endif
   }
   else
   {
      typ = FLAG2TYPE(STflag[id]);
      reg = GetReg(typ);
      switch(typ)
      {
      case T_INT:
         inst = LD;
         break;
      case T_LONG:
         inst = LDL;
         break;
      case T_FLOAT:
         inst = FLD;
         break;
      case T_DOUBLE:
         inst = FLDD;
         break;
      default:
         fprintf(stderr, "type=%d!!\n\n", typ);
      }
   }
   InsNewInst(NULL, NULL, inst, -reg, SToff[id].sa[2], 0);
   return(reg);
}

static void LocalStore(short id, short sreg)
{
   short inst;
   switch(FLAG2TYPE(STflag[id-1]))
   {
   case T_INT:
      inst = ST;
      break;
   case T_LONG:
      inst = STL;
      break;
   case T_FLOAT:
      inst = FST;
      break;
   case T_DOUBLE:
      inst = FSTD;
      break;
   }
   InsNewInst(NULL, NULL, inst, SToff[id-1].sa[2], -sreg, 0);
}

void DoConvert(short dest, short src)
{
   fprintf(stderr, "Conversions not yet supported\n");
   exit(-1);
}

void DoFpConstLoad(short dest, short src)
{
   char *ln;
   int type;
   
fprintf(stderr, "Handling fpconst!\n");
/*
 * Reserve integer reg for @ ld (store in op3), as well as reg for actual
 * move.  Encode fp const load as special case of FMOV.
 */
   type = FLAG2TYPE(STflag[dest-1]);
   InsNewInst(NULL, NULL, type == T_FLOAT ? FMOV : FMOVD, 
              -GetReg(type), src, __LINE__);
}

void DoMove(short dest, short src)
{
   short rsrc;
   int sflag;
   sflag = STflag[src-1];
fprintf(stderr, "DoMove %d %d (%s %s)\n", dest, src, STname[dest-1]?STname[dest-1] : "NULL", STname[src-1]?STname[src-1] : "NULL");
   if (IS_CONST(sflag) && (IS_FLOAT(sflag) || IS_DOUBLE(sflag)))
      DoFpConstLoad(dest, src);
   if (FLAG2TYPE(STflag[dest-1]) == FLAG2TYPE(sflag))
   {
      rsrc = LocalLoad(src);
      LocalStore(dest, rsrc);
   }
   else DoConvert(dest, src);
   GetReg(-1);
}

static void FixDeref(short ptr)
/*
 * This routine takes a deref entry of type:
 *    <STloc> <STloc> <mul> <STconst>
 * And translates into a fully-specified legal index for the machine:
 *    <reg0> <reg1> <mul> <STconst>
 * where the address is then provided by <reg0> + <reg1>*mul + <STconst>.
 * On some machines, mul may need to be 1, resulting in an extra shift
 * instruction, and on others you may be able to use one of <reg1> or <STconst>
 * at a time, resulting in an additional add.
 */
{
   short k, type;

   k = (ptr-1)<<2;
   type = FLAG2TYPE(STflag[DT[k]-1]);
/*
 * Load beginning of array
 */
   DT[k] = LocalLoad(DT[k]);
/*
 * Load index register if needed
 */
   if (DT[k+1])
   {
      DT[k+1] = LocalLoad(DT[k+1]);
/*
 *    Some architectures cannot multiply the index register by some (or any)
 *    constants, and in this case generate an extra shift instruction
 */
      if (!ArchHasLoadMul(DT[k+2]))
      {
         InsNewInst(NULL, NULL, ASHL, -DT[k+1], -DT[k+1], 
                    STiconstlookup(type2shift(type)));
         DT[k+2] = 1;
      }
/*
 *    On machines with fixed-size instructions, you usually need to choose
 *    _either_ an index register, _or_ a constant addition.  If we have both
 *    on such a machine, add the constant to the index register
 */
      #ifndef ArchConstAndIndex
         if (DT[k+3])
         {
            InsNewInst(NULL, NULL, AADD, -DT[k+1], -DT[k+1], DT[k+3]);
            DT[k+3] = 0;
         }
      #endif
   }
}

void DoArrayStore(short ptr, short id)
{
   INSTQ *ip;
   short inst, lreg, ireg=0, preg, k, type;

   type = FLAG2TYPE(STflag[id-1]);
   assert(FLAG2TYPE(STflag[DT[k]-1]) == type);

   lreg = LocalLoad(id);
   k = (ptr-1)<<2;
   FixDeref(ptr);
   switch(type)
   {
   IS_INT:
      inst = ST;
      break;
   IS_LONG:
      inst = STL;
      break;
   IS_FLOAT:
      inst = FST;
      break;
   IS_DOUBLE:
      inst = FSTD;
      break;
   }
   InsNewInst(NULL, NULL, inst, ptr, -lreg, 0);
   GetReg(-1);
}

void DoArrayLoad(short id, short ptr)
{
   short k, ireg=0, areg, type, ld;

   k = (ptr-1)<<2;
   type = FLAG2TYPE(STflag[DT[k]-1]);
   FixDeref(ptr);
   switch(type)
   {
   IS_INT:
      ld = LD;
      break;
   IS_LONG:
      ld = LDL;
      break;
   IS_FLOAT:
      ld = FLD;
      break;
   IS_DOUBLE:
      ld = FLDD;
      break;
   }
   areg = GetReg(type);
   InsNewInst(NULL, NULL, ld, -areg, ptr, 0);
   LocalStore(id, areg);
   GetReg(-1);
}

void HandlePtrArith(short dest, short src0, char op, short src1)
/*
 * Ptr arithmetic must be of form <ptr> = <ptr> [+,-] <int/const>
 */
{
   short rd, rs0, rs1, flag, type;

   if (op != '+' && op != '-')
      yyerror("pointers may take only + and - operators");
   if (!IS_PTR(STflag[src0-1]))
      yyerror("Expecting <ptr> = <ptr> + <int>");

   type = FLAG2TYPE(STflag[dest-1]);
   rs0 = LocalLoad(src0);
   flag = STflag[src1-1];


   if (IS_CONST(flag))
   {
      if (IS_INT(flag))
         rs1 = -STiconstlookup(SToff[src1-1].i*type2len(type));
      else
         yyerror("Pointers may only be incremented by integers");
   }
   else
   {
      rs1 = LocalLoad(src1);
      InsNewInst(NULL, NULL, ASHL, -rs1, -rs1, 
                 STiconstlookup(type2shift(type)));
   }
   InsNewInst(NULL, NULL, op == '+' ? AADD : ASUB, -rs0, -rs0, -rs1);
   LocalStore(dest, rs0);
   GetReg(-1);
}

void DoArith(short dest, short src0, char op, short src1)
{
   short rd, rs0, rs1, type;
   enum inst inst;
   extern int DTnzerod, DTnzero, DTabs, DTabsd;

   if (IS_PTR(STflag[dest-1]))
   {
      HandlePtrArith(dest, src0, op, src1);
      return;
   }
   type = FLAG2TYPE(STflag[dest-1]);
   rd = rs0 = LocalLoad(src0);
   if (op != 'n' && op != 'a')
   {
      if (src0 != src1)
      {
         if (IS_CONST(STflag[src1-1])) rs1 = -src1;
         else rs1 = LocalLoad(src1);
      }
      else rs1 = rs0;
   }
   else rs1 = 0;
   if ( (op == '%' || op == '>' || op == '<') && 
        (type != T_INT && type != T_LONG) )
      yyerror("modulo and shift not defined for non-integer types");
   switch(op)
   {
   case '+': /* dest = src0 + src1 */
      switch(type)
      {
      case T_INT:
         inst = ADD;
         break;
      case T_LONG:
         inst = ADDL;
         break;
      case T_FLOAT:
         inst = FADD;
         break;
      case T_DOUBLE:
         inst = FADDD;
         break;
      }
      break;
   case '-': /* dest = src0 - src1 */
      switch(type)
      {
      case T_INT:
         inst = SUB;
         break;
      case T_LONG:
         inst = SUBL;
         break;
      case T_FLOAT:
         inst = FSUB;
         break;
      case T_DOUBLE:
         inst = FSUBD;
         break;
      }
      break;
   case '*': /* dest = src0 * src1 */
      switch(type)
      {
      case T_INT:
         inst = MUL;
         break;
      case T_LONG:
         inst = MULL;
         break;
      case T_FLOAT:
         inst = FMUL;
         break;
      case T_DOUBLE:
         inst = FMULD;
         break;
      }
      break;
   case '/': /* dest = src0 / src1 */
      switch(type)
      {
      case T_INT:
         inst = DIV;
         break;
      case T_LONG:
         inst = DIVL;
         break;
      case T_FLOAT:
         inst = FDIV;
         break;
      case T_DOUBLE:
         inst = FDIVD;
         break;
      }
      break;
   case '%': /* dest = src0 % src1 */
      yyerror("% not yet supported");
      break;
   case '>': /* dest = src0 >> src1 */
      if (type == T_INT) inst = SHR;
      else inst = SHRL;
      break;
   case '<': /* dest = src0 << src1 */
      if (type == T_INT) inst = SHL;
      else inst = SHLL;
      break;
   case 'm': /* dest += src0 * src1 */
      if (type == T_FLOAT || type == T_DOUBLE)
      {
         rd = LocalLoad(dest);
         #ifdef ArchHasMAC
            if (type == T_FLOAT) inst = FMAC;
            else isnt = FMACD;
         #else
            if (type == T_FLOAT)
            {
               InsNewInst(NULL, NULL, FMUL, -rs0, -rs0, -rs1);
               inst = FADD;
            }
            else
            {
               InsNewInst(NULL, NULL, FMULD, -rs0, -rs0, -rs1);
               inst = FADDD;
            }
         #endif
      }
      else yyerror("MAC available for floating point operands only!");
      break;
   case 'n': /* dest = -src0 */
      switch(type)
      {
      case T_INT:
         inst = NEG;
         break;
      case T_LONG:
         inst = NEGL;
         break;
      case T_FLOAT:
         inst = FNEG;
	 DTnzerod = -1;
         break;
      case T_DOUBLE:
         inst = FNEGD;
	 DTnzerod = -1;
         break;
      }
      break;
   case 'a': /* dest = abs(src0) */
      switch(type)
      {
/*
      case T_INT:
         inst = ABS;
         break;
      case T_LONG:
         inst = ABSL;
         break;
*/
      case T_FLOAT:
         inst = FABS;
	 DTabs = -1;
         break;
      case T_DOUBLE:
         inst = FABSD;
	 DTabsd = -1;
         break;
      default:
         fko_error(__LINE__, "ABS available for floating point only!\n");
      }
      break;
   }
   InsNewInst(NULL, NULL, inst, -rd, -rs0, -rs1);
   LocalStore(dest, rs0);
   GetReg(-1);
}

void DoReturn(short rret)
{
   int retreg, srcreg;
   int mov, ld, type;
   if (rret)
   {
      type = STflag[rret-1];
      if (IS_PTR(type))
         #if ArchPtrIsLong
	    type = T_LONG;
         #else 
	    type = T_INT;
	 #endif
      else type = FLAG2TYPE(type);
      switch(type)
      {
      case T_LONG:
	 ld = LDL;
	 mov = MOVL;
         rout_flag |= IRET_BIT;
         retreg = IRETREG;
         break;
      case T_INT:
         mov = MOV;
	 ld = LD;
         rout_flag |= IRET_BIT;
         retreg = IRETREG;
         break;
      case T_FLOAT:
         mov = FMOV;
         ld = FLD;
         rout_flag |= FRET_BIT;
         retreg = FRETREG;
         break;
      case T_DOUBLE:
         mov = FMOVD;
         ld = FLDD;
         rout_flag |= DRET_BIT;
         retreg = DRETREG;
         break;
      }
#if 0
      if (IS_CONST(STflag[rret-1])) srcreg = -rret;
      else srcreg = LocalLoad(rret);
      InsNewInst(NULL, NULL, mov, -retreg, -srcreg, 0);
#else
      if (IS_CONST(STflag[rret-1]))
         InsNewInst(NULL, NULL, mov, -retreg, rret, 0);
      else 
         InsNewInst(NULL, NULL, ld, -retreg, SToff[rret-1].sa[2], 0);
#endif
   }
   InsNewInst(NULL, NULL, JMP, STstrconstlookup("IFKO_EPILOGUE"), 0, 0);
   GetReg(-1);
}

void DoLabel(char *name)
{
   InsNewInst(NULL, NULL, LABEL, STdef(name, T_LABEL, 0), 0, 0);
}
