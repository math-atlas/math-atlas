/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
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
      {
/*
 *       Majedul: checking for possible memory leak.
 */
         assert(!blk->instN);
         ip = blk->inst1 = blk->instN = NewInst(blk, NULL, NULL, 
                                                ins, dest, src1, src2);
      }
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
   return(add);
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

INSTQ *FindFirstLILforHIL(INSTQ *ipX)
/* Assumptions: 
 * 1. applied before repeatable inst, so, the initial LIL structure prevails
 * 2. ip is an active instq (not consists of comment or cmpflag)
 * returns the instq pointer of starting of LIL inst of a block 
 *    (converted from HIL)
 */
{
   INSTQ *ip, *ip0;
/*
 * return same ip if jmp/ret. They consist one LIL inst 
 */
   if (IS_BRANCH(ipX->inst[0]) && !IS_COND_BRANCH(ipX->inst[0])) /* JMP, RET */
      return(ipX);
   if (ipX->inst[0] == LABEL)
      return(ipX);
/*
 * last inst of a block would always be the store or branch 
 * so, check for the active inst which is successor of a store/branch/LABEL/NULL
 */
   if (IS_STORE(ipX->inst[0]) || IS_COND_BRANCH(ipX->inst[0]) 
         || IS_PREF(ipX->inst[0]))
      ip = ipX->prev;
   else 
      ip = ipX;
   ip0 = ip;
   while (ip && !IS_BRANCH(ip->inst[0]) && ip->inst[0] != LABEL 
         && !IS_STORE(ip->inst[0]) && !IS_PREF(ip->inst[0]))
   {
      if (ACTIVE_INST(ip->inst[0]))
         ip0 = ip;
      ip = ip->prev;
   }
/*
 * FIXME: for some specific compiler generated instructions, 1st inst may not be
 * the load of vars, like: 
 *    FZEROD reg0;
 *    FSTD v0, reg0;
 */
   /*assert(IS_LOAD(ip0->inst[0]));*/
   return(ip0);
}

/*
 * Following routines translate LIL to human-readable form
 */
static
char *head0a="INUM INSTRUCT   DESTINATION       SOURCE 1       SOURCE 2      ",
     *head1a="==== ======== =============== =============== ===============  ",
#if 0
     *head0b="      USES              SETS              DEADS\n",
     *head1b="================= ================= =================\n\n",
     *form=
     "%4d %8.8s %4d,%10.10s %4d,%10.10s %4d,%10.10s  %17.17s %17.17s %17.17s\n",
#endif
     *shortform=
     "%4d %8.8s %4d,%10.10s %4d,%10.10s %4d,%10.10s\n";

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
#if 0
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
/* 
 * Majedul: As right now reg alias is implemented, we just need to print
 * the original num of single reg type ... .. 
 */
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
#endif
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
               sprintf(sptr, "l(%7ld)", SToff[op-1].l);
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
#if 0
/*      fprintf(fpout, form, i, instmnem[inst], op1, op2str(op1),
              op2, op2str(op2), op3, op2str(op3),
              BV2NumList(ip->use), BV2NumList(ip->set),
              BV2NumList(ip->deads));
*/
      fprintf(fpout, form, i, instmnem[inst], op1, op2str(op1),
              op2, op2str(op2), op3, op2str(op3),
              PrintVecList(ip->use,0), PrintVecList(ip->set,0),
              PrintVecList(ip->deads,0));

#else
/*
 *    This is to print LIL without live-vars. To enable live-vars prints,
 *    enable the #if and enable header prints in PrintInst function
 */   
      fprintf(fpout, shortform, i, instmnem[inst], op1, op2str(op1),
              op2, op2str(op2), op3, op2str(op3));
#endif
   }
}

void PrintThisInstQ(FILE *fpout, INSTQ *ip)
{
   short inst, op1, op2, op3;
   int i;
   i = 1;
   for ( ; ip; ip = ip->next, i++)
   {
      inst = GET_INST(ip->inst[0]);
      op1 = ip->inst[1];
      op2 = ip->inst[2];
      op3 = ip->inst[3];
      if (inst == COMMENT)
      {
         if (!DO_KILLCOMMENTS(FKO_FLAG))
         {
            fprintf(fpout, "%4d %8.8s %s\n", i, "COMMENT:",
                    op1 ? STname[op1 - 1] : "");
         }
      }
      else
      {
#if 0
         /*      fprintf(fpout, form, i, instmnem[inst], op1, op2str(op1),
                       op2, op2str(op2), op3, op2str(op3),
                       BV2NumList(ip->use), BV2NumList(ip->set),
                       BV2NumList(ip->deads));
          */
         fprintf(fpout, form, i, instmnem[inst], op1, op2str(op1),
                 op2, op2str(op2), op3, op2str(op3),
                 PrintVecList(ip->use, 0), PrintVecList(ip->set, 0),
                 PrintVecList(ip->deads, 0));

         /*fprintf(stderr,"\n %s : %d (use)\n",instmnem[ip->inst[0]],ip->use);*/
         /*      if (ip->inst[0] == FMACD || ip->inst[0] == VDMAC)
                  fprintf(stderr, "\ninst: %s use id: %d val: %s\n",
                          instmnem[ip->inst[0]], ip->use,
                          PrintVecList(ip->use,0));
          */
#else
         /*
          *    This is to print LIL without live-vars.To enable live-vars prints
          *    enable the #if and enable header prints in PrintInst function
          */
         fprintf(fpout, shortform, i, instmnem[inst], op1, op2str(op1),
                 op2, op2str(op2), op3, op2str(op3));
#endif
      }
   }
}
void PrintThisBlockInst(FILE *fpout, BBLOCK *bp)
{
   INSTQ *ip;
   int i;

   i = 1;
#if 0
   fprintf(fpout, "%s", head0a);
   fprintf(fpout, "%s", head0b);
   fprintf(fpout, "%s", head1a);
   fprintf(fpout, "%s", head1b);
#else
   fprintf(fpout, "%s", head0a);
   fprintf(fpout, "\n");
   fprintf(fpout, "%s", head1a);
   fprintf(fpout, "\n");
#endif

   fprintf(fpout, "\n  ** BLOCK %d **\n", bp->bnum);
   for (ip=bp->inst1; ip; ip = ip->next)
   {
      PrintThisInst(fpout, i, ip);
      i++;
   }
}
void PrintInst(FILE *fpout, BBLOCK *bbase)
{
   BBLOCK *bp;
   INSTQ *ip;
   int i=1;
#if 0
   fprintf(fpout, "%s", head0a);
   fprintf(fpout, "%s", head0b);
   fprintf(fpout, "%s", head1a);
   fprintf(fpout, "%s", head1b);
#else
   fprintf(fpout, "%s", head0a);
   fprintf(fpout, "\n");
   fprintf(fpout, "%s", head1a);
   fprintf(fpout, "\n");
#endif
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

static
char *mark="\n#############################################################\n";

void PrintOptInst(FILE *fpout, int iopt, int op, BLIST *scope, 
                  int global, int changes)
/*
 * Majedul: Prints LIL instruction with Optimization information
 * like: number of optimization already done, immediate opt that has done,
 * global or local, list of blocks in scope, number of changes that has made
 * after the last opt, etc.
 */ 
{
   BLIST *iscope;

   fprintf(fpout,"%s",mark);
   fprintf(fpout,"\t\t OPTIMIZATION META DATA \n");
   fprintf(fpout,"%s",mark);
   
   fprintf(fpout,"Number of optimization done so far = %d\n",iopt);
   fprintf(fpout,"Last Optimization Done = %s\n",optmnem[op]);
   if (global)
      fprintf(fpout,"Last Optimization is a GLOBAL optimization\n");
   else
      fprintf(fpout,"Last Optimization is a LOCAL optimization\n");
   
   switch(op)
   {
   case GlobRegAsg:
      fprintf(fpout, " Block List for OptLoop: ");
      iscope = optloop->blocks;
      for ( ; iscope ; iscope = iscope->next )
         fprintf(fpout, "%d ",iscope->blk->bnum);
      fprintf(fpout,"\n");
      break;
   case RegAsg:
   case CopyProp:
   case RemoveOneUseLoads:
   case LastUseLoadRemoval:
   case ReverseCopyProp:
   case EnforceLoadStore:
      fprintf(fpout, " Block List for the scope: ");
      for (iscope = scope ; iscope ; iscope = iscope->next )
         fprintf(fpout, "%d ",iscope->blk->bnum);
      fprintf(fpout,"\n");
      break;
   case UselessLabElim:
   case UselessJmpElim:
   case BranchChain:
   case DoNothing:
      break;
   default:
      break;
   }

   fprintf(fpout,"Number of changes done in last opt  = %d\n",changes);
   fprintf(fpout,"%s",mark);
   fprintf(fpout,"\t\t LIL AFTER THE LAST OPT \n");
   fprintf(fpout,"%s",mark);
   PrintInst(fpout,bbbase);

}
