#include "fko.h"
#include "fko_arch.h"

static int LocalLoad(short id)
/*
 * Loads the value stored in local id
 * RETURNS: register loaded to
 */
{
   short inst=5555, reg, typ;

   id--;
   if (IS_CONST(STflag[id]))
   {
      typ = FLAG2TYPE(STflag[id]);
      reg = GetReg(typ);
      switch(typ)
      {
      case T_INT:
         inst = MOV;
         break;
      case T_FLOAT:
         inst = FMOV;
         break;
      case T_DOUBLE:
         inst = FMOVD;
         break;
      default:
         fprintf(stderr, "type=%d!!\n\n", typ);
      }
      InsNewInst(NULL, NULL, NULL, inst, -reg, id+1, 0);
      return(reg);
   }
   else if (IS_PTR(STflag[id]))
   {
      inst = LD;
      reg = GetReg(T_INT);
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
   InsNewInst(NULL, NULL, NULL, inst, -reg, SToff[id].sa[2], 0);
   return(reg);
}

static void LocalStore(short id, short sreg)
{
   short inst;

   switch(FLAG2PTYPE(STflag[id-1]))
   {
   case T_INT:
      inst = ST;
      break;
   case T_FLOAT:
      inst = FST;
      break;
   case T_DOUBLE:
      inst = FSTD;
      break;
   default:
      fko_error(__LINE__, "Unknown type!\n");
   }
   InsNewInst(NULL, NULL, NULL, inst, SToff[id-1].sa[2], -sreg, 0);
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
 * Encode fp const load as special case of FMOV.
 */
   type = FLAG2TYPE(STflag[dest-1]);
   InsNewInst(NULL, NULL, NULL, type == T_FLOAT ? FMOV : FMOVD, 
              -GetReg(type), src, 0);
}

void DoComment(char *str)
{
   int i;
   for (i=0; str[i]; i++);
   if (str[--i] == '\n') str[i] = '\0';
   InsNewInst(NULL, NULL, NULL, COMMENT, STstrconstlookup(str), 0, 0);
}

void DoMove(short dest, short src)
{
   short rsrc;
   int sflag, type;
   enum inst mov;
   sflag = STflag[src-1];

   if (IS_CONST(sflag))
   {
      type = FLAG2PTYPE(sflag);
      rsrc = GetReg(type);
      if (IS_CONST(sflag) && (type == T_INT) && SToff[src-1].i == 0)
         InsNewInst(NULL, NULL, NULL, XOR, -rsrc, -rsrc, -rsrc);
      else
      {
         if (type == T_INT) mov = MOV;
         else if (type == T_FLOAT) mov = FMOV;
         else 
         {
            assert(type == T_DOUBLE);
            mov = FMOVD;
         }
         InsNewInst(NULL, NULL, NULL, mov, -rsrc, src, 0);
      }
      LocalStore(dest, rsrc);
   }
   else if (FLAG2TYPE(STflag[dest-1]) == FLAG2TYPE(sflag))
   {
      rsrc = LocalLoad(src);
      LocalStore(dest, rsrc);
   }
   else DoConvert(dest, src);
   GetReg(-1);
}

short AddArrayDeref(short array, short index, int offset)
/*
 * offset is integer constant.  array,index are entries in the ST.
 *    array[index+offset]
 */
{
   int mul=4, flag;
   flag = STflag[array-1];
   if (IS_DOUBLE(flag)) mul = 8;
   else if (IS_CHAR(flag)) mul = 1;
   assert(!IS_VEC(flag));
   return(AddDerefEntry(array, index, mul, offset, array));
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

   ptr--;
   type = FLAG2TYPE(STflag[SToff[ptr].sa[0]-1]);
/* fprintf(stderr, "FixDeref: [%d, %d, %d, %d]\n", SToff[ptr].sa[0], SToff[ptr].sa[1], SToff[ptr].sa[2], SToff[ptr].sa[3]); */
/*
 * Load beginning of array
 */
   SToff[ptr].sa[0] = -LocalLoad(SToff[ptr].sa[0]);
/*
 * Multiply constant by mul
 */
   if (SToff[ptr].sa[2]) SToff[ptr].sa[3] *= SToff[ptr].sa[2];
/*
 * Load index register if needed
 */
   if (SToff[ptr].sa[1])
   {
      SToff[ptr].sa[1] = -LocalLoad(SToff[ptr].sa[1]);
/*
 *    Some architectures cannot multiply the index register by some (or any)
 *    constants, and in this case generate an extra shift instruction
 */
      if (!ArchHasLoadMul(SToff[ptr].sa[2]))
      {
         InsNewInst(NULL, NULL, NULL, SHL, SToff[ptr].sa[1], SToff[ptr].sa[1], 
                    STiconstlookup(type2shift(type)));
         SToff[ptr].sa[2] = 1;
      }
/*
 *    On machines with fixed-size instructions, you usually need to choose
 *    _either_ an index register, _or_ a constant addition.  If we have both
 *    on such a machine, add the constant to the index register
 */
      #ifndef ArchConstAndIndex
         if (SToff[ptr].sa[3])
         {
            InsNewInst(NULL, NULL, NULL, ADD, SToff[ptr].sa[1], 
                       SToff[ptr].sa[1], STiconstlookup(SToff[ptr].sa[3]));
            SToff[ptr].sa[3] = 0;
         }
      #endif
   }
}

void DoArrayStore(short ptr, short id)
{
   INSTQ *ip;
   short lreg, ireg=0, preg, k, type;

   k = ptr-1;
   type = FLAG2TYPE(STflag[id-1]);
   #if IFKO_DEBUG_LEVEL > 1
      fprintf(stderr, "pnam=%s, pflag=%d, idname='%s', idflag=%d\n", 
              STname[SToff[k].sa[0]-1], STflag[SToff[k].sa[0]-1], 
              STname[id-1], STflag[id-1]);
   #endif

   lreg = LocalLoad(id);
   assert(FLAG2TYPE(STflag[SToff[k].sa[0]-1]) == type);
   FixDeref(ptr);
   switch(type)
   {
   case T_INT:
      #ifdef X86_64
         assert(lreg < 8);
         InsNewInst(NULL, NULL, NULL, STS, ptr, -lreg, 0);
      #else
         InsNewInst(NULL, NULL, NULL, ST, ptr, -lreg, 0);
      #endif
      break;
   case T_FLOAT:
      InsNewInst(NULL, NULL, NULL, FST, ptr, -lreg, 0);
      break;
   case T_DOUBLE:
      InsNewInst(NULL, NULL, NULL, FSTD, ptr, -lreg, 0);
      break;
   default:
      fko_error(__LINE__, "Unknown type %d\n", type);
   }
   GetReg(-1);
}

void DoArrayLoad(short id, short ptr)
/*
 * id is ST index, ptr is DT index
 */
{
   short k, ireg=0, areg, type, ld;

   k = (ptr-1)<<2;
   type = FLAG2TYPE(STflag[id-1]);
   FixDeref(ptr);
   switch(type)
   {
   case T_INT:
/*
 *    Special case for integer arrays, which are still 32-bits on x86-64
 */
      #ifdef X86_64
         areg = GetReg(T_SHORT);
         InsNewInst(NULL, NULL, NULL, LDS, -areg, ptr, 0);
         if (!IS_UNSIGNED(STflag[id-1]))
         {
/*
 *          FIXME: should use CVTSI (movslq) instruction to upgrade
 */
            k = STiconstlookup(31);
            InsNewInst(NULL, NULL, NULL, SHL, -areg, -areg, k);
            InsNewInst(NULL, NULL, NULL, SAR, -areg, -areg, k);
            LocalStore(id, areg);
            GetReg(-1);
            return;
         }
      #endif
      ld = LD;
      break;
   case T_FLOAT:
      ld = FLD;
      break;
   case T_DOUBLE:
      ld = FLDD;
      break;
   default:
      fko_error(__LINE__, "Unknown type %d\n", type);
   }
   areg = GetReg(type);
   InsNewInst(NULL, NULL, NULL, ld, -areg, ptr, 0);
   LocalStore(id, areg);
   GetReg(-1);
}
void HandlePtrArithNoSizeofUpate(short dest, short src0, char op, short src1)
/*
 * this function is similar to HandlePtrArith but doesn't multiply lda with 
 * size to fix the offset. 
 */
{
   short rd, rs0, rs1, flag, type, dflag, k;
   if (op != '+' && op != '-')
      yyerror("pointers may take only + and - operators");
   if (!IS_PTR(STflag[src0-1]))
      yyerror("Expecting <ptr> = <ptr> + <int>");
      
   rs0 = LocalLoad(src0); /* load src0 */ 
   flag = STflag[src1-1];
   if (!IS_CONST(flag))
      rs1 = LocalLoad(src1); /* load src1 */
   else
      yyerror("expecting var as 2nd src as a special case");

   InsNewInst(NULL, NULL, NULL, op == '+' ? ADD : SUB, -rs0, -rs0, -rs1);
   LocalStore(dest, rs0);
   GetReg(-1);
}

void HandlePtrArith(short dest, short src0, char op, short src1)
/*
 * Ptr arithmetic must be of form <ptr> = <ptr> [+,-] <int/const>
 */
{
   short rd, rs0, rs1, flag, type, dflag, k;
   short nvar;

   if (op != '+' && op != '-')
      yyerror("pointers may take only + and - operators");
   if (!IS_PTR(STflag[src0-1]))
      yyerror("Expecting <ptr> = <ptr> + <int>");
   
/*
 * Majedul: The concept of LIL as three address code violets here. We have 
 * multiple operations in single load-store block. For example:
 *    A1 = A0  + lda is actually treated as 
 *    A1 = A0 + (lda * size)
 * So, we have addition and shift in same load-store block. This would create
 * redundant computation. We eliminate the redundant computation of (lda*size),
 * we need to split this expression out and treated this as two HIL instruction
 * while converting this into LIL, like:
 *    _lda = lda * size
 *    A1 = A0 + _lda
 * NOTE: All variables starting with '_' are compiler's internal variables, 
 * should not be used as variable name in HIL
 */

   dflag = STflag[dest-1];
   type = FLAG2TYPE(dflag);
   flag = STflag[src1-1];

   if (IS_CONST(flag))
   {
      rs0 = LocalLoad(src0); /* load src0 */
      if (IS_INT(flag))
      #ifdef X86_64
      {
         if (IS_INT(dflag)) rs1 = -STiconstlookup(SToff[src1-1].i*4);
         else rs1 = -STiconstlookup(SToff[src1-1].i*type2len(type));
      }
      #else
         rs1 = -STiconstlookup(SToff[src1-1].i*type2len(type));
      #endif
      else
         yyerror("Pointers may only be incremented by integers");
   }
   else
   {
/*
 *    Majedul: 2nd source is variable. So, need extra variable to calculate
 *    the actual offset value to be added with ptr
 *    should be safe to use same var over and over again.
 */
#if 0
      nvar = InsertNewLocal("_T_var_lda", flag);
      rs1 = LocalLoad(src1);
      #ifdef X86_64
         if (IS_INT(dflag)) k = STiconstlookup(2);
         else k = STiconstlookup(type2shift(type));
         InsNewInst(NULL, NULL, NULL, SHL, -rs1, -rs1, k);
      #else
         InsNewInst(NULL, NULL, NULL, SHL, -rs1, -rs1, 
                    STiconstlookup(type2shift(type)));
      #endif
      LocalStore(nvar, rs1);
      GetReg(-1);
/*
 *    Majedul: now, we will load new variable 
 */
      rs0 = LocalLoad(src0); /* load src0 */
      rs1 = LocalLoad(nvar);
#else
/*
 *    Equivalent code for old strategy 
 */
      rs0 = LocalLoad(src0); /* load src0 */
      rs1 = LocalLoad(src1);
      #ifdef X86_64
         if (IS_INT(dflag)) k = STiconstlookup(2);
         else k = STiconstlookup(type2shift(type));
         InsNewInst(NULL, NULL, NULL, SHL, -rs1, -rs1, k);
      #else
         InsNewInst(NULL, NULL, NULL, SHL, -rs1, -rs1, 
                    STiconstlookup(type2shift(type)));
      #endif
#endif
   }
   InsNewInst(NULL, NULL, NULL, op == '+' ? ADD : SUB, -rs0, -rs0, -rs1);
   LocalStore(dest, rs0);
   GetReg(-1);
}

void Handle2dArrayPtrArith(short dest, short src0, char op, short src1)
/*
 * ptr arithmatic for 2D array
 */
{
   int i;
   short arrid, lda, ur, arrN;
/*
 * only supported where src0 and dest are same....
 */
   if (dest != src0 )
      fko_error(__LINE__, "dest and src0 should be same for 2D array arith");

   arrid = SToff[dest-1].sa[3];
#if 0   
   ur = STarr[arrid-1].urlist[0];
   ur = SToff[ur-1].i;
   for (i=0; i < ur; i++)
   {
#else
   ur = STarr[arrid-1].colptrs[0];
   for (i=1; i<=ur; i++)
   {
#endif
      arrN = STarr[arrid-1].colptrs[i];
      HandlePtrArith(arrN, arrN, op, src1);
   }
}


void DoArith(short dest, short src0, char op, short src1)
{
   short rd, rs0, rs1, type;
   enum inst inst;
   extern int DTnzerod, DTnzero, DTabs, DTabsd;

   if (IS_PTR(STflag[dest-1]))
   {
#if 1
      if (SToff[dest-1].sa[3]) /* 2D array pointer? */
         Handle2dArrayPtrArith(dest, src0, op, src1);
      else
#endif
         HandlePtrArith(dest, src0, op, src1);
      return;
   }
   type = FLAG2TYPE(STflag[dest-1]);
/*
 * x86 insts must handle integer division specially
 */
   #ifdef X86
      if (op == '/' && IS_INT(type))
      {
         #ifdef X86_64
            rd = iName2Reg("@rax");
            rs0 = iName2Reg("@rdx");
            rs1 = iName2Reg("@rcx");
         #else
            rd = iName2Reg("@eax");
            rs0 = iName2Reg("@edx");
            rs1 = iName2Reg("@ecx");
         #endif
         InsNewInst(NULL, NULL, NULL, LD, -rd, SToff[src0-1].sa[2], 0);
         InsNewInst(NULL, NULL, NULL, MOV, -rs0, -rd, 0);
         InsNewInst(NULL, NULL, NULL, SAR, -rs0, -rs0, STiconstlookup(31));
         if (IS_CONST(STflag[src1-1]))
            InsNewInst(NULL, NULL, NULL, MOV, -rs1, 
                       STiconstlookup(SToff[src1-1].i), 0);
         else
            InsNewInst(NULL, NULL, NULL, LD, -rs1, SToff[src1-1].sa[2], 0);
         InsNewInst(NULL, NULL, NULL, DIV, -rd, -rs0, -rs1);
         fprintf(stderr, "DIV %d, %d, %d\n", -rd, -rs0, -rs1);
         LocalStore(dest, rd);
         GetReg(-1);
         return;
      }
   #endif
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
        (type != T_INT && type != T_SHORT) )
      yyerror("modulo and shift not defined for non-integer types");
   switch(op)
   {
   case '+': /* dest = src0 + src1 */
      switch(type)
      {
      case T_INT:
         inst = ADD;
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
      break;
   case '<': /* dest = src0 << src1 */
      if (type == T_INT) inst = SHL;
      break;
   case 'm': /* dest += src0 * src1 */
      if (type == T_FLOAT || type == T_DOUBLE)
      {
         #ifdef ArchHasMAC
            rd = LocalLoad(dest);
            if (type == T_FLOAT) inst = FMAC;
            else inst = FMACD; /*Majedul: was a typo, corrected it. */
         #else
            if (type == T_FLOAT)
            {
               InsNewInst(NULL, NULL, NULL, FMUL, -rs0, -rs0, -rs1);
               inst = FADD;
            }
            else
            {
               InsNewInst(NULL, NULL, NULL, FMULD, -rs0, -rs0, -rs1);
               inst = FADDD;
            }
            rs1 = rs0;
            rs0 = rd = LocalLoad(dest);
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
      case T_FLOAT:
         inst = FNEG;
	 DTnzero = -1;
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
   InsNewInst(NULL, NULL, NULL, inst, -rd, -rs0, -rs1);
/*
 * Majedul: rd should be stored to dest (not rs0) to be consistant though
 * without FMAC it doesn't create any problem. In case of FMAC, rs0 and rd 
 * are different. 
 */
   /* LocalStore(dest, rs0);*/
   LocalStore(dest, rd);
   GetReg(-1);
}

void DoEmptyReturn()
{
   InsNewInst(NULL, NULL, NULL, JMP, -PCREG, 
              STlabellookup("_IFKO_EPILOGUE"), 0);
}
void DoReturn(short rret)
{
   int retreg, srcreg;
   int mov, ld, type;
   extern int DTx87, DTx87d;
   if (rret)
   {
      type = FLAG2PTYPE(STflag[rret-1]);
      switch(type)
      {
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
         #ifdef X86_32
            DTx87 = -1;
         #endif
         break;
      case T_DOUBLE:
         mov = FMOVD;
         ld = FLDD;
         rout_flag |= DRET_BIT;
         retreg = DRETREG;
         #ifdef X86_32
            DTx87d = -1;
         #endif
         break;
      default:
         fprintf(stderr, "UNKNOWN TYPE %d on line %d of %s\n", 
                 type, __LINE__, __FILE__);
         exit(-1);
      }
#if 0
      if (IS_CONST(STflag[rret-1])) srcreg = -rret;
      else srcreg = LocalLoad(rret);
      InsNewInst(NULL, NULL, NULL, mov, -retreg, -srcreg, 0);
#else
      if (IS_CONST(STflag[rret-1]))
         InsNewInst(NULL, NULL, NULL, mov, -retreg, rret, 0);
      else 
         InsNewInst(NULL, NULL, NULL, ld, -retreg, SToff[rret-1].sa[2], 0);
#endif
   }
   InsNewInst(NULL, NULL, NULL, JMP, -PCREG, 
              STlabellookup("_IFKO_EPILOGUE"), 0);
   GetReg(-1);
}

static short GetSignInfo(short k)
/*
 * Calculates sign info for ST entry k
 */
{
   int flag, i;
   short kret=0;
   flag = STflag[k-1];
   if (IS_CONST(flag))
   {
      assert(IS_INT(flag));
      i = SToff[k-1].i;
      if (i > 0) kret = 2;
      else if (i == 0) kret = 1;
      else if (i < 0) kret = -2;
      else kret = -1;
   }
   else if (IS_UNSIGNED(flag)) kret = 1;
   return(kret);
}

LOOPQ *DoLoop(short I, short start, short end, short inc,
              short sst, short send, short sinc)
/*
 * sst, send, sinc indicate sign of each parameter:
 *    0 : unknown
 *    1 : >= 0
 *    2 : >  0
 *   -1 : <= 0
 *   -2 : <  0
 */
{
   int flag=0;
   LOOPQ *lp;
   short ireg;
   char lnam[128];
/*
 * If signs of loop parts are unknown, see if we can deduce them
 * Ignore markup and deduce if variable is compile-time constant
 */
   if (!sst || IS_CONST(STflag[start-1]))
      sst = GetSignInfo(start);
   if (!send || IS_CONST(STflag[end-1]))
      send = GetSignInfo(end);
/*
 * Majedul: fixed to check the 'inc' from 'end'
 */
   /*if (!sinc || IS_CONST(STflag[end-1]))*/
   if (!sinc || IS_CONST(STflag[inc-1]))
      sinc = GetSignInfo(inc);
   if (sst)
   {
      if (sst == 2) flag |= L_PSTART_BIT;
      else if (sst == 1) flag |= L_PSTART_BIT | L_ZSTART_BIT;
      else if (sst == -1) flag |= L_NSTART_BIT | L_ZSTART_BIT;
      else /* if (sst == -2) */ flag |= L_NSTART_BIT;
   }
   if (send)
   {
      if (send == 2) flag |= L_PEND_BIT;
      else if (send == 1) flag |= L_PEND_BIT | L_ZEND_BIT;
      else if (send == -1) flag |= L_NEND_BIT | L_ZEND_BIT;
      else /* if (send == -2) */ flag |= L_NEND_BIT;
   }
   if (sinc)
   {
      if (sinc == -2) flag |= L_NINC_BIT;
      else if (sinc == 3) flag |= L_MINC_BIT;
      else flag |= L_PINC_BIT;
   }
/*
 * Majedul: the production "loop->loop_beg loop_markups loop_body" only used 
 * for optloop right now. 
 */
   lp = optloop = NewLoop(flag);
   lp->I = I;
   lp->beg = start;
   lp->end = end;
   lp->inc = inc;

/*   lp->header = NewBasicBlock(NULL, NULL); */
   InsNewInst(NULL, NULL, NULL, CMPFLAG, CF_LOOP_INIT, lp->loopnum, 0);
   flag = STflag[start-1];
   assert(!IS_PTR(flag));
   if (IS_CONST(flag)) DoMove(I, start);
   else
   {
      ireg = LocalLoad(start);
      LocalStore(I, ireg);
      GetReg(-1);
   }
   InsNewInst(NULL, NULL, NULL, CMPFLAG, CF_LOOP_BODY, lp->loopnum, 0);
   sprintf(lnam, "_LOOP_%d", lp->loopnum);
   lp->body_label = STstrconstlookup(lnam);
   InsNewInst(NULL, NULL, NULL, LABEL, lp->body_label, lp->loopnum, 0);
   return(lp);
}

void FinishLoop(LOOPQ *lp)
/*
 * After loop_begin and loop_body written, writes loop_update and test
 */
{
   short ireg, iend;
   int flag;
   char lnam[64];
/*
 * Update loop counter
 */
   InsNewInst(NULL, NULL, NULL, CMPFLAG, CF_LOOP_UPDATE, lp->loopnum, 0);
   DoArith(lp->I, lp->I, '+', lp->inc);
   InsNewInst(NULL, NULL, NULL, CMPFLAG, CF_LOOP_TEST, lp->loopnum, 0);
   ireg = LocalLoad(lp->I);
   flag = STflag[lp->end-1];
   if (IS_CONST(flag)) iend = lp->end;
   else iend = -LocalLoad(lp->end);
   InsNewInst(NULL, NULL, NULL, CMP, -ICC0, -ireg, iend);
   InsNewInst(NULL, NULL, NULL, (lp->flag & L_NINC_BIT) ? JGT : JLT, 
              -PCREG, -ICC0, lp->body_label);
   InsNewInst(NULL, NULL, NULL, CMPFLAG, CF_LOOP_END, lp->loopnum, 0);
   sprintf(lnam, "_LOOP_END_%d", lp->loopnum);
   lp->end_label = STstrconstlookup(lnam);
   InsNewInst(NULL, NULL, NULL, LABEL, lp->end_label, lp->loopnum, 0);
   GetReg(-1);
}

void DoGoto(char *name)
{
   InsNewInst(NULL, NULL, NULL, JMP, -PCREG, STlabellookup(name), 0);
}
void DoLabel(char *name)
{
   InsNewInst(NULL, NULL, NULL, LABEL, STlabellookup(name), 0, 0);
}

void DoIf(char op, short id, short avar, char *labnam)
{
   int flag, type;
   short k, cmp, ireg, ireg1, freg0, freg1, br, label;

   label = STlabellookup(labnam);
   assert(id > 0 && label > 0 && avar > 0);
   flag = STflag[avar-1];
   type = FLAG2PTYPE(STflag[id-1]);
   if (type == T_INT)
   {
      ireg = LocalLoad(id);
      if (IS_CONST(flag)) ireg1 = -avar;
      else ireg1 = LocalLoad(avar);
      if (op != '&') InsNewInst(NULL, NULL, NULL, CMP, -ICC0, -ireg, -ireg1);
      else
         #ifdef PPC
            InsNewInst(NULL, NULL, NULL, ANDCC, -ireg, -ireg, -ireg1);
         #else
            InsNewInst(NULL, NULL, NULL, CMPAND, -ireg, -ireg, -ireg1);
         #endif
      switch(op)
      {
      case '>':
         InsNewInst(NULL, NULL, NULL, JGT, -PCREG, -ICC0, label);
         break;
      case 'g':  /* >= */
         InsNewInst(NULL, NULL, NULL, JGE, -PCREG, -ICC0, label);
         break;
      case '<':
         InsNewInst(NULL, NULL, NULL, JLT, -PCREG, -ICC0, label);
         break;
      case 'l':  /* <= */
         InsNewInst(NULL, NULL, NULL, JLE, -PCREG, -ICC0, label);
         br = JLE;
         break;
      case '=':
         InsNewInst(NULL, NULL, NULL, JEQ, -PCREG, -ICC0, label);
         break;
      case '!':
         InsNewInst(NULL, NULL, NULL, JNE, -PCREG, -ICC0, label);
         break;
      case '&':
         InsNewInst(NULL, NULL, NULL, JEQ, -PCREG, -ICC0, label);
         break;
      }
   }
#ifndef X869999
   else
   {
      freg0 = LocalLoad(id);
      freg1 = LocalLoad(avar);
      cmp = (type == T_FLOAT) ? FCMP : FCMPD;
      switch(op)
      {
      case '>':
         br = JGT;
         break;
      case 'g':  /* >= */
         br = JGE;
         break;
      case '<':
         br = JLT;
         break;
      case 'l':  /* <= */
         br = JLE;
         break;
      case '=':
         br = JEQ;
         break;
      case '!':
         br = JNE;
         break;
      default:
         fko_error(__LINE__, "Illegal fp comparitor '%c'\n", op);
      }
      InsNewInst(NULL, NULL, NULL, cmp, -FCC0, -freg0, -freg1);
      InsNewInst(NULL, NULL, NULL, br, -PCREG, -FCC0, label);
   }
#elif 0
      else
      {
         assert(type == T_FLOAT || type == T_DOUBLE);
         ireg = GetReg(T_INT);
         freg0 = LocalLoad(id);
         freg1 = LocalLoad(avar);
         switch(op)
         {
         case '>':
            k = STiconstlookup(2);
            br = JEQ;
            break;
         case 'l':  /* <= */
            k = STiconstlookup(2);
            br = JNE;
            break;
         case 'g':  /* >= */
            k = STiconstlookup(1);
            br = JEQ;
            break;
         case '<':
            k = STiconstlookup(1);
            br = JNE;
            break;
         case '=':
            k = STiconstlookup(0);
            br = JNE;
            break;
         case '!':
            k = STiconstlookup(0);
            br = JEQ;
            break;
         }
         InsNewInst(NULL, NULL, NULL, (type == T_FLOAT) ? FCMPW:FCMPWD, 
                    -freg0, -freg1, k);
         InsNewInst(NULL, NULL, NULL, (type == T_FLOAT) ? CVTBFI:CVTBDI,
                    -ireg, -freg0, 0);
         InsNewInst(NULL, NULL, NULL, CMP, -ICC0, -ireg, STiconstlookup(0));
         InsNewInst(NULL, NULL, NULL, br, -PCREG, -ICC0, label);
      }
#endif
/*
 * Majedul: the scope of if-statement is finished. Why not we re-initialize
 * The register bank.
 * NOTE: it doesn't work. If-statement doesn't store any value, may be 
 * That's the reason, we can't initiate regs here.
 * HERE HERE, fails for which kernels ????
 */
 GetReg(-1); 
}

/*=============================================================================
 *       To Manage 2D array access
 *
 *============================================================================*/
short AddOpt2dArrayDeref(short base, short hdm, short ldm, int unroll)
/*
 * special for optimized 2d array access, depends on the unroll factor
 * base = STarr index of the base array ptr,
 * hdm = st index of high dimension
 * ldm = st index of low dimension
 * unroll = unroll factor
 */
{
   int i;
   int hdmi, ldmi;
   short dt, ptr1, ptr2, ptr3;
   short ldaS, nldaS, m3ldaS;
/*
 * assuming both indice of the array are const
 */
   assert(IS_CONST(STflag[hdm-1]) && IS_CONST(STflag[ldm-1]));
   assert(STarr[base-1].colptrs && STarr[base-1].colptrs[0]);

   hdmi = SToff[hdm-1].i;
   ldmi = SToff[ldm-1].i;
/*
 * init ptr and lda
 */
   ptr1 = STarr[base-1].colptrs[1]; /* atleast one ptr is needed in all cases */
   ldaS = STarr[base-1].cldas[1];
/*
 * special ldas 
 */
   if (unroll >= 3)
      nldaS = STarr[base-1].cldas[2];
   
   if (unroll==6 || unroll==7 || unroll==13 || unroll==14 )
      m3ldaS = STarr[base-1].cldas[3];
/*
 * extra pointer 
 */
   if (unroll >= 8)
      ptr2 = STarr[base-1].colptrs[2];

   if (unroll >= 15)
      ptr3 =  STarr[base-1].colptrs[3];
/*
 * figure out the DT reference for specific column and unroll factor 
 * Assumption: hdm <= unroll which is already verified while parsing the 
 * grammar for prederef in hil_gram.y
 */
   if (unroll==1 || unroll==2)
   {
      switch(hdmi)
      {
         case 0:
            dt = AddDerefEntry(ptr1, 0, 1, ldmi, ptr1);
            break;
         case 1:
            dt = AddDerefEntry(ptr1, ldaS, 1, ldmi, ptr1);
            break;
      }
   }
   else if (unroll >=3 && unroll <=16)
   {
/*
 *    hdmi 0-4 are same for all unroll factor
 */
      switch(hdmi)
      {
         case 0:
            dt = AddDerefEntry(ptr1, nldaS, 2, ldmi, ptr1);
            break;
         case 1:
            dt = AddDerefEntry(ptr1, nldaS, 1, ldmi, ptr1);
            break;
         case 2:
            dt = AddDerefEntry(ptr1, 0, 1, ldmi, ptr1);
            break;
         case 3:
            dt = AddDerefEntry(ptr1, ldaS, 1, ldmi, ptr1);
            break;
         case 4:
            dt = AddDerefEntry(ptr1, ldaS, 2, ldmi, ptr1);
            break;
      }
/*
 *    for unroll fatcor 6 and 7, index 5 and 6 are different 
 */
      if (unroll==6 || unroll==7)
      {
         switch(hdmi)
         {
            case 5:
               dt = AddDerefEntry(ptr1, m3ldaS, 1, ldmi, ptr1);
               break;
            case 6:
               dt = AddDerefEntry(ptr1, ldaS, 4, ldmi, ptr1);
               break;
         }
      }
/*
 *    symmetrical unroll factor 8~12 and 15~16 
 */
      else if( (unroll >= 8 && unroll <= 12) || 
               (unroll >=15 && unroll <= 16) )
      {
         switch(hdmi)
         {
            case 5:
               dt = AddDerefEntry(ptr2, nldaS, 2, ldmi, ptr2);
               break;
            case 6:
               dt = AddDerefEntry(ptr2, nldaS, 1, ldmi, ptr2);
               break;
            case 7:
               dt = AddDerefEntry(ptr2, 0, 1, ldmi, ptr2);
               break;
            case 8:
               dt = AddDerefEntry(ptr2, ldaS, 1, ldmi, ptr2);
               break;
            case 9:
               dt = AddDerefEntry(ptr2, ldaS, 2, ldmi, ptr2);
               break;
            case 10:
               dt = AddDerefEntry(ptr1, ldaS, 8, ldmi, ptr1);
               break;
            case 11:
               dt = AddDerefEntry(ptr2, ldaS, 4, ldmi, ptr2);
               break;
            case 12:
               dt = AddDerefEntry(ptr3, 0, 1, ldmi, ptr3);
               break;
            case 13:
               dt = AddDerefEntry(ptr3, ldaS, 1, ldmi, ptr3);
               break;
            case 14:
               dt = AddDerefEntry(ptr3, ldaS, 2, ldmi, ptr3);
               break;
            case 15:
               dt = AddDerefEntry(ptr2, ldaS, 8, ldmi, ptr2);
               break;
         }
      }
      else if (unroll == 13 || unroll == 14)
      {
         switch(hdmi)
         {
            case 5:
               dt = AddDerefEntry(ptr2, nldaS, 4, ldmi, ptr2);
               break;
            case 6:
               dt = AddDerefEntry(ptr1, ldaS, 4, ldmi, ptr1);
               break;
            case 7:
               dt = AddDerefEntry(ptr2, nldaS, 2, ldmi, ptr2);
               break;
            case 8:
               dt = AddDerefEntry(ptr2, nldaS, 1, ldmi, ptr2);
               break;
            case 9:
               dt = AddDerefEntry(ptr2, 0, 1, ldmi, ptr2);
               break;
            case 10:
               dt = AddDerefEntry(ptr2, ldaS, 1, ldmi, ptr2);
               break;
            case 11:
               dt = AddDerefEntry(ptr2, ldaS, 2, ldmi, ptr2);
               break;
            case 12:
               dt = AddDerefEntry(ptr2, m3ldaS, 1, ldmi, ptr2);
               break;
            case 13:
               dt = AddDerefEntry(ptr2, ldaS, 4, ldmi, ptr2);
               break;
         }
      }
   }
   else
      fko_error(__LINE__, "unroll fatcor not supported yet");

#if 0
   switch(unroll)
   {
      case 1:     /* unroll factor is 1, access each column at once */
         dt = AddDerefEntry(ptr, 0, 1, ldmi, ptr);
         break;

      case 2:
         if (!hdmi)
            dt = AddArrayDeref(ptr, 0, ldmi);
         else if (hdmi==1)
            dt = AddDerefEntry(ptr, ldaS, 1, ldmi, ptr);
         else
            fko_error(__LINE__, "wrong index !!!");
         break;

      case 3:
         if (!hdmi)
            dt = AddDerefEntry(ptr, nldaS, 1, ldmi, ptr);
         else if (hdmi==1)
            dt = AddDerefEntry(ptr, 0, 1, ldmi, ptr);
         else if (hdmi == 2)
            dt = AddDerefEntry(ptr, ldaS, 1, ldmi, ptr);
         else
            fko_error(__LINE__, "wrong index !!!");
         break;

      case 4:
         if (!hdmi)
            dt = AddDerefEntry(ptr, nldaS, 1, ldmi, ptr);
         else if (hdmi==1)
            dt = AddDerefEntry(ptr, 0, 1, ldmi, ptr);
         else if (hdmi == 2)
            dt = AddDerefEntry(ptr, ldaS, 1, ldmi, ptr);
         else if (hdmi == 3)
            dt = AddDerefEntry(ptr, ldaS, 2, ldmi, ptr);
         else
            fko_error(__LINE__, "wrong index !!!");
         break;

      case 5:
         if (!hdmi)
            dt = AddDerefEntry(ptr, nldaS, 2, ldmi, ptr);
         else if (hdmi==1)
            dt = AddDerefEntry(ptr, nldaS, 1, ldmi, ptr);
         else if (hdmi == 2)
            dt = AddDerefEntry(ptr, 0, 1, ldmi, ptr);
         else if (hdmi == 3)
            dt = AddDerefEntry(ptr, ldaS, 1, ldmi, ptr);
         else if (hdmi == 4)
            dt = AddDerefEntry(ptr, ldaS, 2, ldmi, ptr);
         else
            fko_error(__LINE__, "wrong index !!!");
         break;

      case 6:
         if (!hdmi)
            dt = AddDerefEntry(ptr, nldaS, 2, ldmi, ptr);
         else if (hdmi==1)
            dt = AddDerefEntry(ptr, nldaS, 1, ldmi, ptr);
         else if (hdmi == 2)
            dt = AddDerefEntry(ptr, 0, 1, ldmi, ptr);
         else if (hdmi == 3)
            dt = AddDerefEntry(ptr, ldaS, 1, ldmi, ptr);
         else if (hdmi == 4)
            dt = AddDerefEntry(ptr, ldaS, 2, ldmi, ptr);
         else if (hdmi == 5)
            dt = AddDerefEntry(ptr, m3ldaS, 1, ldmi, ptr);
         else
            fko_error(__LINE__, "wrong index !!!");
         break;
      
      case 7:
         if (!hdmi)
            dt = AddDerefEntry(ptr, nldaS, 2, ldmi, ptr);
         else if (hdmi==1)
            dt = AddDerefEntry(ptr, nldaS, 1, ldmi, ptr);
         else if (hdmi == 2)
            dt = AddDerefEntry(ptr, 0, 1, ldmi, ptr);
         else if (hdmi == 3)
            dt = AddDerefEntry(ptr, ldaS, 1, ldmi, ptr);
         else if (hdmi == 4)
            dt = AddDerefEntry(ptr, ldaS, 2, ldmi, ptr);
         else if (hdmi == 5)
            dt = AddDerefEntry(ptr, m3ldaS, 1, ldmi, ptr);
         else if (hdmi == 6)
            dt = AddDerefEntry(ptr, ldaS, 4, ldmi, ptr);
         else
            fko_error(__LINE__, "wrong index !!!");
         break;

      case 8: 
         if (!hdmi)
            dt = AddDerefEntry(ptr, nldaS, 2, ldmi, ptr);
         else if (hdmi==1)
            dt = AddDerefEntry(ptr, nldaS, 1, ldmi, ptr);
         else if (hdmi == 2)
            dt = AddDerefEntry(ptr, 0, 1, ldmi, ptr);
         else if (hdmi == 3)
            dt = AddDerefEntry(ptr, ldaS, 1, ldmi, ptr);
         else if (hdmi == 4)
            dt = AddDerefEntry(ptr, ldaS, 2, ldmi, ptr);
         else if (hdmi == 5)
            dt = AddDerefEntry(ptr, m3ldaS, 1, ldmi, ptr);
         else if (hdmi == 6)
            dt = AddDerefEntry(ptr, ldaS, 4, ldmi, ptr);
         else
            fko_error(__LINE__, "wrong index !!!");
         break;

      default:
         fko_error(__LINE__, "unroll fatcor not supported yet");
         break;
   }
#endif   
   return dt;
}

