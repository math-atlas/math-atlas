/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#include "fko.h"
#include "fko_arch.h"

short STderef=0;

void HandleUseSet(INT_BVI iv, INT_BVI iuse, int I)
{
   int i, flag;
   static int tnreg=0;


   if (I < 0)
      BitVecComb(iv, iv, Reg2Regstate(-I), '|');
   else if (I > 0)
   {
      flag = STflag[--I];
/*
 *    derefs always use their index variables
 */
      if (IS_DEREF(flag))
      {
         if (!tnreg) tnreg = NumberArchRegs();
         i = I;
         if (SToff[i].sa[0] < 0)
            BitVecComb(iuse, iuse, Reg2Regstate(-SToff[i].sa[0]), '|');
         if (SToff[i].sa[1] < 0)
            BitVecComb(iuse, iuse, Reg2Regstate(-SToff[i].sa[1]), '|');
         else if (SToff[i].sa[1])  /* local deref means use/set of local */
#if 1            
            SetVecBit(iv, SToff[I].sa[1]-1+tnreg, 1);
#else
/*
 *          SToff[].sa[1] and STpts2[] are same but not for new vector type
 *          here, SToff[].sa[1] is the parent vetcor and STpts2[] is the scalar
 *          elem... still on test
 */
         {
            SetVecBit(iv, SToff[I].sa[1]-1+tnreg, 1);
            SetVecBit(iv, STpts2[i]-1+tnreg, 1);
         }
#endif
/*
 *       Indicate non-local deref in set/use
 */
         if (SToff[I].sa[1] <= 0)
             SetVecBit(iv, tnreg+STderef-1, 1);
      }
   }
}

void CalcThisUseSet(INSTQ *ip)
/*
 * calculates use/set for given instruction
 */
{
   short inst, except;

   inst = ip->inst[0];
   except = inst >> 14;
   inst &= 0x3FFF;

   if (!ip->use) ip->use = NewBitVec(32);
   else SetVecAll(ip->use, 0);
   if (!ip->set) ip->set = NewBitVec(32);
   else SetVecAll(ip->set, 0);
   if (inst == LABEL) return;

   if (ACTIVE_INST(inst))
   {
      #ifdef X86
         if (inst == DIV || inst == UDIV)
         {
            #ifdef X86_64
               HandleUseSet(ip->set, ip->use, Reg2Int("@rax"));
               HandleUseSet(ip->set, ip->use, Reg2Int("@rdx"));
               HandleUseSet(ip->use, ip->use, Reg2Int("@rax"));
               HandleUseSet(ip->use, ip->use, Reg2Int("@rdx"));
               HandleUseSet(ip->use, ip->use, ip->inst[3]);
            #else
               HandleUseSet(ip->set, ip->use, Reg2Int("@eax"));
               HandleUseSet(ip->set, ip->use, Reg2Int("@edx"));
               HandleUseSet(ip->use, ip->use, Reg2Int("@eax"));
               HandleUseSet(ip->use, ip->use, Reg2Int("@edx"));
               HandleUseSet(ip->use, ip->use, ip->inst[3]);
            #endif
         }
         else
         {
      #endif
/*
 *    Majedul: for FMAC type instruction, dest is also use before set. 
 *    it should be reflected here. So, handled MAC specially.
 *    NOTE: There are other instruction like: FCMOV1 
 */
#if 0            
      if (IS_MAC(inst))   
         HandleUseSet(ip->use, ip->use, ip->inst[1]);
#else
      if (IS_DEST_INUSE_IMPLICITLY(inst))
         HandleUseSet(ip->use, ip->use, ip->inst[1]);
#endif
      HandleUseSet(ip->set, ip->use, ip->inst[1]);
/*
 *    A XOR op with src1 == src2 does not really use the src
 */
      if ( (inst != XOR && inst != XORS) || (ip->inst[2] != ip->inst[3]) )
      {
         HandleUseSet(ip->use, ip->use, ip->inst[2]);
         HandleUseSet(ip->use, ip->use, ip->inst[3]);
      }
      if (except == 1 || except == 3)
         HandleUseSet(ip->set, ip->use, ip->inst[2]);
      if (except == 2 || except == 3)
         HandleUseSet(ip->set, ip->use, ip->inst[3]);

      #ifdef X86
         }
         if (inst >= OR && inst <= NEG)
            HandleUseSet(ip->set, ip->use, -ICC0);
         else if (inst == VGR2VR16)
            HandleUseSet(ip->use, ip->use, ip->inst[1]);
      #else
         if (IS_IOPCC(inst))
            HandleUseSet(ip->set, ip->use, -ICC0);
      #endif
   }
   #if IFKO_DEBUG_LEVEL >= 1
      assert(ip->use > 0 && ip->set > 0);
   #endif
}

void CalcUseSet(BBLOCK *bp)
{
   INSTQ *ip;

   for (ip=bp->inst1; ip; ip = ip->next)
      CalcThisUseSet(ip);
}

void CalcUsesDefs(BBLOCK *bp)
{
   INT_BVI vstmp;
   INSTQ *ip;
   extern INT_BVI FKO_BVTMP;

   if (!INUSETU2D)
      CalcUseSet(bp);
   if (!bp->uses) bp->uses = NewBitVec(32);
   else SetVecAll(bp->uses, 0);
   if (!bp->defs) bp->defs = NewBitVec(32);
   else SetVecAll(bp->defs, 0);

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   vstmp = FKO_BVTMP;

   for (ip = bp->ainst1; ip; ip = ip->next)
   {
      if (ACTIVE_INST(ip->inst[0]))
      {
         BitVecComb(vstmp, ip->use, bp->defs, '-');
         BitVecComb(bp->uses, bp->uses, vstmp, '|');
         BitVecComb(vstmp, ip->set, bp->uses, '-');
         BitVecComb(bp->defs, bp->defs, vstmp, '|');
      }
   }
}

void CalcInsOuts(BBLOCK *base)
{
   BBLOCK *bp;
   int CHANGES;
   INT_BVI vstmp;
   extern INT_BVI FKO_BVTMP;
   extern BBLOCK *bbbase;

   if (base == bbbase && !CFU2D)
      bbbase = base = NewBasicBlocks(base);

   if (!STderef) STderef = STdef("_NONLOCDEREF", PTR_BIT|DEREF_BIT, 0);

   for (bp=base; bp; bp = bp->down)
   {
      if (!bp->ins) bp->ins = NewBitVec(32);
      else SetVecAll(bp->ins, 0);
      if (!bp->outs) bp->outs = NewBitVec(32);
      if (!CFUSETU2D)
         CalcUsesDefs(bp);
   }
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   vstmp = FKO_BVTMP;
   do
   {
      CHANGES = 0;
      for (bp=base; bp; bp = bp->down)
      {
         SetVecAll(bp->outs, 0);
         if (bp->usucc) BitVecComb(bp->outs, bp->outs, bp->usucc->ins, '|');
         if (bp->csucc) BitVecComb(bp->outs, bp->outs, bp->csucc->ins, '|');
         BitVecCopy(vstmp, bp->ins);
         BitVecComb(bp->ins, bp->outs, bp->defs, '-');
         BitVecComb(bp->ins, bp->ins, bp->uses, '|');
         CHANGES |= BitVecComp(vstmp, bp->ins);
      }
   }
   while(CHANGES);
   if (base == bbbase)
   {
      INUSETU2D = 1;
      CFUSETU2D = 1;
   }
}

void CalcBlocksDeadVariables(BBLOCK *bp)
/*
 * Majedul: No way, it can be called with NULL. So, I updated to re-init the
 * static vars if it is called by NULL.
 */
{
   static INT_BVI mask=0;
   INT_BVI seenwrite;
   short inst;
   INSTQ *ip;
   extern INT_BVI FKO_BVTMP;
   extern BBLOCK *bbbase;

   if (bp)
   {
      if (!mask)
      {
         if (!STderef) STderef = STdef("_NONLOCDEREF", PTR_BIT|DEREF_BIT, 0);
         mask = NewBitVec(32);
         SetVecBit(mask, REG_SP-1, 1);
         SetVecBit(mask, -1-Reg2Int("PC"), 1);
         SetVecBit(mask, STderef-1+NumberArchRegs(), 1);
      }
      if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
      seenwrite = FKO_BVTMP;
      if (!INUSETU2D) CalcUseSet(bp);
      if (!CFUSETU2D)
      {
         assert(FindBlockByNumber(bbbase, bp->bnum) == bp);
         CalcInsOuts(bbbase);
      }
/*
 *    Treat each referenced var known to be dead on leaving the block as having
 *    seen a range-killing write
 */
      BitVecComb(seenwrite, bp->uses, bp->defs, '|');
      BitVecComb(seenwrite, seenwrite, bp->outs, '-');
      for (ip=bp->ainstN; ip; ip = ip->prev)
      {
         if (ip->deads) SetVecAll(ip->deads, 0);
         else ip->deads = NewBitVec(32);
         inst = GET_INST(ip->inst[0]);
         if (ACTIVE_INST(inst))
         {
/*
 *          Any sets in this instruction are added to range-killing writes
 */
            BitVecComb(seenwrite, seenwrite, ip->set, '|');
/*
 *          Dead values occur when we have previously seen a range-killing 
 *          write, and we now see a last read
 */   
            BitVecComb(ip->deads, seenwrite, ip->use, '&');
            BitVecComb(ip->deads, ip->deads, mask, '-');
/*
 *          A last use negates a previous write (we are now in new range)
 */
            BitVecComb(seenwrite, seenwrite, ip->deads, '-');
         }
      }
   }
   else
   {
      if (mask) KillBitVec(mask);
      mask = 0;
   }
}

void CalcAllDeadVariables()
{
   BBLOCK *bp;
   extern BBLOCK *bbbase;
   if (!CFUSETU2D) CalcInsOuts(bbbase);
   for (bp=bbbase; bp; bp = bp->down)
      CalcBlocksDeadVariables(bp);
   INDEADU2D = 1;
}

char *BV2VarNames(INT_BVI iv)
/*
 * Translates bitvec iv to string of var names
 */
{
   short *vals;
   int n, i, tnreg, j=0;
   static char ln[4096];

   ln[0] = '\0';
   if (iv)
   {
      tnreg = NumberArchRegs();
      vals = BitVec2Array(iv, 1);
      for (n=vals[0]+1, i=1; i != n; i++)
      {
         assert(vals[i]);
         if (vals[i] < tnreg) vals[i] = -vals[i];
         else vals[i] -= tnreg;
         if (vals[i] < 0) 
            j += sprintf(ln+j, "%s,", Int2Reg(vals[i])); 
         else
            j += sprintf(ln+j, "%s,", STi2str(vals[i]));
      }
      free(vals);
      if (j) ln[j-1] = '\0';
   }
   return(ln);
}

void AddSetUseComments(BBLOCK *base)
{
   BBLOCK *bp;
   INSTQ *ip;
   short inst;

   for (bp=base; bp; bp = bp->down)
   {
      for (ip=bp->inst1; ip; ip = ip->next)
      {
         inst = ip->inst[0] & 0x3FFF;
         if (inst != COMMENT && inst != CMPFLAG)
         {
            PrintComment(bp, ip, NULL,"  uses=%s", BV2VarNames(ip->use));
            PrintComment(bp, ip, NULL,"  sets=%s", BV2VarNames(ip->set));
         }
      }
   }
}

void AddDeadComments(BBLOCK *base)
{
   BBLOCK *bp;
   INSTQ *ip;
   short inst;

   for (bp=base; bp; bp = bp->down)
   {
      for (ip=bp->inst1; ip; ip = ip->next)
      {
         inst = ip->inst[0] & 0x3FFF;
         if (inst != COMMENT && inst != CMPFLAG)
            PrintComment(bp, ip, NULL,"  deads=%s", BV2VarNames(ip->deads));
      }
   }
}

void FindThisInstUseSet(INSTQ *ip, INT_BVI use, INT_BVI set)
{
   short inst, except;

   inst = ip->inst[0];
   except = inst >> 14; /* ???? */
   inst &= 0x3FFF;

   if (ACTIVE_INST(inst))
   {
#ifdef X86
      if (inst == DIV || inst == UDIV)
      {
#ifdef X86_64
         HandleUseSet(set, use, Reg2Int("@rax"));
         HandleUseSet(set, use, Reg2Int("@rdx"));
         HandleUseSet(use, use, Reg2Int("@rax"));
         HandleUseSet(use, use, Reg2Int("@rdx"));
         HandleUseSet(use, use, ip->inst[3]);
#else
         HandleUseSet(set, use, Reg2Int("@eax"));
         HandleUseSet(set, use, Reg2Int("@edx"));
         HandleUseSet(use, use, Reg2Int("@eax"));
         HandleUseSet(use, use, Reg2Int("@edx"));
         HandleUseSet(use, use, ip->inst[3]);
#endif
      }
      else
      {
#endif
/*
 *    Majedul: for FMAC type instruction, dest is also use before set. 
 *    it should be reflected here. So, handled MAC specially.
 *    NOTE: There are other instruction like: FCMOV1 
 */
         if (IS_DEST_INUSE_IMPLICITLY(inst))
            HandleUseSet(use, use, ip->inst[1]);
         HandleUseSet(set, use, ip->inst[1]);
         /*
          *    A XOR op with src1 == src2 does not really use the src
          */
         if ( (inst != XOR && inst != XORS) || ip->inst[2] != ip->inst[3])
         {
            HandleUseSet(use, use, ip->inst[2]);
            HandleUseSet(use, use, ip->inst[3]);
         }
         if (except == 1 || except == 3)
            HandleUseSet(set, use, ip->inst[2]);
         if (except == 2 || except == 3)
            HandleUseSet(set, use, ip->inst[3]);

      #ifdef X86
         }
         if (inst >= OR && inst <= NEG)
            HandleUseSet(set, use, -ICC0);
         else if (inst == VGR2VR16)
            HandleUseSet(use, use, ip->inst[1]);
      #else
         if (IS_IOPCC(inst))
            HandleUseSet(set, use, -ICC0);
      #endif
         }
   #if IFKO_DEBUG_LEVEL >= 1
      assert(use > 0 && set > 0);
   #endif
}


void CheckUseSet()
/*
 * this function checks whether the use/set of instruction is correct. 
 * In many transformations (such as, copy propagation, reverse copy propagation)
 * we manually compute use/set of updated inst. This function will check whether
 * they are Okay., 
 */
{
   int check;
   BBLOCK *bp;
   INSTQ *ip; 
   short inst;
   INT_BVI use, set;
   extern BBLOCK *bbbase;
   extern INT_BVI FKO_BVTMP; 


   if (!FKO_BVTMP)
      FKO_BVTMP = NewBitVec(32);   
   SetVecAll(FKO_BVTMP,0);
   use = NewBitVec(32);
   set = NewBitVec(32);

   for (bp=bbbase; bp; bp=bp->down)
   {
      for (ip=bp->ainst1; ip; ip=ip->next)
      {
         inst = ip->inst[0];
         /*except = inst >> 14;*/
         inst &= 0x3FFF;

         SetVecAll(use, 0);
         SetVecAll(set, 0);
         if (inst == LABEL) 
            continue;

         FindThisInstUseSet(ip, use, set);
         if (ip->use)
         {
            check = BitVecCheckComb(ip->use, use, '-');
            if (check)
            {
               fprintf(stderr, "======== DEBUG INFO =======\n");
               /*PrintST(stderr);*/
               PrintThisInst(stderr, 0, ip);
               fprintf(stderr, "USE=%s\n", BV2VarNames(use));
               fprintf(stderr, "IP->USE=%s\n", BV2VarNames(ip->use));
               fprintf(stderr, "===============================\n");
               assert(!check);
            }
         }
         if (ip->set)
         {
            check = BitVecCheckComb(ip->set, set, '-');
            if (check)
            {
               fprintf(stderr, "set??");
               assert(!check);
            }
         }
#if 0
      PrintThisInst(stderr, ip);
      fprintf(stderr, "SET=%s\n", BV2VarNames(set));
      fprintf(stderr, "USE=%s\n", BV2VarNames(use));
#endif      
         
      }
   }
   KillBitVec(use);
   KillBitVec(set);
}

INT_BVI FilterOutRegs(INT_BVI iv)
{
   int i;
   extern short STderef;
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);
   /*SetVecBit(iv, TNREG-1, 0);*/ // skip STderef... which is used for mem
   return(iv);
}
