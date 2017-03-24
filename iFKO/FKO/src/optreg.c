/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#include <fko.h>
#include <fko_arch.h>

static IGNODE **IG=NULL;
static int NIG=0, TNIG=0;
void DumpIG(FILE *fpout, int N, IGNODE **igs);

int ireg2type(int ireg)
{
   int iret;
   iret = -1;
   if (ireg >= IREGBEG && ireg < IREGEND)
      iret = T_INT;
   else if (ireg >= FREGBEG && ireg < FREGEND)
      iret = T_FLOAT;
   else if (ireg >= DREGBEG && ireg < DREGEND)
      iret = T_DOUBLE;
#ifdef VFREGBEG
   else if (ireg >= VFREGBEG && ireg < VFREGEND)
      iret = T_VFLOAT;
#endif
#ifdef VDREGBEG
   else if (ireg >= VDREGBEG && ireg < VDREGEND)
      iret = T_VDOUBLE;
#endif
#ifdef VIREGBEG
   else if (ireg >= VIREGBEG && ireg < VIREGEND)
      iret = T_VINT;
#endif
   return(iret);
}
short Reg2Int(char *regname)
/*
 * Given a register of regname, returns integer number
 */
{
   int i;
   if (regname[0] == 'P' && regname[1] == 'C' && regname[2] == '\0')
      return(-PCREG);
   if (regname[0] == 'S' && regname[1] == 'P' && regname[2] == '\0')
      return(-REG_SP);
   for (i=IREGBEG; i < IREGEND; i++)
      if (!strcmp(archiregs[i-IREGBEG], regname)) return(-i);
   for (i=FREGBEG; i < FREGEND; i++)
      if (!strcmp(archfregs[i-FREGBEG], regname)) return(-i);
   for (i=DREGBEG; i < DREGEND; i++)
      if (!strcmp(archdregs[i-DREGBEG], regname)) return(-i);
#ifdef VFREGBEG
   for (i=VFREGBEG; i < VFREGEND; i++)
      if (!strcmp(archvfregs[i-VFREGBEG], regname)) return(-i);
#endif
#ifdef VDREGBEG
   for (i=VDREGBEG; i < VDREGEND; i++)
      if (!strcmp(archvdregs[i-VDREGBEG], regname)) return(-i);
#endif
#ifdef VIREGBEG
   for (i=VIREGBEG; i < VIREGEND; i++)
      if (!strcmp(archviregs[i-VIREGBEG], regname)) return(-i);
#endif
   for (i=ICC0; i < NICC; i++)
      if (!strcmp(ICCREGS[i], regname)) return(-i);
   for (i=FCC0; i < NFCC; i++)
      if (!strcmp(FCCREGS[i], regname)) return(-i);
   return(0);
}

int NumberArchRegs()
{
   return(TNREG);
}
char *Int2Reg0(int i)
/*
 * Translates integral encoding to machine-specific registers
 */
{
   static char ln[128];
   char *ret=ln;

   assert (i < 0);
   i = -i;
   if (i >= IREGBEG && i < IREGEND)
      sprintf(ln, "%s", archiregs[i-IREGBEG]);
   else if (i >= FREGBEG && i < FREGEND)
      sprintf(ln, "%s", archfregs[i-FREGBEG]);
   else if (i >= DREGBEG && i < DREGEND)
      sprintf(ln, "%s", archdregs[i-DREGBEG]);
   else if (i >= ICCBEG && i < ICCEND)
      sprintf(ln, "%s", ICCREGS[i-ICCBEG]);
   else if (i >= FCCBEG && i < FCCEND)
      sprintf(ln, "%s", FCCREGS[i-FCCBEG]);
   else if (i == PCREG)
      sprintf(ln, "%s", "PC");
#ifdef VFREGBEG
   else if (i >= VFREGBEG && i < VFREGEND)
      sprintf(ln, "%s", archvfregs[i-VFREGBEG]);
#endif
#ifdef VDREGBEG
   else if (i >= VDREGBEG && i < VDREGEND)
      sprintf(ln, "%s", archvdregs[i-VDREGBEG]);
#endif
#ifdef VIREGBEG
   else if (i >= VIREGBEG && i < VIREGEND)
      sprintf(ln, "%s", archviregs[i-VIREGBEG]);
#endif
   else
      ret = NULL;
   return(ret);
}
char *Int2Reg(int i)
/*
 * Translates integral encoding to machine-specific registers
 */
{
   char *ret;
   ret = Int2Reg0(i);
   if (!ret)
   {
   fprintf(stderr, 
      "I=[%d,%d); F=[%d,%d); D=[%d,%d); ICC=[%d,%d); FCC=[%d,%d); PC=%d\n",
           IREGBEG, IREGEND,FREGBEG, FREGEND,DREGBEG, DREGEND,
           ICCBEG, ICCEND, FCCBEG, FCCEND, PCREG);
      fko_error(__LINE__, "Unknown register index %d, file=%s\n",
                i, __FILE__);
   }
   return(ret);
}

void SetAllTypeReg(INT_BVI iv, int type)
/*
 * Modify regstate iv to set all registers of type type to 1, leaving the rest
 * of the state unchanged
 */
{
   int i, ibeg, iend;
   switch(type)
   {
#ifdef X86_64
   case T_SHORT:
#endif
   case T_INT:
      ibeg = IREGBEG;
      iend = IREGEND;
      break;
   case T_FLOAT:
      ibeg = FREGBEG;
      iend = FREGEND;
      break;
#ifdef ArchHasVec
   case T_VFLOAT:
      ibeg = VFREGBEG;
      iend = VFREGEND;
      break;
   case T_DOUBLE:
      ibeg = DREGBEG;
      iend = DREGEND;
      break;
   case T_VDOUBLE:
      ibeg = VDREGBEG;
      iend = VDREGEND;
      break;
   case T_VINT:
      ibeg = VIREGBEG;
      iend = VIREGEND;
      break;
#endif
   default:
      fko_error(__LINE__, "unknown type %d, file=%s\n", type, __FILE__);
   }
   for (i=ibeg; i < iend; i++)
      SetVecBit(iv, i-1, 1);
}

int Reg2Regstate(int k)
/*
 * Given register k, set regstate so that all registers used by k are
 * represented (i.e., on some archs, float and double regs are aliased)
 * RETURNS: bitvec with appropriate register numbers set
 */
{
   static INT_BVI iv=0;
   #ifdef SPARC
      int i;
   #endif

   if (k <= 0)
   {
      if (iv) KillBitVec(iv);
      iv = 0;
   }
   else
   {
      if (!iv) iv = NewBitVec(TNREG);
      SetVecAll(iv, 0);
      SetVecBit(iv, k-1, 1);
      if (k >= FREGBEG && k < FREGEND)
      {
         #if defined(X86) || defined(PPC)
            SetVecBit(iv, k-FREGBEG+DREGBEG-1, 1);
            #ifdef X86
               SetVecBit(iv, k-FREGBEG+VDREGBEG-1, 1);
               SetVecBit(iv, k-FREGBEG+VFREGBEG-1, 1);
               #ifdef VIREGBEG
                  SetVecBit(iv, k-FREGBEG+VIREGBEG-1, 1);   
               #endif
            #endif
         #elif defined(SPARC)
            SetVecBit(iv, ((k-FREGBEG)>>1)+DREGBEG-1, 1);
         #endif
      }
      else if (k >= DREGBEG && k < DREGEND)
      {
         #if defined(X86) || defined(PPC)
            SetVecBit(iv, k-DREGBEG+FREGBEG-1, 1);
            #ifdef X86
               SetVecBit(iv, k-DREGBEG+VDREGBEG-1, 1);
               SetVecBit(iv, k-DREGBEG+VFREGBEG-1, 1);
               #ifdef VIREGBEG
                  SetVecBit(iv, k-DREGBEG+VIREGBEG-1, 1);   
               #endif
            #endif
         #elif defined(SPARC)
            i = k - DREGBEG;
            if (i < 16)
            {
               SetVecBit(iv, FREGBEG+i*2-1, 1);
               SetVecBit(iv, FREGBEG+i*2, 1);
            }
         #endif
      }
   #ifdef X86
      else if (k >= VFREGBEG && k < VFREGEND)
      {
         SetVecBit(iv, k-VFREGBEG+VDREGBEG-1, 1);
         SetVecBit(iv, k-VFREGBEG+FREGBEG-1, 1);
         SetVecBit(iv, k-VFREGBEG+DREGBEG-1, 1);
         #ifdef VIREGBEG
            SetVecBit(iv, k-VFREGBEG+VIREGBEG-1, 1);   
         #endif
      }
      else if (k >= VDREGBEG && k < VDREGEND)
      {
         SetVecBit(iv, k-VDREGBEG+VFREGBEG-1, 1);
         SetVecBit(iv, k-VDREGBEG+FREGBEG-1, 1);
         SetVecBit(iv, k-VDREGBEG+DREGBEG-1, 1);
         #ifdef VIREGBEG
            SetVecBit(iv, k-VDREGBEG+VIREGBEG-1, 1);   
         #endif
      }
      #ifdef VIREGBEG
      else if (k >= VIREGBEG && k < VIREGEND) /* for new type V_INT*/
      {
         SetVecBit(iv, k-VIREGBEG+VDREGBEG-1, 1);
         SetVecBit(iv, k-VIREGBEG+VFREGBEG-1, 1);
         SetVecBit(iv, k-VIREGBEG+FREGBEG-1, 1);
         SetVecBit(iv, k-VIREGBEG+DREGBEG-1, 1);
      }
      #endif
   #endif
   }
#if 0
   else if (!(k >= IREGBEG && k < IREGEND))
      fko_error(__LINE__, "Unknown register index %d, file=%s\n",
                  k, __FILE__);
#endif
   return(iv);
}

int FindLiveregs(INSTQ *here)
/*
 * Finds the livereg up to instruction here (here itself is not included
 * in the analysis)
 * RETURNS: bitvector with all live regs set
 */
{
   INSTQ *ip;
   int i, k;
   static INT_BVI liveregs=0, mask, vtmp;

   if (here)
   {
      if (!liveregs)
      {
         liveregs = NewBitVec(TNREG);
         mask     = NewBitVec(TNREG);
         vtmp     = NewBitVec(TNREG);
/*
 *       Set only those regs we're interested in
 */
         SetVecAll(mask, 0);
         SetAllTypeReg(mask, T_INT);
         SetAllTypeReg(mask, T_FLOAT);
         SetAllTypeReg(mask, T_DOUBLE);
         SetAllTypeReg(mask, T_VFLOAT);
         SetAllTypeReg(mask, T_VDOUBLE);
         #ifdef VIREGBEG         
            SetAllTypeReg(mask, T_VINT);
         #endif
         SetVecBit(mask, REG_SP-1, 0);
         #ifdef X86_32
/*
 *          FIXED:  Reg2Int always return -ve index!!! -- Majedul 
 */
            SetVecBit(mask, -Reg2Int("@st")-1, 0);
         #endif
      }
      else
         SetVecAll(liveregs, 0);

/*
 *    Add all regs live on block entry to liveregs by adding regs from ins to lr
 */
      BitVecComb(vtmp, here->myblk->ins, mask, '&');
/*
 *    FIXME: Reg2Regstate() always set all the alias regs
 */
      for (i=1; (k = GetSetBitX(vtmp, i)); i++)
         BitVecComb(liveregs, liveregs, Reg2Regstate(k), '|');
      for (ip=here->myblk->inst1; ip != here; ip = ip->next)
      {
/*
 *       Remove all dead regs from livereg
 *       FIXME: Creates problem when we have more than one alias regs is live. 
 *       one of them is dead but still it should not be leased. 
 */
         if (ip->deads)
         {
            BitVecComb(vtmp, ip->deads, mask, '&');
            for (i=1; (k = GetSetBitX(vtmp, i)); i++)
               BitVecComb(liveregs, liveregs, Reg2Regstate(k), '-');
         }
/*
 *       If a register is set, add it to liveregs
 */
         if (ip->set)
         {
            BitVecComb(vtmp, ip->set, mask, '&');
            for (i=1; (k = GetSetBitX(vtmp, i)); i++)
               BitVecComb(liveregs, liveregs, Reg2Regstate(k), '|');
         }
      }
   }
   else
   {
      if (liveregs)
      {
         KillBitVec(liveregs);
         KillBitVec(mask);
         KillBitVec(vtmp);
      }
      vtmp = mask = liveregs = 0;
   }
   return(liveregs);
}

int GetRegForAsg(int type, INT_BVI iv, INT_BVI livereg)
{
   SetVecAll(iv, 0);
   SetAllTypeReg(iv, type);
   BitVecComb(iv, iv, livereg, '-');
   SetVecBit(iv, REG_SP-1, 0);
/*
 * Don't use [f,d]retreg (%st) for x86-32
 */
   #ifdef X86_32
      SetVecBit(iv, FRETREG-1, 0);
      SetVecBit(iv, DRETREG-1, 0);
   #endif
   return(AnyBitsSet(iv));
}

int GetRegFindLR(int type, INT_BVI iv, INSTQ *here)
{
   return(GetRegForAsg(type, iv, FindLiveregs(here)));
}

void NewIGTable(int chunk)
{
   int i, n;
   IGNODE **new;
   n = TNIG + chunk;
   new = malloc(n*sizeof(IGNODE*));
   assert(new);

   for (i=0; i != TNIG; i++)
      new[i] = IG[i];
   for (; i != n; i++)
      new[i] = NULL;
   if (IG) free(IG);
   IG = new;
   TNIG = n;
}

void KillIGNode(IGNODE *ig)
{
   if (ig->blkbeg)
      KillBlockList(ig->blkbeg);
   if (ig->blkend)
      KillBlockList(ig->blkend);
   if (ig->blkspan)
      KillBlockList(ig->blkspan);
   if (ig->ldhoist)
      KillBlockList(ig->ldhoist);
   if (ig->stpush)
      KillBlockList(ig->stpush);
   if (ig->myblkvec)
      KillBitVec(ig->myblkvec);
   if (ig->liveregs)
      KillBitVec(ig->liveregs);
   if (ig->conflicts)
      KillBitVec(ig->conflicts);
   IG[ig->ignum] = NULL;
   free(ig);
}

void KillIGTableEntries()
{
   int i;
   for (i=0; i < NIG; i++)
   {
      if (IG[i])
      {
         KillIGNode(IG[i]);
         IG[i] = NULL;
      }
   }
   NIG = 0;
}

void KillIGTable()
{
   KillIGTableEntries();
   free(IG);
   IG = NULL;
   TNIG = 0;
}

int AddIG2Table(IGNODE *node)
{
   if (NIG == TNIG)
      NewIGTable(64);
   IG[NIG] = node;
   node->ignum = NIG;
   return(NIG++);
}

IGNODE *NewIGNode(BBLOCK *blk, short var)
{
   IGNODE *new;
   new = malloc(sizeof(IGNODE));
   assert(new);
   new->blkbeg = new->blkend = new->blkspan = new->ldhoist = new->stpush = 
                 NULL;
   if (blk)
   {
      new->myblkvec = NewBitVec(blk->bnum);
      assert(new->myblkvec);
      SetVecBit(new->myblkvec, blk->bnum-1, 1);
   }
   else
      new->myblkvec = 0;
   new->liveregs = NewBitVec(TNREG);
   new->conflicts = NewBitVec(TNREG);
   new->nread = new->nwrite = 0;
   new->var = var;
   if (var)
   {
      if (IS_DEREF(STflag[var-1]))
         new->deref = var;
      else
      {
         assert(IS_VAR(STflag[var-1])); 
         new->deref = SToff[var-1].sa[2];
         assert(new->deref > 0);
      }
   }
   else new->deref = 0;
   new->ignum = AddIG2Table(new);
   new->reg = 0;
   return(new);
}

INT_BVI AllRegVec(int k)
/*
 * Majedul: this func is changed to add opportunity to reset the static var
 * k is for reset the static var iv.
 */
{
   static INT_BVI iv=0;
   int i;
   if (k <= 0)
   {
      if (iv) KillBitVec(iv);
      iv = 0;
   }
   else
   {
      if (!iv)
      {
         iv = NewBitVec(TNREG);
         for (i=0; i < TNREG; i++)
            SetVecBit(iv, i, 1);
      }
   }
   return(iv);
}

void CalcBlockIG(BBLOCK *bp)
{
   int i, j, n, k;
   IGNODE *node;
   INSTQ *ip;
   INT_BVI liveregs, imask, iv;
   const int chunk = 32;
   short *vals;
   IGNODE **myIG = NULL;   /* array of this block's IGNODES */
   int igN=0, nn=0;
   short inst;
   extern INT_BVI FKO_BVTMP;
   extern BBLOCK *bbbase;

   if (!CFUSETU2D)
      CalcInsOuts(bbbase);
   if (!INDEADU2D)
      CalcAllDeadVariables();
   if (FKO_BVTMP)
   {
      liveregs = FKO_BVTMP;
      SetVecAll(liveregs, 0);
   }
   else FKO_BVTMP = liveregs = NewBitVec(TNREG);
   if (!bp->ignodes) bp->ignodes = NewBitVec(TNREG+64);
   else SetVecAll(bp->ignodes, 0);
   if (!bp->conin) bp->conin = NewBitVec(TNREG+64);
   else SetVecAll(bp->conin, 0);
   if (!bp->conout) bp->conout = NewBitVec(TNREG+64);
   else SetVecAll(bp->conout, 0);
/*
 * Mask out unneeded regs from analysis
 */
   imask = NewBitVec(TNREG);
   SetVecBit(imask, REG_SP-1, 1);
   SetVecBit(imask, PCREG-1, 1);
   for (k=ICCBEG; k < ICCEND; k++)
      SetVecBit(imask, k-1, 1);
   for (k=FCCBEG; k < FCCEND; k++)
      SetVecBit(imask, k-1, 1);
   #ifdef X86_32
      SetVecBit(imask, -Reg2Int("@st")-1, 1);
   #endif
/*
 * Create ignode for all variables live on block entry, and add it
 * blocks conin, conout, and ignodes
 */
   vals = BitVec2StaticArray(bp->ins);
   for (n=vals[0], i=1; i <= n; i++)
   {
      k = vals[i];
      if (k >= TNREG)
      {
         node = NewIGNode(bp, k-TNREG+1);
         if (nn == igN)
            myIG = NewPtrTable(&igN, myIG, chunk);
         myIG[nn++] = node;
         SetVecBit(bp->conin, node->ignum, 1);
         SetVecBit(bp->conout, node->ignum, 1);
         SetVecBit(bp->ignodes, node->ignum, 1);
      }
/*
 *    If a register is live on block entry, add it to liveregs
 */
      else if (!BitVecCheck(imask, k))
      {
         iv = Reg2Regstate(k+1);
         BitVecComb(liveregs, liveregs, iv, '|');
      }
   }
/*
 * For all created IGNODEs, set it to conflict with all other conin,
 * and init its livereg to conin's livereg
 * FIXME: ignum can exceed short. So, it can't be hold in vals for large code
 * Will get seg fault from here.
 */
   vals = BitVec2StaticArray(bp->ignodes);
   for (n=vals[0], i=1; i <= n; i++)
   {
      k = vals[i];
      node = IG[k];
      node->conflicts = BitVecCopy(node->conflicts, bp->conin);
      SetVecBit(node->conflicts, node->ignum, 0);
      node->liveregs = BitVecCopy(node->liveregs, liveregs);
   }
   for (ip=bp->ainst1; ip; ip = ip->next)
   {
      inst = GET_INST(ip->inst[0]);
      if (!ACTIVE_INST(inst)) continue;
/*
 *   If var is referenced as a use, update nread
 */
      vals = BitVec2StaticArray(ip->use);
      for (n=vals[0], i=1; i <= n; i++)
      {
         k = vals[i];
         if (k >= TNREG)
         {
            k += 1 - TNREG;
            for (j=nn-1; j >= 0; j--)
               if (myIG[j] && myIG[j]->var == k) break;
            assert(j >= 0);
            myIG[j]->nread++;
         }
      }
/*
 *    Handle deads
 */
      vals = BitVec2StaticArray(ip->deads);
      for (n=vals[0], i=1; i <= n; i++)
      {
         k = vals[i];
         if (k >= TNREG)  /* dead item is a var */
         {
/*
 *          Find ignode associated with var
 */
            k += 1 - TNREG;
            for (j=nn-1; j >= 0; j--)
               if (myIG[j] && myIG[j]->var == k) break;
            assert(j >= 0);
/*
 *          If dying range ends with a write, indicate it
 */
            if (BitVecCheck(ip->set, k+TNREG-1))
            {
              /*fprintf(stderr,
                       "Inst dead on write, block=%d, inst='%s %s %s %s'!\n",
                       bp->bnum, instmnem[ip->inst[0]], op2str(ip->inst[1]),
                       op2str(ip->inst[2]), op2str(ip->inst[3]));
               fprintf(stderr, "k=%d, STentry=%d\n", k, k+TNREG-1);
               PrintInst(fopen("err.l", "w"), bbbase); exit(-1);*/
               fko_error(__LINE__, "inst dead on write???");
               myIG[j]->nwrite++;
            }
/*
 *          If var is dead, delete it from myIG and block's conout, and
 *          indicate that this is a block and instruction where range dies
 */
            SetVecBit(bp->conout, myIG[j]->ignum, 0);
            node = myIG[j];
            myIG[j] = NULL;
            #if IFKO_DEBUG_LEVEL >= 1
               assert(!node->blkend);
            #endif
            node->blkend = AddBlockToList(node->blkend, bp);
            node->blkend->ptr = ip;
         }
/*
 *       If it is a register that's dead, delete it from liveregs
 *       HERE HERE NOTE: shouldn't we use Reg2RegState here?
 *
 *       FIXME: we can't release all alias registers when more than one alias
 *       register is live !!!
 */
         else
            BitVecComb(liveregs, liveregs, Reg2Regstate(k+1), '-');
/*            SetVecBit(liveregs, k, 0); */
      }
/* 
 *    Handle sets
 */
      vals = BitVec2StaticArray(ip->set);
      for (n=vals[0], i=1; i <= n; i++)
      {
         k = vals[i];
         if (k >= TNREG) /* variable is being set */
         {
            k += 1 - TNREG;
            for (j=nn-1; j >= 0; j--)
               if (myIG[j] && myIG[j]->var == k) break;
/*
 *          If variable is set and not already live, then:
 *             a. create new ignode for it
 *             b. set a conflict between it and all other active ignodes
 *             c. set ignode's liveregs to current liveregs
 *             d. add it to blocks ignodes
 */
            if (j < 0)
            {
               node = NewIGNode(bp, k);
               if (nn == igN)
                  myIG = NewPtrTable(&igN, myIG, chunk);
               myIG[nn++] = node;
               for (j=0; j != nn; j++)
                  if (myIG[j])
                     SetVecBit(myIG[j]->conflicts, node->ignum, 1);
               node->conflicts = BitVecCopy(node->conflicts, bp->conout);
               SetVecBit(bp->conout, node->ignum, 1);
               node->liveregs = BitVecCopy(node->liveregs, liveregs);
               node->blkbeg = AddBlockToList(node->blkbeg, bp);
               node->blkbeg->ptr = ip;
               SetVecBit(bp->ignodes, node->ignum, 1);
               node->nwrite++;
            }
/*
 *          NOTE: already live but set, there may be no read of the live-range.
 *          When the value is set before being used, node->nread is zero. RegAsg
 *          may incorrectly skip that ig node. 
 *          useless expression elimination should delete them.
 */
            else
            {
               if (myIG[j]->var 
                     && strcmp(STname[myIG[j]->var-1], "_NONLOCDEREF")
                     && !myIG[j]->nread)
               {
                  fko_error(__LINE__, "Live range of %s(%d) is set again without "
                           "being used! Need to apply useless expression "
                           "elimination first!", STname[myIG[j]->var-1], 
                           myIG[j]->var);
               }
            }
         }
/*
 *       If it's a register being set, add it to list of live regs as well
 *       as to the regstate of all currently live ranges
 */
         else if (!BitVecCheck(imask, k))
         {
            iv = Reg2Regstate(k+1);
            BitVecComb(liveregs, liveregs, iv, '|');
            for (j=0; j != nn; j++)
               if (myIG[j])
                  BitVecComb(myIG[j]->liveregs, myIG[j]->liveregs, iv, '|');
         }
      }
   }
/*
 * If this block neither begins or ends LR, it is spanned
 */
   for (i=0; i < nn; i++)
   {
      node = myIG[i];
      if (node && !node->blkbeg && !node->blkend)
         node->blkspan = AddBlockToList(node->blkspan, bp);
   }
   if (myIG) free(myIG);
   KillBitVec(imask);
}

void CombineLiveRanges(BLIST *scope, BBLOCK *pred, int pig,
                       BBLOCK *succ, int sig)
/*
 * Merge successor IG (sig) into pred ig (pig); both blocks are known to
 * be in the register scope.
 */
{
   IGNODE *pnode, *snode, *node;
   BLIST *bl;
   short *vals;
   int i, n;

   pnode = IG[pig];
   snode = IG[sig];

   assert(pnode->var == snode->var);
   pnode->blkbeg  = MergeBlockLists(pnode->blkbeg, snode->blkbeg);
   pnode->blkend  = MergeBlockLists(snode->blkend, pnode->blkend);
   pnode->blkspan = MergeBlockLists(pnode->blkspan, snode->blkspan);
   snode->blkbeg = snode->blkend = snode->blkspan = NULL;
/*
 * If succ block is not ending block, it becomes a spanned block
 */
   if ( !FindBlockInList(pnode->blkend, succ) && 
        !FindBlockInList(pnode->blkspan, succ) )
      pnode->blkspan = AddBlockToList(pnode->blkspan, succ);
/*
 * The range ending in succ block may change due to new beginning in pred
 */
#if 0
   bl = FindInList(pnode->blkend, succ);
   if (bl)
   {
      i = pnode->var + TNREG-1;
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (BitVecCheck(ip->deads, i))
         {
            bl->ptr = ip;
            break;
         }
      }
   }
#endif
   if (snode->ldhoist)
   {
      pnode->ldhoist = MergeBlockLists(pnode->ldhoist, snode->ldhoist);
      snode->ldhoist = NULL;
   }
   if (snode->stpush)
   {
      pnode->stpush = MergeBlockLists(pnode->stpush, snode->stpush);
      snode->stpush = NULL;
   }
   pnode->nread += snode->nread;
   pnode->nwrite += snode->nwrite;
   BitVecComb(pnode->myblkvec, pnode->myblkvec, snode->myblkvec, '|');
   BitVecComb(pnode->liveregs, pnode->liveregs, snode->liveregs, '|');
   BitVecComb(pnode->conflicts, pnode->conflicts, snode->conflicts, '|');
/*
 * Change snode conflicts to pnode conflicts
 */
   vals = BitVec2StaticArray(snode->conflicts);
   for (i=1, n=vals[0]; i <= n; i++)
   {
      node = IG[vals[i]];
      SetVecBit(node->conflicts, sig, 0);
      SetVecBit(node->conflicts, pig, 1);
   }
/*
 * Update block-carried IG info
 */
   for (bl=scope; bl; bl = bl->next)
   {
      if (BitVecCheck(bl->blk->ignodes, sig))
      {
         SetVecBit(bl->blk->ignodes, sig, 0);
         SetVecBit(bl->blk->ignodes, pig, 1);
         if (BitVecCheck(bl->blk->conin, sig))
         {
            SetVecBit(bl->blk->conin, sig, 0);
            SetVecBit(bl->blk->conin, pig, 1);
         }
         if (BitVecCheck(bl->blk->conout, sig))
         {
            SetVecBit(bl->blk->conout, sig, 0);
            SetVecBit(bl->blk->conout, pig, 1);
         }
      }
   }
   KillIGNode(snode);
}

void CombineBlockIG(BLIST *scope, INT_BVI scopeblks, BBLOCK *pred, BBLOCK *succ)
/*
 * Attempt to combine preds IG with succ
 */
{
   IGNODE *node;
   int i, j, n, nn;
   short *sig, *pig, svar;
   short k, kk;
   BLIST *bl;
/*
 * If both blocks are in scope, attempt to combine their live ranges
 */
   if (BitVecCheck(scopeblks, succ->bnum-1) && 
       BitVecCheck(scopeblks, pred->bnum-1) )
   {
      pig = BitVec2Array(pred->conout, 0);
      sig = BitVec2Array(succ->conin, 0);
      for (n=sig[0], i=1; i <= n; i++)
      {
         k = sig[i];
         if (!IG[k]) continue;
         svar = IG[k]->var;
         for (nn=pig[0], j=1; j <= nn; j++)
         {
            kk = pig[j];
/*
 *          HERE, HERE: put this in to protect when sig is joined to pig
 */
            if (!IG[kk] || !IG[k]) continue;
            if (k != kk && svar == IG[kk]->var)
               CombineLiveRanges(scope, pred, kk, succ, k);
         }
      }
      free(pig);
      free(sig);
   }
/*
 * If pred in scope, but successor not, and LR includes a store, must push LR
 * stores to succ. iff the variable is live on entering succ.
 * Assign the next inst of the pushed stores to node->stpush->ptr.
 */
   else if (BitVecCheck(scopeblks, pred->bnum-1))
   {
      pig = BitVec2StaticArray(pred->conout);
/*
 *    NOTE: successor must be a posttail; that means, successor can't have
 *    predeccessor other than this pred blk. 
 */
      if (succ->preds->next) /* has other preds */
         fko_error(__LINE__, "Posttail error: posttail has other preds than tail");
      
      for (n=pig[0], i=1; i <= n; i++)
      {
         node = IG[pig[i]];
/*
 *       nwrite doesn't reflect the whole scope but just the combining 
 *       blks... so, we should not count this now !!!
 *       But if a variable is not set inside the scope, we don't need to 
 *       push it outside as the variable is not changed at all!
 *       NOTE: we can consider it for stpush now but scope out after combining
 *       all the IG nodes!!!
 */
         /*if (node->nwrite && BitVecCheck(succ->ins, node->var+TNREG-1) )*/
         if (BitVecCheck(succ->ins, node->var+TNREG-1))
         {
            node->stpush = AddBlockToList(node->stpush, succ);
            if (succ->ainst1)
            {
               k = GET_INST(succ->ainst1->inst[0]);
               node->stpush->ptr = (k == LABEL) ? 
                                   succ->ainst1->next : succ->ainst1;
            }
            else node->stpush->ptr = NULL;
         }
      }
   }
/*
 * If succ in scope but pred not, pred becomes load hoist target
 * Find the prev of the hoisted loads
 */
   else /* if (BitVecCheck(scopeblks, succ->bnum-1, 1)) */
   {
      sig = BitVec2StaticArray(succ->conin);
/*
 *    Preheader: 
 *    1. head should be the only successor
 *    2. head shouldn't have any pred other than prehead out of the scope
 */
      if ( (pred->csucc && pred->csucc != succ)
            || (pred->usucc && pred->usucc != succ) )
         fko_error(__LINE__, "Prehead error: " 
                   "prehead has successor other than head");
     
      for (bl=succ->preds; bl; bl=bl->next)
      {
         if (bl->blk != pred && !BitVecCheck(scopeblks, bl->blk->bnum-1))
            fko_error(__LINE__, "Prehead error: "
                   "head can't have other predecessor out of scope than prehead");
      }

      for (n=sig[0], i=1; i <= n; i++)
      {
         node = IG[sig[i]];
         node->ldhoist = AddBlockToList(node->ldhoist, pred);
         if (pred->ainstN)
         {
            k = GET_INST(pred->ainstN->inst[0]);
/*
 *          FIXME: if the predecessor has any successor other than this blk,
 *          adding load host before branch would create problem: to load hoist,
 *          we shouldn't use live register on this blk!!!!
 *          NOTE: if we always consider scope as a loop, only head of the loop
 *          has a predecessor which is pre-header. pre-header can never have a 
 *          successor other than the head blk of the loop.
 */
            node->ldhoist->ptr = IS_BRANCH(k) ? pred->ainstN->prev : 
                                                pred->ainstN;
         }
         else node->ldhoist->ptr = NULL;
      }
   }
}

int CheckIG(int N, IGNODE **igs)
{
   int i, j, n;
   short *sp;
   IGNODE *ig, *ig2;
   int nerr=0;

   for (i=0; i < N; i++)
   {
      if (igs[i])
      {
         ig = igs[i];
/*
 *       Make sure register is not assigned to conflicting node
 */
         if (ig->reg)
         {
            sp = BitVec2StaticArray(ig->conflicts);
            if (sp)
            {
               for (j=1, n=sp[0]; j <= n; j++)
               {
                  ig2 = IG[sp[j]];
                  if (ig2->reg == ig->reg)
                  {
                     fprintf(stderr, 
                        "Conflicting IG %d and %d both use register %d!!\n",
                             ig->ignum, ig2->ignum, ig->reg);
                     nerr++;
                  }
               }
            }
         }
      }
      else
         fprintf(stderr, "NULL IG %d\n", i);
   }
   assert(nerr == 0);
   return(nerr);
}

int Scope2BV(BLIST *scope)
{
   static INT_BVI iv=0;
   BLIST *bl;

   if (scope)
   {
      if (!iv)
         iv = NewBitVec(32);
      SetVecAll(iv, 0);
      for (bl = scope; bl; bl = bl->next)
         SetVecBit(iv, bl->blk->bnum-1, 1);
   }
   else if (iv)
   {
      KillBitVec(iv);
      iv = 0;
   }
   return(iv);
}

int CalcScopeIG(BLIST *scope)
/*
 * RETURNS: total number of IG
 * Majedul: parameter NULL is used to re-init the static var blkvec. 
 * So, make sure it is not used with NULL ( adding assert ) in normal case
 */
{
   static INT_BVI blkvec=0;
   BLIST *bl, *lp;
   int i, j;

   if (scope)
   {
      if (!INDEADU2D)
         CalcAllDeadVariables();
      else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
         CalcInsOuts(bbbase);
/*
 *    Set blkvec to reflect all blocks in scope, and calculate each block's IG
 */
      if (blkvec) SetVecAll(blkvec, 0);
      else blkvec = NewBitVec(32);
      assert(blkvec);
      for (bl=scope; bl; bl = bl->next)
      {
         SetVecBit(blkvec, bl->blk->bnum-1, 1);
         CalcBlockIG(bl->blk);
      }
/*    DumpIG(stderr, NIG, IG); */
/*
 *    Try to combine live ranges across basic blocks
 */
      for (bl=scope; bl; bl = bl->next)
      {
         if (bl->blk->usucc)
            CombineBlockIG(scope, blkvec, bl->blk, bl->blk->usucc);
/*
 *       Majedul: 
 *       NOTE: we should not assume any order of execution!!! 
 *       stpush depends on the nwrite... which may not be seen in a 
 *       specific order!!! FIXED.
 */
         if (bl->blk->csucc)
            CombineBlockIG(scope, blkvec, bl->blk, bl->blk->csucc);
         for (lp=bl->blk->preds; lp; lp = lp->next)
         {
            if (!BitVecCheck(blkvec, lp->blk->bnum-1) &&
                  lp->blk != bl->blk->usucc && lp->blk != bl->blk->csucc)
               CombineBlockIG(scope, blkvec, lp->blk, bl->blk);
         }
      }
/*
 *       FIXED:
 *       only !nwrite should have stpush!! 
 *       variables, those are not updated inside the scope, are not a candidate 
 *       for stpush, but we can't have updated nwrite until all blks are 
 *       explored. so, check for the updated nwrite here... It will solve all 
 *       issues related with the missing stpush for some cases!!! 
 */
#if 1
      for (i=0; i < NIG; i++)
      {

         if (IG[i] && !IG[i]->nwrite && IG[i]->stpush)
         {
            KillBlockList(IG[i]->stpush);
            IG[i]->stpush = NULL;
         }
      }
#endif
      for (j=i=0; i < NIG; i++)
         if (IG[i]) j++;
      return(j);
   }
   else
   {
      if (blkvec) KillBitVec(blkvec);
      blkvec = 0;
      return(0);
   }
}

#if 0
int IsConflictMoreThanNRegs(int N, IGNODE **igarr)
{
   int i, j, n, max=0;
   short *sp;
   IGNODE *ig, *igv, *igm=NULL;
#if 1
   extern BBLOCK *bbbase;
   PrintInst(stdout, bbbase);
#endif
   for (i=0; i < N; i++)
   {
      ig = igarr[i];
      sp = BitVec2StaticArray(ig->conflicts); 
#if 1 
      if (sp)
      {
         fprintf(stderr, "[%d]*****%s -> conflicts(%d) = ", sp[0],
               STname[ig->var-1], sp[0]);
         for (j=1, n=sp[0]; j <=n; j++)
         {
            igv = igarr[j];
            fprintf(stderr, "%s(%d), ", 
                  STname[igv->var-1]? STname[igv->var-1]:"NULL", igv->var);
         }
         fprintf(stderr, "\n\n");
      }
#endif
      if (sp && sp[0] > max && FLAG2TYPE(STflag[ig->var-1])==T_DOUBLE )
      {
         max = sp[0];
         igm = ig;
      }
   }
   fprintf(stderr, "Max number of conflicts = %d\n", max);
   if (igm)
   {
      fprintf(stderr, "   TYPE = %d, var=%s\n", FLAG2TYPE(STflag[igm->var-1]), 
               STname[igm->var-1]? STname[igm->var-1]:"NULL");
      sp = BitVec2StaticArray(igm->conflicts); 
      fprintf(stderr, "conflicts = ");
      for (i=1, n=sp[0]; i <=n; i++)
         fprintf(stderr, "%s(%d), ", 
               STname[sp[i]-1]? STname[sp[i]-1]:"NULL", sp[i]);
      fprintf(stderr, "\n");
   }
   exit(0);
}
#endif

void SortUnconstrainedIG(int N, IGNODE **igarr)
/*
 * Given an N-length contiguous array if IGNODEs, sorts them by nref using
 * a simple selection sort
 */
{
   int i, j, nref, maxref, maxi;
   IGNODE *maxptr;
/*
 * Perform simple selection sort
 */
   for (i=0; i < N-1; i++)
   {
      maxref = igarr[i]->nread + igarr[i]->nwrite;
      maxi = i;
      for (j=i+1; j < N; j++)
      {
         nref = igarr[j]->nread + igarr[j]->nwrite;
         if (nref > maxref)
         {
            maxi = j;
            maxref = nref;
         }
      }
      if (maxi != i)
      {
         maxptr = igarr[maxi];
         igarr[maxi] = igarr[i];
         igarr[i] = maxptr;
      }
   }
}
#if 0
int FindInstPos(INSTQ *inst)
{
   int i;
   BBLOCK *bp;
   INSTQ *ip;
   extern BBLOCK *bbbase;
   
   if (!inst)
      return(0);

   for (i=0, bp=bbbase; bp; bp=bp->down)
   {
      for (ip=bp->ainst1; ip; ip=ip->next, i++)
      {
         if (ip == inst)
            return(i+1);
      }
   }
   return(0);
}

INSTQ *FindIpFromIG(IGNODE *ig, int isbeg)
{
   int i, minb;
   INSTQ *ip;
   BLIST *bl, *minbl;
/*
 * find bl with min bnum
 * FIXME: bnum is set by block->down which doesn't always mean it would be a 
 * predecessor!
 */
   ip = NULL;
   minb = 0;
   if (isbeg)
      bl = ig->blkbeg;
   else
      bl = ig->blkend;
   if (bl)
   {
      minb = bl->blk->bnum;
      minbl = bl;
      while (bl)
      {
         i = bl->blk->bnum;
         if (i < minb)
         {
            minb = i;
            minbl = bl;
         }
         bl = bl->next;
      }
/*
 *    now findout the index of the ip of this minbp
 */
      ip = minbl->ptr; 
   }
   return(ip);
}

void DoSortIGsByInst(int end, int start, IGNODE **igarr)
{
   int i, j, imin;
   int iip, iipmin;
   IGNODE *igp;
/*
 * simple selection sort
 */
   for (i=start; i < end-1; i++)
   {
      iipmin = FindInstPos(FindIpFromIG(igarr[i], 0)); 
      imin = i;
      for (j=i+1; j < end; j++)
      {
         iip = FindInstPos(FindIpFromIG(igarr[j], 0)); 
         if (iip < iipmin)
         {
            imin = j;
            iipmin = iip;
         }
      }
      if (imin != i)
      {
         igp = igarr[imin];
         igarr[imin] = igarr[i];
         igarr[i] = igp;
      }
   }
}

void SortIGsByInstPosition(int N, IGNODE **igarr)
{
   int i, j, k, nref;
   IGNODE *igp;
   int iip;

   i= 0;
   while (i < N-1)
   {
      nref = igarr[i]->nread + igarr[i]->nwrite;
      k = j = i + 1;
      while (k < N && nref == (igarr[k]->nread + igarr[k]->nwrite) ) 
         k++;
      if (i != (k+1))
         DoSortIGsByInst(i, k, igarr);
      i = k;
   }
}

int FindDistDead2Alive(IGNODE *ig1, IGNODE *ig2)
{
   int dist;
   INSTQ *iplive, *ipdead;
   
   ipdead = FindIpFromIG(ig1, 0);
   iplive = FindIpFromIG(ig2, 1);
   
   if (ipdead && iplive)
      dist = FindInstPos(iplive) - FindInstPos(ipdead); 
   else
      dist = -1; /* either one is out of scope */
   
   return(dist);
}

IGNODE *FindEarliestAliveIGFromThisDead(int N, IGNODE **igarr, IGNODE *tig)
{
   int i, dist, dmin=0;
   IGNODE *igd=NULL;
   
   for (i=0; i < N; i++)
   {
      if (tig == igarr[i])
         continue;
      if (FLAG2TYPE(STflag[tig->var-1]) != FLAG2TYPE(STflag[igarr[i]->var-1]))
         continue;
      dist = FindDistDead2Alive(tig, igarr[i]); 
      if (dist > 0 && dist < dmin)
      {
         dmin = dist;
         igd = igarr[i];
      }
   }
   return(igd);
}

IGNODE *FindEarliestDeadUpFromThisAlive(int N, IGNODE **igarr, IGNODE *tig)
{
   int i, dist, dmin=0;
   IGNODE *igd=NULL;
  
   for (i=0; i < N; i++)
   {
      if (tig == igarr[i])
         continue;
      if (FLAG2TYPE(STflag[tig->var-1]) != FLAG2TYPE(STflag[igarr[i]->var-1]))
         continue;
/*
 *    since dead code is on up, dest is negative
 */
      dist = -FindDistDead2Alive(igarr[i], tig); 
      if (dist > 0 && dist < dmin)
      {
         dmin = dist;
         igd = igarr[i];
      }
   }
   return(igd);
}
#endif

void KeepVarTogether(int N, IGNODE **igarr)
/*
 * Assumption: igarr is already sorted based on nref (nread + nwrite). 
 * this function will keep the live ranges of same variable with same nref
 * together; this helps register assignment to avoid register spilling for
 * unrolled code
 */
{
   int i, j, k, nref;
   IGNODE *igp;
   short var;
   
   i = 0;
   while (i < N-1)
   {
      nref = igarr[i]->nread + igarr[i]->nwrite;
      var = igarr[i]->var;
/*
 *    skip if ignode of live range is for same var (and same nref) 
 */
      k = i + 1;
      while (k < N && igarr[k]->var == var 
             && nref == (igarr[k]->nread + igarr[k]->nwrite) )  
         k++;
      i = k - 1;
/*
 *    find node for same var with same nref and swap it to the next position 
 */
      j = i + 1;
      while(j < N)
      {
/*
 *       ignodes are sorted based on nref.check until number of ref becomes less
 */
         if ( nref > (igarr[j]-> nread + igarr[j]->nwrite) )
         {
            break;
         }
/*
 *       because of previous condition, no need to check the nref
 */
         else if (igarr[j]->var == var) // && nref == igarr[j]->nread + ig...
         {
/*
 *          swap with next node if not the same
 */
            if (j != i + 1)
            {
               igp = igarr[i+1];
               igarr[i+1] = igarr[j];
               igarr[j] = igp;
               i++;  /* var & nref are same for this position too */
            }
         }
         j++;
      }
      i++;
   }

}

void SortConstrainedIG(int N, IGNODE **igs)
/*
 * Right now, this routine just calls the unconstrained sort, but eventually
 * it should be changed to optimize assignment.  For instance, when deciding
 * between conflicting variables, if one variable's liverange (LR) spans the LR
 * of multiple conflicting vars, you must compare against the sum of the refs
 * of the spanned LRs.
 */
{
   SortUnconstrainedIG(N, igs);
}

void KillUnusedIG(int N, IGNODE **igs)
/*
 * Kills all IGNODEs in IG that do not also appear in igs
 */
{
   IGNODE *node;
   int n, i, j;
   BLIST *bl;
   short *sp;

   for (j=0; j < NIG; j++)
   {
      node = IG[j];
      if (node)
      {
         for (i=0; i < N && node != igs[i]; i++);
         if (i == N)
         {
/*
 *          Delete from block-carried IG
 */
            for (bl=node->blkbeg; bl; bl = bl->next)
            {
               SetVecBit(bl->blk->conin, node->ignum, 0);
               SetVecBit(bl->blk->conout, node->ignum, 0);
               SetVecBit(bl->blk->ignodes, node->ignum, 0);
            }
            for (bl=node->blkend; bl; bl = bl->next)
            {
               SetVecBit(bl->blk->conin, node->ignum, 0);
               SetVecBit(bl->blk->conout, node->ignum, 0);
               SetVecBit(bl->blk->ignodes, node->ignum, 0);
            }
            for (bl=node->blkspan; bl; bl = bl->next)
            {
               SetVecBit(bl->blk->conin, node->ignum, 0);
               SetVecBit(bl->blk->conout, node->ignum, 0);
               SetVecBit(bl->blk->ignodes, node->ignum, 0);
            }
/*
 *          Delete from conflicts
 */
            sp = BitVec2StaticArray(node->conflicts);
            for (n=sp[0], i=1; i <= n; i++)
               SetVecBit(IG[sp[i]]->conflicts, node->ignum, 0);
/*
 *          Free node space, and delete from array
 */
            KillIGNode(node);
            IG[j] = NULL;
         }
      }
   }
}

IGNODE **SortIG(int *N, int thresh)
/* 
 * Sorts IGNODEs by # refs.
 * RETURNS: N-length contiguous array of sorted IGNODEs
 * This routine also ignores live ranges (LR) with too few references to
 * be worth assigning.
 */
{
   int n, ncon, i, j;
   INT_BVI iv;
   IGNODE *ig;
   IGNODE **igarr;
   extern INT_BVI FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(TNREG);
   iv = FKO_BVTMP;
/*
 * Find number of IG nodes used, allocate array to store, and copy to
 * contiguous storage
 */
   for (n=i=0; i < NIG; i++)
   {
      if (IG[i] && IG[i]->nwrite + IG[i]->nread >= thresh &&
          (!STname[IG[i]->var-1] || 
            strcmp(STname[IG[i]->var-1], "_NONLOCDEREF")))
            n++;
   }
   if (n == 0)
   {
      *N = 0;
      return(NULL);
   }
   igarr = malloc(n*sizeof(IGNODE *));
   assert(igarr);
   for (j=i=0; i < NIG; i++)
      if (IG[i] && IG[i]->nwrite + IG[i]->nread >= thresh &&
          (!STname[IG[i]->var-1] || 
            strcmp(STname[IG[i]->var-1], "_NONLOCDEREF")))
         igarr[j++] = IG[i];
   KillUnusedIG(n, igarr);
/*
 * Now, sort array into two sub arrays, with the contrained nodes in the first
 * ncon elements of the array.  Constrained nodes are those nodes where the #
 * of conflicts is greater than the number of available registers.  Therefore
 * unconstrained nodes can use any register, so do reg asg on constrained nodes
 * first
 */
   for (ncon=i=0; i != n; i++)
   {
     ig = igarr[i];
     SetVecAll(iv, 0);
     SetAllTypeReg(iv, FLAG2PTYPE(STflag[ig->var-1]));
     BitVecComb(iv, iv, ig->liveregs, '-');
/*
 *   If # of regs available (j) is less than the number of conflicts, node is
 *   constrained
 */
     j = CountBitsSet(iv);
     if (j < CountBitsSet(ig->conflicts))
     {
        igarr[i] = igarr[ncon];
        igarr[ncon++] = ig;
     }
   }
   SortConstrainedIG(ncon, igarr);
/*
 * keep live ranges of same variable (which access count are same) together
 */
   KeepVarTogether(ncon, igarr);
   /*SortIGsByInstPosition(ncon, igarr);*/
   SortUnconstrainedIG(n-ncon, igarr+ncon);
   *N = n;
   return(igarr);
}

int DoIGRegAsg(int N, IGNODE **igs, int *nspill)
/*
 * Given an N-length array of sorted IGNODEs, perform register assignment
 * Right now, use simple algorithm for assignment, improve later.
 * RETURNS: # of IG assigned.
 * Majedul: nspill saves the number of spilling of the registers  
 */
{
   int i, j, n;
   int iret = 0;
   IGNODE *ig, *ig2;
   short *sp;
   INT_BVI iv, ivused;
   extern INT_BVI FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(TNREG);
   iv = FKO_BVTMP;

/*
 * init nspills with 0
 */
   for (i=0; i < NTYPES; i++)
      nspill[i] = 0;

   for (i=0; i < N; i++)
   {
      ig = igs[i];
      ig->reg = GetRegForAsg(FLAG2PTYPE(STflag[ig->var-1]), iv, ig->liveregs);
      if (ig->reg)
      {
         ivused = Reg2Regstate(ig->reg);
         ig->reg--;
         BitVecComb(ig->liveregs, ig->liveregs, ivused, '|');
         assert(ig->reg+1 != REG_SP);
/*
 *       Add assigned register to liveregs of conflicting IGNODEs
 */
         sp = BitVec2StaticArray(ig->conflicts);
         if (sp)
         {
            for (j=1, n=sp[0]; j <= n; j++)
            {
               ig2 = IG[sp[j]];
               BitVecComb(ig2->liveregs, ig2->liveregs, ivused, '|');
            }
         }
         ig->reg++;
         iret++;
      }
      else
      {
         fko_warn(__LINE__, "NO FREE REGISTER FOR LR %d of VAR %s!!!\n", 
               ig->ignum, STname[ig->var-1]);
         nspill[FLAG2PTYPE(STflag[ig->var-1])]++; /* update spilling */
      }
   }
   return(iret);
}

#if 0
short FindRegFromEarliestDead(int N, IGNODE **igs, IGNODE *ig)
{
   short reg = 0;
   IGNODE *iig;

   iig = FindEarliestDeadUpFromThisAlive(N, igs, ig);
   if (ig->reg)
      reg = ig->reg;
   return(reg);
}

void UpdateConflicts(short reg, IGNODE *ig)
{
   int j, n;
   INT_BVI ivused;
   IGNODE *ig2;
   short *sp;
   
   if (!reg)
      return;

   ivused = Reg2Regstate(ig->reg);
   ig->reg--;
   BitVecComb(ig->liveregs, ig->liveregs, ivused, '|');
   assert(ig->reg+1 != REG_SP);
/*
 * Add assigned register to liveregs of conflicting IGNODEs
 */
   sp = BitVec2StaticArray(ig->conflicts);
   if (sp)
   {
      for (j=1, n=sp[0]; j <= n; j++)
      {
         ig2 = IG[sp[j]];
         BitVecComb(ig2->liveregs, ig2->liveregs, ivused, '|');
      }
   }
   ig->reg++;
}
#endif

int VarUse2RegUse(IGNODE *ig, BBLOCK *blk, INSTQ *instbeg, INSTQ *instend)
/*
 * Changes all uses of ig->var to ig->reg in block blk, between the indicated
 * instructions (inclusive).  If either is NULL, we take instbeg/end.
 */
{
   INSTQ *ip;
   int CHANGE=0;
   short k;

   assert(INUSETU2D);
   assert(ig && blk);
   if (!instbeg)
      instbeg = blk->ainst1;
   if (!instend)
      instend = blk->ainstN;
   if (!instbeg || !ig->reg)
      return(0);
   assert(instend);
   do
   {
      ip = instbeg;
/*
 *    Change all uses of this ig->var to ig->reg
 */
/*
 *    Majedul: 
 *    A invalid read is reported here by valgrind!
 *    FIXED: every bitvec is allocated with size of 32 first. ig->var+TNREG-1
 *    may exceed the size. I fixed this issue by extending the bitvec when 
 *    size smaller than the index it is checked. May be it would be used later.
 */
      if (ip && ACTIVE_INST(ip->inst[0]) && 
          BitVecCheck(ip->use, ig->var+TNREG-1)) 
      {
         SetVecBit(ip->use, ig->var+TNREG-1, 0);
         SetVecBit(ip->use, ig->reg-1, 0);
         assert(ip->inst[2] == ig->deref || ip->inst[3] == ig->deref);
         if (ip->inst[2] == ig->var || ip->inst[2] == ig->deref)
            ip->inst[2] = -ig->reg;
         if (ip->inst[3] == ig->var || ip->inst[3] == ig->deref)
            ip->inst[3] = -ig->reg;
         switch(GET_INST(ip->inst[0]))
         {
/*
 *       FIXME: nothing about LDS! not works by mov. We will need something to 
 *       convert into 32 bit int reg
 */
         case LD:
            ip->inst[0] = MOV;
            break;
         case FLD:
            ip->inst[0] = FMOV;
            break;
         case FLDD:
            ip->inst[0] = FMOVD;
            break;
         case VFLD:
            ip->inst[0] = VFMOV;
            break;
         case VDLD:
            ip->inst[0] = VDMOV;
            break;
      #ifdef VIREGBEG
         case VLD:
            ip->inst[0] = VMOV;
            break;
      #endif
         case VDLDS:
            #ifdef X86
               k = ig->reg;
               if (k >= DREGBEG && k < DREGEND)
                  k = k - DREGBEG + VDREGBEG;
               if (k != -ip->inst[1])
                  CalcThisUseSet(InsNewInst(NULL, NULL, ip, VDZERO, 
                                    ip->inst[1], 0, 0));
            #endif
            ip->inst[0] = VDMOVS;
            ip->inst[2] = -ig->reg;
            ip->inst[3] = ip->inst[1]; /* made dest in use too */
            break;
         case VFLDS:
            #ifdef X86
               k = ig->reg;
               if (k >= FREGBEG && k < FREGEND)
                  k = k - FREGBEG + VFREGBEG;
               if (k != -ip->inst[1])
                  CalcThisUseSet(InsNewInst(NULL, NULL, ip, VFZERO, 
                                    ip->inst[1], 0, 0));
            #endif
            ip->inst[0] = VFMOVS;
            ip->inst[2] = -ig->reg;
            ip->inst[3] = ip->inst[1]; /* made dest in use too */
            break;
      #ifdef VIREGBEG
/*
 *       We should not use VSLDS directly in LIL. It will always be replaced 
 *       with VSMOVS and CVTSI. 
 */

         /*case VLDS:
            #ifdef X86
               k = ig->reg;
               if (k >= IREGBEG && k < IREGEND)
                  k = k - IREGBEG + VIREGBEG;
               if (k != -ip->inst[1])
                  CalcThisUseSet(InsNewInst(NULL, NULL, ip, VIZERO, 
                                    ip->inst[1], 0, 0));
            #endif
            ip->inst[0] = VMOVS;
            ip->inst[2] = -ig->reg;
            break;*/
         case VSLDS:
            assert(0);
            break;
         case VILDS:
            #ifdef X86
               k = ig->reg;
               if (k >= IREGBEG && k < IREGEND)
                  k = k - IREGBEG + VIREGBEG;
               if (k != -ip->inst[1])
                  CalcThisUseSet(InsNewInst(NULL, NULL, ip, VIZERO, 
                                    ip->inst[1], 0, 0));
            #endif
            ip->inst[0] = VIMOVS;
            ip->inst[2] = -ig->reg;
            ip->inst[3] = ip->inst[1];
            break;
      #endif
         default:
            fko_warn(__LINE__,"\n\ninst %d being var2reged!!\n\n", ip->inst[0]);
         }
         CalcThisUseSet(ip);
         CHANGE++;
      }
      instbeg = instbeg->next;
      if (!instbeg) break;
   }
   while(ip != instend);
   if (CHANGE) 
      INDEADU2D = CFUSETU2D = 0;
   return(CHANGE);
}

int DoRegAsgTransforms(IGNODE *ig)
/*
 * Given the fully qualified IG, perform all of the code transformations for
 * register assignment: variable uses to register ref, variable sets become
 * register moves, and hoist ld and/or push stores for blocks not in scope
 */
{
   INSTQ *ip;
   BLIST *bl;
   enum inst mov, ld, st, inst, sts, movs;
   int i, CHANGE=0;

   if (!ig->reg)  /* return if their was no available register */
      return(0);
/*
 * NOTE: In vector-to-scalar reduction, scalar value may be stored by a 
 * V2SST (vector to scalar store inst). so, we need sts for scalar value type.
 */
   i = STflag[ig->var-1];
   switch(FLAG2PTYPE(i))
   {
#ifdef X86_64
   case T_SHORT:
      mov = MOVS;
      ld  = LDS;
      st  = STS;
      sts = VSSTS;
      movs = VSMOVS;
      break;
#endif
   case T_INT:
      mov = MOV;
      ld  = LD;
      st  = ST;
      sts = VISTS;
      movs = VIMOVS;
      break;
   case T_FLOAT:
      mov = FMOV;
      ld  = FLD;
      st  = FST;
      sts = VFSTS;
      movs = VFMOVS;
      break;
   case T_DOUBLE:
      mov = FMOVD;
      ld  = FLDD;
      st  = FSTD;
      sts = VDSTS;
      movs = VDMOVS;
      break;
   case T_VDOUBLE:
      mov = VDMOV;
      ld  = VDLD;
      st  = VDST;
      break;
   case T_VFLOAT:
      mov = VFMOV;
      ld  = VFLD;
      st  = VFST;
      break;
#if 1
   case T_VINT:
      mov = VMOV;
      ld  = VLD;
      st  = VST;
      break;
#endif
   default:
      fko_error(__LINE__, "Unknown type %d (flag=%d) in %s\n", FLAG2TYPE(i),
                i, __FILE__);
   }
/*
 * Handle initial set of ig->reg required for every blkbeg
 */
   for (bl=ig->blkbeg; bl; bl = bl->next)
   {
/*
 *    If LR begins with a set, change it to a reg-reg move
 *    NOTE: we have no memory-output instructions, so assert set is ST
 */
      ip = bl->ptr;
      if (ip && BitVecCheck(ip->set, ig->var+TNREG-1))
      {
         inst = GET_INST(ip->inst[0]);
         #if IFKO_DEBUG_LEVEL >= 1
            assert(bl->ptr);
         #endif
         CHANGE++;
         if (!IS_STORE(inst))
         {
            PrintThisInst(stderr, __LINE__, ip);
            exit(-1);
         }
/*
 *       If Store is of same type as variable, we just change to reg-reg move
 */
         if (inst == st)
         {
            ip->inst[0] = mov;
            ip->inst[1] = -ig->reg;
         }
/*
 *       Majedul: If we have vector to scalar store, we can use movs
 */
#if 1         
         else if (IS_V2SST(inst) && inst == sts)
         {
            ip->inst[0] = movs;
            ip->inst[1] = -ig->reg;
         }
#endif
/*
 *       If Store is of different type than variable (eg., fpconst init),
 *       we must insert a ld inst to the LR register
 */
         else
         {
            ip = PrintComment(bl->blk, ip, NULL, "Inserted LD from %s",
                              STname[ig->var-1] ? STname[ig->var-1] : "NULL");
            #if IFKO_DEBUG_LEVEL > 1
            fprintf(stderr, "var=%s ireg=%d getting LD ip=%p," 
                            "nr=%d, nw=%d!\n\n", 
                       STname[ig->var-1], ig->reg, ip, ig->nread, ig->nwrite);
            #endif
            ip = InsNewInst(bl->blk, ip, NULL, ld, -ig->reg, ig->deref, 0);
            if (ip->next) bl->ptr = ip->next;
            else
               bl->ptr = InsNewInst(bl->blk, ip, NULL, COMMENT, 0, 0, 0);
         }
         CalcThisUseSet(ip);
      }
/*
 *    If LR does not begin with a set, we must add a load to ig->reg,
 *    and so we expect ldhoist to be set
 */
      else assert(0);
   }
/*
 * If this IG starts and ends in one block, with start before end, handle
 * seperately
 */
   if ( ig->blkbeg && !ig->blkbeg->next && ig->blkend && !ig->blkend->next &&
        ig->blkbeg->blk == ig->blkend->blk &&
        FindInstNum(ig->blkbeg->blk, ig->blkbeg->ptr) <
        FindInstNum(ig->blkend->blk, ig->blkend->ptr) )
   {
      assert(!ig->blkspan);
      CHANGE += VarUse2RegUse(ig, ig->blkbeg->blk, ig->blkbeg->ptr, 
                              ig->blkend->ptr);
   }
   else
   {
/*
 *    Change all use of var in LR's starting and ending blocks to access
 *    ig->reg instead
 */
      for (bl=ig->blkend; bl; bl = bl->next)
         CHANGE += VarUse2RegUse(ig, bl->blk, NULL, bl->ptr);
      for (bl=ig->blkbeg; bl; bl = bl->next)
      {
         ip = bl->ptr;
         if (ip)
            CHANGE += VarUse2RegUse(ig, bl->blk, ip, NULL);
      }
   }
/*
 * Hoist the ld to all extra-scope blocks that may need it
 */
   for (bl=ig->ldhoist; bl; bl = bl->next)
   {
      #if IFKO_DEBUG_LEVEL >= 1
         assert(bl->ptr);
         assert(ig->deref > 0);
      #endif
      ip = InsNewInst(bl->blk, bl->ptr, NULL, ld, -ig->reg, ig->deref, 0);
      CalcThisUseSet(ip);
      CHANGE++;
   }
/*
 * Push any needed store to extra-scope blocks that require it
 */
   for (bl=ig->stpush; bl; bl = bl->next)
   {
      #if IFKO_DEBUG_LEVEL >= 1
         assert(bl->ptr);
      #endif
      ip = InsNewInst(bl->blk, NULL, bl->ptr, st, ig->deref, -ig->reg, 0);
/*
 *    Majedul:
 *    V0_sum should be a candidate for stpush for the modified dasum where
 *    fall-thru path is used as scalar_restart path when speculative 
 *    vectorization is applied !!!
 *    imax also should be the candidate for stpush when -p 2 is 
 *    applied
 *    FIXED: see CalcScopeIG
 */
      CalcThisUseSet(ip);
      CHANGE++;
   }
/*
 * Now we change all use of var in LR span to access ig->reg instead
 */
   for (bl=ig->blkspan; bl; bl = bl->next)
      CHANGE += VarUse2RegUse(ig, bl->blk, NULL, NULL);
   if (CHANGE) 
      INDEADU2D = CFUSETU2D = 0;
   return(CHANGE);
}

int DoScopeRegAsg(BLIST *scope, int thresh, int *tnig, int *nspill)
/*
 * Performs interference graph-based register assignment on blocks listed in
 * scope.  tnig is output para giving total number of IG we found.
 * RETURNS: # of IG applied.
 * Majedul: tnig is unused in caller though I add one parameter to save the 
 * number of spilling.
 */
{
   IGNODE **igs;
   int i, N, nret;
   extern FILE *fpIG, *fpLIL, *fpST;
   extern BBLOCK *bbbase;
#if 0   
   static int nc = 0; /* just to debug ... diff cfg */
   char file[256];
#endif
/*
 * Majedul: 
 * scope can't be NULL, it shouldn't be. NULL is used to re-init the static 
 * variable used for bitvec. 
 */
   assert(scope);
   *tnig = CalcScopeIG(scope);
   igs = SortIG(&N, thresh);
   nret = DoIGRegAsg(N, igs, nspill);
   CheckIG(N, igs);
   if (fpIG)
   {
      DumpIG(fpIG, N, igs);
      fclose(fpIG);
   }
   if (fpLIL)
   {
      PrintInst(fpLIL, bbbase);
      fclose(fpLIL);
      fpLIL = NULL;
   }
   if (fpST)
   {
      PrintST(fpST);
      fclose(fpST);
      fpST = NULL;
   }
#if 1
   for (i=0; i != N; i++)
      DoRegAsgTransforms(igs[i]);
#endif
   if (igs) free(igs);
   KillIGTable();
   return(nret);
}


static void PrintBlockWithNum(FILE *fpout, BLIST *bl)
{
   if (!bl)
   {
      fprintf(fpout, "NONE\n");
      return;
   }
   fprintf(fpout, "%4d(%5d)", bl->blk->bnum, FindInstNum(bl->blk, bl->ptr));
   for (bl=bl->next; bl; bl = bl->next)
      fprintf(fpout, ", %4d(%5d)", bl->blk->bnum, 
              FindInstNum(bl->blk, bl->ptr));
   fprintf(fpout, "\n");
}

void PrintIGNode (FILE *fpout, IGNODE *ig)
{
   BLIST *bl;

   fprintf(fpout, 
    "*** IG# = %5d, VAR = %5d(%s), REG = %5d, NREAD = %6d, NWRITE = %3d\n",
              ig->ignum, ig->var, STname[ig->var-1], ig->reg, ig->nread, 
              ig->nwrite);
   fprintf(fpout, "    begblks : ");
   PrintBlockWithNum(fpout, ig->blkbeg);
   fprintf(fpout, "    spanblk : ");
   if (!ig->blkspan)
      fprintf(fpout, " NONE\n");
   else
   {
      fprintf(fpout, " %5d", ig->blkspan->blk->bnum);
      for (bl=ig->blkspan->next; bl; bl = bl->next)
         fprintf(fpout, ", %5d", bl->blk->bnum);
      fprintf(fpout, "\n");
   }
   fprintf(fpout, "    endblks : ");
   PrintBlockWithNum(fpout, ig->blkend);
   fprintf(fpout, "    ldhoist : ");
   PrintBlockWithNum(fpout, ig->ldhoist);
   fprintf(fpout, "    stpush  : ");
   PrintBlockWithNum(fpout, ig->stpush);
   fprintf(fpout, "    conflict: %s\n", PrintVecList(ig->conflicts, 0));
   fprintf(fpout, "\n\n"); 
}

void DumpIG(FILE *fpout, int N, IGNODE **igs)
/*
 * Dumps IG information to file
 */
{
   int i;
   IGNODE *ig;
   BLIST *bl;
   fprintf(fpout, "Printing information for %d IGNODES:\n\n", N);
   for (i=0; i < N; i++)
   {
      ig = igs[i];
      if (!ig) continue;
#if 0
      fprintf(fpout, 
         "*** IG# = %5d, VAR = %5d, REG = %5d, NREAD = %6d, NWRITE = %3d\n",
              ig->ignum, ig->var, ig->reg, ig->nread, ig->nwrite);
#else
      fprintf(fpout, 
         "*** IG# = %5d, VAR = %5d(%s), REG = %5d, NREAD = %6d, NWRITE = %3d\n",
              ig->ignum, ig->var, STname[ig->var-1], ig->reg, ig->nread, 
              ig->nwrite);
#endif
      fprintf(fpout, "    begblks : ");
      PrintBlockWithNum(fpout, ig->blkbeg);
      fprintf(fpout, "    spanblk : ");
      if (!ig->blkspan)
         fprintf(fpout, " NONE\n");
      else
      {
         fprintf(fpout, " %5d", ig->blkspan->blk->bnum);
         for (bl=ig->blkspan->next; bl; bl = bl->next)
            fprintf(fpout, ", %5d", bl->blk->bnum);
         fprintf(fpout, "\n");
      }
      fprintf(fpout, "    endblks : ");
      PrintBlockWithNum(fpout, ig->blkend);
      fprintf(fpout, "    ldhoist : ");
      PrintBlockWithNum(fpout, ig->ldhoist);
      fprintf(fpout, "    stpush  : ");
      PrintBlockWithNum(fpout, ig->stpush);
      fprintf(fpout, "    conflict: %s\n", PrintVecList(ig->conflicts, 0));
      fprintf(fpout, "\n\n");
   }
}

/*
 * ===========================================================================
 * For every local live entering or leaving the innermost loop, we globally
 * assign it to a register for the length of the loop.  This is done before
 * any other asignment, and is part of getting the loop to our normal form.
 * The following routines are involved in doing this initial global assignment
 * ===========================================================================
 */
int AsgGlobalLoopVars(LOOPQ *loop, short *iregs, short *fregs, short *dregs)
/*
 * Finds locals live on loop entry or exit: these locals will be assigned
 * to registers for the entire loop (i.e., global instead of live range
 * assignment) so that their load/stores may be moved outside the loop.
 * For each such local, search through available registers (0 in [i,f,d]regs
 * entry means reg is available), and assign the appropriate registers.
 * NOTE: only done on innermost loop
 * RETURNS: 0 on success, non-zero on failure.
 */
{
   int i, k, n, m;
   BLIST *bl;
   INSTQ *ip;
   short *sa, *s;
   INT_BVI iv;
   extern INT_BVI FKO_BVTMP;
   /*int j; */
/*
 * Find all variables set in list
 */
   if (!loop->sets) loop->sets = NewBitVec(TNREG+32);
   else SetVecAll(loop->sets, 0);
   for (bl=loop->blocks; bl; bl = bl->next)
   {
      for (ip=bl->blk->inst1; ip; ip = ip->next)
         if (ip->set)
            BitVecComb(loop->sets, loop->sets, ip->set, '|');
   }
/*
 * Find regs and vars live on loop entry and exit(s)
 */
   if (!loop->outs) loop->outs = NewBitVec(TNREG+32);
   else SetVecAll(loop->outs, 0);
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(TNREG+32);
   else SetVecAll(FKO_BVTMP, 0);
   for (bl=loop->posttails; bl; bl = bl->next)
      BitVecComb(FKO_BVTMP, FKO_BVTMP, bl->blk->ins, '|');
   for (bl=loop->tails; bl; bl = bl->next)
      BitVecComb(loop->outs, loop->outs, bl->blk->outs, '|');
   BitVecComb(loop->outs, loop->outs, FKO_BVTMP, '&');
   FKO_BVTMP = iv = BitVecComb(FKO_BVTMP, loop->outs, loop->header->ins, '|');
/*
 * we will preserve one register
 */
   k = STstrlookup("_NONLOCDEREF");
   SetVecBit(iv, k+TNREG-1, 0);

   sa = BitVec2Array(iv, 1);
   for (n=0, i=1; i <= sa[0]; i++) 
   {
      k = sa[i];
/*
 *    For locals (ignore registers), search appropriate type array, and
 *    assign variable to the first available register
 */
      if (k > TNREG)
      {
         k = STflag[k-1-TNREG];
         k = FLAG2PTYPE(k);

#if 0    /* previous implementation*/
         if (k == T_INT)
         {
            s = iregs;
            n = NIR;
         }
         else if (k == T_DOUBLE)
         {
            s = dregs;
            n = NDR;
         }
         else if (k == T_FLOAT)
         {
            s = fregs;
            n = NFR;
         }
         else
         {
            fko_error(__LINE__, "Unknown type %d, file=%s\n", k, __FILE__);
            return(1);
         }
         for (k=0; k != n && s[k]; k++);
         if (k != n)
            s[k] = sa[i] - TNREG;
         else
         {
            fko_error(__LINE__, "Out of regs in global asg, var=%s, file=%s\n",
                      STname[sa[i]-TNREG], __FILE__);
            return(1);
         }
#else
/*    
 *       Majedul: for float or double variable, ST index is stored in 
 *       appropriate regs array and store -1 to block the other. for 
 *       example: if a float variable is assigned, a float register is marked
 *       and corresponding double register is blocked storing -1  
 */ 
         if (k == T_INT)
         {
            s = iregs;
            n = NIR;
         }
         else if (k == T_DOUBLE)
         {
            s = dregs;
            n = NDR;
         }
         else if (k == T_FLOAT)
         {
            s = fregs;
            n = NFR;
         }
         else
         {
            fko_error(__LINE__, "Unknown type %d, file=%s\n", k, __FILE__);
            return(1);
         }
         for (m = 0; m != n && s[m]; m++);
         if (m != n)
         {
            s[m] = sa[i] - TNREG;   /* store ST index of the var */
            #ifdef X86                 /* X86: block the alias regs */
               if (k == T_FLOAT) dregs[m] = -1;
               if (k == T_DOUBLE) fregs[m] = -1;
            #endif
         }
         else
         {
            fko_warn(__LINE__, 
                      "Out of regs in global asg, id=%d, var=%s, file=%s\n",
                      sa[i]-TNREG, STname[sa[i]-1-TNREG], __FILE__);
            return(1);
         }
#endif
      }
   }
   free(sa);
   return(0);
}

void FindInitRegUsage(BLIST *bp, short *iregs, short *fregs, short *dregs)
/*
 * Finds all registers already being used in blocks
 * for each data type, return an array of shorts NREG long.  0 means that
 * register is not used, -1 means it is being used.
 */
{
   
   int i, j, n;
   INT_BVI iv;
   extern INT_BVI FKO_BVTMP;
   short *sp;

/*
 * Init all regs to unused
 */
   for (i=0; i < IREGEND-IREGBEG; i++)
      iregs[i] = 0;
   for (i=0; i < FREGEND-FREGBEG; i++)
      fregs[i] = 0;
   for (i=0; i < DREGEND-DREGBEG; i++)
      dregs[i] = 0;
/*
 * Majedul: Should check whether the blk info is updated or not!!! 
 * previous optimization (like: Enforced Load Store) might change the 
 * blk->uses, blk->defs, etc. So, need to recalculate those staffs  
 */ 
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);
/*
 * Find all vars & regs used or defed in all blocks
 */
   if (bp)
   {
      FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, bp->blk->uses);
      BitVecComb(iv, iv, bp->blk->defs, '|');
      for (bp=bp->next; bp; bp = bp->next)
      {
         BitVecComb(iv, iv, bp->blk->uses, '|');
         BitVecComb(iv, iv, bp->blk->defs, '|');
      }
   }
/*
 * Mark all registers used for unknown purposes
 */
   sp = BitVec2Array(iv, 1);
   for (n=sp[0], i=0; i <= n; i++)
   {
      j = sp[i];
      if (j < TNREG)
      {
         if (j >= IREGBEG && j < IREGEND)
            iregs[j-IREGBEG] = -1;
         else if (j >= FREGBEG && j < FREGEND)
            fregs[j-FREGBEG] = -1;
         else if (j >= DREGBEG && j < DREGEND)
            dregs[j-DREGBEG] = -1;
      }
   }
   free(sp);
}

int LoadStoreToMove(BLIST *blocks, int n, short *vars, short *regs)
/*
 * Replaces all loads and stores of vars in blocks with MOVs from the
 * registers indicated in regs
 */
{
   int i, changes=0;
   short is;
   BLIST *bl;
   INSTQ *ip;
   extern INT_BVI FKO_BVTMP;
   enum inst *movs;

/*
 * Set up n-length array showing what mov instruction to use based on data type
 */
   movs = malloc(n*sizeof(enum inst));
   assert(movs);
   for (i=0; i != n; i++)
   {
      is = -regs[i];
      if (is >= IREGBEG && is < IREGEND) movs[i] = MOV;
      else if (is >= FREGBEG && is < FREGEND) movs[i] = FMOV;
      else if (is >= DREGBEG && is < DREGEND) movs[i] = FMOVD;
/*      else if (is >= VREGBEG && is < VREGEND) movs[i] = VFMOV; */
   }
   for (bl=blocks; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         is = GET_INST(ip->inst[0]);
         switch(is)
         {
         case LD:
         case FLD:
         case FLDD:
         case VFLD:
/* 
 *       See if variable being loaded from (inst[2]) is one of our targets
 *       change LD instruction to MOV instruction
 */
            is = ip->inst[2];
            for (i=0; i != n && is != SToff[vars[i]-1].sa[2]; i++);
            if (i != n)
            {
               changes++;
               ip->inst[0] = movs[i];
               ip->inst[2] = regs[i];
/*
 *             Change var use to reg use
 */
               SetVecBit(ip->use, is-1, 0);
               BitVecComb(ip->use, ip->use, Reg2Regstate(-regs[i]), '|');
            }
            break;
         case ST:
         case FST:
         case FSTD:
         case VFST:
/* 
 *       See if variable being stored to (inst[1]) is one of our targets
 *       change ST instruction to MOV instruction
 */
            is = ip->inst[1];
            for (i=0; i != n && is != SToff[vars[i]-1].sa[2]; i++);
            if (i != n)
            {
               changes++;
               ip->inst[0] = movs[i];
               ip->inst[1] = regs[i];
/*
 *             Change var set to reg set
 */
               SetVecBit(ip->set, is-1, 0);
               BitVecComb(ip->set, ip->set, Reg2Regstate(-regs[i]), '|');
            }
            break;
         default:;
         }
      }
   }
   free(movs);
/*
 * This routine keeps the instq set/use up to date, but invalidates
 * block-level set/use (CFUSETU2D) and deads (INDEADU2D)
 */
   INDEADU2D = CFUSETU2D = 0;
   return(changes);
}

int DoLoopGlobalRegAssignment(LOOPQ *loop)
/* 
 * This routine does global register assignment for all locals referenced
 * in the loop, and live on loop entry or exit
 */
{
   int i, j, n, iret;
   short k;
   short iregs[NIR], fregs[NFR], dregs[NDR]; 
   short *vars, *regs;

   FindInitRegUsage(loop->blocks, iregs, fregs, dregs);
/*
 * let the code continue though not all variables got a register to assign
 */
   AsgGlobalLoopVars(loop, iregs, fregs, dregs); 
/*
 * Find total number of global assignments done, and allocate space to hold
 * mapping
 */
   for (n=i=0; i < NIR; i++)
      if (iregs[i] > 0) n++;
   for (i=0; i < NFR; i++)
      if (fregs[i] > 0) n++;
   for (i=0; i < NDR; i++)
      if (dregs[i] > 0) n++;
   regs = malloc(2*n*sizeof(short));
   assert(regs);
   vars = regs + n;
/*
 * Setup variable-to-register mapping, and perform register assignment
 * on body of loop
 */
   for (j=i=0; i < NIR; i++)
   {
      if (iregs[i] > 0)
      {
         vars[j] = iregs[i];
         regs[j++] = -i - IREGBEG;
      }
   }
   for (i=0; i < NFR; i++)
   {
      if (fregs[i] > 0)
      {
         vars[j] = fregs[i];
         regs[j++] = -i - FREGBEG;
      }
   }
   for (i=0; i < NDR; i++)
   {
      if (dregs[i] > 0)
      {
         vars[j] = dregs[i];
         regs[j++] = -i - DREGBEG;
      }
   }
   iret = i = LoadStoreToMove(loop->blocks, n, vars, regs);
   free(regs);
/*
 * Insert appopriate LD in preheader, and ST in post-tails
 */
   assert(loop->preheader);
   assert(loop->posttails);
   PrintComment(loop->preheader, NULL, NULL, 
                "START global asg preheader load");
   PrintComment(loop->posttails->blk, NULL, loop->posttails->blk->inst1, 
                "DONE  global asg sunk stores");
   for (i=0; i < NIR; i++)
   {
      if (iregs[i] > 0)
      {
         k = SToff[iregs[i]-1].sa[2];
         if (BitVecCheck(loop->header->ins, iregs[i]+TNREG-1))
            CalcThisUseSet(InsNewInst(loop->preheader, NULL, NULL, LD, 
                                      -i-IREGBEG, k, 0));
         if (BitVecCheck(loop->outs, iregs[i]+TNREG-1) && 
             BitVecCheck(loop->sets, iregs[i]+TNREG-1))
            InsInstInBlockList(loop->posttails, 1, ST, k, -i-IREGBEG, 0);
      }
   }
   for (i=0; i < NFR; i++)
   {
      if (fregs[i] > 0)
      {
         k = SToff[fregs[i]-1].sa[2];
         if (BitVecCheck(loop->header->ins, fregs[i]+TNREG-1))
            CalcThisUseSet(InsNewInst(loop->preheader, loop->preheader->instN,
                                      NULL, FLD, -i-FREGBEG, k, 0));
         if (BitVecCheck(loop->outs, fregs[i]+TNREG-1) &&
             BitVecCheck(loop->sets, fregs[i]+TNREG-1))
            InsInstInBlockList(loop->posttails, 1, FST, k, -i-FREGBEG, 0);
      }
   }
   for (i=0; i < NDR; i++)
   {
      if (dregs[i] > 0)
      {
         k = SToff[dregs[i]-1].sa[2];
         if (BitVecCheck(loop->header->ins, dregs[i]+TNREG-1))
            CalcThisUseSet(InsNewInst(loop->preheader, loop->preheader->instN,
                                      NULL, FLDD, -i-DREGBEG, k, 0));
         if (BitVecCheck(loop->outs, dregs[i]+TNREG-1) &&
             BitVecCheck(loop->sets, dregs[i]+TNREG-1))
            InsInstInBlockList(loop->posttails, 1, FSTD, k, -i-DREGBEG, 0);
      }
   }
   PrintComment(loop->preheader, NULL, NULL, 
                "DONE  global asg preheader load");
   PrintComment(loop->posttails->blk, NULL, loop->posttails->blk->inst1, 
                "START global asg sunk stores");
   CFUSETU2D = INUSETU2D = INDEADU2D = 0;
   return(iret);
}

static INSTQ *FindReg2RegMove(BBLOCK *bp, INSTQ *start, INSTQ *end)
/*
 * Finds first register-to-register move in bp, between [start,end]
 * if start == NULL, start at beginning of block, if end == NULL go to end
 */
{
   INSTQ *ip;
   enum inst inst;
   if (!bp) return(NULL);
   if (!start) start = bp->ainst1;
   if (!start) return(NULL);
   if (!end) end = bp->ainstN;
   do
   {
      ip = start;
      inst = GET_INST(ip->inst[0]);
      if (IS_MOVE(inst) && ip->inst[1] < 0 && ip->inst[2] < 0)
         return(ip);
      start = start->next;
   }
   while(start && ip != end);
   return(NULL);
}

int Dist2RefInBlock(BBLOCK *bp, int var, INSTQ *start, INSTQ *end)
/*
 * Finds number of instructions between start and next ref of var
 * if start == NULL, start at beginning of block, if end == NULL go to end
 * RETURNS:  # of inst from start otherwise (starting from 1), the negative
 *           # of inst in block from start if not found in block
 */
{
   INSTQ *ip;
   enum inst inst;
   int i=1;

   if (!bp) return(0);
   if (!start) start = bp->ainst1;
   if (!start) return(0);
   if (!end) end = bp->ainstN;
   do
   {
      ip = start;
      inst = GET_INST(ip->inst[0]);
      if (ACTIVE_INST(inst) && inst != LABEL && inst != NOP && inst != UNIMP)
      {
         i++;
         if (BitVecCheck(ip->use, var) || BitVecCheck(ip->set, var))
            return(i);
      }
      start = start->next;
   }
   while(start && ip != end);
   return(-i);
}

int Dist2Ref(BBLOCK *bp, int scopeblks, int var, INSTQ *start, INSTQ *end)
{
   int i, j=0, k=0;
   i = Dist2RefInBlock(bp, var, start, end);
   if (i < 0 && BitVecCheck(bp->outs, var))
   {
      if (bp->usucc && BitVecCheck(bp->usucc->ins, var) &&
          BitVecCheck(scopeblks, bp->usucc->bnum-1))
            j = Dist2Ref(bp->usucc, scopeblks, var, NULL, NULL);
      if (bp->csucc && BitVecCheck(bp->csucc->ins, var) &&
          BitVecCheck(scopeblks, bp->csucc->bnum-1))
         k = Dist2Ref(bp->csucc, scopeblks,var, NULL, NULL);
      if (k > j) j = k;
      i += j;
   }
   else if (i < 0) 
      i = -i;
   return(i);
}

int SubRegUse(short *op, short reg, short sub)
/*
 * Finds if register reg is used in instruction operand op; if it is, replaces
 * its use with a use of sub.
 * RETURNS: 1 if register was found, 0 otherwise.
 */
{
   int found=0;
   short op1 = *op;

   if (op1 == reg)
   {
      *op = sub;
      found = 1;
   }
   else if (op1 > 0 && IS_DEREF(STflag[op1-1]))
   {
      if (SToff[op1-1].sa[0] == reg)
      {
         found = 1;
         SToff[op1-1].sa[0] = sub;
      }
      if (SToff[op1-1].sa[1] == reg)
      {
         found = 1;
         SToff[op1-1].sa[1] = sub;
      }
   }
   return(found);
}

static int SuccIsCopyPropTarg(BBLOCK *ob, /* origin block */
                              BBLOCK *sb, /* successor block */
                              short dest, /* reg we are replacing */
                              short src)  /* reg we are CopyProping */
/*
 * Returns: 0 if successor sb rules out cross-block copy prop, 1 otherwise
 */
{
   int iret = 1;
   BLIST *bl;

   if (sb)
   {
/*
 *    If succ does not have dest live on entry, CopyProp is not ruled out
 */
      if (BitVecCheck(sb->ins, dest-1))
      {
/*
 *       If succ has source live on entry, CopyProp is ruled out
 */
         iret = !BitVecCheck(sb->ins, src-1);
         if (iret)
         {
/*
 *          If any pred aside from ob has dest live on exit, rule it out
 */
            for (bl=sb->preds; bl; bl = bl->next)
               if (BitVecCheck(bl->blk->outs, dest-1) && bl->blk != ob)
                  break;
            iret = bl ? 0 : 1;
         }
      }
   }
   return(iret);
}

int CopyPropTrans0(int SRCLIVE, BLIST *scope, INT_BVI scopeblks, BBLOCK *blk,
                   INSTQ *ipstart, short mov, short dest, short src)
/*
 * Performs copy prop starting from ipret, where src reg is dead
 * When src is dead, we can replace all use and set of dest with src, until
 * dest is dead or src is live again.
 */
{
   INSTQ *ip;
   int FoundIt, j, LIVEDONE=0;
   int change=0;
   BLIST *bl;
   INT_BVI ivsrc, ivdst;
   extern INT_BVI FKO_BVTMP;
   extern BBLOCK *bbbase;
   /*int flag = 0;*/

   bl = FindInList(scope, blk);
   if (bl->ptr)
      change = 1;
   ivdst = FKO_BVTMP = BitVecCopy(FKO_BVTMP, Reg2Regstate(dest));
   ivsrc = Reg2Regstate(src);

   for (ip=ipstart?ipstart->next:blk->ainst1; ip; ip = ip->next)
   {
      j = GET_INST(ip->inst[0]);
      if (!ACTIVE_INST(j))
         continue;
/*
 *    If src becomes live again, put move back, and stop copy prop, unless
 *    src becomes live due to a move from dest
 */
      if (BitVecCheckComb(ip->set, ivsrc, '&') &&
          (j != mov || ip->inst[1] != -src || ip->inst[2] != -dest))
         goto PUTMOVEBACK;
/*
 *    Stop copy prop if we see idiv on x86
 */
      #ifdef X86
         if((j == DIV || j == UDIV) && 
            (BitVecCheckComb(ip->set, ivdst, '&') ||
             BitVecCheckComb(ip->use, ivdst, '&') ||
             BitVecCheckComb(ip->use, ivsrc, '&')))
            goto PUTMOVEBACK;
      #endif
/*
 *    If we have a set of dest, change it to a use of src, and keep going with
 *    propogation if this dest updated itself
 */
      if (BitVecCheck(ip->set, dest-1))
      {
/*
 *       If the src is live, a set of dest ends the CopyProp, but not until
 *       after we have changed this inst use of dest to src
 */
         if (SRCLIVE)
         {
            #ifdef X86
               if (ip->inst[2] == -dest)
                  goto PUTMOVEBACK;
            #endif
               if (BitVecCheck(ip->use, dest-1))
                  LIVEDONE = 1;
               else
                  goto PUTMOVEBACK;
         }
         else if (ip->inst[1] == -dest)
         {
            ip->inst[1] = -src;
            BitVecComb(ip->set, ip->set, ivdst, '-');
            BitVecComb(ip->set, ip->set, ivsrc, '|');
            if (BitVecCheck(ip->deads, dest-1))
            {
               BitVecComb(ip->deads, ip->deads, ivdst, '-');
               BitVecComb(ip->deads, ip->deads, ivsrc, '|');
            }
/*
 *          NOTE: for instructions where destination is also in-use (like: FMA)
 *          we need to update the ip->use with dst too.
 */
            if (IS_DEST_INUSE_IMPLICITLY(ip->inst[0]))
            {
/*
 *             FIXED: if we have same register as source and destination too;
 *             updating ip->use would prevent the source to change it later.
 *             so, change the op2 here before updating the liverange
 */
               if (ip->inst[2] == -dest) /* op1 == op2*/ 
                  ip->inst[2] = -src;
               if (ip->inst[3] == -dest) /* op1 == op3*/ 
                  ip->inst[3] = -src;
               BitVecComb(ip->use, ip->use, ivdst, '-');
               BitVecComb(ip->use, ip->use, ivsrc, '|');   
            }
            change++;
          }
          else  /* stop copy prop on implicit set */
             goto PUTMOVEBACK;
      }
/*
 *    If we have a use of former dest, change it to a use of src
 */
      if (BitVecCheck(ip->use, dest-1))
      {
         FoundIt = 0;
         if (ip->inst[0] != RET)
         {
            FoundIt = SubRegUse(&ip->inst[2], -dest, -src);
            FoundIt |= SubRegUse(&ip->inst[3], -dest, -src);
/*
 *          If we have a store, may be implicit use in memory address
 */
            if (IS_STORE(ip->inst[0]) && 1)
            {
               assert(ip->inst[1] > 0);
               FoundIt |= SubRegUse(&ip->inst[1], -dest, -src);
            }
            if (FoundIt)
            {
               BitVecComb(ip->use, ip->use, ivdst, '-');
               BitVecComb(ip->use, ip->use, ivsrc, '|');
               change++;
               if (LIVEDONE)
               {
                  ip = ip->next;
                  INDEADU2D = 0;
                  return(change);
               }
            }
            else /* not found, is implicit use, put move back & stop */
               goto PUTMOVEBACK;
         }
         else  /* for implicit use like return, put move back & stop */
            goto PUTMOVEBACK;
      }
/*
 *    If dest is dead without updating itself, end copy prop
 */
      if (BitVecCheck(ip->deads, dest-1))
      {
         BitVecComb(ip->deads, ip->deads, ivdst, '-');
         BitVecComb(ip->deads, ip->deads, ivsrc, '|');
         INDEADU2D = 0;
         return(change);
      }
/*
 *    If src is live and we see that it dies, change SRCLIVE
 *    NOTE: a dead src by a set is handled as 1st case in loop
 */
      if (SRCLIVE && BitVecCheck(ip->deads, src-1))
         SRCLIVE = 0;
   }
/*
 * If dest was still live on exit, see if cross-block prop is possible
 * NOTE: if src is live on block entry, we can't do cross-blk CP, so
 *       always say SRCLIVE=0 initially for succ blks
 */
   if ((blk->csucc || blk->usucc) && BitVecCheck(blk->outs, dest-1))
   {
      if (SuccIsCopyPropTarg(blk, blk->usucc, dest, src) &&
          SuccIsCopyPropTarg(blk, blk->csucc, dest, src) && 1)
      {
      if (mov == MOV)
         j = 0;
         change++;
         if (blk->usucc && BitVecCheck(blk->usucc->ins, dest-1))
         {
            bl = FindInList(scope, blk->usucc);
            if (bl && !bl->ptr) /* majedul: bl can be NULL */
            {
               bl->ptr = (void*) 1;
               change += CopyPropTrans0(0, scope, scopeblks, blk->usucc,
                                        NULL, mov, dest, src);
               bl->ptr = NULL;
               BitVecComb(blk->usucc->ins, blk->usucc->ins, ivdst, '-');
               BitVecComb(blk->usucc->ins, blk->usucc->ins, ivsrc, '|');
            }
         }
         if (blk->csucc && BitVecCheck(blk->csucc->ins, dest-1))
         {
            bl = FindInList(scope, blk->csucc);
            if (bl && !bl->ptr) /* majedul: bl can be NULL*/
            {
               bl->ptr = (void*) 1;
               change += CopyPropTrans0(0, scope, scopeblks, blk->csucc,
                                        NULL, mov, dest, src);
               bl->ptr = NULL;
               BitVecComb(blk->csucc->ins, blk->csucc->ins, ivdst, '-');
               BitVecComb(blk->csucc->ins, blk->csucc->ins, ivsrc, '|');
            }
         }
         CFUSETU2D = 0;
         BitVecComb(blk->outs, blk->outs, ivdst, '-');
         BitVecComb(blk->outs, blk->outs, ivsrc, '|');
      }
      else
      {
         if (blk->ainstN && IS_BRANCH(blk->ainstN->inst[0]))
            ip = blk->ainstN;
         else 
            ip = NULL;
         goto PUTMOVEBACK;
      }
   }
   if (change) INDEADU2D = 0;
   return(change);

PUTMOVEBACK:
   if (change)
   {
      ip = InsNewInst(blk, NULL, ip, mov, -dest, -src, 0);
      CalcThisUseSet(ip);
      assert(ip->set > 0 && ip->use > 0);
      INDEADU2D = 0;
   }
   return(change);
}

INSTQ *CopyPropTrans(BLIST *scope, INT_BVI scopeblks, BBLOCK *blk, INSTQ *ipret)
/*
 * Attempts to do copy prop in block blk starting at ipret
 * RETURNS: instruction following ipret after transforms or NULL if no
 *          transform is done.
 */
{
   int change;
   short dest, src, mov;
   /*INSTQ *ip; */

   dest = -ipret->inst[1];
   src = -ipret->inst[2];
/*
 * If it's a useless move, simply delete it
 */
   if (dest == src)
   {
      /* INDEADU2D = 0; */
      CFUSETU2D = INDEADU2D = 0;
/*
 *    Majedul: reported error form Valgrind! inst deleted would be accessed
 *    later. FIXED this issue. see DoCopyProp()
 */
#if 1
      return(DelInst(ipret));
#else
      
      ip = ipret;
      fprintf(stderr,"DEL: %p ip->inst = %s\t%d %d %d\n", ip,
                          instmnem[ip->inst[0]], ip->inst[1], ip->inst[2],
                          ip->inst[3]);
      ipret = DelInst(ipret);
      if (!ipret) fprintf(stderr, "NULL!!\n");
      fflush(stderr);
      return(ipret);
#endif
   }
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);

   mov = ipret->inst[0];
/*
 * Do not do copy prop if src or dest is x87 reg
 */
   #ifdef X86_320000
      sp = Int2Reg(-src);
      if (sp[1] == 's' && sp[2] == 't')
         return(NULL);
      sp = Int2Reg(-dest);
      if (sp[1] == 's' && sp[2] == 't')
         return(NULL);
   #endif
/*
 * Do not do copy prop if src and dest are not registers of the same type
 */
   if (ireg2type(src) != ireg2type(dest))
      return(NULL);

   if (BitVecCheck(ipret->deads, src-1))
      change = CopyPropTrans0(0, scope, scopeblks, blk, ipret, mov, dest, src);
   else 
      change = CopyPropTrans0(1, scope, scopeblks, blk, ipret, mov, dest, src);
   if (change)
   {
      /*PrintThisInst(stderr, -1, ipret );*/
      ipret = DelInst(ipret);
   }
   else
      ipret = NULL;
   return(ipret);
}

int DoCopyProp(BLIST *scope)
/*
 * Performs copy propogation on scope.
 */
{
   int CHANGE=0;
   INT_BVI scopeblks;
   INSTQ *ip=NULL, *next, *ipnext;
   BLIST *bl, *epil=NULL, *lp;
   extern BBLOCK *bbbase;

   scopeblks = Scope2BV(scope);
   for (bl=scope; bl; bl = bl->next)
   {
      if (bl != epil)
      {
         do
         {
            ip = FindReg2RegMove(bl->blk, ip, NULL);
            if (ip) 
            {
/*
 *             Majedul: ip itself may be deleted in CopyPropTrans()
 *             so, why not save the ip->next before
 */
               ipnext = ip->next;
               next = CopyPropTrans(scope, scopeblks, bl->blk, ip);
               for (lp=bl; lp; lp = lp->next)
                  lp->ptr = NULL;
               
               if (next)
               {
                  ip = next;
                  CHANGE++;
               }
/*
 *             Majedul: Invalid read reported by valgrind!
 *             FIXED: CopyPropTrans() may change the ip itself but 
 *             returning NULL! What if DelInst() delete ip itself!!!! 
 *             use ipnext to keep track previous one.
 *             NOTE: if ip->next is deleted and CopyPropTrans returns NULL as
 *             the next inst of ip->next is NULL !!! 
 */
               else
                  ip = ipnext;
            }
         }
         while(ip);
      }
   }
/*   fprintf(stderr, "\nCopyProp CHANGE=%d\n", CHANGE); */
   if (CHANGE)
      CFUSETU2D = INDEADU2D = 0;
   return(CHANGE);
}

int DoRevCopyPropTrans(INSTQ *ipsrc,  /* inst where src is set */
                       INSTQ *ipdst,  /* inst where dest is set */
                       int nuse)      /* # of intervening uses of src */
/*
 * Actually perform Reverse Copy Prop transform on [ipsrc,ipdst] region of blk
 * Changes all ref of src to dest
 */
{
   INSTQ *ip;
   int i, nseen=(-1);
   short src, dest, op;

   assert(ipsrc && ipdst)
   src = -ipdst->inst[2];
   dest = -ipdst->inst[1];
   
   for (ip = ipsrc; ip && ip != ipdst && nseen < nuse; ip = ip->next)
   {
      if (!ACTIVE_INST(ip->inst[0]))
         continue;

      if (ip->use && (BitVecCheck(ip->use, src-1)||BitVecCheck(ip->set,src-1)))
      {
         for (i=1; i < 4; i++)
         {
            op = ip->inst[i];
            if (op == -src)
               ip->inst[i] = -dest;
            else if (op > 0 && IS_DEREF(STflag[op-1]))
            {
               if (SToff[op-1].sa[0] == -src)
                  SToff[op-1].sa[0] = -dest;
               if (SToff[op-1].sa[1] == -src)
                  SToff[op-1].sa[1] = -dest;
            }
         }
         CalcThisUseSet(ip);
         nseen++;
      }
   }
/*      ipdst->inst[2] = -dest;
        CalcThisUseSet(ipdst); */      
      DelInst(ipdst);
   return(1);
}

int DoReverseCopyProp(BLIST *scope)
/*
 * This crippled version does only in-block rcp
 */
{
   BLIST *bl;
   INSTQ *ip, *ipp, *ips, *ipps;
   int nchanges=0, nuse;
   enum inst inst;
   short src, dest;

   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainstN; ip; ip = ipp)
      {
         ipp = ip->prev;
         inst = ip->inst[0];
/*
 *       FIXED: dest is not defined yet!!!
 *       FIXME: need to check for the short int instruction not to use 
 *       r8 to r15 instruciton
 */
         /*if (IS_MOVE(inst) && ip->inst[1] < 0 && ip->inst[2] < 0 &&
             ireg2type(-ip->inst[1]) == ireg2type(-dest)) */
         if (IS_MOVE(inst) && ip->inst[1] < 0 && ip->inst[2] < 0 &&
             ireg2type(-ip->inst[1]) == ireg2type(-ip->inst[2]))
         {            
/*
 *          If src reg is dead on reg2reg move, possible reverseCP candidate
 *          NOTE: leave src==dest moves for CP to clean up
 */
             src = -ip->inst[2];
             dest = -ip->inst[1];

             if (BitVecCheck(ip->deads, src-1) && src != dest &&
                 ireg2type(src) == ireg2type(dest) && src != REG_SP)
             {
/*
 *              Look for inst that set src
 */
                nuse = 0;
/*
 *              FIXED: why don't we traverse upward to find out the set of src!
 *              We only check previous inst and go to the next .... !!!!
 */
                for (ips=ip->prev; ips; ips = ipps)
                {
                   ipps = ips->prev;
                   if (!ips->set)
                      continue;
/*
 *                 NOTE: We can't update operands of short integer (32 bit in 
 *                 64 bit arch) inst with register likes : R8~R15. Specially
 *                 in the prologue of the function.
 */
               #if defined (X86_64)   
                   if (IS_SHORT_INT_OP(ips->inst[0]))
                   {
                      if (dest >= (IREGBEG+NSR) && 
                            (BitVecCheck(ips->set, src-1) || 
                            BitVecCheck(ips->use, src-1)) )
                      {
                         ips = NULL;
                         break;
                      }
                   }
               #endif
/*
 *             FIXED: incase of DIV or UDIV, we have restrictions on register
 *             we can't apply reverse copy prop on them.
 */
               #ifdef X86
                  if (ips->inst[0] == DIV || ips->inst[0] == UDIV)
                  {
/*
 *                   for idiv, we can't change dest and src1 (rax, rdx) at all
 */
                  #if 1
                     if (src == -ips->inst[1] || src == -ips->inst[2] 
                           || dest == -ips->inst[1] || dest == -ips->inst[2])
                  #endif
                     {
                        ips = NULL;
                        break;
                     }
                  }
               #endif
                   if (BitVecCheck(ips->set, src-1) && 
                       !BitVecCheck(ips->use, src-1))
                      break;
                   if (BitVecCheck(ips->use, src-1))
                      nuse++;
                   if (BitVecCheck(ips->use, dest-1) ||
                       BitVecCheck(ips->set, dest-1))
                   {
                      ips = NULL;
                      break;
                   }
/*
 *                 NOTE: no need to handle this case specially, since 
 *                 BitVecCheck(ips->use, src-1) would catch that
 */
#if 0
                   if (BitVecCheck(ips->set, src-1) && 
                        IS_DEST_INUSE_IMPLICITLY(ips->inst[0]))
                   {
                      nuse++;
                   }
#endif
                }
                if (ips && ips->inst[1] == -src)
                {
/*
 *                all inst of DEST_INUSE_IMPLICITLY should be catched by 
 *                BitVecCheck(ips->use, src-1) conditionals
 */               
#if 0                  
                  if (IS_DEST_INUSE_IMPLICITLY(ips->inst[0]))
                  {
                     //   fprintf(stderr, "AFTER in use\n");
                  }
                  else
#endif   
                  assert(!IS_DEST_INUSE_IMPLICITLY(ips->inst[0]));
                  nchanges += DoRevCopyPropTrans(ips, ip, nuse);
                }
             }
         }
      }
   }
   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
   /*fprintf(stderr, "RCP: nchanges=%d\n", nchanges);*/
   return(nchanges);
}
int DoEnforceLoadStore(BLIST *scope)
/*
 * transforms all instructions that directly access memory to LD/ST followed
 * by inst operating on registers
 * NOTE: presently can't do this, because then reg asg deletes store of
 *       absval to mem, which is needed for cleanup; you wind up with
 *       illegal reg move (from VFREG to FREG)
 * FIXED: called RevealArchMemUses() before starting of any repeatable 
 * optimizations
 */
{
   BLIST *bl;
   INSTQ *ip, *ipN;
   enum inst inst;
   int j, ir;
   int nchanges=0;
   short op;
   extern INT_BVI FKO_BVTMP;
   /*extern int DTabs, DTabsd, DTnzero, DTnzerod;*/
   /*extern int DTabss, DTabsds, DTnzeros, DTnzerods;*/

   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(TNREG);
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);

   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         inst = ip->inst[0];
         if (ACTIVE_INST(inst) && !IS_LOAD(inst) && !IS_STORE(inst) &&
             inst != LABEL)
         {
            op = ip->inst[3];
            if (op < 1)
               op = 0;
            else if (!IS_DEREF(STflag[op-1]))
               op = 0;
/*
 *          Majedul: I skipped the update of the 2nd src operand
 *          from here, added a new function to reveal this kind of arch event
 */
            #if defined(X86) && 0 
               if (ip->inst[3] >= 0)
               {
               if (inst == FABS)
                  ip->inst[3] = op = SToff[DTabs-1].sa[2];
               else if (inst == VFABS)
                  ip->inst[3] = op = SToff[DTabs-1].sa[2];
               else if (inst == FABSD)
                  ip->inst[3] = op = SToff[DTabsd-1].sa[2];
               else if (inst == VDABS)
                  ip->inst[3] = op = SToff[DTabsd-1].sa[2];
               else if (inst == FNEG)
                  ip->inst[3] = op = SToff[DTnzero-1].sa[2];
               else if (inst == FNEGD)
                  ip->inst[3] = op = SToff[DTnzerod-1].sa[2];
               }
            #endif
            if (!op)
            {
               op = ip->inst[2];
/*
 *             Majedul: op can be negative if reg. There may be an invalid read
 *             I corrected this.
 */           
               if ( op > 0)
                  if (!IS_DEREF(STflag[op-1]))
                     op = 0;
            }
            if (inst == PREFR || inst == PREFW ||
                inst == PREFRS || inst == PREFWS)
               op = 0;
            if (op > 0)
            {
               assert(ip->inst[1] < 0);  /* dest op must be register */
               j = ireg2type(-ip->inst[1]);
               ir = GetRegFindLR(j, FKO_BVTMP, ip);
               if (ir)
               {
                  switch(j)
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
                  case T_VFLOAT:
                     inst = VFLD;
                     break;
                  case T_VDOUBLE:
                     inst = VDLD; 
                     break;
               #ifdef VIREGBEG
                  case T_VINT:
                     inst = VLD;
                     break;
               #endif
                  }
                  ipN = InsNewInst(NULL, NULL, ip, inst, -ir, op, 0);
                  if (op == ip->inst[3])
                     ip->inst[3] = -ir;
                  else
                  {
                     assert(ip->inst[2] == op);
                     ip->inst[2] = -ir;
                  }
                  CalcThisUseSet(ipN);
                  CalcThisUseSet(ip);
                  nchanges++;
               }
            }
         }
      }
   }
/* fprintf(stderr, "ELS, NCHANGES=%d\n\n",nchanges); */
   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
   return(nchanges);
}

INSTQ *FindNextUseInBlock(INSTQ *ip0, int var)
/*
 * RETURNS: INSTQ of next instruction that uses all vars set in iv within
 *          block starting at ip0, NULL if not found
 */
{
   INSTQ *ip;
   if (ip0)
   {
      for (ip=ip0->next; ip; ip = ip->next)
      {
         if (ip->use && BitVecCheck(ip->use, var))
            return(ip);
      }
   }
   return(NULL);
}

int DoRemoveOneUseLoads(BLIST *scope)
/*
 * Changes ld followed by one and only one use to mem-src instruction
 * (possible only on x86)
 */
{
   INSTQ *ip, *ipn, *ipuse;
   BLIST *bl;
   int nchanges=0;
   enum inst inst;
   short k;
   /*INT_BVI iv;*/
   /*extern INT_BVI FKO_BVTMP;*/

/*
 * Only x86 has these from-memory operations
 */
   #ifndef X86
      return(0);
   #endif
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);
   /*if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(TNREG);
   iv = FKO_BVTMP;*/

   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ipn)
      {
         ipn = ip->next;
         inst = ip->inst[0];
/*
 *       FIXED: we can't combine unaligned vector load
 */
         if (IS_LOAD(inst) && inst != VFLDS && inst != VDLDS 
               && inst != VFLDSB && inst != VDLDSB
               && !IS_UNALIGNED_VLOAD(inst))
         {
            k = -ip->inst[1];
            ipuse = FindNextUseInBlock(ip, k-1);
            if (ipuse)
            {
               inst = ipuse->inst[0];
               if (ipuse->inst[3] == ip->inst[1] && !IS_NOMEM(inst) &&
                   ipuse->inst[2] != -k && BitVecCheck(ipuse->deads, k-1))
               {
                  ipuse->inst[3] = ip->inst[2];
                  DelInst(ip);
                  CalcThisUseSet(ipuse);
                  nchanges++;
               }
            }
         }
      }
   }
   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
   /*fprintf(stderr, "U1: nchanges=%d\n", nchanges);*/
   return(nchanges);
}

int DoOptArchSpecInst(BLIST *scope)
/*
 * we replace SHL/ADD with LEA here, can be extended later  
 */
{
   int i, k;
   INSTQ *ip, *ip1, *ipn;
   BLIST *bl;
   BBLOCK *bp;
   enum inst inst;
   int nchanges = 0;

/*
 * only X86 supports LEA 
 */
   #ifndef X86
      return(0);
   #endif
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);
/*
 * if scope not defined, add all block list
 */
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ipn)
      {
         ipn = ip->next;
         if (ip->inst[0] == SHL && ip->next->inst[0] == ADD)
         {
            ip1 = ip->next;
            assert(ip->inst[1] == ip->inst[2]);
            k = -ip->inst[2];
            if (k == -ip1->inst[3] && BitVecCheck(ip1->deads, k-1) 
                  && IS_CONST(STflag[ip->inst[3]-1]))
            {
               i = SToff[ip->inst[3]-1].i;
               if (i ==1 || i == 2 || i == 3 )
               {
                  switch(i)
                  {
                     case 1:  // 2
                        ip->inst[0] = LEA2;
                        break;
                     case 2:  // 4
                        ip->inst[0] = LEA4;
                        break;
                     case 3:  // 8
                        ip->inst[0] = LEA8;
                        break;
                  }
                  ip->inst[1] = ip1->inst[1];
                  ip->inst[2] = ip1->inst[2];
                  ip->inst[3] = ip1->inst[3];
                  CalcThisUseSet(ip);
                  ipn = ip1->next; 
                  DelInst(ip1);
                  nchanges++;
               }
            }
         }
      }
   }
   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
   return(nchanges);
}

static enum inst type2move(int type)
{
   enum inst mov;
   switch(type)
   {
   case T_INT    :
      mov = MOV;
      break;
   case T_FLOAT  :
      mov = FMOV;
      break;
   case T_DOUBLE :
      mov = FMOVD;
      break;
   case T_VFLOAT :
      mov = VFMOV;
      break;
   case T_VDOUBLE:
      mov = VDMOV;
      break;
#ifdef VIREGBEG
   case T_VINT:
      mov = VMOV;
      break;
#endif
   default:
      fko_error(__LINE__, "Unknown type %d, file=%s\n", type, __FILE__);
   }
   return(mov);
}
int DoLastUseLoadRemoval(BLIST *scope)
/*
 * If there's a load that can be made implicit, and it's not handled by
 * RemoveOneUseLoads due to being set, moves it into memory by doing
 * a register-reg move (which should be cleaned up by CP & RCP)
 */
{
   BLIST *bl;
   INSTQ *ipld, *ip, *ipn, *ipN;
   int nchanges=0, j, k, typ;
   enum inst inst;
/*
 * Only x86 has these from-memory operations
 */
   #ifndef X86
      return(0);
   #endif
   if (!INDEADU2D)
      CalcAllDeadVariables();
   else if (!CFUSETU2D || !CFU2D || !INUSETU2D)
      CalcInsOuts(bbbase);

   for (bl=scope; bl; bl = bl->next)
   {
      for (ipld=bl->blk->ainst1; ipld; ipld = ipn)
      {
         ipn = ipld->next;
         inst = ipld->inst[0];
/*
 *       Majedul: we can't skip unaligned vector load and combine them
 *       so, we can't do it with unaligned load
 *       FIXED.
 */
         if (IS_LOAD(inst) && inst != VFLDS && inst != VDLDS
               && inst != VFLDSB && inst != VDLDSB
               && !IS_UNALIGNED_VLOAD(inst))
         {
            k = -ipld->inst[1];
            for (ip=ipld->next; ip; ip = ip->next)
            {
               if (!ip->use)
                  continue;
               if (BitVecCheck(ip->use, k-1) || BitVecCheck(ip->set, k-1))
                  break;
            }
            if (ip)
            {
               j = -ip->inst[3];
               typ =  j > 0 ? ireg2type(j) : 0;
               inst = ip->inst[0];
               if (j > 0 && j != k && ip->inst[1] == -k && ip->inst[2] == -k &&
                   BitVecCheck(ip->deads, j-1) && typ == ireg2type(k) && 
                   !IS_NOMEM(inst) && IS_REORD(inst))
               {
                  ipN = InsNewInst(NULL, NULL, ip, type2move(typ), -k, -j, 0);
                  CalcThisUseSet(ipN);
                  ip->inst[3] = ipld->inst[2];
                  CalcThisUseSet(ip);
                  DelInst(ipld);
                  nchanges++;
               }
            }
         }
      }
   }

   if (nchanges)
      CFUSETU2D = INDEADU2D = 0;
   return(nchanges);
}
/*
 * To debug: print NIG and TNIG
 */
void PrintNIG(FILE *fout)
{
   fprintf(fout,"Total IG :\n");
   fprintf(fout, "NIG = %d\n", NIG);
   fprintf(fout, "TNIG = %d\n", TNIG);
}


