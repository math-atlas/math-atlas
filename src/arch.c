#include "ifko.h"
#define ARCH_DECLARE
#include "fko_arch.h"

int GetArchAlign(int nvd, int nvf, int nd, int nf, int nl, int ni)
/*
 *  Returns required architectural alignment given the number of
 *  vector double, vector float, double, float, long, and ints you
 *  want to save (actually, these only need to be boolean).
 */
{
   int align = 0;
   if (nvd) align = FKO_DVLEN*8;
   else if (nvd) align = FKO_SVLEN*4;
   #ifdef X86_32
      else if (nd || nf || nl || ni) align = 4;
   #else
      else if (nd || nl) align = 8;
      else if (nf || ni) align = 4;
   #endif
   return(align);
}
short iName2Reg(char *rname)
{
   short i;
   for (i=0; i < TNIR; i++)
     if (!strcmp(rname, archiregs[i])) return(i+1);
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
#elif defined(x86_32)
   #define ASPALIGN 4
#else
   #define ASPALIGN 4
#endif
#ifdef ADDR64
   #define AMOV MOVL
   #define ASUB SUBL
   #define AADD ADDL
   #define ASHL SHLL
   #define ASHR SHRL
   #define AST  STL
   #define ALD  LDL
   #define ASIZE  8
#else
   #define AMOV MOV
   #define ASUB SUB
   #define AADD ADD
   #define ASHL SHL
   #define ASHR SHR
   #define AST  ST
   #define ALD  LD
   #define ASIZE  4
#endif

void CreateSysLocals()
/*
 *  If required, creates any locals needed to support instructions
 */
{
#ifdef x86
   extern int DTnzerod, DTabsd, DTnzero, DTabs;
   if (DTnzerod == -1)
      DTnzerod = STdef("_NEGZEROD", VEC_BIT | T_DOUBLE | LOCAL_BIT, 0);
   if (DTabsd == -1)
      DTabsd = STdef("_ABSVALD", VEC_BIT | T_DOUBLE | LOCAL_BIT, 0);
   if (DTnzero == -1)
      DTnzero = STdef("_NEGZERO", VEC_BIT | T_DOUBLE | LOCAL_BIT, 0);
   if (DTabs == -1)
      DTabs = STdef("_ABSVAL", VEC_BIT | T_DOUBLE | LOCAL_BIT, 0);
#else
#endif
}

void FPConstStore(INSTQ *next, short rsav, short k, short reg)
{
   int flag;
   int *ip;
   short *sp;
   double d;
   short i;
   float f;
   flag = STflag[k];
   if (IS_VEC(flag))
   {
      fprintf(stderr, "Vector constants not yet supported!\n");
      exit(-1);
   }
   else if (IS_DOUBLE(flag))
   {
      d = SToff[SToff[k].sa[0]].d;
      if (d == 0.0)
      {
         InsNewInst(NULL, next, SUB, -reg, -reg, -reg);
         InsNewInst(NULL, next, ST, SToff[k].sa[2], -reg, __LINE__);
	 i = SToff[k].sa[2] - 1;
	 i = DT[(i<<2)+3] + 4;
         InsNewInst(NULL, next, ST, AddDerefEntry(rsav,0,0,i), -reg, __LINE__);
      }
      #ifdef x86
         ip = (int*) &(SToff[SToff[k].sa[0]].d);
         InsNewInst(NULL, next, MOV, -reg, STiconstlookup(*ip), __LINE__);
         InsNewInst(NULL, next, ST, SToff[k].sa[2], -reg, __LINE__);
         InsNewInst(NULL, next, MOV, -reg, STiconstlookup(ip[1]), __LINE__);
	 i = SToff[paras[k]].sa[2] - 1;
	 i = DT[(i<<2)+3] + 4;
         InsNewInst(NULL, next, ST, AddDerefEntry(rsav,0,0,i), -reg, __LINE__);
/*
 *    Other archs must load operands 16 bits at a time
 */
      #else
         sp = (short*) &(SToff[SToff[k].sa[0]].d);
         i = STiconstlookup(16);
         InsNewInst(NULL, next, MOV, -reg, STiconstlookup(*sp), __LINE__);
         InsNewInst(NULL, next, SHL, -reg, i, __LINE__);
         InsNewInst(NULL, next, ADD, -reg, -reg, STiconstlookup(sp[1]));
         InsNewInst(NULL, next, SHL, -reg, i, __LINE__);
         InsNewInst(NULL, next, ADD, -reg, -reg, STiconstlookup(sp[2]));
         InsNewInst(NULL, next, SHL, -reg, i, __LINE__);
         InsNewInst(NULL, next, ADD, -reg, -reg, STiconstlookup(sp[3]));
      #endif
   }
   else
   {
      assert(IS_FLOAT(flag));
      f = SToff[SToff[k].sa[0]].f;
      if (f == 0.0e0)
      {
         InsNewInst(NULL, next, SUB, -reg, -reg, -reg);
         InsNewInst(NULL, next, ST, SToff[k].sa[2], -reg, __LINE__);
      }
   }
}

void FPConst2Local(INSTQ *next, INSTQ *end, short rsave, short reg1)
/*
 * Saves correct bit patterns to frame for fpconst locals, and then
 * look at all instructions between *next and *end.  Find any inst that
 * do FMOV[d] on fp const, and change them to local loads
 */
{
   short i, N, FPUSE=0;
   int flag;
   INSTQ *ip;
   for (i=0, N=STlen(); i < N; i++)
   {
      flag = STflag[i];
fprintf(stderr, "%s(%d): N=%d, i=%d, CO=%d, LO=%d\n",__FILE__,__LINE__,N,i,IS_CONST(flag),IS_LOCAL(flag));
      if (IS_CONST(flag) && IS_LOCAL(flag))
      {
         assert(IS_FLOAT(flag) || IS_DOUBLE(flag));
         FPConstStore(next, rsave, i, reg1);
         FPUSE++;
      }
   }
fprintf(stderr, "%s(%d): N=%d, i=%d\n",__FILE__,__LINE__,N,i);
/*
 * Change FMOV[d] of fpconst to appopriate local load
 */
fprintf(stderr, "FOUND000:\n");
   if (FPUSE)
   {
fprintf(stderr, "FOUND00:\n");
      for (ip=next; ip != end; ip = ip->next)
      {
         if (ip->inst[0] == FMOV || ip->inst[0] == FMOVD)
         {
fprintf(stderr, "FOUND0:\n");
            i = ip->inst[2];
            if (i > 0)
            {
fprintf(stderr, "FOUND!\n");
               ip->inst[0] = (ip->inst[0] == FMOV) ? FLD : FLDD;
               i = FindLocalFPConst(i);
               assert(i > 0);
               ip->inst[2] = SToff[i-1].sa[2];
            }
         }
      }
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
   int nbytes=0;
   short *paras;
   char nam[8];

   if (rsav) fsize = 0;
   else rsav = -REG_SP;
   if (NPARA)
   {
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
fprintf(stderr, "para #%d\n", SToff[i].sa[0]);
         assert(SToff[i].sa[0] <= NPARA);
         paras[SToff[i].sa[0]-1] = i;
         j++;
      }
   }
   #ifdef X86_32
      reg1 = ir = GetReg(T_INT);
      for (j=i=0; i < NPARA; i++)
      {
         flag = STflag[paras[i]];
         InsNewInst(NULL, next, LD, -ir,
                    AddDerefEntry(rsav, 0, 0, fsize+4+j*4), 0);
         InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, __LINE__);
         j++;
         if (IS_DOUBLE(flag))
         {
            InsNewInst(NULL, next, LD, -ir,
                       AddDerefEntry(rsav, 0, 0, fsize+4+j*4), 0);
	    k = SToff[paras[i]].sa[2] - 1;
	    k = DT[(k<<2)+3] + 4;
            InsNewInst(NULL, next, ST, AddDerefEntry(rsav, 0, 0, k), 
	               -ir, __LINE__);
            j++;
         }
      }
      if (DTnzerod > 0)
      {
         InsNewInst(NULL, next, MOV, -ir, STiconstlookup(0), 0);
         InsNewInst(NULL, next, ST, SToff[DTnzerod].sa[2], -ir, __LINE__);
         InsNewInst(NULL, next, MOV, -ir, STiconstlookup(-2147483648), 0);
         k = (SToff[DTnzerod].sa[2])<<2;
         k = DT[k+3] + 4;
         k = AddDerefEntry(-REG_SP, 0, 0, k);
         InsNewInst(NULL, next, ST, k, -ir, __LINE__);
      }
      if (DTabsd)
      {
         InsNewInst(NULL, next, MOV, -ir, STiconstlookup(-1), 0);
         InsNewInst(NULL, next, ST, SToff[DTabsd].sa[2], -ir, __LINE__);
         InsNewInst(NULL, next, MOV, -ir, STiconstlookup(2147483647), 0);
         k = (SToff[DTabsd].sa[2])<<2;
         k = DT[k+3] + 4;
         k = AddDerefEntry(-REG_SP, 0, 0, k);
         InsNewInst(NULL, next, ST, k, -ir, __LINE__);
      }
   #endif
   #ifdef SPARC
      nam[0] = '@@';
      nam[1] = 'i';
      nam[3] = '\0';
      for (j=i=0; i < NPARA; i++)
      {
         flag = STflag[paras[i]];
         if (IS_PTR(flag) || IS_INT(flag) || IS_FLOAT(flag))
         {
            if (j < 6)
            {
               nam[2] = j + '0';
               ir = iName2Reg(nam);
fprintf(stderr, "STORE: %d, %d\n", SToff[paras[i]].sa[2], -ir);
               InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, __LINE__);
            }
            else
            {
               InsNewInst(NULL, next, LD, -ir,
                          AddDerefEntry(rsav, 0, 0, fsize+68+j*4), 0);
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
               InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, __LINE__);
               j++;
               nam[2] = j + '0';
               ir = iName2Reg(nam);
               k = (SToff[paras[i]].sa[2])<<2;
               k = DT[k+3] + 4;
               k = AddDerefEntry(-REG_SP, 0, 0, k);
               InsNewInst(NULL, next, ST, k, -ir, __LINE__);
               j++;
            }
            else if (j == 5)
            {
               nam[2] = j + '0';
               ir = iName2Reg(nam);
               InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, __LINE__);
               j++;
               k = (SToff[paras[i]].sa[2])<<2;
               k = DT[k+3] + 4;
               k = AddDerefEntry(-REG_SP, 0, 0, k);
               InsNewInst(NULL, next, LD, -ir,
                          AddDerefEntry(iName2Reg("@@i6"), 0, 0, 68+j*4), 0);
               InsNewInst(NULL, next, ST, k, -ir, __LINE__);
               j++;
            }
            else
            {
               InsNewInst(NULL, next, LD, -ir,
                          AddDerefEntry(rsav, 0, 0, fsize+68+j*4), 0);
               InsNewInst(NULL, next, ST, SToff[paras[i]].sa[2], -ir, __LINE__);
               j++;
               k = (SToff[paras[i]].sa[2])<<2;
               k = DT[k+3] + 4;
               k = AddDerefEntry(-REG_SP, 0, 0, k);
               InsNewInst(NULL, next, LD, -ir,
                          AddDerefEntry(iName2Reg("@@i6"), 0, 0, 68+j*4), 0);
               InsNewInst(NULL, next, ST, k, -ir, __LINE__);
               j++;
            }
         }
      }
   #endif
/*
 * Store floating point constants to stack frame
 */
   if (!reg1) reg1 = GetReg(T_INT);
   FPConst2Local(next, end, rsav, reg1);
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
   for (i=0; i < nfr; i++)
      InsNewInst(NULL, NULL, FLD, -fr[i],
                 AddDerefEntry(-REG_SP, 0, 0, Soff+ndr*8+i*4), 0);
   for (i=0; i < nir; i++)
      InsNewInst(NULL, NULL, LD, -ir[i],
                 AddDerefEntry(-REG_SP, 0,0, Soff+ndr*8+nfr*4+i*4), 0);
/*
 * Restore stack pointer and return
 */
   if (savesp)
      InsNewInst(NULL, NULL, ALD, REG_SP, 
                 AddDerefEntry(-REG_SP, 0, 0, savesp), 0);
   else
      InsNewInst(NULL, NULL, AADD, -REG_SP, -REG_SP, STiconstlookup(fsize));
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
   int i, maxalign=align, Aoff, tsize, ssize=0;
   INSTQ *ip, *oldhead, *oldtail;
   extern INSTQ *iqhead;
   int Soff, Loff;
   int SAVESP=0;  /* must we save SP to stack? */

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

   if (!align) align = 4;
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
 * Figure stack frame, ensuring all parts have correct alignment
 */
   #ifdef SPARC
      Aoff = 68;
      if (csize && csize < 6*4) csize = 6*4;
   #elif defined(X86_32)
      Aoff  = 0;
   #elif defined(OSX_PPC)
      if (csize < 32) csize = 32;
      Aoff = 24;
   #elif defined(LINUX_PPC)
      Aoff = 8;
   #endif
/*   tsize = Aoff + csize + ssize + lsize; */
   Soff = Aoff + csize;
/*
 * We assume sp already 4-byte aligned but may need to make 8-byte aligned
 * if demanded by save 
 */
   if (ndr)
   {
      if ((Soff>>3)<<3 != Soff) Soff = 8 + ((Soff>>3)<<3);
      #ifndef x86_32
         if (maxalign < 8) maxalign = 8;
      #endif
   }
   if (maxalign > ASPALIGN)
   {
      Loff = Soff + 8*ndr + 4*nfr + 4*nir + ASIZE;
      SAVESP = Loff-ASIZE;
   }
   else Loff = Soff + 8*ndr + 4*nfr + 4*nir;
   if (Loff%align) Loff = (Loff/align)*align + align;
   tsize = Loff + lsize;
   if (tsize % ASPALIGN) tsize = (tsize/ASPALIGN)*ASPALIGN + ASPALIGN;
fprintf(stderr, "tsize=%d, SAVESP=%d\n\n", tsize, SAVESP);
   if (SAVESP)
   {
      rsav = GetReg(T_INT);
      assert(rsav <= NSIR);
      InsNewInst(NULL, oldhead, AMOV, -rsav, -REG_SP, 0);
   }
   InsNewInst(NULL, oldhead, ASUB, -REG_SP, -REG_SP, STiconstlookup(tsize));
   if (SAVESP)
   {
      i = const2shift(maxalign);
      assert(i >= 3);
      i = STiconstlookup(i);
      InsNewInst(NULL, oldhead, ASHR, -REG_SP, -REG_SP, i);
      InsNewInst(NULL, oldhead, ASHL, -REG_SP, -REG_SP, i);
      InsNewInst(NULL, oldhead, AST, AddDerefEntry(-REG_SP, 0, 0, SAVESP), 
                 -rsav, 0);
   }
   CorrectLocalOffsets(Loff);
/*
 * Save registers
 */
   for (i=0; i < ndr; i++)
      InsNewInst(NULL, oldhead, FSTD, 
                 AddDerefEntry(-REG_SP, 0, 0, Soff+i*8), -dr[i], 0);
   for (i=0; i < nfr; i++)
      InsNewInst(NULL, oldhead, FST, 
                 AddDerefEntry(-REG_SP, 0, 0, Soff+ndr*8+i*4), -fr[i], 0);
   for (i=0; i < nir; i++)
      InsNewInst(NULL, oldhead, ST, 
                 AddDerefEntry(-REG_SP, 0, 0, Soff+ndr*8+nfr*4+i*4), -ir[i], 0);
   Extern2Local(oldhead, oldtail, rsav, tsize);
   GetReg(-1);
   CreateEpilogue(tsize, Soff, SAVESP, nir, ir, nfr, fr, ndr, dr);
}
