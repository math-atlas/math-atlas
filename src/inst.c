#include "fko.h"

INSTQ *NewInst(BBLOCK *myblk, INSTQ *prev, INSTQ *next, enum inst ins,
               short dest, short src1, short src2)
{
   INSTQ *ip;
   ip = malloc(sizeof(INSTQ));
   assert(ip);
   ip->myblk = myblk;
   ip->next = next;
   ip->prev = prev;
   ip->inst[0] = ins;
   ip->inst[1] = dest;
   ip->inst[2] = src1;
   ip->inst[3] = src2;
   ip->use = ip->set = ip->deads = 0;
   return(ip);
}

INSTQ *KillThisInst(INSTQ *kp)
{
   INSTQ *kn;
   kn = kp->next;
   if (kp->use)
      KillBitVec(kp->use);
   if (kp->set)
      KillBitVec(kp->set);
   if (kp->deads)
      KillBitVec(kp->deads);
   free(kp);
   return(kn);
}
void KillAllInst(INSTQ *base)
{
   while (base)
      base = KillThisInst(base);
}

INSTQ *InsNewInst(BBLOCK *blk, INSTQ *prev, INSTQ *next, enum inst ins,
                  short dest, short src1, short src2)
/*
 * Insert an instruction in queue pointed at by iqhead.
 * If prev is set, add after prev inst.
 * Otherwise, if next is set, add before next in queue.
 * Otherwise (both prev and next are NULL), instruction is added to end.
 */
{
   INSTQ *ip, *p;
   extern BBLOCK *bbbase;
/*
 * Adding to end of queue (which may not yet exist)
 */
   if (!blk)
   {
      if (next && next->myblk) blk = next->myblk;
      else if (prev && prev->myblk) blk = prev->myblk;
      else blk = bbbase;
   }
   assert(blk);
   if (!blk->inst1 || !(prev || next))
   {
      if (blk->inst1)  /* add to end of already existing queue */
      {
         ip = NewInst(blk->inst1->myblk, blk->instN, NULL, ins, 
                      dest, src1, src2);
         blk->instN->next = ip;
         blk->instN = ip;
      }
      else
         ip = blk->inst1 = blk->instN = NewInst(blk, NULL, NULL, 
                                                ins, dest, src1, src2);
   }
/*
 * Adding after prev
 */
   else if (prev)
   {
      ip = prev->next = NewInst(blk->inst1->myblk, prev, prev->next, ins,
                                dest, src1, src2);
      if (ip->next) ip->next->prev = ip;
      else blk->instN = ip;
   }
   else /* if (next) */
   {
      ip = next->prev = NewInst(blk->inst1->myblk, next->prev, next,
                                ins, dest, src1, src2);
      if (ip->prev) ip->prev->next = ip;
      else blk->inst1 = ip;
   }
/*
 * Reset ainst1 and ainstN if necessary
 */
   if (ACTIVE_INST(ins))
   {
      for (p=blk->inst1; p && !ACTIVE_INST(p->inst[0]); p = p->next);
      blk->ainst1 = p;
      for (p=blk->instN; p && !ACTIVE_INST(p->inst[0]); p = p->prev);
      blk->ainstN = p;
   }
   return(ip);
}

INSTQ *InsNewInstAfterLabel(BBLOCK *blk, enum inst ins,
                            short dest, short src1, short src2)
/*
 * Inserts the instruction in the block blk as first active instruction,
 * but after any existing label
 */
{
   INSTQ *ip;
   ip = blk->ainst1;
   if (ip && ip->inst[0] == LABEL)
      ip = ip->next;
   ip = InsNewInst(blk, NULL, ip, ins, dest, src1, src2);
   return(ip);
}

void InsInstInBlockList(BLIST *blist, int FIRST, enum inst ins,
                        short dest, short src1, short src2)
/*
 * Inserts the given instruction in all blocks pointed to by blist.
 * if FIRST is non-zero, insert as first instruction in block, else
 * insert as last
 */
{
   INSTQ *next = NULL;
   for(; blist; blist = blist->next)
   {
      if (FIRST)
         CalcThisUseSet(InsNewInstAfterLabel(blist->blk, ins, dest,src1,src2));
      else
         CalcThisUseSet(InsNewInst(blist->blk, NULL, NULL, ins,dest,src1,src2));
   }
}

INSTQ *InsertInstBeforeQEntry(INSTQ *list, INSTQ *add)
/*
 * Inserts instruction add to INSTQ list before entry list
 */
{
   assert(list && add);
   add->prev = list->prev;
   add->next = list;
   list->prev->next = add;
   list->prev = add;
}

INSTQ *RemoveInstFromQ(INSTQ *del)
/*
 * Removes inst del from Q, keeping links cosher
 * RETURNS: next inst in queue
 * NOTE: instruction must be in a basic block
 */
{
   INSTQ *ip;
   if (!del) return(NULL);
   assert(del->myblk);
/*
 * If necessary, reset ainst[1,N] and inst[1,N]
 */
   if (del->myblk->ainst1 == del)
   {
      for (ip=del->next; ip && !ACTIVE_INST(ip->inst[0]); ip = ip->next);
      del->myblk->ainst1 = ip;
   }
   if (del->myblk->ainstN == del)
   {
      for (ip=del->prev; ip && !ACTIVE_INST(ip->inst[0]); ip = ip->prev);
      del->myblk->ainstN = ip;
   }
   if (del->myblk->inst1 == del)
      del->myblk->inst1 = del->next;
   if (del->myblk->instN == del)
      del->myblk->instN = del->prev;
/*
 * Remove instruction from queue and return
 */
   if (del->prev)
      del->prev->next = del->next;
   if (del->next)
      del->next->prev = del->prev;
   return(del->next);
}

INSTQ *DelInst(INSTQ *del)
/*
 * Deletes inst del from Q, keeping links cosher
 * RETURNS: next inst in queue
 * NOTE: instruction must be in a basic block
 */
{
   INSTQ *ip;
/*
 * Remove instruction from queue, kill any bitvectors, and return
 */
   ip = RemoveInstFromQ(del);
   if (del->use)
      KillBitVec(del->use);
   if (del->set)
      KillBitVec(del->set);
   if (del->deads)
      KillBitVec(del->deads);
   free(del);
   return(ip);
}

/*
 * Following routines translate LIL to human-readable form
 */
static
char *head0a="INUM INSTRUCT   DESTINATION       SOURCE 1       SOURCE 2      ",
     *head1a="==== ======== =============== =============== ===============  ",
     *head0b="      USES              SETS              DEADS\n",
     *head1b="================= ================= =================\n\n",
     *form=
     "%4d %8.8s %4d,%10.10s %4d,%10.10s %4d,%10.10s  %17.17s %17.17s %17.17s\n";

int FindInstNum(BBLOCK *blk, INSTQ *inst)
/*
 * RETURNS: instruction number, starting from 1, of the instruction ip
 */
{
   BBLOCK *bp;
   INSTQ *ip;
   int i;
   extern BBLOCK *bbbase;

   for (i=1,bp=bbbase; bp != blk; bp = bp->down)
      for (ip=bp->inst1; ip; ip = ip->next) i++;
   for (ip=blk->inst1; ip != inst; ip = ip->next) i++;
   return(i);
}
static char *BV2NumList(int iv)
{
   static char lns[3][32];
   static int k=0;
   char *sptr, *sp;
   int i, j, n, tnreg;
   short *v;

   if (!iv) return("NOT SET");
   v = BitVec2StaticArray(iv);
   if (!v) return("0");
   n = v[0];
   if (!n) return("0");

   tnreg = NumberArchRegs();
   sptr = &lns[k][0];
   if (++k == 3) k = 0;

   j = v[1]+1;
   if (j < tnreg) j = -j;
   else j -= tnreg;
   sp = sptr + sprintf(sptr, "%d", j);
   for (i=2; i <= n; i++)
   {
      j = v[i];
      if (j >= tnreg) j -= tnreg + 1;
      else j = -j;
      sp += sprintf(sp, ",%d", j);
   }
   return(sptr);
}

char *op2str(short op)
/*
 * Translates op to expanded mneumonic
 */
{
   static int i=0;
   int flag;
   static char lns[3][16];
   char *sptr;

   if (!op)
      return("");

   sptr = &lns[i][0];
   sptr[8] = '\0';
   if (++i == 3) i = 0;

   if (op < 0)
      sprintf(sptr, "R(%7.7s)", Int2Reg(op));
   else
   {
      flag = STflag[op-1];
      if (IS_CONST(flag))
      {
         #ifdef X86_64
            if (IS_INT(flag))
               sprintf(sptr, "i(%7d)", SToff[op-1].i);
            else if (IS_SHORT(flag))
               sprintf(sptr, "l(%7d)", SToff[op-1].l);
         #else
            if (IS_INT(flag) || IS_SHORT(flag))
               sprintf(sptr, "i(%7d)", SToff[op-1].i);
         #endif
         else if (IS_DOUBLE(flag))
            sprintf(sptr, "d(%7e)", SToff[op-1].d);
         else if (IS_FLOAT(flag))
            sprintf(sptr, "f(%7e)", SToff[op-1].f);
      }
      else
      {
         if (IS_PTR(flag)) *sptr = 'P';
         else if (IS_LOCAL(flag)) *sptr = 'L';
         else if (IS_DEREF(flag))
         {
            if (SToff[op-1].sa[1] > 0)
            {
               op = SToff[op-1].sa[1];
               *sptr = 'L';
            }
            else *sptr = 'D';
         }
         else *sptr = 'O';
         sprintf(sptr+1, "(%6.6s)", STname[op-1] ? STname[op-1] : "NULL");
      }
   }
   return(sptr);
}

void PrintThisInst(FILE *fpout, int i, INSTQ *ip)
{
   short inst, op1, op2, op3;

   inst = GET_INST(ip->inst[0]);
   op1 = ip->inst[1];
   op2 = ip->inst[2];
   op3 = ip->inst[3];
   if (inst == COMMENT)
   {
      if (!DO_KILLCOMMENTS(FKO_FLAG))
      {
         fprintf(fpout, "%4d %8.8s %s\n", i, "COMMENT:", 
                 op1 ? STname[op1-1] : "");
      }
   }
   else
   {
      fprintf(fpout, form, i, instmnem[inst], op1, op2str(op1),
              op2, op2str(op2), op3, op2str(op3),
              BV2NumList(ip->use), BV2NumList(ip->set),
              BV2NumList(ip->deads));
   }
}

void PrintInst(FILE *fpout, BBLOCK *bbase)
{
   BBLOCK *bp;
   INSTQ *ip;
   int i=1;

   fprintf(fpout, head0a);
   fprintf(fpout, head0b);
   fprintf(fpout, head1a);
   fprintf(fpout, head1b);
   for (bp=bbase; bp; bp = bp->down)
   {
      fprintf(fpout, "\n  ** BLOCK %d **\n", bp->bnum);
      for (ip=bp->inst1; ip; ip = ip->next)
      {
         PrintThisInst(fpout, i, ip);
         i++;
      }
   }
}

void WriteLILToBinFile(char *fname, BBLOCK *bbase)
/*
 * Write all inst to a binary file, with last entry being total number of
 * instructions in program
 */
{
   FILE *fp;
   BBLOCK *bp;
   INSTQ *ip;
   int n=0;

   fp = fopen(fname, "wb");
   assert(fp);
   for (bp = bbase; bp; bp = bp->down)
   {
      for (ip=bp->inst1; ip; ip = ip->next)
      {
         assert(fwrite(ip->inst, sizeof(short), 4, fp) == 4);
         n++;
      }
   }
   assert(fwrite(&n, sizeof(int), 1, fp) == 1);
   #if IFKO_DEBUG_LEVEL > 1
      fprintf(stderr, "WROTE OUT %d INST\n", n);
   #endif
   fclose(fp);
}

void ReadLILFromBinFile(char *fname)
{
   FILE *fp;
   int i, n;
   short inst[4];
   extern BBLOCK *bbbase;

   KillAllBasicBlocks(bbbase);
   bbbase = NewBasicBlock(NULL, NULL);

   fp = fopen(fname, "rb");
   assert(fp);
   assert(!fseek(fp, -sizeof(int), SEEK_END));
   assert(fread(&n, sizeof(int), 1, fp) == 1);
   #if IFKO_DEBUG_LEVEL > 1
      fprintf(stderr, "READING IN %d NEW INST\n", n);
   #endif
   rewind(fp);
   for (i=0; i < n; i++)
   {
      assert(fread(inst, sizeof(short), 4, fp) == 4);
      InsNewInst(bbbase, NULL, NULL, inst[0], inst[1], inst[2], inst[3]);
   }
   fclose(fp);
}
