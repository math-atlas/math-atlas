/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#define ARCH_DECLARE
#include "fko.h"
struct locinit *LIhead=NULL,       /* Locals to be init to constant vals */
               *ParaDerefQ=NULL;   /* Derefs created for parameters */
#ifdef X86_64
   #define ISIZE 8
#else
   #define ISIZE 4
#endif

short type2len(int type)
{
   short len=4;
/*
 * Majedul: in X86_64, PTR is 64 bit but INT is still 32 bit. So, the size of
 * T_INT should be 4 byte; otherwise, it will create problem in loop unroll 
 * for INT array (itst9.b). ArchPtrIsLong may not be appropriate here.    
 */   
#if 0   
   #ifdef ArchPtrIsLong
      if (type == T_INT || type == T_DOUBLE) len = 8;
   #else
      if (type == T_DOUBLE) len = 8;
   #endif
#endif
   if (type == T_DOUBLE) len = 8;
   else if (IS_VEC(type))
      #if defined(X86) && defined(AVX)
         len = 32;
      #else      
         len = 16;
      #endif
   return(len);
}

short type2shift(int type)
{
   short len=2;
/* Majedul: ArchPtrIsLong may not be appropriate here. */   
#if 0    
   #ifdef  ArchPtrIsLong 
      if (type == T_DOUBLE || type == T_INT) len = 3;
   #else
      if (type == T_DOUBLE) len = 3;
   #endif
#endif
   if (type == T_DOUBLE) len = 3;
   else if (IS_VEC(type))
      #if defined(X86) && defined(AVX)
         len = 5;
      #else
         len = 4;
      #endif
   return(len);
}

#ifdef X86
short vtype2elem(int type)
/*
 * returns element count in vector depending on the vector type
 */
{
   short nelem;
   #if defined(AVX)
      if (type == T_VDOUBLE)
         nelem = 4;
      else if (type == T_VFLOAT) 
         nelem = 8;
   #else
      if (type == T_VDOUBLE)
         nelem = 2;
      else if (type == T_VFLOAT) 
         nelem = 4;
   #endif
   else
      fko_error(__LINE__, "Must be a vector type!");
   return nelem;
}

short GetVecAlignByte()
/*
 * alignment needed in bytes for vector unit
 */
{
   #if defined(AVX)
      return(32);
   #else
      return(16);
   #endif
}

int RevealArchMemUses(void)
/*
 * reveals all the mem uses for DTabs/DTnzero, 
 * returns number of changes
 * Note: Do we need the LD of those ABSVAL??? Otherwise format of LIL 
 * may be violated!!
 */
{
   int j;
   BBLOCK *bp;
   INSTQ *ip, *ipN;
   enum inst inst;
   /*short op, ir;*/
   short op3, vr, ir;
   int nchanges;
   extern int DTabs, DTabsd, DTnzero, DTnzerod;
   /*extern int DTabss, DTabsds, DTnzeros, DTnzerods;*/
   extern BBLOCK *bbbase;

   nchanges = 0;
   for (bp=bbbase; bp; bp=bp->down)
   {
      for (ip=bp->ainst1; ip; ip = ip->next)
      {
         inst = ip->inst[0];
/*
 *       Since x86 only supports vector operation for and/xor, we always change
 *       following instructions into compatible x86 inst.
 */
         if (inst == FABS || inst == FABSD || inst == FNEG || inst == FNEGD 
             || inst == VFABS || inst == VDABS || inst == VFNEG 
             || inst == VDNEG)
         {
            j = ireg2type(-ip->inst[1]);
            ir = GetReg(j);
            while(ir == -ip->inst[1] || ir == -ip->inst[2])
               ir = GetReg(j);
         }
         /*if (ip->inst[3] >= 0)*/
         switch(inst)
         {
            case FABS:
               op3 = SToff[DTabs-1].sa[2];
               vr = ir - FREGBEG + VFREGBEG;
               ipN = InsNewInst(NULL, NULL, ip, VFLD, -vr, op3, 0);
               ip->inst[0] = VFSABS;
               ip->inst[3] = -vr;
               GetReg(-1);
               CalcThisUseSet(ipN);
               CalcThisUseSet(ip);
               nchanges++;
               break;
            case FABSD:
               op3 = SToff[DTabsd-1].sa[2];
               vr = ir - DREGBEG + VDREGBEG;
               ipN = InsNewInst(NULL, NULL, ip, VDLD, -vr, op3, 0);
               ip->inst[0] = VDSABS;
               ip->inst[3] = -vr;
               GetReg(-1);
               CalcThisUseSet(ipN);
               CalcThisUseSet(ip);
               nchanges++;
               break;
            case FNEG:
               op3 = SToff[DTnzero-1].sa[2];
               vr = ir - FREGBEG + VFREGBEG;
               ipN = InsNewInst(NULL, NULL, ip, VFLD, -vr, op3, 0);
               ip->inst[0] = VFSNEG;
               ip->inst[3] = -vr;
               GetReg(-1);
               CalcThisUseSet(ipN);
               CalcThisUseSet(ip);
               nchanges++;
               break;
            case FNEGD:
               op3 = SToff[DTnzerod-1].sa[2];
               vr = ir - DREGBEG + VDREGBEG;
               ipN = InsNewInst(NULL, NULL, ip, VDLD, -vr, op3, 0);
               ip->inst[0] = VDSNEG;
               ip->inst[3] = -vr;
               GetReg(-1);
               CalcThisUseSet(ipN);
               CalcThisUseSet(ip);
               nchanges++;
               break;
            case VFABS:
               op3 = SToff[DTabs-1].sa[2];
               j = T_VFLOAT;
               vr = ir;
               ipN = InsNewInst(NULL, NULL, ip, VFLD, -vr, op3, 0);
               ip->inst[3] = -vr;
               GetReg(-1);
               CalcThisUseSet(ipN);
               CalcThisUseSet(ip);
               nchanges++;
                  break;
            case VDABS:
               op3 = SToff[DTabsd-1].sa[2];
               vr = ir;
               ipN = InsNewInst(NULL, NULL, ip, VDLD, -vr, op3, 0);
               ip->inst[3] = -vr;
               GetReg(-1);
               CalcThisUseSet(ipN);
               CalcThisUseSet(ip);
               nchanges++;
               break;
            case VFNEG:
               op3 = SToff[DTnzero-1].sa[2];
               vr = ir;
               ipN = InsNewInst(NULL, NULL, ip, VFLD, -vr, op3, 0);
               ip->inst[3] = -vr;
               GetReg(-1);
               CalcThisUseSet(ipN);
               CalcThisUseSet(ip);
               nchanges++;
               break;
            case VDNEG:
               op3 = SToff[DTnzerod-1].sa[2];
               vr = ir;
               ipN = InsNewInst(NULL, NULL, ip, VDLD, -vr, op3, 0);
               ip->inst[3] = -vr;
               GetReg(-1);
               CalcThisUseSet(ipN);
               CalcThisUseSet(ip);
               nchanges++;
               break;  
            default:
               break;
         }
      }
   }
   if (nchanges)
      CFUSETU2D = INUSETU2D = INDEADU2D = 0;
   return nchanges;
}
#endif

void FindRegUsage(BBLOCK *bbase, int *ni0, int *iregs, 
                  int *nf0, int *fregs, int *nd0, int *dregs)
/*
 * Searches through all instructions to find registers that are used
 */
{
   short op;
   int nd, nf=0, ni=0;
   BBLOCK *bp;
   INSTQ *ip;
   const int iend = IREGBEG+TNIR, fend = FREGBEG+TNFR, dend = DREGBEG+TNDR;
   extern int USEALLIREG;

   for (nd=0; nd < TNIR; nd++) iregs[nd] = USEALLIREG;
   for (nd=0; nd < TNFR; nd++) fregs[nd] = 0;
   for (nd=0; nd < TNDR; nd++) dregs[nd] = 0;
   nd = 0;

   for (bp=bbase; bp; bp = bp->down)
   {
      for(ip=bp->inst1; ip; ip = ip->next)
      {
         op = -ip->inst[1];
         if (op >= IREGBEG && op < iend)
         {
            if (!iregs[op-IREGBEG]++) ni++;
         }
         else if (op >= FREGBEG && op < fend)
         {
            if (!fregs[op-FREGBEG]++) nf++;
         }
         else if (op >= DREGBEG && op < dend)
         {
            if (!dregs[op-DREGBEG]++) nd++;
         }
         op = -ip->inst[2];
         if (op >= IREGBEG && op < iend)
         {
            if (!iregs[op-IREGBEG]++) ni++;
         }
         else if (op >= FREGBEG && op < fend)
         {
            if (!fregs[op-FREGBEG]++) nf++;
         }
         else if (op >= DREGBEG && op < dend)
         {
            if (!dregs[op-DREGBEG]++) nd++;
         }
         op = -ip->inst[3];
         if (op >= IREGBEG && op < iend)
         {
            if (!iregs[op-IREGBEG]++) ni++;
         }
         else if (op >= FREGBEG && op < fend)
         {
            if (!fregs[op-FREGBEG]++) nf++;
         }
         else if (op >= DREGBEG && op < dend)
         {
            if (!dregs[op-DREGBEG]++) nd++;
         }
      }
   }
   #if IFKO_DEBUG_LEVEL > 1
      fprintf(stderr, "\nUSED %d IREGS: ", ni);
      for (op=0; op < TNIR; op++) 
         if (iregs[op]) fprintf(stderr, "%s, ", archiregs[op]);
      fprintf(stderr, "\n");
      fprintf(stderr, "USED %d FREGS: ", nf);
      for (op=0; op < TNFR; op++) 
         if (fregs[op]) fprintf(stderr, "%s, ", archfregs[op]);
      fprintf(stderr, "\n");
      fprintf(stderr, "USED %d DREGS: ", nd);
      for (op=0; op < TNDR; op++)
         if (dregs[op]) fprintf(stderr, "%s, ", archdregs[op]);
      fprintf(stderr, "\n\n");
   #endif

   *ni0 = ni;
   *nf0 = nf;
   *nd0 = nd;
}

int RemoveNosaveregs(int rstart, int nr, int *regs, int *saves)
/*
 * Zeros reg entry in regs that doesn't need to be saved
 * RETURNS: number of registers removed
 */
{
   int n=0, i;
   for (i=0; i < nr; i++)
   {
      if (!saves[i] && regs[i])
      {
         regs[i] = 0;
         n++;
      }
   }
   return(n);
}

int GetRegSaveList(int rstart, int nr, int *regs)
/*
 * takes list where entry i means save reg i, and return list with
 * each entry indicating the actual register to save
 * RETURNS: number of registers to save.
 */
{
   int i, j;
   for (j=i=0; i < nr; i++)
   {
      if (regs[i])
         regs[j++] = i + rstart;
   }
   return(j);
}
int GetArchAlign(int nvd, int nvf, int nvi, int nd, int nf, int nl, int ni)
/*
 *  Returns required architectural alignment given the number of
 *  vector double, vector float, double, float, long, and ints you
 *  want to save (actually, these only need to be boolean).
 */
{
   #ifdef X86_64
/*    return(16); */
/* 
 *    Majedul: Required alignment is changed for AVX. 32 byte alignment may be
 *    required.
 */   
      #ifdef AVX
         int align = 0;
         if (nvd) align = FKO_DVLEN*8;
         else if (nvf) align = FKO_SVLEN*4;
         #ifdef FKO_IVELN
            else if (nvi) align = FKO_IVLEN*4;
         #endif
         else align = 16;
         return (align);
      #else
         return (16);
      #endif
   #else
   int align = 0;
      #ifdef ArchHasVec
         if (nvd) align = FKO_DVLEN*8;
         else if (nvf) align = FKO_SVLEN*4;
         #ifdef FKO_IVLEN
            else if (nvi) align = FKO_IVLEN*4;
         #endif
         #ifdef X86_32
            else if (nd || nf || nl || ni) align = 4;
         #else
         else if (nd || nl) align = 8;
         else if (nf || ni) align = 4;
         #endif
      #else
         #ifdef X86_32
            if (nd || nf || nl || ni) align = 4;
         #else
            if (nd || nl) align = 8;
            else if (nf || ni) align = 4;
         #endif
      #endif
   return(align);
   #endif
}
short dName2Reg(char *rname)
{
   short i;
   for (i=0; i < TNDR; i++)
     if (!strcmp(rname, archdregs[i])) return(i+DREGBEG);
   return(0);
}
short fName2Reg(char *rname)
{
   short i;
   for (i=0; i < TNFR; i++)
     if (!strcmp(rname, archfregs[i])) return(i+FREGBEG);
   return(0);
}
short iName2Reg(char *rname)
{
   short i;
   for (i=0; i < TNIR; i++)
     if (!strcmp(rname, archiregs[i])) return(i+IREGBEG);
   return(0);
}
short GetReg(short type)
/*
 * Get registers for local use, dying if we run out
 * NOTE: no handling of long so far
 */
{
   static int dr=0, fr=0, ir=1, vir=0;
   int iret=0;
   extern int lnno;
   if (type == T_DOUBLE)
   {
      #ifdef SPARC
         if ((fr>>1)<<1 != fr) fr++;
         iret = DREGBEG + (fr>>1);
         fr += 2;
         if (fr > NFR*2)
            fko_error(__LINE__, "Out of double registers on line %d", lnno);
      #else
         iret = DREGBEG + dr;
         if (++dr > NDR)
            fko_error(__LINE__, "Out of double registers on line %d", lnno);
      #endif
   }
#ifdef X86
   else if (type == T_VDOUBLE)
   {
      iret = VDREGBEG + dr;
      if (++dr > NDR)
         fko_error(__LINE__, "Out of double registers on line %d", lnno);
   }
#endif
   else if (type == T_FLOAT)
   {
      iret = FREGBEG + fr;
      if (++fr > NFR)
         fko_error(__LINE__, "Out of float registers on line %d", lnno);
   }
#ifdef X86
   else if (type == T_VFLOAT)
   {
      iret = VFREGBEG + fr;
      if (++fr > NFR)
         fko_error(__LINE__, "Out of double registers on line %d", lnno);
   }
#endif
   else if (type == T_INT)
   {
      iret = IREGBEG + ir;
      if (++ir > NIR)
         fko_error(__LINE__, "Out of integer registers on line %d", lnno);
   }
#ifdef X86
   else if (type == T_VINT)
   {
      iret = VIREGBEG + vir;
      if (++vir > NVIR)
         fko_error(__LINE__, "Out of vec integer registers on line %d", lnno);
   }
#endif
#ifdef X86_64
   else if (type == T_SHORT)
   {
      iret = IREGBEG + ir;
      if (++ir > NSR)
         fko_error(__LINE__, "Out of short registers on line %d", lnno);
   }
#endif
   else
   {
      assert(type == -1);
      dr = fr = 0;
      vir = 0;
      ir = 1;
   }
   return(iret);
}

/*
 * Mandated stack pointer alignment by architecture
 */
#ifdef SPARC
   #define ASPALIGN 8
#elif defined(LINUX_PPC)
   #define ASPALIGN 8
#elif defined(OSX_PPC)
   #define ASPALIGN 4
#elif defined(X86_32)
   #define ASPALIGN 4
#elif defined(X86_64)
   #define ASPALIGN 16
#else
   #define ASPALIGN 4
#endif

void CreateSysLocals()
/*
 *  If required, creates any locals needed to support instructions
 *  NOTE: only the vectors are created, because x86 only supports vector and op
 *  for floating point.
 */
{
#ifdef X86
   extern int DTnzerod, DTabsd, DTnzero, DTabs;
   #ifdef X86_32
      extern int DTx87, DTx87d;
   #endif
   int k;
   if (DTnzerod)
   {
      if (DTnzerod == -1)
      {
         DTnzerod = STdef("_NEGZEROD", T_VDOUBLE | LOCAL_BIT, 0);
         k =AddDerefEntry(-REG_SP, DTnzerod, -DTnzerod, 0, DTnzerod);
         SToff[DTnzerod-1].sa[2] = k; 
      }
      else
      {
         k = SToff[DTnzerod-1].sa[2]-1;
         SToff[k].sa[0] = -REG_SP;
         SToff[k].sa[1] = DTnzerod;
         SToff[k].sa[2] = -DTnzerod;
         /*SToff[k].sa[3] = DTnzerod;*/
         SetDTcon(k+1, DTnzerod);
      }
   }
   if (DTabsd)
   {
      if (DTabsd == -1)
      {
         DTabsd = STdef("_ABSVLD", T_VDOUBLE | LOCAL_BIT, 0);
         k = AddDerefEntry(-REG_SP, DTabsd, -DTabsd, 0, DTabsd);
         SToff[DTabsd-1].sa[2] = k; 
      }
      else
      {
         k = SToff[DTabsd-1].sa[2]-1;
         SToff[k].sa[0] = -REG_SP;
         SToff[k].sa[1] = DTabsd;
         SToff[k].sa[2] = -DTabsd;
         /*SToff[k].sa[3] = DTabsd;*/
         SetDTcon(k+1, DTabsd);
      }
   }
   if (DTnzero)
   {
      if (DTnzero == -1)
      {
         DTnzero = STdef("_NEGZERO", T_VFLOAT | LOCAL_BIT, 0);
         SToff[DTnzero-1].sa[2] = AddDerefEntry(-REG_SP, DTnzero, -DTnzero, 0,
                                                DTnzero);
      }
      else
      {
         k = SToff[DTnzero-1].sa[2]-1;
         SToff[k].sa[0] = -REG_SP;
         SToff[k].sa[1] = DTnzero;
         SToff[k].sa[2] = -DTnzero;
         /*SToff[k].sa[3] = DTnzero;*/
         SetDTcon(k+1, DTnzero);
      }
   }
   if (DTabs)
   {
      if (DTabs == -1)
      {
         DTabs = STdef("_ABSVAL", T_VFLOAT | LOCAL_BIT, 0);
         SToff[DTabs-1].sa[2] = AddDerefEntry(-REG_SP, DTabs, -DTabs, 0,
                                              DTabs);
      }
      else
      {
         k = SToff[DTabs-1].sa[2]-1;
         SToff[k].sa[0] = -REG_SP;
         SToff[k].sa[1] = DTabs;
         SToff[k].sa[2] = -DTabs;
         /*SToff[k].sa[3] = DTabs;*/
         SetDTcon(k+1, DTabs);
      }
   }
   #ifdef X86_32
   if (DTx87)
   {
      if (DTx87 == -1)
      {
         DTx87 = STdef("_x87f", T_FLOAT | LOCAL_BIT, 0);
         k = AddDerefEntry(-REG_SP, DTx87, -DTx87, 0, DTx87);
         SToff[DTx87-1].sa[2] = k; 
      }
      else
      {
         k = SToff[DTx87-1].sa[2]-1;
         SToff[k].sa[0] = -REG_SP;
         SToff[k].sa[1] = DTx87;
         SToff[k].sa[2] = -DTx87;
         /*SToff[k].sa[3] = DTx87;*/
         SetDTcon(k+1, DTx87);
      }
   }
   if (DTx87d)
   {
      if (DTx87d == -1)
      {
         DTx87d = STdef("_x87d", T_DOUBLE | LOCAL_BIT, 0);
         k = AddDerefEntry(-REG_SP, DTx87d, -DTx87d, 0, DTx87d);
         SToff[DTx87d-1].sa[2] = k; 
      }
      else
      {
         k = SToff[DTx87d-1].sa[2]-1;
         SToff[k].sa[0] = -REG_SP;
         SToff[k].sa[1] = DTx87d;
         SToff[k].sa[2] = -DTx87d;
         /*SToff[k].sa[3] = DTx87d;*/
         SetDTcon(k+1, DTx87d);
      }
   }
   #endif
#else
#endif
}

void bitload(INSTQ *next, int reg, int nbits, int I)
/*
 * Given a 32 bit value I, load it nbits at a time
 */
{
   int i, j, k, b, r;
   if (!I)
   {
      InsNewInst(NULL, NULL, next, XOR, -reg, -reg, -reg);
      return;
   }
   for (i=0; !(I & (1<<i)); i++);  /* find least sig bit non-zero bit */
   for (j=31; !(I & (1<<j)); j--); /* find most significant non-zero bit */
/*
 * Can we do it with a simple move?
 */
   if (j < nbits)
      InsNewInst(NULL, NULL, next, MOV, -reg, STiconstlookup(I), 0);
/*
 * Must load & shift
 */
   else
   {
      j++;
      k = (j-i+nbits-1) / nbits - 1;  /* number of ors required */
      r = (j-i) % nbits; 
      if (!r) r = nbits;   /* size of first shift */
      b = I >> (j-r);
      InsNewInst(NULL, NULL, next, MOV, -reg, STiconstlookup(b), 0);
      I ^= b << (j-r);
      j -= r;
      /*for (k; k; k--)*/
      for (; k; k--)
      {
         j -= nbits;
         b = I >> j;
         InsNewInst(NULL, NULL, next, SHL, -reg, -reg, 
                    STiconstlookup(nbits));
         InsNewInst(NULL, NULL, next, OR, -reg, -reg, STiconstlookup(b));
         I ^= b << j;
      }
      if (i) InsNewInst(NULL, NULL, next, SHL, -reg, -reg,
                        STiconstlookup(i));
   }
}
#ifdef X86
/*
 * These funcs generate FP vector consts using iregs
 */
   void VSConstGen(INSTQ *next, int ir, int fr, long iconst)
{
   #ifdef X86_64
      InsNewInst(NULL, NULL, next, MOV, -ir, STlconstlookup(iconst), 0);
   #else
      InsNewInst(NULL, NULL, next, MOV, -ir, STiconstlookup(iconst), 0);
   #endif
   InsNewInst(NULL, NULL, next, VGR2VR16, -fr, -ir, STiconstlookup(0));
   InsNewInst(NULL, NULL, next, SHR, -ir, -ir, STiconstlookup(16));
   InsNewInst(NULL, NULL, next, VGR2VR16, -fr, -ir, STiconstlookup(1));
   InsNewInst(NULL, NULL, next, VFSHUF, -fr, -fr, STiconstlookup(0));
}

/*
 *    Majedul: no longer use to gen vector const but scalar one
 */
#ifdef X86_64
void VDConstGen(INSTQ *next, int ir, int fr, long icon0)
#else
void VDConstGen(INSTQ *next, int ir, int fr, long icon0, long icon1)
#endif
{
   #ifdef X86_64
      InsNewInst(NULL, NULL, next, MOV, -ir, STlconstlookup(icon0), 0);
   #else
      InsNewInst(NULL, NULL, next, MOV, -ir, STiconstlookup(icon0), 0);
   #endif
   InsNewInst(NULL, NULL, next, VGR2VR16, -fr, -ir, STiconstlookup(0));
   InsNewInst(NULL, NULL, next, SHR, -ir, -ir, STiconstlookup(16));
   InsNewInst(NULL, NULL, next, VGR2VR16, -fr, -ir, STiconstlookup(1));
   #ifdef X86_32
      InsNewInst(NULL, NULL, next, MOV, -ir, STiconstlookup(icon1), 0);
   #else
      InsNewInst(NULL, NULL, next, SHR, -ir, -ir, STiconstlookup(16));
   #endif
   InsNewInst(NULL, NULL, next, VGR2VR16, -fr, -ir, STiconstlookup(2));
   InsNewInst(NULL, NULL, next, SHR, -ir, -ir, STiconstlookup(16));
   InsNewInst(NULL, NULL, next, VGR2VR16, -fr, -ir, STiconstlookup(3));
   InsNewInst(NULL, NULL, next, VDSHUF, -fr, -fr, STiconstlookup(0));
}
#endif
void SignalSet(INSTQ *next, 
               short id,     /* local that has been initialized */
               short deref,  /* deref of id that is not a local */
               short reg)    /* register of same type as id */
/*
 * When we have used a non-native type to initialize id, we issue this
 * load/store pair to signal that id has indeed been set
 */
{
   int type, ld, st;
   type = FLAG2PTYPE(STflag[id-1]);
   if (type == T_FLOAT)
   {
      ld = FLD;
      st = FST;
   }
   else if (type == T_DOUBLE)
   {
      ld = FLDD;
      st = FSTD;
   }
   else fko_error(__LINE__, "Unknown type %d in file %s\n", type, __FILE__);

   InsNewInst(NULL, NULL, next, ld, -reg, deref, 0);
   InsNewInst(NULL, NULL, next, st, SToff[id-1].sa[2], -reg, 0);
}

void FPConstStore(INSTQ *next, short id, short con, 
                  short reg, short freg, short dreg)
{
   int flag;
   int *ip;
   double d;
   float f;
   #ifdef X86_64
      long *lp;
   #endif
   #if defined(PPC) || defined(SPARC)
      INT_DTC i;
   #endif
   #ifndef X86
      short k;
      extern short STderef;
   #endif

   flag = STflag[id-1];
   if (IS_VDOUBLE(flag) || IS_VFLOAT(flag))
   {
      fprintf(stderr, "Vector constants not yet supported!\n");
      exit(-1);
   }
   else if (IS_DOUBLE(flag))
   {
      d = SToff[con-1].d;
      if (d == 0.0)
      {
         #ifndef PPC
            InsNewInst(NULL, NULL, next, FZEROD, -dreg, 0, 0);
            InsNewInst(NULL, NULL, next, FSTD, SToff[id-1].sa[2], -dreg, 0);
         #else
            i = GetDTcon(SToff[SToff[id-1].sa[2]-1].sa[3]);
            k = AddDerefEntry(-REG_SP, STderef, -STderef, i, con);
            InsNewInst(NULL, NULL, next, XOR, -reg, -reg, -reg);
            InsNewInst(NULL, NULL, next, ST, k, -reg, 0);
            /*InsNewInst(NULL, NULL, next, ST, 
                       AddDerefEntry(-REG_SP, STderef, -STderef, i+4), -reg, 0,
                                     con);*/
            InsNewInst(NULL, NULL, next, ST, 
                       AddDerefEntry(-REG_SP, STderef, -STderef, i+4, con), 
                                     -reg, 0); /* fixed compile error */
            SignalSet(next, id, k, dreg);
         #endif
      }
      else
      {
         #ifdef X86_32
            ip = (int*) &d; 
            InsNewInst(NULL, NULL, next, MOV, -reg, STiconstlookup(*ip), 0);
            /*i = SToff[SToff[id-1].sa[2]-1].sa[3];*/
            InsNewInst(NULL, NULL, next, VGR2VR16, -dreg, -reg, 
                       STiconstlookup(0));
            InsNewInst(NULL, NULL, next, SHR, -reg, -reg, STiconstlookup(16));
            InsNewInst(NULL, NULL, next, VGR2VR16, -dreg, -reg, 
                       STiconstlookup(1));
            InsNewInst(NULL, NULL, next, MOV, -reg, STiconstlookup(ip[1]), 0);
            InsNewInst(NULL, NULL, next, VGR2VR16, -dreg, -reg, 
                       STiconstlookup(2));
            InsNewInst(NULL, NULL, next, SHR, -reg, -reg, STiconstlookup(16));
            InsNewInst(NULL, NULL, next, VGR2VR16, -dreg, -reg, 
                       STiconstlookup(3));
/*            InsNewInst(NULL, NULL, next, FSTD, k, -dreg, 0); */
            InsNewInst(NULL, NULL, next, FSTD, SToff[id-1].sa[2], -dreg, 0);
         #elif defined(X86_64)
            lp = (long*) &d;
            InsNewInst(NULL, NULL, next, MOV, -reg, STlconstlookup(*lp), 0);
            InsNewInst(NULL, NULL, next, ST, SToff[id-1].sa[2], -reg, 0);
/*
 *       Sparc has 13-bit constants, use 12 to rule out sign prob
 */
         #elif defined(SPARC)
            ip = (int*) &d;
            i = GetDTcon(SToff[SToff[id-1].sa[2]-1].sa[3]);
            k = AddDerefEntry(-REG_SP, STderef, -STderef, i, con);
            bitload(next, reg, 12, *ip);
            InsNewInst(NULL, NULL, next, ST, k, -reg, 0);
            bitload(next, reg, 12, ip[1]);
            InsNewInst(NULL, NULL, next, ST, 
               AddDerefEntry(-REG_SP, STderef, -STderef, i+4, con), -reg, 0);
/*
 *       PPC loads 16 bits at a time
 */
         #elif defined(PPC)
            ip = (int*) &d;
            i = GetDTcon(SToff[SToff[id-1].sa[2]-1].sa[3]);
            k = AddDerefEntry(-REG_SP, STderef, -STderef, i, con);
            bitload(next, reg, 16, *ip);
            InsNewInst(NULL, NULL, next, ST, k, -reg, 0);
            bitload(next, reg, 16, ip[1]);
            /*InsNewInst(NULL, NULL, next, ST,
                       AddDerefEntry(-REG_SP, STderef, -STderef, i+4), -reg, 0,
                                     con);*/
            InsNewInst(NULL, NULL, next, ST,
                       AddDerefEntry(-REG_SP, STderef, -STderef, i+4, con), 
                                     -reg, 0); /* fixed compile error */
         #endif
         #ifndef X86
            SignalSet(next, id, k, dreg);
         #endif
      }
   }
   else
   {
      assert(IS_FLOAT(flag));
      f = SToff[con-1].f;
      if (f == 0.0e0)
      {
         #ifndef PPC
            InsNewInst(NULL, NULL, next, FZERO, -freg, 0, 0);
            InsNewInst(NULL, NULL, next, FST, SToff[id-1].sa[2], -freg, 0);
         #else
            k = AddDerefEntry(-REG_SP, STderef, -STderef, 
                              GetDTcon(SToff[SToff[id-1].sa[2]-1].sa[3]), con);
            InsNewInst(NULL, NULL, next, XOR, -reg, -reg, -reg);
            InsNewInst(NULL, NULL, next, ST, k, -reg, 0);
            SignalSet(next, id, k, freg);
         #endif
      }
      else
      {
         ip = (int*) &f;
         #ifndef X86
            k = AddDerefEntry(-REG_SP, STderef, -STderef, 
                              GetDTcon(SToff[SToff[id-1].sa[2]-1].sa[3]), con);
         #endif
         #ifdef X86
            InsNewInst(NULL, NULL, next, MOV, -reg, STiconstlookup(*ip), 0);
            InsNewInst(NULL, NULL, next, VGR2VR16, -dreg, -reg, 
                       STiconstlookup(0));
            InsNewInst(NULL, NULL, next, SHR, -reg, -reg, STiconstlookup(16));
            InsNewInst(NULL, NULL, next, VGR2VR16, -dreg, -reg, 
                       STiconstlookup(1));
            InsNewInst(NULL, NULL, next, FST, SToff[id-1].sa[2], -freg, 0);
         #elif defined(X86_64)
            assert(reg <= 8);
            InsNewInst(NULL, NULL, next, MOVS, -reg, STiconstlookup(*ip), 0);
            InsNewInst(NULL, NULL, next, STS, k, -reg, 0);
         #elif defined(SPARC)
            bitload(next, reg, 12, *ip);
            InsNewInst(NULL, NULL, next, ST, k, -reg, 0);
         #elif defined(PPC)
            bitload(next, reg, 16, *ip);
            InsNewInst(NULL, NULL, next, ST, k, -reg, 0);
         #endif
         #ifndef X86
            SignalSet(next, id, k, freg);
         #endif
      }
   }
}

void IConstStore(INSTQ *next, short id, short con, short reg)
{
   #ifdef X86_32
      InsNewInst(NULL, NULL, next, MOV, -reg, con, 0);
/*
 * Sparc has 13-bit constants, use 12 to rule out sign prob
 */
   #elif defined(SPARC)
      bitload(next, reg, 12, SToff[con-1].i);
/*
 * Must load constants 16 bits at a time
 */
   #else
      bitload(next, reg, 16, SToff[con-1].i);
   #endif
   InsNewInst(NULL, NULL, next, ST, SToff[id-1].sa[2], -reg, 0);
}

void InitLocalConst(INSTQ *next, short reg, short freg, short dreg)
{
   struct locinit *lp;
   int flag;
   InsNewInst(NULL, NULL, next, COMMENT, 0, 0, 0);
   InsNewInst(NULL, NULL, next, COMMENT, 
              STstrconstlookup("Initialize locals to constants"), 0, 0);
   InsNewInst(NULL, NULL, next, COMMENT, 0, 0, 0);
   for (lp=LIhead; lp; lp = LIhead)
   {
      LIhead = lp->next;
      flag = FLAG2PTYPE(STflag[lp->id-1]);
      if (IS_FLOAT(flag) || IS_DOUBLE(flag))
         FPConstStore(next, lp->id, lp->con, reg, freg, dreg);
      else
         IConstStore(next, lp->id, lp->con, reg);
      free(lp);
   }
}

void Extern2Local(INSTQ *next, int rsav)
/*
 * After stack frame fully qualified, inserts proper store instructions before
 * next in queue in order to save parameter/system/fp const values to local
 * frame.
 *
 * For some alignments, we may need to address the callers frame by a register
 * other than sp (because we have lost track of size of frame in forcing
 * the alignment beyond the machine's native alignment).  If so, rsav is
 * set to what amounts to the callers sp.  Otherwise, we are indexing by
 *    %sp + fsize
 */
{
   extern int NPARA, DTnzerod, DTnzero, DTabsd, DTabs; 
   /*extern int        DTnzerods, DTnzeros, DTabsds, DTabss;*/
   /*extern int FKO_FLAG;*/
   short i, j=0, flag, ir, k, reg1=0, freg, dreg, vfreg, vdreg;
   int USED;
   #ifdef X86_64
      int nof, ni, nd, dr;
      char *rpara[6] = {"@rdi", "@rsi", "@rdx", "@rcx", "@r8", "@r9"};
      char fnam[8];
   #elif defined(FKO_ANSIC)
      int ld, st;
   #endif
   #if defined(SPARC) || defined(PPC)
      char nam[8];
      int ii;
      extern short STderef;
   #endif
   short *paras;
   /*char nam[8];*/
   #ifdef PPC
      char fnam[8];
      int fc, fr=0;
   #endif

   assert(!ParaDerefQ);
   dreg = GetReg(T_DOUBLE);
   freg = GetReg(T_FLOAT);
   #ifdef X86
      vfreg = GetReg(T_VFLOAT);
      vdreg = GetReg(T_VDOUBLE);
   #endif
   if (NPARA)
   {
      InsNewInst(NULL, NULL, next, COMMENT, 0, 0, 0);
      InsNewInst(NULL, NULL, next, COMMENT, 
                 STstrconstlookup("Store parameters to local frame"), 0, 0);
      InsNewInst(NULL, NULL, next, COMMENT, 0, 0, 0);
      InsNewInst(NULL, NULL, next, CMPFLAG, CF_PARASAVE, 1, 0);
      paras = malloc(NPARA * sizeof(short));
      assert(paras);
   }
   else paras = NULL;
/*
 * Find all parameters, and put them in left-to-right order
 */
   for (i=0; j < NPARA; i++)
   {
      flag = STflag[i];
      if (IS_PARA(flag))
      {
        #if IFKO_DEBUG_LEVEL > 1
           fprintf(stderr, "para #%d - '%s', I=%d, flag=%d\n", SToff[i].sa[0], 
                    STname[i]?STname[i]:"NULL", i, STflag[i]);
         #endif
         assert(SToff[i].sa[0] <= NPARA);
         paras[SToff[i].sa[0]-1] = i;
         j++;
      }
   }
/*   MarkUnusedParams(NPARA, paras); */
   #ifdef FKO_ANSIC
      for (i=0; i < NPARA; i++)
      {
         USED = SToff[SToff[paras[i]].sa[2]-1].sa[0];
         if (USED)
            PrintComment(NULL, NULL, next, "para %d, name=%s", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         else
            PrintComment(NULL, NULL, next, "para %d, name=%s: UNUSED", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         flag = STflag[paras[i]];
         k = FLAG2PTYPE(flag);
         switch(k)  /* HERE HERE HERE */
         {
         T_INT:
            ld = LD;
            st = ST;
            break;
         T_FLOAT:
            ld = FLD;
            st = FST;
            break;
         T_DOUBLE:
            ld = FLDD;
            st = FSTD;
            break;
         default:
         }
   #endif
   #ifdef X86_64
      reg1 = GetReg(T_INT);
      while (iparareg[reg1-IREGBEG]) reg1 = GetReg(T_INT);
/*
 *    NOTE: must be a register with short version in X64
 */
      assert(reg1 <= NSR);
            
      #ifdef AVX   
         fnam[0] = '@';
         fnam[1] = 'y';
         fnam[2] = 'm';
         fnam[3] = 'm';
         fnam[5] = '\0';
      #else
         fnam[0] = '@';
         fnam[1] = 'x';
         fnam[2] = 'm';
         fnam[3] = 'm';
         fnam[5] = '\0';
      #endif
      for (i=nof=nd=ni=0; i < NPARA; i++)
      {
         USED = SToff[SToff[paras[i]].sa[2]-1].sa[0];
         if (USED)
            PrintComment(NULL, NULL, next, "para %d, name=%s", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         else
            PrintComment(NULL, NULL, next, "para %d, name=%s: UNUSED", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         flag = STflag[paras[i]];
         if (IS_PTR(flag))
         {
            if (ni < 6) ir = iName2Reg(rpara[ni]);
            else
            {
               ir = reg1;
               k = AddDerefEntry(rsav, 0, 0, nof*8, paras[i]+1);
               ParaDerefQ = NewLocinit(k, 0, ParaDerefQ);
               if (USED)
                  InsNewInst(NULL, NULL, next, LD, -ir, k, 0);
               nof++;
            }
            if (USED)
               InsNewInst(NULL, NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
            ni++;
         }
         else if (IS_INT(flag))
         {
/*
 *          Load 32 bit value
 */
            if (ni < 6) ir = iName2Reg(rpara[ni]);
            else
            {
               ir = reg1;
               k = AddDerefEntry(rsav, 0, 0, nof*8, paras[i]);
               ParaDerefQ = NewLocinit(k, 0, ParaDerefQ);
               if (USED) InsNewInst(NULL, NULL, next, LDS, -ir, k, 0);
               nof++;
            }
/*
 *          Convert to 64-bit value, no conversion required for unsigned
 */
            if (USED && !IS_UNSIGNED(flag))
            {
            #if 1               
               k = STiconstlookup(32);
               InsNewInst(NULL, NULL, next, SHL, -ir, -ir, k);
               InsNewInst(NULL, NULL, next, SAR, -ir, -ir, k);
            #else
/*
 *             FIXME: this won't work. we can't use 32 bit of r8-r15 register
 *             in X64. 
 */
               InsNewInst(NULL, NULL, next, CVTSI, -ir, -ir, 0);
            #endif
            }
/*
 *          Store 64-bit integer
 */
            if (USED)
               InsNewInst(NULL, NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
            ni++;
         }
         else if (IS_FLOAT(flag))
         {
            if (nd < 8)
            {
               fnam[4] = nd + '0';
               dr = fName2Reg(fnam);
               if (USED)
                  InsNewInst(NULL, NULL, next, FST, SToff[paras[i]].sa[2], 
                             -dr, 0);
            }
            else
            {
               ir = reg1;
               if (USED)
               {
                  k = AddDerefEntry(rsav, 0, 0, nof*8, paras[i]+1);
                  ParaDerefQ = NewLocinit(k, 0, ParaDerefQ);
                  InsNewInst(NULL, NULL, next, FLD, -freg, k, 0);
                  InsNewInst(NULL, NULL, next, FST, SToff[paras[i]].sa[2],
                             -freg, 0);
               }
               nof++;
            }
            nd++;
         }
         else
         {
            if (nd < 8)
            {
               fnam[4] = nd + '0';
               dr = dName2Reg(fnam);
               if (USED) 
                  InsNewInst(NULL, NULL, next, FSTD, SToff[paras[i]].sa[2], 
                             -dr, 0);
            }
            else
            {
               ir = reg1;
               if (USED)
               {
                  k = AddDerefEntry(rsav, 0, 0, nof*8, paras[i]+1);
                  ParaDerefQ = NewLocinit(k, 0, ParaDerefQ);
                  InsNewInst(NULL, NULL, next, FLDD, -dreg, k, 0);
                  InsNewInst(NULL, NULL, next, FSTD, SToff[paras[i]].sa[2],
                             -dreg, 0);
               }
               nof++;
            }
            nd++;
         }
      }
      InsNewInst(NULL, NULL, next, COMMENT, STstrconstlookup("done paras"),
                 0, 0);
      ir = reg1;
/*
 *    FIXED: Consider DTnzerod, DTnzero, DTabsd and DTabs; 
 *    In x86, we don't have scalar AND/XOR operation, like: andss / andsd
 *    So, we will always create vectors, even though we don't apply 
 *    vectorization. Need to handle it specially in finalizing function
 *    prologue and epilogue.
 *    NOTE: don't use following DT entry anymore:
 *          DTabss, DTabsds, DTnzeros, DTnzerods
 */

      if (DTnzerod > 0)
      {
         PrintComment(NULL, NULL, next, "Writing -0 as a vector for negation");
         VDConstGen(next, ir, vdreg, 0x8000000000000000);
         InsNewInst(NULL, NULL, next, VDST, SToff[DTnzerod-1].sa[2], -vdreg, 0);
      }
      if (DTnzero > 0)
      {
         PrintComment(NULL, NULL, next, "Writing -0 for negation");
         VSConstGen(next, ir, vfreg, 0x80000000);
         InsNewInst(NULL, NULL, next, VFST, SToff[DTnzero-1].sa[2], -vfreg, 0);
      }
      if (DTabsd)
      {
         PrintComment(NULL, NULL, next, "Writing ~(-0) as a vector for absd");
         VDConstGen(next, ir, vdreg, 0x7FFFFFFFFFFFFFFF);
         InsNewInst(NULL, NULL, next, VDST, SToff[DTabsd-1].sa[2], -vdreg, 0);
      }
      if (DTabs)
      {
         PrintComment(NULL, NULL, next, "Writing ~(-0) for abss");
         VSConstGen(next, ir, vfreg, 0x7fffffff);
         InsNewInst(NULL, NULL, next, VFST, SToff[DTabs-1].sa[2], -vfreg, 0);
      }
      InsNewInst(NULL, NULL, next, COMMENT, STstrconstlookup("done archspec"), 
                 0, 0);
   #endif
   #ifdef X86_32
      reg1 = GetReg(T_INT);
      while (iparareg[reg1-IREGBEG]) reg1 = GetReg(T_INT);
      ir = reg1;
      for (j=i=0; i < NPARA; i++)
      {
         USED = SToff[SToff[paras[i]].sa[2]-1].sa[0];
         if (USED)
            PrintComment(NULL, NULL, next, "para %d, name=%s", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         else
            PrintComment(NULL, NULL, next, "para %d, name=%s: UNUSED", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         flag = STflag[paras[i]];
         if ( IS_DOUBLE(flag) && !IS_PTR(flag))
         {
            if (USED)
            {
               k = AddDerefEntry(rsav, 0, 0, j*4, paras[i]+1);
               ParaDerefQ = NewLocinit(k, 0, ParaDerefQ);
               InsNewInst(NULL, NULL, next, FLDD, -dreg, k, 0);
               InsNewInst(NULL, NULL, next, FSTD, SToff[paras[i]].sa[2], 
                          -dreg, 0);
            }
            j++;
         }
         else if (USED)
         {
            k = AddDerefEntry(rsav, 0, 0, j*4, paras[i]+1);
            ParaDerefQ = NewLocinit(k, 0, ParaDerefQ);
            if (FLAG2PTYPE(flag) == T_FLOAT)
            {
               InsNewInst(NULL, NULL, next, FLD, -freg, k, 0);
               InsNewInst(NULL, NULL, next, FST, SToff[paras[i]].sa[2],-freg,0);
            }
            else
            {
               InsNewInst(NULL, NULL, next, LD, -ir, k, 0);
               InsNewInst(NULL, NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
            }
         }
         j++;
      }
      InsNewInst(NULL, NULL, next, COMMENT, STstrconstlookup("done paras"),
                 0, 0);
/*
 *    FIXED: Consider DTnzerod, DTnzero, DTabsd and DTabs; 
 *    In x86, we don't have scalar AND/XOR operation, like: andss / andsd
 *    So, we will always create vectors, even though we don't apply 
 *    vectorization. Need to handle it specially in finalizing function
 *    prologue and epilogue.
 *    NOTE: don't use following DT entry anymore:
 *          DTabss, DTabsds, DTnzeros, DTnzerods
 */
      if (DTnzerod > 0)
      {
         PrintComment(NULL, NULL, next, "Writing -0 as a vector for negation");
         VDConstGen(next, ir, vdreg, 0x0, 0x80000000);
         InsNewInst(NULL, NULL, next, VDST, SToff[DTnzerod-1].sa[2], -vdreg, 0);
      }
      if (DTnzero > 0)
      {
         PrintComment(NULL, NULL, next, "Writing -0 to memory for negation");
         VSConstGen(next, ir, vfreg, 0x80000000);
         InsNewInst(NULL, NULL, next, VFST, SToff[DTnzero-1].sa[2], -vfreg, 0);
      }
      if (DTabsd)
      {
         PrintComment(NULL, NULL, next, "Writing ~(-0) as a vector for absd");
         VDConstGen(next, ir, vdreg, 0xffffffff, 0x7fffffff);
         InsNewInst(NULL, NULL, next, VDST, SToff[DTabsd-1].sa[2], -vdreg, 0);
      }
      if (DTabs)
      {
         PrintComment(NULL, NULL, next, "Writing ~(-0) as a vector for abss");
         VSConstGen(next, ir, vfreg, 0x7fffffff);
         InsNewInst(NULL, NULL, next, VFST, SToff[DTabs-1].sa[2], -vfreg, 0);
      }
      InsNewInst(NULL, NULL, next, COMMENT, STstrconstlookup("done archspec"), 
            0, 0);
   #endif
   #ifdef SPARC
      nam[0] = '@';
      nam[1] = 'i';
      nam[3] = '\0';
      for (j=i=0; i < NPARA; i++)
      {
         USED = SToff[SToff[paras[i]].sa[2]-1].sa[0];
         if (USED)
            PrintComment(NULL, NULL, next, "para %d, name=%s", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         else
            PrintComment(NULL, NULL, next, "para %d, name=%s: UNUSED", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         flag = STflag[paras[i]];
         if (IS_PTR(flag) || IS_INT(flag))
         {
            if (j < 6)
            {
               nam[2] = j + '0';
               ir = iName2Reg(nam);
               if (USED)
                  InsNewInst(NULL, NULL, next, ST, SToff[paras[i]].sa[2], 
                             -ir, 0);
            }
            else if (USED)
            {
               k = AddDerefEntry(rsav, 0, 0, j*4, paras[i]+1);
               ParaDerefQ = NewLocinit(k, 0, ParaDerefQ);
               InsNewInst(NULL, NULL, next, LD, -ir, k, 0);
               InsNewInst(NULL, NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
            }
            j++;
         }
         else if (IS_FLOAT(flag))
         {
            if (j < 6)
            {
               nam[2] = j + '0';
               ir = iName2Reg(nam);
               if (USED)
               {
                  k = AddDerefEntry(-REG_SP, STderef, -STderef, 
                                 GetDTcon(SToff[SToff[paras[i]].sa[2]-1].sa[3]),
                                    paras[i]+1);
                  InsNewInst(NULL, NULL, next, ST, k, -ir, 0);
                  SignalSet(next, paras[i]+1, k, freg);
               }
            }
            else if (USED)
            {
               k = AddDerefEntry(rsav, 0, 0, j*4, paras[i]+1);
               ParaDerefQ = NewLocinit(k, 0, ParaDerefQ);
               InsNewInst(NULL, NULL, next, FLD, -freg, k, 0);
               InsNewInst(NULL, NULL, next, FST, SToff[paras[i]].sa[2],
                          -freg, 0);
            }
            j++;
         }
         else
         {
            assert(IS_DOUBLE(flag));
            if (j < 5)
            {
               nam[2] = j + '0';
               ir = iName2Reg(nam);
               j++;
               if (USED)
               {
                  kk = GetDTcon(SToff[SToff[paras[i]].sa[2]-1].sa[3]);
                  k = AddDerefEntry(-REG_SP, STderef, -STderef, kk, paras[i]+1);
                  InsNewInst(NULL, NULL, next, ST, k, -ir, 0);
                  nam[2] = j + '0';
                  ir = iName2Reg(nam);
                  InsNewInst(NULL, NULL, next, ST, 
                             AddDerefEntry(-REG_SP, STderef, -STderef, kk+4, 
                                           paras[i]+1), -ir, 0);
                  SignalSet(next, paras[i]+1, k, dreg);
               }
               j++;
            }
            else if (j == 5)
            {
               nam[2] = j + '0';
               ir = iName2Reg(nam);
               j++;
               if (USED)
               {
                  kk = GetDTcon(SToff[SToff[paras[i]].sa[2]-1].sa[3]);
                  k = AddDerefEntry(-REG_SP, STderef, -STderef, kk, paras[i]+1);
                  InsNewInst(NULL, NULL, next, ST, k, -ir, 0);

                  ii = AddDerefEntry(rsav, 0, 0, j*4, paras[i]+1);
                  ParaDerefQ = NewLocinit(ii, 0, ParaDerefQ);
                  InsNewInst(NULL, NULL, next, LD, -ir, ii, 0);

                  ii = AddDerefEntry(-REG_SP, STderef, -STderef, kk+4,
                                     paras[i]+1);
                  InsNewInst(NULL, NULL, next, ST, ii, -ir, 0);
                  SignalSet(next, paras[i]+1, k, dreg);
               }
               j++;
            }
            else
            {
               if (USED)
               {
                  strcpy(nam, archdregs[dreg-DREGBEG]);
                  k = fName2Reg(nam);
                  ii = AddDerefEntry(rsav, 0, 0, j*4, paras[i]+1);
                  ParaDerefQ = NewLocinit(ii, 0, ParaDerefQ);
                  InsNewInst(NULL, NULL, next, FLD, -k, ii, 0);
                  k++;
                  ii = AddDerefEntry(rsav, 0, 0, j*4+4, paras[i]+1);
                  ParaDerefQ = NewLocinit(ii, 0, ParaDerefQ);
                  InsNewInst(NULL, NULL, next, FLD, -k, ii, 0);
                  InsNewInst(NULL, NULL, next, FSTD, SToff[paras[i]].sa[2], 
                             -dreg, 0);
               }
               j += 2;
            }
         }
      }
   #endif
   #ifdef OSX_PPC
      nam[0] = 'r';
      nam[3] = nam[2] = '\0';
      fnam[0] = 'f';
      fnam[2] = fnam[3] = '\0';
      for (fc=j=i=0; i < NPARA; i++)
      {
         USED = SToff[SToff[paras[i]].sa[2]-1].sa[0];
         if (USED)
            PrintComment(NULL, NULL, next, "para %d, name=%s", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         else
            PrintComment(NULL, NULL, next, "para %d, name=%s: UNUSED", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         flag = STflag[paras[i]];
         if (IS_PTR(flag) || IS_INT(flag))
         {
            if (USED)
            {
               if (j < 8)
               {
                  if (j < 7) nam[1] = j + '3';
                  else { nam[1] = '1'; nam[2] = '0'; }
                  ir = iName2Reg(nam);
                  InsNewInst(NULL, NULL, next, ST, SToff[paras[i]].sa[2], 
                             -ir, 0);
               }
               else
               {
                  ii = AddDerefEntry(rsav, 0, 0, j*4, paras[i]+1);
                  ParaDerefQ = NewLocinit(ii, 0, ParaDerefQ);
                  if (j == 8)
                  {
                     ir = GetReg(T_INT);
                     while (iparareg[ir-IREGBEG]) ir = GetReg(T_INT);
                  }
                  InsNewInst(NULL, NULL, next, LD, -ir, ii, 0);
                  InsNewInst(NULL, NULL, next, ST, SToff[paras[i]].sa[2], 
                             -ir, 0);
               }
            }
            j++;
         }
         else if (IS_FLOAT(flag))
         {
            if (j == 8)
            {
               ir = GetReg(T_INT);
               while (iparareg[ir-IREGBEG]) ir = GetReg(T_INT);
            }
            if (USED)
            {
               if (fc < 13)
               {
                  if (fc < 9) fnam[1] = '0' + fc + 1;
                  else { fnam[1] = '1'; fnam[2] = '0' + fc - 9; }
                  fr = fName2Reg(fnam);
                  InsNewInst(NULL, NULL, next, FST, SToff[paras[i]].sa[2], 
                             -fr, 0);
               }
               else
               {
                  ii = AddDerefEntry(rsav, 0, 0, j*4, paras[i]+1);
                  ParaDerefQ = NewLocinit(ii, 0, ParaDerefQ);
                  InsNewInst(NULL, NULL, next, FLD, -freg, ii, 0);
                  InsNewInst(NULL, NULL, next, FST, SToff[paras[i]].sa[2], 
                             -freg, 0);
               }
            }
            fc++;
            j++;
         }
         else
         {
            assert(IS_DOUBLE(flag));
            if (j == 8 || j == 7)
            {
               ir = GetReg(T_INT);
               while (iparareg[ir-IREGBEG]) ir = GetReg(T_INT);
            }
            if (USED)
            {
               if (fc < 13)
               {
                  if (fc < 9) fnam[1] = '0' + fc + 1;
                  else { fnam[1] = '1'; fnam[2] = '0' + fc - 9; }
                  fr = dName2Reg(fnam);
                  InsNewInst(NULL, NULL, next, FSTD, SToff[paras[i]].sa[2], 
                             -fr, 0);
                  j += 2;
               }
               else
               {
                  ii = AddDerefEntry(rsav, 0, 0, j*4, paras[i]+1);
                  ParaDerefQ = NewLocinit(ii, 0, ParaDerefQ);
                  InsNewInst(NULL, NULL, next, FLDD, -dreg, ii, 0);
                  InsNewInst(NULL, NULL, next, FSTD, SToff[paras[i]].sa[2],
                             -dreg, 0);
                  j += 2;
               }
            }
            else j += 2;
            fc++;
         }
      }
   #endif
   InsNewInst(NULL, NULL, next, CMPFLAG, CF_PARASAVE, 2, 0);
/*
 * Majedul: FIXED: possible memory leak! need to free local paras
 */
   if (paras) free(paras);
/*
 * Initialize constants
 */
   if (!reg1) reg1 = GetReg(T_INT);
   InitLocalConst(next, reg1, freg, dreg);
   GetReg(-1);
}

void FinalizeEpilogue(BBLOCK *bbase,
                      int fsize,  /* frame size of returning func */
                      int Soff,   /* start of reg save area in frame */
                      int savesp, /* offset we saved sp at */
#ifdef X86_64
                      int saverbp, /* need to restore rbp? */
#endif
                      int nir,    /* number of int regs saved */
                      int *ir,    /* int regs saved */
                      int nfr,    /* number of single regs saved */
                      int *fr,    /* float regs saved */
                      int ndr,    /* number of double regs saved */
                      int *dr     /* double regs saved */
                      )
{
   int i;
   INSTQ *next;
   BBLOCK *blk;
   int k;

   i = STlabellookup("_IFKO_EPILOGUE");
   blk = FindBlockWithLabel(bbase, i);
   assert(blk);
/* 
 * Find place to insert save statements
 * NOTE: used to use CMPFLAG, but this gets moved around, so instead look
 *       for RET inst
 */
   for (next=blk->inst1; next; next = next->next)
#if 0
      if (next->inst[0] == CMPFLAG && next->inst[1] == CF_REGRESTORE) break;
#else
      if (next->inst[0] == RET) break;
#endif
   assert(next);
/*
 * Restore registers
 */
   for (i=0; i < ndr; i++)
      InsNewInst(blk, NULL, next, FLDD, -dr[i],
                 AddDerefEntry(-REG_SP, 0, 0, Soff+i*8, 0), 0);
   for (i=0; i < nir; i++)
      InsNewInst(blk, NULL, next, LD, -ir[i],
                 AddDerefEntry(-REG_SP, 0,0, Soff+ndr*8+i*ISIZE, 0), 0);
   for (i=0; i < nfr; i++)
      InsNewInst(blk, NULL, next, FLD, -fr[i],
                 AddDerefEntry(-REG_SP, 0, 0, Soff+ndr*8+nir*ISIZE+i*4, 0), 0);
/*
 * Restore stack pointer
 */
   if (savesp >= 0)
      InsNewInst(blk, NULL, next, LD, -REG_SP, 
                 AddDerefEntry(-REG_SP, 0, 0, savesp, 0), 0);
   else
      InsNewInst(blk, NULL, next, ADD, -REG_SP, -REG_SP,
                 STiconstlookup(fsize));

#if defined(X86_64) && 1
/*
 * Majedul: restore back the rbp
 */
   if (saverbp)
   {
      k = iName2Reg("@rbp");
      InsNewInst(blk, NULL, next, LD, -k, 
                    AddDerefEntry(-REG_SP, 0, 0, -8, 0), 0);
   }
#endif   
}

int FindUsedParaRegs(BBLOCK *bp, int *ir)
/*
 * Find explicit register use between instruction pair:
 *    CMPFLAG CF_PARASAVE, 1, 0
 *    CMPFLAG CF_PARASAVE, 2, 0
 * RETURNS: number of integer registers used
 * NOTE: ignores implicit use of registers (i.e., as derefs), but this is OK
 *       since register must first be set before such use, except for SP, which
 *       we don't care about here.
 */
{
   int i, ni=0;
   short op;
   INSTQ *ip;

   for (i=0; i < TNIR; i++)
      ir[i] = 0;
   for (ip=bp->inst1; ip; ip = ip->next)
      if (ip->inst[0] == CMPFLAG && ip->inst[1] == CF_PARASAVE && 
          ip->inst[2] == 1)
         break;
   if (ip)
      ip = ip->next;
   if (!ip)
      return(0);
   do
   {
      op = -ip->inst[1];
      if (op >= IREGBEG && op < IREGEND)
      {
         if (!ir[op-IREGBEG]++) ni++;
      }
      op = -ip->inst[2];
      if (op >= IREGBEG && op < IREGEND)
      {
         if (!ir[op-IREGBEG]++) ni++;
      }
      op = -ip->inst[3];
      if (op >= IREGBEG && op < IREGEND)
      {
         if (!ir[op-IREGBEG]++) ni++;
      }
      ip = ip->next;
   }
   while(ip && (ip->inst[0] != CMPFLAG || ip->inst[1] != CF_PARASAVE || 
                ip->inst[2] != 2));
   #if IFKO_DEBUG_LEVEL > 1 
      fprintf(stderr, "\nUSED PARAREGS (%d):", ni);
      for (i=0; i < TNIR; i++)
         if (ir[i]) fprintf(stderr, "%s, ", archiregs[i]);
      fprintf(stderr, "\n\n");
   #endif
   return(ni);
}
#if 0
/* NOT USED ANYMORE!!!
 * FIXME:
 * Majedul: FinalizePrologueEpilogue is called after the repeatable 
 * optimization. So, some param reg to stack store may be deleted. Analyzing
 * the 1st block now may not capture those param regs and may return a param
 * regs to save the sp. It will overwrite the value of param regs and come up
 * with erroneous result, even segfault!
 * I skipped this operation.
 */
static int GimmeRegSave(BBLOCK *bp, int *savedregs)
/*
 * Given the bp is the BLOCK where the parameters are stored to the frame,
 * RETURNS: 
 *          -ireg : non-scratch reg used to save
 *          ireg  : scratch reg used to save
 *          0     : no available register
 */
{
   int ir[TNIR];
   int i;
/*
 * FIXME: need to skip the used parameter too. Following function doesn't 
 * ensure to scope out all 
 */
   FindUsedParaRegs(bp, ir);
   for (i=1; i < TNIR; i++)
      if (!ir[i] && !icalleesave[i])
         return(i+1);
   for (i=1; i < TNIR; i++)
      if (!ir[i] && savedregs[i])
         return(-i-1);
   return(0);
}
static INSTQ *LastIntLd(BBLOCK *blk, INSTQ *ipstart)
/*
 * Finds first load of integer register from memory starting from ipstart and
 * going backwards.  If ipstart == NULL, starts at end of block.
 */
{
   INSTQ *ip;
   for (ip=ipstart?ipstart:blk->ainstN; ip; ip = ip->prev)
   {
      if (ip->inst[0] == LD && ip->inst[1] >= IREGBEG && ip->inst[2] < IREGEND)
         return(ip);
   }
   return(NULL);
}

static INSTQ *FindNextUseInBlock(INSTQ *ipstart, int val)
{
   INSTQ *ip;
   if (ipstart)
   {
      for (ip=ipstart; ip; ip = ip->next)
         if (ip->use && BitVecCheck(ip->use, val))
            return(ip);
   }
   return(NULL);
}
static INSTQ *FindPrevUseInBlock(INSTQ *ipstart, int val)
{
   INSTQ *ip;
   if (ipstart)
   {
      for (ip=ipstart; ip; ip = ip->prev)
         if (ip->use && BitVecCheck(ip->use, val))
            return(ip);
   }
   return(NULL);
}

static int DammitGimmeRegSave(BBLOCK *blk)
/*
 * Given the blk is the BLOCK where the parameters are stored to the frame,
 * RETURNS: 
 *          -ireg : non-scratch reg used to save
 *          ireg  : scratch reg used to save
 *          0     : no available register
 * This routine creates such a register when they are all used up by moving
 * the last integer register load to end of param load block, and using
 * that register as SAVESP.
 */
{
   int KeepOn, k;
   INSTQ *ipstart, *ipend, *ip, *ipld;
/*
 * NOTE: this won't work because register loading to may be non-integer.
 * instead, for case where all iregs used up, should have loads only in
 * para section, so maybe just make one load live on exit, and save it
 * until last?
 *
 * Best idea probably to find an integer register which is set but not used
 * (should be true for all).  Move this the load of this register to end of
 * block, and then use it's register as SP save reg.
 */

/*
 * Find beginning and ending of parameter saving instructions
 */
   for (ip=blk->inst1; ip; ip = ip->next)
      if (ip->inst[0] == CMPFLAG && ip->inst[1] == CF_PARASAVE && 
          ip->inst[2] == 1)
         break;
   assert(ip);
   ipstart = ip;
   for (ip=ipstart->next; ip; ip = ip->next)
      if (ip->inst[0] == CMPFLAG && ip->inst[1] == CF_PARASAVE && 
          ip->inst[2] == 2)
         break;
   assert(ip);
   ipend = ip;
/*
 * Find a load to an integer register, where the ireg is not used in the
 * rest of the block.  We will move this load to the end of the block, and
 * use its reg to hold the old stack ptr
 */
   KeepOn = 1;
   ipld = ipend;
   do
   {
/* fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__); */
/*
 *    Find candidate integer load
 */
      for (ipld = ipld->prev; ipld != ipstart; ipld = ipld->prev)
         if (ipld->inst[0] == LD && 
             -ipld->inst[1] >= IREGBEG && -ipld->inst[1] < IREGEND)
            break;
/* fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__); */
      if (ipld == ipstart)
         break;
/*
PrintThisInst(stderr, -1, ipld);
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
 */
/*
 *    Make sure there are no references of loaded register between the load
 *    and the end of the parameter area
 */
      k = -ipld->inst[1]-1+TNREG;
      for (ip=ipld->next; ip != ipend; ip = ip->next)
         if (ip->use && ip->set && 
             (BitVecCheck(ip->set, k) || BitVecCheck(ip->use, k)))
            break;
/*
 *    If no post-load refs, make sure there also no pre-load refs, so we can
 *    use same register throughout
 */
      if (ip == ipend)
      {
/* fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__); */
         for (ip=ipld->prev; ip != ipstart; ip = ip->prev)
            if (ip->use && ip->set && 
                (BitVecCheck(ip->set, k) || BitVecCheck(ip->use, k)))
               break;
         KeepOn = ip != ipstart;
      }
/* fprintf(stderr, "%s(%d) KeepOn = %d\n", __FILE__, __LINE__, KeepOn); */
if (KeepOn)
   fprintf(stderr, "Rejecting candidate %s\n", Int2Reg(ipld->inst[1]));
   }
   while(KeepOn);
   if (ipld == ipstart)
      return(0);
/* fprintf(stderr, "%s(%d) reg=%d\n", __FILE__, __LINE__, ipld->inst[1]); */
   RemoveInstFromQ(ipld);
   InsertInstBeforeQEntry(ipend, ipld);
   k = -ipld->inst[1];
   if (icalleesave[k-1]) k = -k;
   return(k);
}
#endif

int FindRegToSaveSP(BBLOCK *blk)
/*
 * Given the blk is the BLOCK where the parameters are stored to the frame,
 * RETURNS: 
 *          ireg  : reg to save old SP
 *          0     : no available register
 * Main idea: 
 *    We need a register which will save the old SP and save it until the last
 *    parameter is copied to the stack to avoid loading it back and forth (to 
 *    load from old position of parameter). So, this register will be live on 
 *    at least untill then. Now, to find this available register, we need to 
 *    satisfy following three conditions:
 *    1. Not a register which has live parameter passed from caller: they are
 *       live at the very beginning of the program
 *    2. Not a callee-saved register
 *    3. Not in conflict with the registers used upto the last copying of 
 *       parameter in to the new stack.
 * 
 * KNOWN ISSUES:
 * =============
 *
 * 1. If we have variables which are incorrectly used before defined, it may 
 * treat them as live in at the beginning of the function and hence may return 0
 * indicating no available registers!!! 
 *    Currently, this is no way to catch them in compile time. 
 */
{
   int i, j, k;
   int rsav = 0;
   INSTQ *ip;
   INT_BVI liveregs, iv;
   struct locinit *li;
   extern INT_BVI FKO_BVTMP;
   extern BBLOCK *bbbase;
   /*short *vals;*/
/*
 * Methodology, Find the last parameter load to copy parameter in stack which 
 * uses stack pointer, skip all live-in register and select a caller-saved one
 * which is not live
 */
/*
 * Step 1: find the last use of stack pointer to copy parameter in 1st block
 * NOTE: checking 1st block is enough, should be. It should be enough to check
 * upto the end of the function prologue 
 */
   if (!CFUSETU2D)
      CalcInsOuts(bbbase);
   if (!INDEADU2D)
      CalcAllDeadVariables();

   if (FKO_BVTMP)
   {
      liveregs = FKO_BVTMP;
      SetVecAll(liveregs,0);
   }
/*
 * mask of the SP register itsel out from the analysis 
 */
   iv = NewBitVec(TNREG);
   SetVecBit(iv, REG_SP-1, 1);
#if 0
   SetVecBit(iv, PCREG-1, 1);
   for (k=ICCBEG; k < ICCEND; k++)
      SetVecBit(iv, k-1, 1);
   for (k=FCCBEG; k < FCCEND; k++)
      SetVecBit(iv, k-1, 1);
   #ifdef X86_32
      SetVecBit(iv, -Reg2Int("@st")-1, 1);
   #endif
#endif
/*
 * 1. figure out the registers used to pass live parameters: unused/dead 
 *    parameters on register won't be live at the beginning of the function
 */         
   iv = BitVecComb(iv, iv , blk->ins, '|');
/*
 * 2. marked all callee saved registers
 * NOTE: in bit vec, int reg starts from IREGBEG
 */
   for (i=0; i < NIR; i++)
   {
      if (icalleesave[i]) 
         SetVecBit(iv, IREGBEG+i-1, 1);
   }
#if 0 
   vals = BitVec2StaticArray(iv);
   for (n=vals[0], i=1; i <= n; i++)
   {
      k = vals[i] + 1; /* bvec stores it as i-1*/
      if (k >= IREGBEG && k < IREGEND)
      {
         fprintf(stderr, "%s(%d) ", archiregs[k-IREGBEG], k-IREGBEG );
      }
   }
   fprintf(stderr, "\n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif
/*
 * 3. findout conflicting registers upto the last SP access for the parameter 
 * (recopying to new stack) in 1st block
 * NOTE: as rsav has to be live upto the last use of old SP reg, so any reg used
 * in this interval must be conflited with rsav and must not be used
 * FIXME: how to findout the last recopying of param??? 1st block may contain 
 * some additonal statement which uses the param??? 
 * we are checking upto the 'done param' flag... should be a better way to do 
 * so.
 */
   for (ip=blk->ainst1; ip; ip=ip->next)
   {
/*
 *    NOTE: right now, we only check upto CF_PARASAVE flag. If, by any chance, 
 *    CMPFLAG gets dislocated, it will fail. 
 *    CMPFLAG CF_PARASAVE 2 (1 means start, 2 means end)
 */
      if (ip->inst[0]==CMPFLAG && 
          ip->inst[1]==CF_PARASAVE && ip->inst[2]==2)
      {
         /*PrintThisInst(stderr, ip);*/
         break;
      }
      for (j=1; j <=3; j++)
      {
         k = ip->inst[j];
         if ( k < 0 ) /* a register */
         {
            k = -k;
            if (k >= IREGBEG && k < IREGEND) /* int reg */
            {
               /*fprintf(stderr, "reg[%d] = %s\n", j, archiregs[k-IREGBEG]);*/
               SetVecBit(liveregs, k-1, 1);
            }
         }
         else if (k > 0 && IS_DEREF(STflag[k-1])) /* DT, skip k==0 */
         {
            for (li=ParaDerefQ; li; li=li->next)
            {
               if (k == li->id) /* so, now this an inst which loads param */
               {
                  /*PrintThisInst(stderr, ip);*/
                  iv = BitVecComb(iv, iv , liveregs, '|');
                  SetVecAll(liveregs, 0);
               }
            }
         }
      }
   }
   /*
    * rsav is the 1st available register
    */
   for (i=IREGBEG; i < IREGEND; i++)
   {
      if (!BitVecCheck(iv, i-1))
      {
         rsav = i;
         break;
      }
   }
#if 0 
   fprintf(stderr, "***************rsav = %s\n", archiregs[rsav-IREGBEG]);
   vals = BitVec2StaticArray(iv);
   fprintf(stderr, "Regs live-in at the beginning: ");
   for (n=vals[0], i=1; i <= n; i++)
   {
      k = vals[i] + 1;
      if (k >= IREGBEG && k < IREGEND)
      {
         fprintf(stderr, "%s(%d) ", archiregs[k-IREGBEG], k-IREGBEG );
      }
   }
   fprintf(stderr, "\n");
#endif
/*
 * time to kill the bit vec
 */
   KillBitVec(iv);
/*
 * return rsav    
 */
   return (rsav); 
}
void FixParamLoad(BBLOCK *headblk, int rsav, int spderef)
/*
 * This function load old SP using spderef and replaces invalid rsav register
 * with the register which is used to load the paramater itself. 
 * only examines the parameter save area.
 * NOTE: it's not the best way to solve the issue, but it will obviously work
 * for all cases. Moreover, this case doesn't occur frequently.  
 */
{
   short ireg;
   INSTQ *ip;
/*
 * find out the starting of the parameter save zone
 */
   for (ip=headblk->ainst1; ip; ip=ip->next)
      if (ip->inst[0]==CMPFLAG && 
          ip->inst[1]==CF_PARASAVE && ip->inst[2]==1)
         break;
   assert(ip);
   for ( ; ip; ip=ip->next)
   {
/*
 *    check until the end of the parameter save zone reached
 */   
      if (ip->inst[0]==CMPFLAG && 
          ip->inst[1]==CF_PARASAVE && ip->inst[2]==2)
         break;
      if (IS_LOAD(ip->inst[0]))
      {
         if (SToff[ip->inst[2]-1].sa[0] == rsav)
         {
            ireg = ip->inst[1];
            SToff[ip->inst[2]-1].sa[0] = ireg;
            InsNewInst(headblk, NULL, ip, LD, ireg, spderef, 0 );
         }
      }
   }
}


/*
 *  NOTE: If a const is initialized but not used in body, gen invalid code
 *  with null from GetDeref in assembly. Need to check this issue later
 */
int FinalizePrologueEpilogue(BBLOCK *bbase, int rsav)
/*
 * Calculates required frame size, corrects local and parameter offsets
 * appropriately, and then inserts instructions to save and restore
 * callee-saved regs
 * NOTE: Majedul: all parameters are copied into local area of stack, so we 
 * don't need old stack pointer to find those out. Still need to save the old 
 * one to restore it before function returns. The stack looks like following
 * after this operation:
 * 
 *          sp
 *    ------------------
 *    Local Area for
 *    all params/vars
 *    (larger->below)
 *    ------------------      Align to largest params/vars
 *    Register Save area
 *    ------------------      Atleast Align to largest reg size
 *          New SP
 *
 */
{
   INSTQ *ip, *oldhead;
   int LOAD1=0;          /* load old SP to register after reg save? */
   int i, nir, nfr, ndr;
   int ir[TNIR], fr[TNFR], dr[TNDR]; /*, irsav[TNIR];*/
   int Aoff;  /* offset to arguments, from frame pointer */
   int Soff=0; /* system-dependant skip offset */
   int Loff;   /* called routines frame size excluding locals */
   int SAVESP=(-1);  /* must we save SP to stack? */
   int spderef=0;
   int align;        /* local area required byte alignment */
   int lsize;        /* size of all required locals */
   int tsize;        /* total frame size */
   int maxalign;
   int csize=0;/* call parameter area size */
   extern int LOCALIGN, LOCSIZE;
   #ifdef X86_64
      int k; 
      int SaveRBP = 0;
   #endif
/*
 * FIXED: is LOCSIZE updated? considering the vectorization!!!
 * LOCSIZE is generated before vectorization at GenPrologueEpilogueStubs. So,
 * it only has information of the ABSVAL and salar local variable but not the 
 * vector (neither any vector constant and vector temporary)!!!
 * NOTE: We can solve this issue by re-counting the locals and updating the 
 * deref here again.
 */
#if 1
/*
 * This is the final stage before converting assembly from LIL. So, re-count
 * all the locals and update local derefs accordingly.
 * FIXED: optimized based on actual usage of local DT using 
 * MarkFinalUnusedLocals. 
 * NOTE: this is must because we have some system constants which only be 
 * exposed after some transformations.
 */
   extern BBLOCK *bbbase;
   /*MarkUnusedLocals(bbbase);*/
   MarkFinalUnusedLocals(bbbase); /* handle DTX87, DTX87d separately */
   NumberLocalsByType();
   #ifdef X86_64
      UpdateLocalDerefs(8);
   #else
      UpdateLocalDerefs(4);
   #endif
#endif 
   maxalign = align = LOCALIGN;
   lsize = LOCSIZE;
/*
 * Find registers that need to be saved
 */
   FindRegUsage(bbase, &nir, ir, &nfr, fr, &ndr, dr);
/*
 * If we have mandated an SAVESP register, need to save it if it is callee-saved
 * NOTE: not used anymore : rsav is 0
 */
   if (rsav && icalleesave[rsav-1])
   {
#if 0
      for (i=0; i < nir; i++)
         if (ir[i] == rsav)
            break;
      if (i == nir)
         ir[nir++] = rsav;
      fprintf(stderr, "\n\nSAVE i=%d, nir=%d, rsav=%d\n", i, nir, ir[nir-1]);
#else
      if (ir[rsav-1]) nir++;
      ir[rsav-1]++;
#endif
   }
#if 0 
   for (i=0; i < TNIR; i++) 
      irsav[i] = ir[i];
#endif
   /*k = RemoveNosaveregs(IREGBEG, TNIR, ir, icalleesave);*/
   RemoveNosaveregs(IREGBEG, TNIR, ir, icalleesave);
   nir = GetRegSaveList(IREGBEG, TNIR, ir);
   RemoveNosaveregs(FREGBEG, TNFR, fr, fcalleesave);
   nfr = GetRegSaveList(FREGBEG, TNFR, fr);
   RemoveNosaveregs(DREGBEG, TNDR, dr, dcalleesave);
   ndr = GetRegSaveList(DREGBEG, TNDR, dr);
/*
 * If we return values in a register, no need to save and restore it
 */
   if (rout_flag & IRET_BIT)
   {
      for (i=0; i < nir && ir[i] != IRETREG; i++);
      if (i < nir)
         for (nir--; i < nir; i++) ir[i] = ir[i+1];
   }
   if (rout_flag & FRET_BIT)
   {
      for (i=0; i < nfr && fr[i] != FRETREG; i++);
      if (i < nfr)
         for (nfr--; i < nfr; i++) fr[i] = fr[i+1];
   }
   if (rout_flag & DRET_BIT)
   {
      for (i=0; i < ndr && dr[i] != DRETREG; i++);
      if (i < ndr)
         for (ndr--; i < ndr; i++) dr[i] = dr[i+1];
   }
#if 0
/*
 * print out all registers that need to save 
 */
   fprintf(stderr, "int regs that need to used: ");
   for (i=0; i<nir; i++)
      if (ir[i])
         fprintf(stderr, "[%d,%s] ", ir[i]-IREGBEG, archiregs[ir[i]-IREGBEG]);
   fprintf(stderr, "\n");
#endif   
   #ifdef X86_64
   /* 
    * NOTE: rsp is not aligned 16 byte. rsp-8 is aligned to 16 byte
    */
      if (!align) align = 16;
   #else
      if (!align) align = 4;
   #endif

/* 
 * Find place to insert save statements
 */
   for (ip=bbase->inst1; ip; ip = ip->next)
      if (ip->inst[0] == CMPFLAG && ip->inst[1] == CF_REGSAVE) break;
   assert(ip);
   oldhead = ip;
/*
 * For x86-64, save %rbp, if necessary, to the reserved location of -8(%rsp)
 * Majedul: FIXED: We don't use rbp as base pointer to keep it free to use it
 * inside the routine. 'rbp' is treated like any other general 
 * purpose register here. But conventional rbp is saved right after the 
 * 'return address' that means at -8(%rsp) position and the value of rsp is 
 * saved in rbp when it is used as base pointer. and it is both for x86-32 and
 * x86-64.
 * But I got an idea that it is optional and we always omit frame pointer to
 * have one extra register. I saved rbp at -8(%rsp) though. It can also be 
 * skipped by disabling the following code.
 *
 */
   #if defined(X86_64) && 1
      k = iName2Reg("@rbp"); /* FIXED: @rbp from %rbp*/
      for (i=0; i < nir && ir[i] != k; i++);
      if (i < nir)
      {
         SaveRBP = 1;
         InsNewInst(NULL, NULL, oldhead, ST,
                    AddDerefEntry(-REG_SP, 0, 0, -8, 0), -k, 0);
         for (nir--; i < nir; i++) ir[i] = ir[i+1];
      }
   #endif
/*
 * Figure stack frame, ensuring all parts have correct alignment
 */
   #ifdef SPARC
      Soff = 64;
      Aoff = 68;
      if (csize && csize < 6*4) csize = 6*4;
   #elif defined(X86_64)
      Soff = 8;
      Aoff = 8;
   #elif defined(X86_32)
      Aoff  = 4;
   #elif defined(OSX_PPC)
      if (csize < 32) csize = 32;
      Aoff = 24;
   #elif defined(LINUX_PPC)
      Aoff = 8;
   #endif
   Soff += csize;
   #ifdef X86_64
      Loff = 8*(nir+ndr) + 4*nfr;
/*
 *    Majedul: in case of AVX, maxalign may be 32 byte .
 */
      if (maxalign > ASPALIGN)
      {
         Loff += 8; /* must have saved rsp, keep space for it */ 
/*
 *       FIXED: need to add extra 8 byte as we will always save reg from 
 *       8(rsp). So, if there are 4 regs (including rsp) to save, Loff = 32 
 *       and we save the last reg at 32(rsp). We can't save vector var from 
 *       32(rsp). We need to save that from 64(rsp). adding extra 8 byte will
 *       ensure that.
 */
         Loff += 8; /* add 8 to keep space as saving position starts from 8 */
         
         SAVESP = 8; /* always save the rsp at 1st location for X64 */
         Soff +=8; /* keep space to save old stack pointer */
/*
 *       Majedul: we will force new sp  align with maxalign anyway
 *       So, need to maintain multiple of align/maxalign
 */
         if (Loff % align) Loff = (Loff/align)*align + align;
      }
      else /* we need to maintain 16 byte alignment at most*/
      {
         if (Loff % ASPALIGN) Loff = (Loff/ASPALIGN)*ASPALIGN + ASPALIGN;
         Loff += Soff; /* to make 16 byte align, we need to add this 8 byte */  
      }
      if (SaveRBP)
         lsize += 8;
      tsize = Loff + lsize;
/*
 *       Majedul: lsize may only be multiple of 4 byte. We need 8 byte alignment
 *       for saving register. Here, Aligning total size by 16 byte will 
 *       actually ensure the alignment of 8 byte.
 */
      if (tsize % ASPALIGN) tsize = (tsize/ASPALIGN)*ASPALIGN + ASPALIGN;
/*
 *    Majedul: print all offset related params
 */
#if 0
      fprintf(stderr, "Stack Offset related\n");
      fprintf(stderr, "--------------------\n");
      fprintf(stderr, "Loff = %d\n",Loff);
      fprintf(stderr, "Soff = %d\n",Soff);
      fprintf(stderr, "Aoff = %d\n",Aoff);
      fprintf(stderr, "align = %d\n",align);
      fprintf(stderr, "ASPALIGN = %d\n",ASPALIGN);
      fprintf(stderr, "maxalign = %d\n",maxalign);
      fprintf(stderr, "lsize = %d\n",lsize);
      fprintf(stderr, "tsize = %d\n",tsize);
      fprintf(stderr, "SAVESP = %d\n",SAVESP);
      
      extern void PrintSymtabStaticMember(FILE *fpout);
      PrintSymtabStaticMember(stderr);
#endif

   #else
/*
 *    We assume sp already 4-byte aligned but may need to make 8-byte aligned
 *    if demanded by save
 */
      if (ndr)
      {
         if ((Soff>>3)<<3 != Soff) Soff = 8 + ((Soff>>3)<<3);
         #ifndef X86_32
            if (maxalign < 8) maxalign = 8;
         #endif
      }
      if (maxalign > ASPALIGN)
      {
         Loff = Soff + 8*ndr + 4*nfr + 4*nir + 4;
         SAVESP = Loff-4;
      }
      else Loff = Soff + 8*ndr + 4*nfr + 4*nir;
      if (Loff%align) Loff = (Loff/align)*align + align;
      tsize = Loff + lsize;
      if (tsize % ASPALIGN) tsize = (tsize/ASPALIGN)*ASPALIGN + ASPALIGN;
#if 0
      fprintf(stderr, "Stack Offset related\n");
      fprintf(stderr, "--------------------\n");
      fprintf(stderr, "Loff = %d\n",Loff);
      fprintf(stderr, "Soff = %d\n",Soff);
      fprintf(stderr, "Aoff = %d\n",Aoff);
      fprintf(stderr, "align = %d\n",align);
      fprintf(stderr, "ASPALIGN = %d\n",ASPALIGN);
      fprintf(stderr, "maxalign = %d\n",maxalign);
      fprintf(stderr, "lsize = %d\n",lsize);
      fprintf(stderr, "tsize = %d\n",tsize);
      fprintf(stderr, "SAVESP = %d\n",SAVESP);
      
      extern void PrintSymtabStaticMember(FILE *fpout);
      PrintSymtabStaticMember(stderr);
#endif
   #endif
/*
 *    Majedul: now we consider sp save for both 32 bit and 64 bit.
 */
      if (SAVESP >= 0)
      {
         PrintMajorComment(bbase, NULL, oldhead, 
     "To ensure greater alignment than sp, save old sp to stack and move sp");
#if 0
/*
 *       Majedul: 
 *       FIXME: GimmeRegSave only checks 1st block where parameter is stored
 *       to identify the used param regs. But it doesn't work. this function is
 *       called after reg asg and may be some useful reg asg is deleted. But
 *       we should not overwrite the param regs which may be used later. 
 *       Example: in many kernel (like: irk1amax), rsi is used to pass ptr but
 *       deleted from 1st block!!! we should not overwrite rsi to store rsp!!
 *       
 *       My logic to determine the register to save sp:
 *          find available register which is neither used parameter nor 
 *          calleesave regs.
 *       FIXME: when we have morethan 5 int params, it fails!!! because same 
 *       logic is used to determine the available regs to fetch params! e.g.-
 *       rax is used as availbale reg to store the params in stack, but same 
 *       reg is holding the stack value!!!!!!
 */
   #if 1         
         if (!rsav)
         {
            rsav = GimmeRegSave(oldhead->myblk, irsav);
            fprintf(stderr, "rsav = %d: %s\n", rsav, archiregs[rsav-IREGBEG]);
            if (!rsav)
            {
               rsav = DammitGimmeRegSave(oldhead->myblk);
               if (!rsav)
                  return(1);
            }
         }
         else if (icalleesave[rsav-1]) rsav = -rsav;
         if (rsav < 0)
         {
            LOAD1 = -rsav;
            rsav = IREGBEG+1;
            while(rsav < IREGEND && 
                  (iparareg[rsav-IREGBEG] || icalleesave[rsav-1]))
               rsav++;
            assert(rsav < IREGEND);
         }
   #else
         rsav = GetReg(T_INT);
         rsav = GetReg(T_INT);
         while (iparareg[rsav-IREGBEG]) rsav = GetReg(T_INT);
      #ifdef X86_64
         assert(rsav <= NSIR);
      #endif
   #endif
#endif

#if 0         
/*
 * Majedul: we can safely choice a reg which is not iparareg nor calleesave 
 * rsav is always 0 in new state implementation so far.
 * FIXME: same logic is used for reg1 to fetch params in case of params > 5
 */
         assert(!rsav);
         rsav = GetReg(T_INT);
         rsav = GetReg(T_INT);
         while (iparareg[rsav-IREGBEG] || icalleesave[rsav-IREGBEG]) 
            rsav = GetReg(T_INT);
/*
 *       FIXED: To skip the register which may be used to load the parameter 
 *       when int params > 5. It's a temporary fixed, not tested for all 
 *       cases ... ... ... 
 *       r10 reg is used!!!
 */
         rsav = GetReg(T_INT);
         while (iparareg[rsav-IREGBEG] || icalleesave[rsav-IREGBEG]) 
            rsav = GetReg(T_INT);
#endif
   
      rsav = FindRegToSaveSP(oldhead->myblk);
/*
 * FIXED: there still may not be any available register, because of the 
 * conflict. But there will always be availabke registers if we only consider
 * to save the SP and reload the old SP when we need to copy the parameters!!!
 * If we don't have available register which is not in conflict, just use 
 * a register which is neither callee-saved and used as parameter, like: rax
 */
      /*assert(rsav);*/  /* must be available */
      if (!rsav)
      {
         fko_warn(__LINE__, "No registers to preserve old SP, loaded in place");
         LOAD1 = -(IREGEND+1); /* mark with an invalid one */
         rsav = GetReg(T_INT);
         while (iparareg[rsav-IREGBEG] || icalleesave[rsav-IREGBEG]) 
            rsav = GetReg(T_INT);
      }
      rsav = -rsav;
      InsNewInst(NULL, NULL, oldhead, MOV, rsav, -REG_SP, 0);
   }
   else
   {
      PrintMajorComment(bbase, NULL, oldhead, "Adjust sp");
   }
   assert(oldhead->next->inst[0] == SUB && oldhead->next->inst[1] == -REG_SP &&
          oldhead->next->inst[2] == -REG_SP && oldhead->next->inst[3] ==
          STiconstlookup(-935));
   DelInst(oldhead->next);
   InsNewInst(NULL, NULL, oldhead, SUB, -REG_SP, -REG_SP,
              STiconstlookup(tsize)); 
   if (SAVESP >= 0)
   {
      i = const2shift(maxalign);
      assert(i >= 3);
      i = STiconstlookup(i);
      InsNewInst(NULL, NULL, oldhead, SHR, -REG_SP, -REG_SP, i);
      InsNewInst(NULL, NULL, oldhead, SHL, -REG_SP, -REG_SP, i);
      spderef = AddDerefEntry(-REG_SP, 0, 0, SAVESP, 0);
      InsNewInst(NULL, NULL, oldhead, ST, spderef, rsav, 0);
/*
 *    NOTE: mark as an invalid one... correct later!! 
 */
      if (LOAD1)
         rsav = -LOAD1;
   }
   PrintMajorComment(bbase, NULL, oldhead, "Save registers");
   CorrectLocalOffsets(Loff);
/*
 * NOTE: when the SP is forced aligned, rsav must be something else 0. 
 * Aoff represents the distance of parameter from the old SP (frame pointer). 
 */
   CorrectParamDerefs(ParaDerefQ, rsav ? rsav : -REG_SP, 
                      rsav ? Aoff : tsize+Aoff);
/*
 * Insert insts in header to save callee-saved registers
 */
   for (i=0; i < ndr; i++)
      InsNewInst(NULL, NULL, oldhead, FSTD,
                 AddDerefEntry(-REG_SP, 0, 0, Soff+i*8, 0), -dr[i], 0);
   for (i=0; i < nir; i++)
      InsNewInst(NULL, NULL, oldhead, ST,
                 AddDerefEntry(-REG_SP, 0, 0, Soff+ndr*8+i*ISIZE,0), -ir[i], 0);
   for (i=0; i < nfr; i++)
      InsNewInst(NULL, NULL, oldhead, FST,
                 AddDerefEntry(-REG_SP, 0, 0, Soff+ndr*8+nir*ISIZE+i*4, 0),
                 -fr[i], 0);
/*
 * If we need old stack pointer in register that must be saved, load it here
 */
   if (LOAD1)
   {
      /*InsNewInst(NULL, NULL, oldhead, LD, rsav, spderef, 0);*/
      FixParamLoad(oldhead->myblk, rsav, spderef);
   }
   GetReg(-1);

#ifdef X86_64
   FinalizeEpilogue(bbase, tsize, Soff, SAVESP, SaveRBP, nir, ir, nfr, fr, ndr, 
                    dr);
#else
   FinalizeEpilogue(bbase, tsize, Soff, SAVESP, nir, ir, nfr, fr, ndr, dr);
#endif
   
   CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = 0;
   return(0);
}

void CreatePrologue(BBLOCK *bbase, int rsav)
{
   short prog;
   INSTQ *oldhead;
   BBLOCK *bp;

   if (!bbase) return;
/*
 * If we return values in a register, no need to save and restore it
 */
   oldhead = bbase->inst1;
   for (bp=bbase; bp->down; bp = bp->down);
/* 
 * Put routine name label
 */
   prog = STlabellookup(rout_name);
   STflag[prog-1] |= GLOB_BIT;
   InsNewInst(NULL, NULL, oldhead, LABEL, prog, 0, 0);

   InsNewInst(NULL, NULL, oldhead, CMPFLAG, CF_REGSAVE, 0, 0);
   InsNewInst(NULL, NULL, oldhead, SUB, -REG_SP, -REG_SP, STiconstlookup(-935));
   Extern2Local(oldhead, rsav ? -rsav : 0);
   InsNewInst(NULL, NULL, oldhead, COMMENT, 0, 0, 0);
   InsNewInst(NULL, NULL, oldhead, COMMENT, 
              STstrconstlookup("END OF FUNCTION PROLOGUE"), 0, 0);
   InsNewInst(NULL, NULL, oldhead, COMMENT, 0, 0, 0);
}
void KillUnusedLocals()  /* HERE, HERE: move to symtab */
{
}

void GenPrologueEpilogueStubs(BBLOCK *bbase, int rsav)
/*
 * Create partially qualified local and para derefs, and generate function
 * prologue/epilogue stubs
 */
{
   BBLOCK *blk;
   int use=0;

   MarkUnusedLocals(bbase); 
   CreateSysLocals();
/*
 * NOTE: this function is called before vectorization. So, vector locals are
 * not considered here (but only system related vector like: ABSVAL, etc.)
 */
   NumberLocalsByType();
   #ifdef X86_64
      UpdateLocalDerefs(8); /* parameter for integer */
   #else
      UpdateLocalDerefs(4);
   #endif
   CreatePrologue(bbase, rsav);
   GetReg(-1);
   for (blk=bbase; blk->down; blk = blk->down);

   blk->down = NewBasicBlock(blk, NULL);
   blk = blk->down;
   InsNewInst(blk, NULL, NULL, LABEL, STlabellookup("_IFKO_EPILOGUE"), 
              0, 0);
   if (rout_flag & IRET_BIT)
      use = -IRETREG;
   else if (rout_flag & FRET_BIT)
      use = -FRETREG;
   else if (rout_flag & DRET_BIT)
      use = -DRETREG;
   InsNewInst(blk, NULL, NULL, CMPFLAG, CF_REGRESTORE, 0, 0);
   InsNewInst(blk, NULL, NULL, RET, 0, use, 0);
}

void FeedbackArchInfo(FILE *fpout)
{
   int i, j, k;
   const int nr = 6;   /* num of type of regs */
   const int nbt = 3;  /* num of basic types, */
   const int next = 4; /* num of ext inst */
   char *ctypes[] = {"i", "f", "d", "vi", "vf", "vd"}; /* basic types first */
   /*char *cbtypes[] = {"i", "f", "d"};*/
   char *cextinst[] = {"MAXINST", "MININST", "CONDMOV", "FMACINST"};
   int maxinst[]  = {0, 0, 0, 0, 0, 0}; 
   int mininst[]  = {0, 0, 0, 0, 0, 0}; 
   int cmovinst[] = {0, 0, 0, 0, 0, 0}; 
   int macinst[]  = {0, 0, 0, 0, 0, 0}; 
   int *extinst[] = {maxinst, mininst, cmovinst, macinst};
   int ne;
   /*char **archregs[] ={archiregs, archfregs, archdregs, archviregs, 
                      archvfregs, archvdregs} ;*/
   char **archregs[nr];  /* arrays of pointer to pointer */
   unsigned int aliasgp[] = {0, 0, 0, 0, 0, 0}; /* each int is a group */
   unsigned int tempgp[] = {0, 0, 0, 0, 0, 0}; /* each int is a group */
   enum tregs {IR, FR, DR, VIR, VFR, VDR};
   enum btypes {INT, FLOAT, DOUBLE};
   int nregs[nr];
   int vlen[nbt];
   int tr, vt;
   const int npipe = 4;
   int np;
   char *cpipe[] = {"PIPELEN_ADD", "PIPELEN_MUL", "PIPELEN_DIV", 
                    "PIPELEN_MAC"};
/*
 * initialized with invalid value which tells us 'undefined' 
 */
   const int inv = -2;  /* -2 is not a valid value for pipeline */
   int addpipe[] = {inv, inv, inv, inv, inv, inv};
   int mulpipe[] = {inv, inv, inv, inv, inv, inv};
   int divpipe[] = {inv, inv, inv, inv, inv, inv};
   int macpipe[] = {inv, inv, inv, inv, inv, inv};
   int *pipe[] = {addpipe, mulpipe, macpipe, divpipe};
/*
 * print pipeline info 
 * 
 */
   np = 0;
   #if defined(PIPELEN_IADD) || defined(PIPELEN_FADD) || defined(PIPELEN_DADD) \
      || defined(PIPELEN_VIADD) || defined(PIPELEN_VFADD) \
      || defined(PIPELEN_VDADD) 
      
      np++;  
      #ifdef PIPELEN_IADD
         addpipe[IR] = PIPELEN_IADD;
      #endif
      #ifdef PIPELEN_FADD
         addpipe[FR] = PIPELEN_FADD;
      #endif
      #ifdef PIPELEN_DADD
         addpipe[DR] = PIPELEN_DADD;
      #endif
      #ifdef PIPELEN_VIADD
         addpipe[VIR] = PIPELEN_VIADD;
      #endif
      #ifdef PIPELEN_VFADD
         addpipe[VFR] = PIPELEN_VFADD;
      #endif
      #ifdef PIPELEN_VDADD
         addpipe[VDR] = PIPELEN_VDADD;
      #endif
   #endif

   #if defined(PIPELEN_IMUL) || defined(PIPELEN_FMUL) || defined(PIPELEN_DMUL) \
      || defined(PIPELEN_VIMUL) || defined(PIPELEN_VFMUL) \
      || defined(PIPELEN_VDMUL) 
      
      np++;  
      #ifdef PIPELEN_IMUL
         mulpipe[IR] = PIPELEN_IMUL;
      #endif
      #ifdef PIPELEN_FMUL
         mulpipe[FR] = PIPELEN_FMUL;
      #endif
      #ifdef PIPELEN_DMUL
         mulpipe[DR] = PIPELEN_DMUL;
      #endif
      #ifdef PIPELEN_VIMUL
         mulpipe[VIR] = PIPELEN_VIMUL;
      #endif
      #ifdef PIPELEN_VFMUL
         mulpipe[VFR] = PIPELEN_VFMUL;
      #endif
      #ifdef PIPELEN_VDMUL
         mulpipe[VDR] = PIPELEN_VDMUL;
      #endif
   #endif
   
   #if defined(PIPELEN_IDIV) || defined(PIPELEN_FDIV) || defined(PIPELEN_DDIV) \
      || defined(PIPELEN_VIDIV) || defined(PIPELEN_VFDIV) \
      || defined(PIPELEN_VDDIV) 
      
      np++;  
      #ifdef PIPELEN_IDIV
         divpipe[IR] = PIPELEN_IDIV;
      #endif
      #ifdef PIPELEN_FDIV
         divpipe[FR] = PIPELEN_FDIV;
      #endif
      #ifdef PIPELEN_DDIV
         divpipe[DR] = PIPELEN_DDIV;
      #endif
      #ifdef PIPELEN_VIDIV
         divpipe[VIR] = PIPELEN_VIDIV;
      #endif
      #ifdef PIPELEN_VFDIV
         divpipe[VFR] = PIPELEN_VFDIV;
      #endif
      #ifdef PIPELEN_VDDIV
         divpipe[VDR] = PIPELEN_VDDIV;
      #endif
   #endif

   #if defined(ARCH_HAS_MAC) 
      #if ARCH_HAS_MAC != 0 
         #if defined(PIPELEN_IMAC) || defined(PIPELEN_FMAC) \
            || defined(PIPELEN_DMAC) || defined(PIPELEN_VIMAC) \
            || defined(PIPELEN_VFMAC) || defined(PIPELEN_VDMAC) 
            np++;
            #ifdef PIPELEN_IMAC
               macpipe[IR] = PIPELEN_IMAC;
            #endif
            #ifdef PIPELEN_FMAC
               macpipe[FR] = PIPELEN_FMAC;
            #endif
            #ifdef PIPELEN_DMAC
               macpipe[DR] = PIPELEN_DMAC;
            #endif
            #ifdef PIPELEN_VIMAC
               macpipe[VIR] = PIPELEN_VIMAC;
            #endif
            #ifdef PIPELEN_VFMAC
               macpipe[VFR] = PIPELEN_VFMAC;
            #endif
            #ifdef PIPELEN_VDMAC
               macpipe[VDR] = PIPELEN_VDMAC;
            #endif
         #endif
      #endif
   #endif

   if (np)
   {
      fprintf(fpout, "PIPELINES=%d\n", np);
      for (i=0; i < npipe; i++)
      {
         k = 0;
         for (j=0; j < nr; j++)
         {
            if (pipe[i][j] != inv)
            {
               k = 1;
               break;
            }
         }
         if (k)
         {
            fprintf(fpout, "   %s:", cpipe[i]);
            for (j=0; j < nr; j++)
               if ( pipe[i][j] != inv ) 
                  fprintf(fpout, " %s=%d", ctypes[j], pipe[i][j]); 
            fprintf(fpout, "\n"); 
         }
      }
   }
   else
      fprintf(fpout, "PIPELINES=0\n");

/*
 * Register info
 */
  
   tr =  0;
   for ( i=0; i < nr; i++)
      nregs[i] = 0;

   #ifdef NIR
      if (NIR > 0)
      {
         nregs[IR] = NIR - 1; /*1st ireg is always sp and not usable*/
         tr++;
      }
   #endif
   #ifdef NFR
      if (NFR > 0)
      {
         nregs[FR] = NFR;
         tr++;
      }
   #endif
   #ifdef NDR
      if (NDR > 0)
      {
         nregs[DR] = NDR;
         tr++;
      }
   #endif
   #ifdef NVIR
      if (NVIR > 0)
      {
         nregs[VIR] = NVIR;
         tr++;
      }
   #endif
   #ifdef NVFR
      if (NVFR > 0)
      {
         nregs[VFR] = NVFR;
         tr++;
      }
   #endif
   #ifdef NVDR
      if (NVDR > 0)
      {
         nregs[VDR] = NVDR;
         tr++;
      }
   #endif
   
   fprintf(fpout, "REGTYPES=%d\n",tr);
   if (tr)
   {
      fprintf(fpout, "   NUMREGS:");
      for (i=0; i < nr; i++)
         if (nregs[i]) fprintf(fpout, " %s=%d", ctypes[i], nregs[i] );
      fprintf(fpout, "\n");
   }
/*
 * register alias info
 * we always have iregs, fregs and dregs; but vector regs are optional
 */
   archregs[0] = archiregs; 
   archregs[1] = archfregs; 
   archregs[2] = archdregs;
   #ifdef INT_VEC
      archregs[3] = archviregs;
   #else
      archregs[3] = NULL;
   #endif
   #ifdef FP_VEC
      archregs[4] = archvfregs;
   #else
      archregs[4] = NULL;
   #endif
   #ifdef DP_VEC
      archregs[5] = archvdregs;
   #else
      archregs[5] = NULL;
   #endif

#if 0
   for(i=0; i < nr; i++)
      fprintf(stderr, "%s : %p\n",ctypes[i], archregs[i]);
#endif
/*
 * figuring out register aliasing
 * Algorithm:
 * ==========
 *    consider nr x nr matrix of bits
 *    each row (here unsigned int) represents each type of register
 *
 *     i f d vi vf vd
 *     ---------------
 *  i |1 0 0 0 0 0  --> skip, power of 2; means no pair
 *  f |0 1 1 1 0 0  --> 0X1C   |
 *  d |0 1 1 1 0 0  --> 0X1C   |  one group
 *  vi|0 1 1 1 0 0  --> 0X1C   |
 *  vf|0 0 0 0 1 1  --> 0X03     |  2nd group
 *  vd|0 0 0 0 1 1  --> 0X03     | 
 *
 */
   assert(nr <= (sizeof(unsigned int)*8));
   for (i=0; i < nr; i++)
      for (j=0; j < nr; j++)
         if (archregs[i] == archregs[j])
            tempgp[i] |= (1 << j);
  
   for (i=0, k=0; i < nr; i++)
   {
      if ( !((tempgp[i]!=0) && !(tempgp[i] & (tempgp[i]-1))) )/*not power of 2*/
      {
         for (j=0; j < k; j++)
            if (tempgp[i] == aliasgp[j])
               break;
         if (j==k) /* no match */
         {
            aliasgp[k++] = tempgp[i];
         }
      }
   }
#if 0   
   for (i=0; i < k; i++)
      fprintf(stderr, "%d -> 0x%x\n", i,  aliasgp[i]);
#endif
   if (k)
   {
      fprintf(fpout,"   ALIASGROUPS=%d\n", k);
      for (i=0; i < k; i++)
      {
         fprintf(fpout, "      ALIASED:");
         for (j=0; j < nr; j++)
            if (aliasgp[i] & (1 << j))
               fprintf(fpout, " %s", ctypes[j]);
         fprintf(fpout, "\n");
      }
   }

/*
 * Print cache information  
 */
   fprintf(fpout, "NCACHES=%d\n", NCACHE);
   fprintf(fpout, "   LINESIZES:");
   for (i=0; i < NCACHE; i++)
      fprintf(fpout, " %d", LINESIZE[i]);
   fprintf(fpout, "\n");
/*
 * vect types
 */
   vt = 0;
   for (i=0; i < nbt; i++)
      vlen[i] = 0;
   #ifdef INT_VEC
      vlen[INT] = FKO_IVLEN;
      vt++;
   #endif
   #ifdef FP_VEC
      vlen[FLOAT] = FKO_SVLEN;
      vt++;
   #endif
   #ifdef DP_VEC
      vlen[DOUBLE] = FKO_DVLEN;
      vt++;
   #endif
   fprintf(fpout, "VECTYPES=%d\n", vt);
   if (vt)
   {
      fprintf(fpout, "   VECLEN:");
      for (i=0; i < nbt; i++)
         if (vlen[i]) fprintf(fpout, " %s=%d", ctypes[i], vlen[i]);
      fprintf(fpout, "\n");
   }
/*
 * extended inst 
 */
   ne = 0;
   #ifdef ArchHasMaxMin
      ne++;
      #ifdef INT_MAX
         maxinst[IR] = 1;
      #endif
      #ifdef FP_MAX
         maxinst[FR] = 1;
      #endif
      #ifdef DP_MAX
         maxinst[DR] = 1;
      #endif
      #ifdef VINT_MAX
         maxinst[VIR] = 1;
      #endif
      #ifdef VFP_MAX
         maxinst[VFR] = 1;
      #endif
      #ifdef VDP_MAX
         maxinst[VDR] = 1;
      #endif
      
      ne++; 
      
      #ifdef INT_MIN
         mininst[IR] = 1;
      #endif
      #ifdef FP_MIN
         mininst[FR] = 1;
      #endif
      #ifdef DP_MIN
         mininst[DR] = 1;
      #endif
      #ifdef VINT_MIN
         mininst[VIR] = 1;
      #endif
      #ifdef VFP_MIN
         mininst[VFR] = 1;
      #endif
      #ifdef VDP_MIN
         mininst[VDR] = 1;
      #endif
   #endif
/*
 * cond mov inst
 */
   #ifdef ArchHasSelect
      ne++;
      #ifdef INT_CMOV
         cmovinst[IR] = 1;
      #endif
      #ifdef FP_CMOV
         cmovinst[FR] = 1;
      #endif
      #ifdef DP_CMOV
         cmovinst[DR] = 1;
      #endif
      #ifdef VINT_CMOV
         cmovinst[VIR] = 1;
      #endif
      #ifdef VFP_CMOV
         cmovinst[VFR] = 1;
      #endif
      #ifdef VDP_CMOV
         cmovinst[VDR] = 1;
      #endif
   #endif
#if 0
/*
 * fmac mov inst
 */
   #ifdef ArchHasMAC
      ne++;
      #ifdef INT_MAC
         macinst[IR] = 1;
      #endif
      #ifdef FP_MAC
         macinst[FR] = 1;
      #endif
      #ifdef DP_MAC
         macinst[DR] = 1;
      #endif
      #ifdef VINT_MAC
         macinst[VIR] = 1;
      #endif
      #ifdef VFP_MAC
         macinst[VFR] = 1;
      #endif
      #ifdef VDP_MAC
         macinst[VDR] = 1;
      #endif
   #endif
#endif
/*
 * print ext inst
 */
#if 0
         fprintf(stderr, "max: ");
         for (i=0; i < nr; i++)
            if(maxinst[i]) fprintf(stderr, " %s", ctypes[i]);
         fprintf(stderr, "\n");
         
         fprintf(stderr, "cmov: ");
         for (i=0; i < nr; i++)
            if(cmovinst[i]) fprintf(stderr, " %s", ctypes[i]);
         fprintf(stderr, "\n");
#endif

   fprintf(fpout, "EXTENDEDINST=%d\n", ne);
   if (ne)
   {
      for (i=0; i < next; i++)
      {
         k = 0;
         for (j=0; j < nr; j++)
         {
            if ( extinst[i][j] )
            { 
               k = 1;
               break;
            }
         }
         if (k)
         {
            fprintf(fpout, "   %s:", cextinst[i]);
            for (j=0; j < nr; j++)
               if ( extinst[i][j] ) fprintf(fpout, " %s", ctypes[j]);
            fprintf(fpout, "\n"); 
         }
      }
   }

}

