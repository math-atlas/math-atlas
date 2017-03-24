/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
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
   else /* variable load */
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
/*
 *    added to support VEC HIL code
 */
#if 1
      case T_VFLOAT:
         inst = VFLD;
         break;
      case T_VDOUBLE:
         inst = VDLD;
         break;
#endif
      default:
         fprintf(stderr, "type=%d!!\n\n", typ);
      }
   }
   InsNewInst(NULL, NULL, NULL, inst, -reg, SToff[id].sa[2], 0);
   return(reg);
}

static int Scalar2VRegLoad(short sid)
/*
 * this function loads a scalar variable into vector register 
 */
{
   short reg, typ, vtyp;
   enum inst inst;
   sid--;
   vtyp = 0;
   typ = FLAG2TYPE(STflag[sid]);
   if (!IS_CONST(STflag[sid]) && !IS_PTR(STflag[sid]))
   {
      switch(typ)
      {
         case T_FLOAT:
            vtyp = T_VFLOAT;
            inst = VFLDS;
            break;
         case T_DOUBLE:
            vtyp = T_VDOUBLE;
            inst = VDLDS;
            break;
         default:
            fko_error(__LINE__, "type not supported!!");
      }
   }
   else
      fko_error(__LINE__, "not supported for const/ptr!!");
   
   reg = GetReg(vtyp);
   InsNewInst(NULL, NULL, NULL, inst, -reg, SToff[sid].sa[2], 0);
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
/*
 *    added to support VEC HIL code
 */
#if 1
      case T_VFLOAT:
         inst = VFST;
         break;
      case T_VDOUBLE:
         inst = VDST;
         break;
#endif
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
   int type;
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
/*
 *    FIXME: need to add const_init for each floating point const, by this way
 *    we can have a placeholder for that const in stack. We should/will create 
 *    the placeholder whenever user uses a floating point const, but until it's
 *    upto user to manage it correctly. 
 *    Don't need any place-holder for the value 0.0. we can use FZERO LIL 
 *    inst for that. It won't maintain the load->Arith->store structure of LIL.
 *    Need to check whether it creates any problem in later transformation!!!
 */
#if 0      
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
#else
      if (FLAG2TYPE(STflag[dest-1]) != FLAG2TYPE(sflag))
         fko_error(__LINE__, "Conversions of constant not yet supported");

      if (type == T_INT)
      { 
         if (SToff[src-1].i == 0)
            InsNewInst(NULL, NULL, NULL, XOR, -rsrc, -rsrc, -rsrc);
         else
         {
            mov = MOV;
            InsNewInst(NULL, NULL, NULL, mov, -rsrc, src, 0);
         }
      }
      else if (type == T_FLOAT)
      {
         if (SToff[src-1].f == 0.0)
            InsNewInst(NULL, NULL, NULL, FZERO, -rsrc, 0, 0);
         else
            fko_error(__LINE__, 
               "Floating point const other than zero must be defined before");
      }
      else if (type == T_DOUBLE)
      {
         if (SToff[src-1].d == 0.0)
            InsNewInst(NULL, NULL, NULL, FZEROD, -rsrc, 0, 0);
         else
            fko_error(__LINE__, 
               "Floating point const other than zero must be defined before");
      }
      else
         fko_error(__LINE__, "unsupported constant!");
      
      LocalStore(dest, rsrc);
#endif
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
/*
 * FIXED: we multiply the offset with mul always... for architectures which 
 * don't support LoadMul/ConstAndIndex, they will be fixed in FixDeref() 
 */
   offset *= mul;
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
   short type;
   short sta;

   ptr--;
   type = FLAG2TYPE(STflag[SToff[ptr].sa[0]-1]);
/* fprintf(stderr, "FixDeref: [%d, %d, %d, %d]\n", SToff[ptr].sa[0], 
 *         SToff[ptr].sa[1], SToff[ptr].sa[2], SToff[ptr].sa[3]); */
/*
 * FIXED: incase of optimized 2D ptr, we want to keep the DT intact.
 * it is applied only on x86. so, it's safe to keep that unchanged
 * NOTE: mulitplying offset with datatype is done while creating the DT entry.
 */
   sta = STarrColPtrlookup(SToff[ptr].sa[0]); 
   if ( (FKO_FLAG & IFF_OPT2DPTR) 
         && sta && STarr[sta-1].ndim > 1)
   {
/*
 *    Load beginning of array
 */
      SToff[ptr].sa[0] = -LocalLoad(SToff[ptr].sa[0]);
/*
 *    Load index register if needed
 *    we kept mul and con intact 
 */
      if (SToff[ptr].sa[1])
         SToff[ptr].sa[1] = -LocalLoad(SToff[ptr].sa[1]);      
   }
   else /* kept the old code and logic unchanged for all other case*/
   {
/*
 *    Load beginning of array
 */
      SToff[ptr].sa[0] = -LocalLoad(SToff[ptr].sa[0]);
/*
 *    Multiply constant by mul
 *    FIXED: for const index, we already multiply datasize at the beginning
 *    ref AddArrayDeref()
 */
      /*if (SToff[ptr].sa[2]) SToff[ptr].sa[3] *= SToff[ptr].sa[2];*/
/*
 *    Load index register if needed
 */
      if (SToff[ptr].sa[1])
      {
         SToff[ptr].sa[1] = -LocalLoad(SToff[ptr].sa[1]);
/*
 *       Some architectures cannot multiply the index register by some (or any)
 *       constants, and in this case generate an extra shift instruction
 */
         if (!ArchHasLoadMul(SToff[ptr].sa[2]))
         {
            InsNewInst(NULL, NULL, NULL, SHL, SToff[ptr].sa[1], SToff[ptr].sa[1], 
                       STiconstlookup(type2shift(type)));
            SToff[ptr].sa[2] = 1;
         }
/*
 *       On machines with fixed-size instructions, you usually need to choose
 *       _either_ an index register, _or_ a constant addition.  If we have both
 *       on such a machine, add the constant to the index register
 */
         #ifndef ArchConstAndIndex
            if (SToff[ptr].sa[3])
            {
               InsNewInst(NULL, NULL, NULL, ADD, SToff[ptr].sa[1], 
                          SToff[ptr].sa[1], 
                          STiconstlookup(GetDTcon(SToff[ptr].sa[3])));
               SToff[ptr].sa[3] = 0;
            }
         #endif
      }
   }
}

void DoArrayStore(short ptr, short id)
{
   short lreg, k, type;

   k = ptr-1;
   type = FLAG2TYPE(STflag[id-1]);
   #if IFKO_DEBUG_LEVEL > 1
      fprintf(stderr, "pnam=%s, pflag=%d, idname='%s', idflag=%d\n", 
              STname[SToff[k].sa[0]-1], STflag[SToff[k].sa[0]-1], 
              STname[id-1], STflag[id-1]);
   #endif
   lreg = LocalLoad(id);
/*
 * NOTE: we added vector store where the array type and variable type
 * may not exactly match. we may have DOUBLE pointer but the variable is 
 * T_VDOUBLE.
 */
#if 0   
   assert(FLAG2TYPE(STflag[SToff[k].sa[0]-1]) == type);
#else
   if (IS_VEC(STflag[id-1]))
   {
      if ( (IS_VDOUBLE(STflag[id-1]) && !IS_DOUBLE(STflag[SToff[ptr-1].sa[0]-1]))
        || (IS_VFLOAT(STflag[id-1]) && !IS_FLOAT(STflag[SToff[ptr-1].sa[0]-1])) )
         fko_error(__LINE__, "type mismatch for vector store!");
   }
   else
   {
      if (FLAG2TYPE(STflag[SToff[k].sa[0]-1]) != type)
         fko_error(__LINE__, "type mismatch for store!");
   }
#endif
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
#if 1
   case T_VFLOAT:
      InsNewInst(NULL, NULL, NULL, VFST, ptr, -lreg, 0);
      break;
   case T_VDOUBLE:
      InsNewInst(NULL, NULL, NULL, VDST, ptr, -lreg, 0);
      break;
#endif
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
   short areg, type, ld;
   #ifdef X86_64   
      short k;
      k = (ptr-1)<<2;
   #endif
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
#if 1
   case T_VFLOAT:
      ld = VFLD;
      break;
   case T_VDOUBLE:
      ld = VDLD;
      break;
#endif
   default:
      fko_error(__LINE__, "Unknown type %d\n", type);
   }
   areg = GetReg(type);
   InsNewInst(NULL, NULL, NULL, ld, -areg, ptr, 0);
   LocalStore(id, areg);
   GetReg(-1);
}

void DoArrayBroadcast(short id, short ptr)
/*
 * special load which load scalar element from memory and broadcast it through
 * vector register;
 * id : ST index of vector variable, ptr: DT entry
 */
{
   short areg, type, ld;

   FixDeref(ptr);

   type = FLAG2TYPE(STflag[id-1]); 
   switch(type)
   {
      case T_VFLOAT:
         ld = VFLDSB;
         break;
      case T_VDOUBLE:
         ld = VDLDSB;
         break;
      default:
         fko_error(__LINE__, "Unknown type %d\n", type);
   }
   areg = GetReg(type);
   InsNewInst(NULL, NULL, NULL, ld, -areg, ptr, 0);
   LocalStore(id, areg);
   GetReg(-1);
}

void DoArrayPrefetch(short lvl, short ptrderef, int wpf)
{
   enum inst inst;

   FixDeref(ptrderef);
   
   if (wpf) inst = PREFW;
   else inst = PREFR;
   
   InsNewInst(NULL, NULL, NULL, inst, 0 , ptrderef, lvl);
}

void HandlePtrArithNoSizeofUpate(short dest, short src0, char op, short src1)
/*
 * this function is similar to HandlePtrArith but doesn't multiply lda with 
 * size to fix the offset. 
 */
{
   short rs0, rs1, flag;
   if (op != '+' && op != '-')
      fko_error(__LINE__,"pointers may take only + and - operators");
   if (!IS_PTR(STflag[src0-1]))
      fko_error(__LINE__,"Expecting <ptr> = <ptr> + <int>");
      
   rs0 = LocalLoad(src0); /* load src0 */ 
   flag = STflag[src1-1];
   if (!IS_CONST(flag))
      rs1 = LocalLoad(src1); /* load src1 */
   else
      fko_error(__LINE__,"expecting var as 2nd src as a special case");

   InsNewInst(NULL, NULL, NULL, op == '+' ? ADD : SUB, -rs0, -rs0, -rs1);
   LocalStore(dest, rs0);
   GetReg(-1);
}

void HandlePtrArith(short dest, short src0, char op, short src1)
/*
 * Ptr arithmetic must be of form <ptr> = <ptr> [+,-] <int/const>
 */
{
   short rs0, rs1, flag, type, dflag;
   #ifdef X86_64
      short k;
   #endif
   if (op != '+' && op != '-')
      fko_error(__LINE__,"pointers may take only + and - operators");
   if (!IS_PTR(STflag[src0-1]))
      fko_error(__LINE__,"Expecting <ptr> = <ptr> + <int>");
   
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
         fko_error(__LINE__,"Pointers may only be incremented by integers");
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
   short arrid, ur, arrN;
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
   short rd, rs0, rs1, rs2, type;
   enum inst inst;
   extern int DTnzerod, DTnzero, DTabs, DTabsd;

   if (IS_PTR(STflag[dest-1]))
   {
      if (SToff[dest-1].sa[3]) /* 2D array pointer? */
         Handle2dArrayPtrArith(dest, src0, op, src1);
      else
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
         /*fprintf(stderr, "DIV %d, %d, %d\n", -rd, -rs0, -rs1);*/
         LocalStore(dest, rd);
         GetReg(-1);
         return;
      }
   #endif
/*
 * if 3 operands supported and dest != src0, use separate regs for destination
 * NOTE: for MAC, dest is also in use, so load dest later
 */
   rs0 = LocalLoad(src0);
   if (op != 'm' && dest != src0)
   {
      switch (type)
      {
         case T_INT:
            #ifdef ArchHasINTthreeOps
               rd = GetReg(type);
            #else
               rd = rs0;
            #endif
            break;
         case T_FLOAT: case T_DOUBLE: 
            #ifdef ArchHasFPthreeOps
               rd = GetReg(type);
            #else
               rd = rs0;
            #endif
            break;
         case T_VINT:
            #ifdef ArchHasVINTthreeOps
               rd = GetReg(type);
            #else
               rd = rs0;
            #endif
            break;
         case T_VFLOAT: case T_VDOUBLE:
            #ifdef ArchHasVFPthreeOps
               rd = GetReg(type);
            #else
               rd = rs0;
            #endif
            break;
         default: 
            fko_error(__LINE__, "Unknown type!!!");
      }
   }
   else
      rd = rs0;
/*
 * 2nd src
 */
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
      fko_error(__LINE__,"modulo and shift not defined for non-integer types");
/*
 * find appropriate inst
 */
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
      case T_VFLOAT:
         inst = VFADD;
         break;
      case T_VDOUBLE:
         inst = VDADD;
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
      case T_VFLOAT:
         inst = VFSUB;
         break;
      case T_VDOUBLE:
         inst = VDSUB;
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
      case T_VFLOAT:
         inst = VFMUL;
         break;
      case T_VDOUBLE:
         inst = VDMUL;
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
      case T_VFLOAT:
         inst = VFDIV;
         break;
      case T_VDOUBLE:
         inst = VDDIV;
         break;
      }
      break;
   case '%': /* dest = src0 % src1 */
      fko_error(__LINE__,"% not yet supported");
      break;
   case '>': /* dest = src0 >> src1 */
      if (type == T_INT) inst = SHR;
      break;
   case '<': /* dest = src0 << src1 */
      if (type == T_INT) inst = SHL;
      break;
   case 'm': /* dest += src0 * src1 */
      /*if (type == T_FLOAT || type == T_DOUBLE)*/
      if (type == T_FLOAT || type == T_DOUBLE || type == T_VFLOAT 
            || type == T_VDOUBLE)
      {
         #ifdef ArchHasMAC
            rd = LocalLoad(dest);
            if (type == T_FLOAT) inst = FMAC;
            else if (type == T_DOUBLE) inst = FMACD;
            else if (type == T_VFLOAT) inst = VFMAC;
            else if (type == T_VDOUBLE) inst = VDMAC;
            else 
               fko_error(__LINE__, "MAC not supported with this type!");
         #else
            switch(type)
            {
               case T_FLOAT: 
                  #ifdef ArchHasFPthreeOps
                     rs2 = GetReg(type);
                  #else
                     rs2 = rs0;
                  #endif
                  InsNewInst(NULL, NULL, NULL, FMUL, -rs2, -rs0, -rs1);
                  inst = FADD;
                  break;
               case T_DOUBLE:
                  #ifdef ArchHasFPthreeOps
                     rs2 = GetReg(type);
                  #else
                     rs2 = rs0;
                  #endif
                  InsNewInst(NULL, NULL, NULL, FMULD, -rs2, -rs0, -rs1);
                  inst = FADDD;
                  break;
               case T_VFLOAT: 
                  #ifdef ArchHasVFPthreeOps
                     rs2 = GetReg(type);
                  #else
                     rs2 = rs0;
                  #endif
                  InsNewInst(NULL, NULL, NULL, VFMUL, -rs2, -rs0, -rs1);
                  inst = VFADD;
               case T_VDOUBLE:
                  #ifdef ArchHasVFPthreeOps
                     rs2 = GetReg(type);
                  #else
                     rs2 = rs0;
                  #endif
                  InsNewInst(NULL, NULL, NULL, VDMUL, -rs2, -rs0, -rs1);
                  inst = VDADD;
                  break;
               default: 
                  fko_error(stderr, "MAC not supported with this type!");
            }
/*
 *          for addition, dest is also src0
 */
            rs1 = rs2;
            rs0 = rd = LocalLoad(dest);
         #endif
      }
      else fko_error(__LINE__,"MAC available for floating point operands only!");
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
 * Majedul: rd should be stored to dest (not rs0). 
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
   int retreg;
   /*int srcreg;*/
   int mov, ld, type;
   #ifdef X86_32
      extern int DTx87, DTx87d;
   #endif
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
   short cmp, ireg, ireg1, freg0, freg1, br, label;

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
   int hdmi, ldmi, dsize;
   short dt, ptr1, ptr2, ptr3;
   short flag;
   short ldaS, nldaS, m3ldaS;
/*
 * assuming both indice of the array are const
 */
   assert(IS_CONST(STflag[hdm-1]) && IS_CONST(STflag[ldm-1]));
   assert(STarr[base-1].colptrs && STarr[base-1].colptrs[0]);

   hdmi = SToff[hdm-1].i;
   ldmi = SToff[ldm-1].i;
#if 1
/*
 * FIXED: lower dimension will be used as offset. It should be multiplied by
 * the datatype 
 */
   dsize = 4;
   flag = STflag[STarr[base-1].ptr-1];
   if (IS_DOUBLE(flag)) dsize = 8;
   else if (IS_CHAR(flag)) dsize =1;
   assert(!IS_VEC(flag));
   ldmi = ldmi * dsize;
#endif
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

/*=============================================================================
 *    To Manage User Guided Vectorization
 *
 * ===========================================================================*/

void DoVecInit(short vid, struct slist *elem)
{
   int k;
   int flag;
   int vtp;
   struct slist *sl;
   short svar;
   short st0;
   short reg;
   enum inst vshuf, vzero;

/*
 * For now, we will only implement two cases:
 *    1. Vx = {a, a, a, a};
 *    2. Vx = {0.0, 0.0, 0.0, a};
 *    3. Vx = {0,0, 0.0, 0.0, 0.0};
 */

/*
 * 1st elem will always be a scalar var considering only the above two cases
 */
   sl = elem;
   svar = sl->id;
   sl=sl->next;
   flag = STflag[sl->id-1];
   if (IS_CONST(flag))
   {
      if (IS_FLOAT(flag))
      {
         st0 = STfconstlookup(0.0);
         vzero = VFZERO;
      }
      else 
      {
         st0 = STdconstlookup(0.0);
         vzero = VDZERO;
      }
      for (; sl; sl=sl->next)
      {
         if (st0 != sl->id)
            fko_error(__LINE__, "this vec init pattern not supported yet\n");
      }
/*
 *    case 2: Vx = {0.0,0.0,0.0,a};
 *    NOTE: both avx/sse, automatically zerod the upper elements while loading
 *    single element from memory (vmovss/vmovsd). So, we don't need to 
 *    explicitly xor them. 
 *    FIXME: this is for genral LIL. should we consider doing xor the reg???
 */
      if (IS_CONST(STflag[svar-1])) /* case 3*/
      {
         reg = GetReg(FLAG2TYPE(STflag[vid-1]));
         InsNewInst(NULL, NULL, NULL, vzero, -reg, -reg, 0);
      }
      else 
         reg = Scalar2VRegLoad(svar);       
      LocalStore(vid, reg);
      GetReg(-1);
   }
   else
   {
      for (; sl; sl=sl->next)
         if (svar != sl->id)
            fko_error(__LINE__, "this vec init pattern not supported yet\n");
/*
 *    case 1: Vx = {a,a,a,a};
 */
      vtp = FLAG2TYPE(STflag[vid-1]);
      if (vtp == T_VFLOAT)
         vshuf = VFSHUF;
      else
         vshuf = VDSHUF;

      k = STiconstlookup(0);
      reg = Scalar2VRegLoad(svar); 
      InsNewInst(NULL, NULL, NULL, vshuf, -reg, -reg, k);
      LocalStore(vid, reg);
   }
}

void DoReduce(short sid, short vid, char op, short iconst)
{
   int i, k;
   int flag, ne, lgne;
   int *sfcode;
   short r0, r1;
   enum inst vinst, vadd, vmax, vmin, vsst, vld, vshuf;
/*
 * select based on type
 */
   flag = STflag[vid-1];
   if (IS_VFLOAT(flag))
   {
      vadd = VFADD;
      vmax = VFMAX;
      vmin = VFMIN;
      vsst = VFSTS;
      vld = VFLD;
      vshuf = VFSHUF;
   }
   else if (IS_VDOUBLE(flag))
   {
      vadd = VDADD;
      vmax = VDMAX;
      vmin = VDMIN;
      vsst = VDSTS;
      vld = VDLD;
      vshuf = VDSHUF;
   }
   else
      fko_error(__LINE__, "unknown vector type for HIL!");
/*
 * NOTE: reduction depends on the element in vector:
 *       number of operation = lgN
 *       2:    op = 1
 *             dest = (1,0) src = (2,3)
 *             shuffle = 0x33 (actually X3, X for don't care)
 *
 *       4:    op = 2
 *             dest = (3,2,1,0) src = (7,6,5,4)
 *             shuffle = 0xXX76  (or, 0x3276)
 *                       0xXXX5  (or, 0x3215)
 *       8:    op = 3
 *             dest = (7,6,5,4,3,2,1,0) src = (F,E,D,C,B,A,9,8)
 *             shuffle = 0xXXXXFEDC (or, 0x7654FEDC)
 *                       0xXXXXXXBA (or, 0x765432BA)
 *                       0xXXXXXXX9 (or, 0x76543219)
 *       
 */
   ne = vtype2elem(FLAG2TYPE(flag));
   for (i=(ne>>1), lgne=0; i; i=i>>1) lgne++;
   
   if (op != 'S')
   {
      sfcode = malloc(lgne*sizeof(int));
      assert(sfcode);
/*
 *    figure out right shuffle code 
 */
      switch(lgne)
      {
         case 1:
            sfcode[0] = 0x33;
            break;
         case 2:
            sfcode[0] = 0x3276;
            sfcode[1] = 0x3715; /*0x3215;*/ /*need to check for float in sse*/
            break;
         case 3:
            sfcode[0] = 0x7654FEDC;
            sfcode[1] = 0x765432BA;
            sfcode[2] = 0x76CD3289; /*0x76543219;*/
            break;
         default:
            fko_error(__LINE__, 
                  "more than eight elements in vector!!!");
      }
/*
 *    select instruction for operation
 */
      switch (op)
      {
         case 'A':
            vinst = vadd;
            break;
         case 'M':
            vinst = vmax;
            break;
         case 'N':
            vinst = vmin;
            break;
         default: 
            fko_error(__LINE__,"unknown vector reduction!");
      }
/*
 *    LIL for reduction
 */
      r0 = GetReg(FLAG2TYPE(flag));
      r1 = GetReg(FLAG2TYPE(flag));
      
      InsNewInst(NULL, NULL, NULL, vld, -r0, SToff[vid-1].sa[2], 0);
      
      for (i=0; i < lgne; i++)
      {
         InsNewInst(NULL, NULL, NULL, vshuf, -r1, -r0, 
                    STiconstlookup(sfcode[i]));
         InsNewInst(NULL, NULL, NULL, vinst, -r0, -r0, -r1);
      }
      InsNewInst(NULL, NULL, NULL, vsst, SToff[sid-1].sa[2], -r0, 0);
      free(sfcode);
      GetReg(-1);
   }
   else /* VHSEL operation*/
   {
/*
 *    dest = 0x10, 0x3210, 0x76543210
 *    src  = 0x32, 0x7654, 0xFEDCBA98
 */    
      i = SToff[iconst-1].i;
#if 0      
      switch(ne)
      {
         case 2:
            k = 0x10;

            break;
         case 4:
            k = 0x3210;
            break;
         case 8:
            k = 0x76543210;
            break;
         default:
            fko_error(__LINE__, 
                  "more than eight elements in vector!!!");
      }
#if 0      
      mask = ~(0xF << (i*4)); /* mask with all 1 but 0 in the desired position*/
      k = k & mask;           /* zerod the desired location in k */
      mask = (i+ne) <<(i*4);  /* shift val in desired position */
      k = k | mask;           /* insert the val in desired position of k */
      /*k = (~(0xF << (i*4) & k ) |  ((i+ne) << (i*4) | k);*/
#endif
      i = i + ne;
      k = k | i;
#else
#if 0      
      switch(ne)
      {
         case 2:
            k = 0x10; 

            break;
         case 4:
            k = 0x3210;
            break;
         case 8:
            k = 0x76543210;
            break;
         default:
            fko_error(__LINE__, 
                  "supported element count in vector: 2,4,8!!!");
      }
      
#endif
/*
 *    NOTE: FIXME: we will make it more general by introducing 'don't care' 
 *    symbol. We don't need to check for the architecture at that level of 
 *    code then.
 */
      if (ne == 2)
      {
         k = 0x10;
         if (i==1)
            k = k | i;
      }
      else if (ne == 4)
      {
         k = 0x3210;
      #if defined(AVX) && !defined(AVX2)
         if (i > 1)
            k = 0x3230;
      #endif
         k = k | i;
      }
      else if (ne == 8)
      {
         k = 0x76543210;
      #if defined(AVX) && !defined(AVX2)
         if (i > 3)
            k = 0x76547650;
      #endif
         k = k | i;
      }
      else
         fko_error(__LINE__, 
                  "supported element count in vector: 2,4,8!!!");
#endif
      r0 = GetReg(FLAG2TYPE(flag));
      InsNewInst(NULL, NULL, NULL, vld, -r0, SToff[vid-1].sa[2], 0);
      if (i)
         InsNewInst(NULL, NULL, NULL, vshuf, -r0, -r0, STiconstlookup(k));
      InsNewInst(NULL, NULL, NULL, vsst, SToff[sid-1].sa[2], -r0, 0);
      GetReg(-1); 
   }
}
