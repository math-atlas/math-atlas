#include "ifko.h"
#define ARCH_DECLARE
#include "fko_arch.h"
struct locinit *LIhead=NULL;

#ifdef X86_64
   #define ISIZE 8
#else
   #define ISIZE 4
#endif

void FindRegUsage(INSTQ *head, int *ni0, int *iregs, 
                  int *nf0, int *fregs, int *nd0, int *dregs)
/*
 * Searches through all instructions to find registers that are used
 */
{
   short op;
   int nd, nf=0, ni=0;
   INSTQ *ip=head;
   const int iend = IREGBEG+TNIR, fend = FREGBEG+TNFR, dend = DREGBEG+TNDR;
   for (nd=0; nd < TNIR; nd++) iregs[nd] = 0;
   for (nd=0; nd < TNFR; nd++) fregs[nd] = 0;
   for (nd=0; nd < TNDR; nd++) dregs[nd] = 0;
   nd = 0;
   do
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
      ip = ip->next;
   }
   while (ip != head);
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

   *nf0 = nf;
   *nd0 = nd;
   *ni0 = ni;
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
      if (!saves[i])
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

int GetArchAlign(int nvd, int nvf, int nd, int nf, int nl, int ni)
/*
 *  Returns required architectural alignment given the number of
 *  vector double, vector float, double, float, long, and ints you
 *  want to save (actually, these only need to be boolean).
 */
{
   #ifdef X86_64
      return(16);
   #else
   int align = 0;
   if (nvd) align = FKO_DVLEN*8;
   else if (nvf) align = FKO_SVLEN*4;
   #ifdef X86_32
      else if (nd || nf || nl || ni) align = 4;
   #else
      else if (nd || nl) align = 8;
      else if (nf || ni) align = 4;
   #endif
fprintf(stderr, "nvd=%d,nvf=%d,nd=%d,nf=%d,nl=%d,ni=%d, align=%d\n",
        nvd, nvf, nd, nf, nl, ni, align);
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
   static int dr=0, fr=0, ir=1;
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
   else if (type == T_FLOAT)
   {
      iret = FREGBEG + fr;
      if (++fr > NFR)
         fko_error(__LINE__, "Out of float registers on line %d", lnno);
   }
   else if (type == T_INT)
   {
      iret = IREGBEG + ir;
      if (++ir > NIR)
         fko_error(__LINE__, "Out of integer registers on line %d", lnno);
   }
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
 */
{
#ifdef X86
   extern int DTnzerod, DTabsd, DTnzero, DTabs;
   if (DTnzerod == -1)
   {
      DTnzerod = STdef("_NEGZEROD", VEC_BIT | T_DOUBLE | LOCAL_BIT, 0);
      SToff[DTnzerod-1].sa[2] = AddDerefEntry(-REG_SP, 0, -DTnzerod, 0);
   }
   if (DTabsd == -1)
   {
      DTabsd = STdef("_ABSVLD", VEC_BIT | T_DOUBLE | LOCAL_BIT, 0);
      SToff[DTabsd-1].sa[2] = AddDerefEntry(-REG_SP, 0, -DTabsd, 0);
fprintf(stderr, "DTabsd = %d,%d\n", DTabsd, SToff[DTabsd-1].sa[2]);
   }
   if (DTnzero == -1)
   {
      DTnzero = STdef("_NEGZERO", VEC_BIT | T_FLOAT | LOCAL_BIT, 0);
      SToff[DTnzero-1].sa[2] = AddDerefEntry(-REG_SP, 0, -DTnzero, 0);
   }
   if (DTabs == -1)
   {
      DTabs = STdef("_ABSVAL", VEC_BIT | T_FLOAT | LOCAL_BIT, 0);
      SToff[DTabs-1].sa[2] = AddDerefEntry(-REG_SP, 0, -DTabs, 0);
   }
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
      InsNewInst(NULL, next, XOR, -reg, -reg, -reg);
      return;
   }
   for (i=0; !(I & (1<<i)); i++);  /* find least sig bit non-zero bit */
   for (j=31; !(I & (1<<j)); j--); /* find most significant non-zero bit */
/*
 * Can we do it with a simple move?
 */
   if (j < nbits)
      InsNewInst(NULL, next, MOV, -reg, STiconstlookup(I), __LINE__);
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
      InsNewInst(NULL, next, MOV, -reg, STiconstlookup(b), __LINE__);
      I ^= b << (j-r);
      j -= r;
      for (k; k; k--)
      {
         j -= nbits;
         b = I >> j;
         InsNewInst(NULL, next, SHL, -reg, -reg, STiconstlookup(nbits));
         InsNewInst(NULL, next, OR, -reg, -reg, STiconstlookup(b));
         I ^= b << j;
      }
      if (i) InsNewInst(NULL, next, SHL, -reg, -reg, STiconstlookup(i));
   }
}

void FPConstStore(INSTQ *next, short id, short con, short reg)
{
   int flag;
   int *ip;
   short *sp;
   double d;
   short i;
   float f;
#ifdef X86_64
   long *lp;
#endif
   flag = STflag[id-1];
   if (IS_VEC(flag))
   {
      fprintf(stderr, "Vector constants not yet supported!\n");
      exit(-1);
   }
   else if (IS_DOUBLE(flag))
   {
      d = SToff[con-1].d;
      if (d == 0.0)
      {
         InsNewInst(NULL, next, XOR, -reg, -reg, -reg);
         InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
         #ifndef X86_64
            i = SToff[id-1].sa[2] - 1;
            i = DT[(i<<2)+3] + 4;
            InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP,0,0,i), -reg, 0);
         #endif
      }
      #ifdef X86_32
         ip = (int*) &d;
         InsNewInst(NULL, next, MOV, -reg, STiconstlookup(*ip), __LINE__);
         InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
         InsNewInst(NULL, next, MOV, -reg, STiconstlookup(ip[1]), __LINE__);
         i = SToff[id-1].sa[2] - 1;
         i = DT[(i<<2)+3] + 4;
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP,0,0,i), -reg, 0);
      #elif defined(X86_64)
         lp = (long*) &d;
         InsNewInst(NULL, next, MOV, -reg, STlconstlookup(*lp), __LINE__);
         InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
/*
 *    Sparc has 13-bit constants, use 12 to rule out sign prob
 */
      #elif defined(SPARC)
         ip = (int*) &d;
         bitload(next, reg, 12, *ip);
         InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
         bitload(next, reg, 12, ip[1]);
         i = SToff[id-1].sa[2] - 1;
         i = DT[(i<<2)+3] + 4;
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP,0,0,i), -reg, 0);
/*
 *    PPC loads 16 bits at a time
 */
      #elif defined(PPC)
         ip = (int*) &d;
         bitload(next, reg, 16, *ip);
         InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
         bitload(next, reg, 16, ip[1]);
         i = SToff[id-1].sa[2] - 1;
         i = DT[(i<<2)+3] + 4;
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP,0,0,i), -reg, 0);
      #endif
   }
   else
   {
      assert(IS_FLOAT(flag));
      f = SToff[con-1].f;
      if (f == 0.0e0)
      {
         InsNewInst(NULL, next, XOR, -reg, -reg, -reg);
         InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
      }
      ip = (int*) &f;
      #ifdef X86_32
         InsNewInst(NULL, next, MOV, -reg, STiconstlookup(*ip), __LINE__);
         InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
      #elif defined(X86_64)
         assert(reg <= 8);
         InsNewInst(NULL, next, MOVS, -reg, STiconstlookup(*ip), __LINE__);
         InsNewInst(NULL, next, STS, SToff[id-1].sa[2], -reg, __LINE__);
      #elif defined(SPARC)
         bitload(next, reg, 12, *ip);
         InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
      #elif defined(PPC)
         bitload(next, reg, 16, *ip);
         InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
      #endif
   }
}

void IConstStore(INSTQ *next, short id, short con, short reg)
{
   int i, j;

   #ifdef X86_32
      InsNewInst(NULL, next, MOV, -reg, con, __LINE__);
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
   InsNewInst(NULL, next, ST, SToff[id-1].sa[2], -reg, __LINE__);
}

void InitLocalConst(INSTQ *next, short reg)
{
   struct locinit *lp;
   int flag;
   InsNewInst(NULL, next, COMMENT, 0, 0, 0);
   InsNewInst(NULL, next, COMMENT, 
              STstrconstlookup("Initialize constant locals"), 0, 0);
   InsNewInst(NULL, next, COMMENT, 0, 0, 0);
   for (lp=LIhead; lp; lp = LIhead)
   {
      LIhead = lp->next;
      flag = FLAG2PTYPE(STflag[lp->id-1]);
      if (IS_FLOAT(flag) || IS_DOUBLE(flag))
         FPConstStore(next, lp->id, lp->con, reg);
      else
         IConstStore(next, lp->id, lp->con, reg);
      free(lp);
   }
}

void Extern2Local(INSTQ *next, INSTQ *end, short rsav, int fsize)
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
   short i, j=0, flag, ir, k, reg1=0;
   int USED;
   #ifdef X86_64
      int nof, ni, nd, dr, dreg1;
      char *rpara[6] = {"@rdi", "@rsi", "@rdx", "@rcx", "@r8", "@r9"};
      char fnam[8];
   #endif
   int nbytes=0;
   short *paras;
   char nam[8];
   #ifdef PPC
      char fnam[8];
      int fc, fr=0;
   #endif

   if (!rsav) rsav = -REG_SP;
fprintf(stderr, "\nOFFSET=%d\n\n", fsize);
   if (NPARA)
   {
      InsNewInst(NULL, next, COMMENT, 0, 0, 0);
      InsNewInst(NULL, next, COMMENT, 
                 STstrconstlookup("Store parameters to local frame"), 0, 0);
      InsNewInst(NULL, next, COMMENT, 0, 0, 0);
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
        fprintf(stderr, "para #%d - '%s', I=%d, flag=%d\n", SToff[i].sa[0], 
                 STname[i]?STname[i]:"NULL", i, STflag[i]);
         assert(SToff[i].sa[0] <= NPARA);
         paras[SToff[i].sa[0]-1] = i;
         j++;
      }
   }
/*   MarkUnusedParams(NPARA, paras); */
   #ifdef X86_64
      reg1 = GetReg(T_INT);
      fnam[0] = '@';
      fnam[1] = 'x';
      fnam[2] = 'm';
      fnam[3] = 'm';
      fnam[5] = '\0';
      for (i=nof=nd=ni=0; i < NPARA; i++)
      {
         USED = DT[(SToff[paras[i]].sa[2]-1)<<2];
         if (USED)
            PrintComment(NULL, next, "para %d, name=%s", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         else
            PrintComment(NULL, next, "para %d, name=%s: UNUSED", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         flag = STflag[paras[i]];
         if (IS_PTR(flag))
         {
            if (ni < 6) ir = iName2Reg(rpara[ni]);
            else
            {
               ir = reg1;
               if (USED)
                  InsNewInst(NULL, next, LD, -ir,
                             AddDerefEntry(rsav, 0, 0, fsize+nof*8), 0);
               nof++;
            }
            if (USED)InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
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
               if (USED) InsNewInst(NULL, next, LDS, -ir,
                                    AddDerefEntry(rsav, 0, 0, fsize+nof*8), 0);
               nof++;
            }
/*
 *          Convert to 64-bit value, no conversion required for unsigned
 */
            if (USED && !IS_UNSIGNED(flag))
            {
               k = STiconstlookup(32);
               InsNewInst(NULL, next, SHL, -ir, -ir, k);
               InsNewInst(NULL, next, SAR, -ir, -ir, k);
            }
/*
 *          Store 64-bit integer
 */
            if (USED)InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
            ni++;
         }
         else if (IS_FLOAT(flag))
         {
            if (nd < 8)
            {
               fnam[4] = nd + '0';
               dr = fName2Reg(fnam);
               if (USED)
                  InsNewInst(NULL, next, FST, SToff[paras[i]].sa[2], -dr, 0);
            }
            else
            {
               ir = reg1;
               if (USED)
               {
                  InsNewInst(NULL, next, LDS, -ir,
                                    AddDerefEntry(rsav, 0, 0, fsize+nof*8), 0);
                  InsNewInst(NULL, next, STS, SToff[paras[i]].sa[2], -ir, 0);
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
                  InsNewInst(NULL, next, FSTD, SToff[paras[i]].sa[2], -dr, 0);
            }
            else
            {
               ir = reg1;
               if (USED)
               {
                  InsNewInst(NULL, next, LD, -ir,
                             AddDerefEntry(rsav, 0, 0, fsize+nof*8), 0);
                  InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
               }
               nof++;
            }
            nd++;
         }
      }
      if (USED)
         InsNewInst(NULL, next, COMMENT, STstrconstlookup("done paras"), 0, 0);
      ir = reg1;
      if (DTnzerod > 0)
      {
         PrintComment(NULL, next, "Writing -0 to memory for negation");
         InsNewInst(NULL, next, MOV, -ir,STlconstlookup(0x8000000000000000),0);
         k = ((SToff[DTnzerod-1].sa[2]-1)<<2) + 3;
         InsNewInst(NULL, next, ST, SToff[DTnzerod-1].sa[2], -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+8),
                    -ir, __LINE__);
      }
      if (DTnzero > 0)
      {
         PrintComment(NULL, next, "Writing -0 to memory for negation");
         InsNewInst(NULL, next, MOV, -ir,STlconstlookup(0x8000000080000000),0);
         InsNewInst(NULL, next, ST, SToff[DTnzero-1].sa[2], -ir, 0);
         k = ((SToff[DTnzero-1].sa[2]-1)<<2) + 3;
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+8),
                    -ir, __LINE__);
      }
      if (DTabsd)
      {
         PrintComment(NULL, next, "Writing ~(-0) to memory for absd");
         InsNewInst(NULL, next, MOV, -ir,STlconstlookup(0x7FFFFFFFFFFFFFFF),0);
         k = ((SToff[DTabsd-1].sa[2]-1)<<2) + 3;
         InsNewInst(NULL, next, ST, SToff[DTabsd-1].sa[2], -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+8),
                    -ir, __LINE__);
      }
      if (DTabs)
      {
         PrintComment(NULL, next, "Writing ~(-0) to memory for abss");
         k = ((SToff[DTabs-1].sa[2]-1)<<2) + 3;
         InsNewInst(NULL, next, MOV, -ir,STlconstlookup(0x7fffffff7fffffff),0);
         InsNewInst(NULL, next, ST, SToff[DTabs-1].sa[2], -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+8),
                    -ir, __LINE__);
      }
      InsNewInst(NULL, next, COMMENT, STstrconstlookup("done archspec"), 0, 0);
   #endif
   #ifdef X86_32
      reg1 = ir = GetReg(T_INT);
      for (j=i=0; i < NPARA; i++)
      {
         USED = DT[(SToff[paras[i]].sa[2]-1)<<2];
         if (USED)
            PrintComment(NULL, next, "para %d, name=%s", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         else
            PrintComment(NULL, next, "para %d, name=%s: UNUSED", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         flag = STflag[paras[i]];
         if (USED)
         {
            InsNewInst(NULL, next, LD, -ir,
                       AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
            InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, __LINE__);
         }
         j++;
         if (!IS_PTR(flag) && IS_DOUBLE(flag))
         {
            if (USED)
            {
               InsNewInst(NULL, next, LD, -ir,
                          AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
               k = SToff[paras[i]].sa[2] - 1;
               k = DT[(k<<2)+3] + 4;
               InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, k), 
                          -ir, __LINE__);
            }
            j++;
         }
      }
      if (USED)
         InsNewInst(NULL, next, COMMENT, STstrconstlookup("done paras"), 0, 0);
      if (DTnzerod > 0)
      {
         PrintComment(NULL, next, "Writing -0 to memory for negation");
         k = ((SToff[DTnzerod-1].sa[2]-1)<<2) + 3;
         InsNewInst(NULL, next, XOR, -ir, -ir, -ir);
         InsNewInst(NULL, next, ST, SToff[DTnzerod-1].sa[2], -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+8),
                    -ir, __LINE__);
         InsNewInst(NULL, next, MOV, -ir, STiconstlookup(0x80000000), 0);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+4),
                    -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+12),
                    -ir, __LINE__);
      }
      if (DTnzero > 0)
      {
         PrintComment(NULL, next, "Writing -0 to memory for negation");
         InsNewInst(NULL, next, MOV, -ir, STiconstlookup(0x80000000), 0);
         InsNewInst(NULL, next, ST, SToff[DTnzero-1].sa[2], -ir, 0);
         k = ((SToff[DTnzero-1].sa[2]-1)<<2) + 3;
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+4),
                    -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+8),
                    -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+12),
                    -ir, __LINE__);
      }
      if (DTabsd)
      {
         PrintComment(NULL, next, "Writing ~(-0) to memory for absd");
         k = ((SToff[DTabsd-1].sa[2]-1)<<2) + 3;
         InsNewInst(NULL, next, XOR, -ir, -ir, -ir);
         InsNewInst(NULL, next, NOT, -ir, -ir, -ir);
         InsNewInst(NULL, next, ST, SToff[DTabsd-1].sa[2], -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+8),
                    -ir, __LINE__);
         InsNewInst(NULL, next, MOV, -ir, STiconstlookup(0x7fffffff), 0);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+4),
                    -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+12),
                    -ir, __LINE__);
      }
      if (DTabs)
      {
         PrintComment(NULL, next, "Writing ~(-0) to memory for abss");
         k = ((SToff[DTabs-1].sa[2]-1)<<2) + 3;
         InsNewInst(NULL, next, MOV, -ir, STiconstlookup(0x7fffffff), 0);
         InsNewInst(NULL, next, ST, SToff[DTabs-1].sa[2], -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+4),
                    -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+8),
                    -ir, __LINE__);
         InsNewInst(NULL, next, ST, AddDerefEntry(-REG_SP, 0, 0, DT[k]+12),
                    -ir, __LINE__);
      }
      InsNewInst(NULL, next, COMMENT, STstrconstlookup("done archspec"), 0, 0);
   #endif
   #ifdef SPARC
      nam[0] = '@';
      nam[1] = 'i';
      nam[3] = '\0';
      for (j=i=0; i < NPARA; i++)
      {
         USED = DT[(SToff[paras[i]].sa[2]-1)<<2];
         if (USED)
            PrintComment(NULL, next, "para %d, name=%s", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         else
            PrintComment(NULL, next, "para %d, name=%s: UNUSED", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         flag = STflag[paras[i]];
         if (IS_PTR(flag) || IS_INT(flag) || IS_FLOAT(flag))
         {
            if (j < 6)
            {
               nam[2] = j + '0';
               ir = iName2Reg(nam);
               if (USED)
                  InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
            }
            else if (USED)
            {
               InsNewInst(NULL, next, LD, -ir,
                          AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
               InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, __LINE__);
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
                  InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
                  nam[2] = j + '0';
                  ir = iName2Reg(nam);
                  k = (SToff[paras[i]].sa[2]-1)<<2;
                  k = DT[k+3] + 4;
                  k = AddDerefEntry(-REG_SP, 0, 0, k);
                  InsNewInst(NULL, next, ST, k, -ir, __LINE__);
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
                  InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
                  k = (SToff[paras[i]].sa[2]-1)<<2;
                  k = DT[k+3] + 4;
                  k = AddDerefEntry(-REG_SP, 0, 0, k);
                  InsNewInst(NULL, next, LD, -ir,
                             AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
                  InsNewInst(NULL, next, ST, k, -ir, __LINE__);
               }
               j++;
            }
            else
            {
               if (USED)
               {
                  InsNewInst(NULL, next, LD, -ir,
                             AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
                  InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
               }
               j++;
               if (USED)
               {
                  k = (SToff[paras[i]].sa[2]-1)<<2;
                  k = DT[k+3] + 4;
                  k = AddDerefEntry(-REG_SP, 0, 0, k);
                  InsNewInst(NULL, next, LD, -ir,
                             AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
                  InsNewInst(NULL, next, ST, k, -ir, __LINE__);
               }
               j++;
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
         USED = DT[(SToff[paras[i]].sa[2]-1)<<2];
         if (USED)
            PrintComment(NULL, next, "para %d, name=%s", i, 
                         STname[paras[i]] ? STname[paras[i]] : "NULL");
         else
            PrintComment(NULL, next, "para %d, name=%s: UNUSED", i, 
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
                  InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
               }
               else
               {
                  if (j == 8) ir = GetReg(T_INT);
                  InsNewInst(NULL, next, LD, -ir,
                             AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
                  InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
               }
            }
            j++;
         }
         else if (IS_FLOAT(flag))
         {
            if (j == 8) ir = GetReg(T_INT);
            if (USED)
            {
               if (fc < 13)
               {
                  if (fc < 9) fnam[1] = '0' + fc + 1;
                  else { fnam[1] = '1'; fnam[2] = '0' + fc - 9; }
                  fr = fName2Reg(fnam);
                  InsNewInst(NULL, next, FST, SToff[paras[i]].sa[2], -fr, 0);
               }
               else
               {
                  InsNewInst(NULL, next, LD, -ir,
                             AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
                  InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
               }
            }
            fc++;
            j++;
         }
         else
         {
            assert(IS_DOUBLE(flag));
            if (j == 8 || j == 7) ir = GetReg(T_INT);
            if (USED)
            {
               if (fc < 13)
               {
                  if (fc < 9) fnam[1] = '0' + fc + 1;
                  else { fnam[1] = '1'; fnam[2] = '0' + fc - 9; }
                  fr = dName2Reg(fnam);
                  InsNewInst(NULL, next, FSTD,SToff[paras[i]].sa[2], -fr, 0);
                  j += 2;
               }
               else
               {
                  InsNewInst(NULL, next, LD, -ir,
                             AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
                  InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, 0);
                  j++;
                  InsNewInst(NULL, next, LD, -ir,
                             AddDerefEntry(rsav, 0, 0, fsize+j*4), 0);
                  k = (SToff[paras[i]].sa[2]-1)<<2;
                  k = AddDerefEntry(-REG_SP, 0, 0, DT[k+3]+4);
                  InsNewInst(NULL, next, ST, k, -ir, __LINE__);
                  j++;
               }
            }
            else j += 2;
            fc++;
         }
      }
   #endif
/*
 * Initialize constants
 */
   if (!reg1) reg1 = GetReg(T_INT);
   InitLocalConst(next, reg1);
}

void CreateEpilogue(int fsize,  /* frame size of returning func */
                    int Soff,   /* start of reg save area in frame */
                    int savesp, /* offset we saved sp at */
                    int nir,    /* number of int regs saved */
                    int *ir,    /* int regs saved */
                    int nfr,    /* number of single regs saved */
                    int *fr,    /* float regs saved */
                    int ndr,    /* number of double regs saved */
                    int *dr     /* double regs saved */
                    )
{
   int i;

   InsNewInst(NULL, NULL, LABEL, STstrconstlookup("IFKO_EPILOGUE"), 0, 0);
/*
 * Restore registers
 */
   for (i=0; i < ndr; i++)
      InsNewInst(NULL, NULL, FLDD, -dr[i],
                 AddDerefEntry(-REG_SP, 0, 0, Soff+i*8), 0);
   for (i=0; i < nir; i++)
      InsNewInst(NULL, NULL, LD, -ir[i],
                 AddDerefEntry(-REG_SP, 0,0, Soff+ndr*8+i*ISIZE), 0);
   for (i=0; i < nfr; i++)
      InsNewInst(NULL, NULL, FLD, -fr[i],
                 AddDerefEntry(-REG_SP, 0, 0, Soff+ndr*8+nir*ISIZE+i*4), 0);
/*
 * Restore stack pointer and return
 */
   if (savesp >= 0)
      InsNewInst(NULL, NULL, LD, -REG_SP, 
                 AddDerefEntry(-REG_SP, 0, 0, savesp), 0);
   else
      InsNewInst(NULL, NULL, ADD, -REG_SP, -REG_SP, STiconstlookup(fsize));
   InsNewInst(NULL, NULL, RET, 0, 0, 0);
   GetReg(-1);
}

void CreatePrologue(int align,  /* local-area required byte-alignment */
                    int lsize,  /* size of all required locals */
                    int csize,  /* call parameter area size */
                    int nir,    /* number of int regs to save */
                    int *ir,    /* int regs to save */
                    int ndr,    /* number of double regs to save */
                    int *dr,    /* double regs to save */
                    int nfr,    /* number of single regs to save */
                    int *fr     /* float regs to save */
                    )
{
   short prog, rsav=0, k;
   int i, maxalign=align, tsize, ssize=0;
   int Aoff;  /* offset to arguments, from frame pointer */
   int Soff=0; /* system-dependant skip offset */
   INSTQ *ip, *oldhead, *oldtail;
   extern INSTQ *iqhead;
   int Loff;   /* called routines frame size excluding locals */
   int SAVESP=(-1);  /* must we save SP to stack? */

fprintf(stderr, "align=%d,lsize=%d,csize=%d\n", align, lsize, csize);
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

   #ifdef X86_64
      if (!align) align = 16;
   #else
      if (!align) align = 4;
   #endif
   oldhead = iqhead;
   oldtail = iqhead->prev;
/* 
 * Put routine name label
 */
   prog = STstrconstlookup(rout_name);
fprintf(stderr, "prog=%d!, rout_name=%s\n", prog, rout_name);
   STflag[prog-1] |= GLOB_BIT;
   InsNewInst(NULL, oldhead, LABEL, prog, 0, 0);

/*
 * For x86-64, save %rbp, if necessary, to the reserved location of 0(%rsp)
 */
   #ifdef X86_64
      k = iName2Reg("%rbp");
      for (i=0; i < nir && ir[i] != k; i++);
      if (i < nir)
      {
         InsNewInst(NULL, oldhead, ST, 
                    AddDerefEntry(-REG_SP, 0, 0, 0), -k, 0);
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
/*   tsize = Aoff + csize + ssize + lsize; */
   Soff += csize;
   #ifdef X86_64
      Loff = 8*(nir+ndr) + 4*nfr;
      if (Loff % ASPALIGN) Loff = (Loff/ASPALIGN)*ASPALIGN + ASPALIGN;
      Loff += Soff;
      tsize = Loff + lsize;
      if (tsize % ASPALIGN) tsize = (tsize/ASPALIGN)*ASPALIGN + ASPALIGN;
fprintf(stderr, "tsize=%d, Loff=%d, Soff=%d lsize=%d\n", tsize, Loff, Soff, lsize);
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
      if (SAVESP >= 0)
      {
         InsNewInst(NULL, oldhead, COMMENT, 0, 0, 0);
         InsNewInst(NULL, oldhead, COMMENT, STstrconstlookup("To ensure greater alignment than sp, save old sp to stack and move sp"), 0, 0);
         InsNewInst(NULL, oldhead, COMMENT, 0, 0, 0);
         rsav = GetReg(T_INT);
         assert(rsav <= NSIR);
         rsav = -rsav;
         InsNewInst(NULL, oldhead, MOV, rsav, -REG_SP, 0);
      }
      else
   #endif
   {
      InsNewInst(NULL, oldhead, COMMENT, 0, 0, 0);
      InsNewInst(NULL, oldhead, COMMENT, STstrconstlookup("Adjust sp"), 0, 0);
      InsNewInst(NULL, oldhead, COMMENT, 0, 0, 0);
   }
   InsNewInst(NULL, oldhead, SUB, -REG_SP, -REG_SP, STiconstlookup(tsize));
   if (SAVESP >= 0)
   {
      i = const2shift(maxalign);
      assert(i >= 3);
      i = STiconstlookup(i);
      InsNewInst(NULL, oldhead, SHR, -REG_SP, -REG_SP, i);
      InsNewInst(NULL, oldhead, SHL, -REG_SP, -REG_SP, i);
      InsNewInst(NULL, oldhead, ST, AddDerefEntry(-REG_SP, 0, 0, SAVESP), 
                 rsav, 0);
   }
fprintf(stderr, "Local offset=%d\n", Loff);
   CorrectLocalOffsets(Loff);
   InsNewInst(NULL, oldhead, COMMENT, 0, 0, 0);
   InsNewInst(NULL, oldhead, COMMENT, STstrconstlookup("Save registers"), 0, 0);
   InsNewInst(NULL, oldhead, COMMENT, 0, 0, 0);
/*
 * Save registers
 */
   for (i=0; i < ndr; i++)
      InsNewInst(NULL, oldhead, FSTD, 
                 AddDerefEntry(-REG_SP, 0, 0, Soff+i*8), -dr[i], 0);
   for (i=0; i < nir; i++)
      InsNewInst(NULL, oldhead, ST, 
                 AddDerefEntry(-REG_SP, 0, 0, Soff+ndr*8+i*ISIZE), -ir[i], 0);
   for (i=0; i < nfr; i++)
      InsNewInst(NULL, oldhead, FST, 
                 AddDerefEntry(-REG_SP, 0, 0, Soff+ndr*8+nir*ISIZE+i*4), 
                 -fr[i], 0);
   Extern2Local(oldhead, oldtail, rsav, rsav ? Aoff : Aoff+tsize);
   GetReg(-1);
   InsNewInst(NULL, oldhead, COMMENT, 0, 0, 0);
   InsNewInst(NULL, oldhead, COMMENT, 
              STstrconstlookup("END OF FUNCTION PROLOGUE"), 0, 0);
   InsNewInst(NULL, oldhead, COMMENT, 0, 0, 0);
   CreateEpilogue(tsize, Soff, SAVESP, nir, ir, nfr, fr, ndr, dr);
}
void KillUnusedLocals()  /* HERE, HERE: move to symtab */
{
}
void FixFrame()
/*
 * As final step before lil-to-assembly conversion, fix the frame info, and
 * generate function prologue and epilogue
 */
{
   extern int LOCALIGN, LOCSIZE;
   extern INSTQ *iqhead;
   int i, ni, nf, nd, isav[TNIR], fsav[TNFR], dsav[TNDR], savr[64];
   MarkUnusedLocals(); 
   CreateSysLocals();
   NumberLocalsByType();
   FindRegUsage(iqhead, &ni, isav, &nf, fsav, &nd, dsav);
   RemoveNosaveregs(IREGBEG, TNIR, isav, icalleesave);
   ni = GetRegSaveList(IREGBEG, ni, isav);
   RemoveNosaveregs(FREGBEG, TNFR, fsav, fcalleesave);
   nf = GetRegSaveList(FREGBEG, nf, fsav);
   RemoveNosaveregs(DREGBEG, TNDR, dsav, dcalleesave);
   nd = GetRegSaveList(DREGBEG, nd, dsav);
   #ifdef X86_64
      UpdateLocalDerefs(8);
   #else
      UpdateLocalDerefs(4);
   #endif
   for (i=0; i < NIR; i++) savr[i] = i+2; 
   CreatePrologue(LOCALIGN, LOCSIZE, 0, ni, isav, nf, fsav, nd, dsav);
}
