#include "ifko.h"

void CalcUseSet(BBLOCK *bp)
{
   INSTQ *ip;
   short inst, except;
   static int tnreg=0;

   if (!tnreg) tnreg = NumberArchRegs();

   for (ip=bp->inst1; ip; ip = ip->next)
   {
      inst = ip->inst[0];
      except = inst >> 14;
      inst &= 0x3FFF;

      if (!ip->use) ip->use = NewBitVec(32);
      else SetVecAll(ip->use, 0);
      if (!ip->set) ip->set = NewBitVec(32);
      else SetVecAll(ip->set, 0);

      if (inst != COMMENT && inst != CMPFLAG)
      {
         op1 = ip->inst[1];
         op2 = ip->inst[2];
         op3 = ip->inst[3];
         if (op1 < 0) op1 = -op1;
         else op1 += tnreg;
         if (op2 < 0) op2 = -op2;
         else op2 += tnreg;
         if (op3 < 0) op3 = -op3;
         else op3 += tnreg;
         if (op1) SetVecBit(ip->set, op1-1, 1);
         if (op2)
         {
            SetVecBit(ip->use, op2-1, 1);
            if (except == 1 || except == 3)
               SetVecBit(ip->set, op2-1, 1);
         }
         if (op3)
         {
            SetVecBit(ip->use, op3-1, 1);
            if (except == 2 || except == 3)
               SetVecBit(ip->set, op3-1, 1);
         }
      }
   }
}

void CalcUsesDefs(BBLOCK *bp)
{
   int vstmp;
   INSTQ *ip;
   extern int FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   vstmp = FKO_BVTMP;

   CalcUseSet(bp);
   if (!bp->uses) bp->uses = NewBitVec(32);
   else SetVecAll(bp->uses, 0);
   if (!bp->set) bp->defs = NewBitVec(32);
   else SetVecAll(bp->defs, 0);
   for (ip = bp->ainst1; ip; ip = ip->next)
   {
      BitVecComb(vstmp, ip->use, bp->defs, '-');
      BitVecComb(bp->uses, bp->uses, vstmp, '|');
      BitVecComb(vstmp, ip->set, bp->uses, '-');
      BitVecComb(bp->defs, bp->defs, vstmp, '|');
   }
}

void CalcInsOuts(BBLOCK *base)
{
   BBLOCK *bp;
   int CHANGES;
   extern int FKO_BVTMP;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   vstmp = FKO_BVTMP;

   for (bp=base; bp; bp = bp->down)
   {
      if (!bp->ins) bp->ins = NewBitVec(32);
      else SetVecAll(bp->ins, 0);
      if (!bp->outs) bp->outs = NewBitVec(32);
      CalcUsesDefs(bp);
   }
   do
   {
      CHANGES = 0;
      for (bp=base; bp; bp = bp->down)
      {
         SetVecAll(bp->outs, 0);
         if (bp->usucc) BitVecComb(bp->outs, bp->outs, usucc->ins);
         if (bp->csucc) BitVecComb(bp->outs, bp->outs, csucc->ins);
         BitVecCopy(vstmp, bp->ins);
         BitVecComb(bp->ins, bp->outs, bp->defs, '-');
         BitVecComb(bp->ins, bp->ins, bp->uses);
         CHANGES |= BitVecComp(vstmp, bp->ins);
      }
   }
   while(CHANGES);
}
