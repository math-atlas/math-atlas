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
      if (!ip->def) ip->def = NewBitVec(32);
      else SetVecAll(ip->def, 0);

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
         if (op1) SetVecBit(ip->def, op1-1, 1);
         if (op2)
         {
            SetVecBit(ip->use, op2-1, 1);
            if (except == 1 || except == 3)
               SetVecBit(ip->def, op2-1, 1);
         }
         if (op3)
         {
            SetVecBit(ip->use, op3-1, 1);
            if (except == 2 || except == 3)
               SetVecBit(ip->def, op3-1, 1);
         }
      }
   }
}
