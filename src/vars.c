#include "ifko.h"

void HandleUseSet(int iv, int iuse, int I)
{
   int i, flag;
   static int tnreg=0;


   if (I < 0)
      SetVecBit(iv, -I-1, 1);
   else if (I > 0)
   {
      flag = STflag[--I];
/*
 *    derefs always use their index variables
 */
#if 0
      if (IS_LOCAL(flag))
      {
         i = SToff[I].sa[2];
         if (SToff[i].sa[0] < 0)
            SetVecBit(iuse, -SToff[i].sa[0]-1, 1);
         if (SToff[i].sa[1] < 0)
            SetVecBit(iuse, -SToff[i].sa[1]-1, 1);
         SetVecBit(iv, I+tnreg, 1);
      }
#endif
      if (IS_DEREF(flag))
      {
         if (!tnreg) tnreg = NumberArchRegs();
         i = I;
         if (SToff[i].sa[0] < 0)
            SetVecBit(iuse, -SToff[i].sa[0]-1, 1);
         if (SToff[i].sa[1] < 0)
            SetVecBit(iuse, -SToff[i].sa[1]-1, 1);
         else if (SToff[i].sa[1])  /* local deref means use/set of local */
            SetVecBit(iv, SToff[I].sa[1]-1+tnreg, 1);
      }
   }
}

void CalcUseSet(BBLOCK *bp)
{
   INSTQ *ip;
   short inst, except;
   int flag;
   short op1, op2, op3;

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
         HandleUseSet(ip->set, ip->use, ip->inst[1]);
         HandleUseSet(ip->use, ip->use, ip->inst[2]);
         HandleUseSet(ip->use, ip->use, ip->inst[3]);
         if (except == 1 || except == 3)
            HandleUseSet(ip->set, ip->use, ip->inst[2]);
         if (except == 2 || except == 3)
            HandleUseSet(ip->set, ip->use, ip->inst[3]);
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
   if (!bp->defs) bp->defs = NewBitVec(32);
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
   int CHANGES, vstmp;
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
         if (bp->usucc) BitVecComb(bp->outs, bp->outs, bp->usucc->ins, '|');
         if (bp->csucc) BitVecComb(bp->outs, bp->outs, bp->csucc->ins, '|');
         BitVecCopy(vstmp, bp->ins);
         BitVecComb(bp->ins, bp->outs, bp->defs, '-');
         BitVecComb(bp->ins, bp->ins, bp->uses, '|');
         CHANGES |= BitVecComp(vstmp, bp->ins);
      }
   }
   while(CHANGES);
}

char *BV2VarNames(int iv)
/*
 * Translates bitvec iv to string of var names
 */
{
   short *vals;
   int n, i, tnreg, j=0;
   static char ln[1024];

   ln[0] = '\0';
   if (iv)
   {
      tnreg = NumberArchRegs();
      vals = BitVec2Array(iv, 1);
      for (n=vals[0]+1, i=1; i != n; i++)
      {
         if (vals[i] <= tnreg) vals[i] = -vals[i];
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
