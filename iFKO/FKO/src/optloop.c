/*
 * Copyright (C) 2003, 2004, 2015 R. Clint Whaley.
 * Code contributers : R. Clint Whaley, Majedul Sujon.
 */
#include "fko.h"

/*FIXED: considered OL_NEINC as special purpose const in symbol table */
static short OL_NEINC=0;

int NonLocalDeref(short dt)
/*
 * Returns 1 if this is deref entry to access array, not local variable
 */
{
   dt--;
   if (!IS_DEREF(STflag[dt])) /* not a DT entry */
      return(0);
   else if (SToff[dt].sa[0] == -REG_SP && SToff[dt].sa[1] >= 0) /*local deref */
      return(0);
   else /* no other way, it's a DT for array access */ 
      return(1);
}

BBLOCK *DupBlock(BBLOCK *bold)
{
   BBLOCK *nb;
   INSTQ *ip;
   short i, k;

   nb = NewBasicBlock(NULL, NULL);
   nb->bnum = bold->bnum;
   nb->ilab = bold->ilab;
   for (ip=bold->inst1; ip; ip = ip->next)
   {
      InsNewInst(nb, NULL, NULL, ip->inst[0], ip->inst[1],
                 ip->inst[2], ip->inst[3]);
/*
 *    Duplicate all derefs
 */
      for(i=1; i < 4; i++)
      {
         k = ip->inst[i]-1;
         if (k >= 0 && NonLocalDeref(k+1))
            ip->inst[i] = AddDerefEntry(SToff[k].sa[0], SToff[k].sa[1],
                                        SToff[k].sa[2], SToff[k].sa[3],
                                        STpts2[k]);
      }
   }
   return(nb);
}

BLIST *DupBlockList(BLIST *scope, INT_BVI ivscope)
/*
 * This function duplicates all block info for scope.  CF in duped code
 * that links to blocks duped scope is changed to reference duped code, while
 * links to code outside the duped area are left as is.
 * RETURNS: new block list of duplicated blocks
 * NOTE: all use/set info is left NULL.  
 */
{
   BLIST *bl, *lbase=NULL, *lp;
   BBLOCK *nb, *ob;

   for (bl=scope; bl; bl = bl->next)
   {
      ob = bl->blk;
      nb = DupBlock(ob);
      lbase = AddBlockToList(lbase, nb);
   }
/* 
 * Things that are filled in on second pass:
 * nb->[up,down,usucc,csucc,preds]
 */
   for (bl=scope; bl; bl = bl->next)
   {
      ob = bl->blk;
      nb = FindBlockInListByNumber(lbase, ob->bnum);
      assert(nb);
      if (ob->up && BitVecCheck(ivscope, ob->up->bnum-1))
         nb->up = FindBlockInListByNumber(lbase, ob->up->bnum);
      else
         nb->up = ob->up;
      if (ob->down && BitVecCheck(ivscope, ob->down->bnum-1))
         nb->down = FindBlockInListByNumber(lbase, ob->down->bnum);
      else
         nb->down = ob->down;
      if (ob->usucc && BitVecCheck(ivscope, ob->usucc->bnum-1))
         nb->usucc = FindBlockInListByNumber(lbase, ob->usucc->bnum);
      else
         nb->usucc = ob->usucc;
      if (ob->csucc && BitVecCheck(ivscope, ob->csucc->bnum-1))
         nb->csucc = FindBlockInListByNumber(lbase, ob->csucc->bnum);
      else
         nb->csucc = ob->csucc;
      for (lp=bl->blk->preds; lp; lp = lp->next)
      {
         if (BitVecCheck(ivscope, lp->blk->bnum-1))
            nb->preds = AddBlockToList(nb->preds, 
                           FindBlockInListByNumber(lbase, lp->blk->bnum-1));
      }
   }
/* 
 * Things that are left alone:
 * nb->[dom,uses,defs,ins,outs,conin,conout,ignodes,loopq]
 */
   return(lbase);
}

#if 0
void IndividualizeDuplicatedBlockList(int ndup, BLIST *scope)
/*
 * This function individualizes duped code by finding all labels, and prefacing
 * them with _CD<ndup> to make them unique.  It then finds all references to
 * former label group, and changes them to the new labels
 */
{
   struct locinit *lbase=NULL, *lp;
   INSTQ *ip;
   BLIST *bl;
   char *sp;
   char ln[256];
   short k, op1, op2, op3;

/*
 * Find all labels in block, and change their names
 */
   for (bl=scope; bl; bl = bl->next)
   {
      if (bl->blk->ilab)
      {
         assert(bl->blk->ainst1->inst[0] == LABEL && 
                bl->blk->ainst1->inst[1] == bl->blk->ilab);
         sp = STname[bl->blk->ilab-1];
/*
 *       Need to increase ndup, not add whole prefix
 */
         if (!strncmp(sp, "_IFKOCD", 7)  && isdigit(sp[7]))
         {
            sp += 7;
            while(*sp && *sp != '_') sp++;
            assert(*sp == '_' && sp[1]);
            sp++;
         }
         sprintf(ln, "_IFKOCD%d_%s", ndup, sp);
         k = STlabellookup(ln);
         lbase = NewLocinit(bl->blk->ilab, k, lbase);
         bl->blk->inst1->inst[1] = bl->blk->ilab = k;
      }
   }
/*
 * Find all refs in block to old labels, and change them to new labels
 */
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip != bl->blk->ainstN; ip = ip->next)
      {
         if (ACTIVE_INST(ip->inst[0]))
         {
            op1 = ip->inst[1];
            op2 = ip->inst[2];
            op3 = ip->inst[3];
            if (op1 > 0 || op2 > 0 || op3 > 0)
            {
               for (lp=lbase; lp; lp = lp->next)
               {
                  if (op1 == lp->id)
                     ip->inst[1] = lp->con;
                  if (op2 == lp->id)
                     ip->inst[2] = lp->con;
                  if (op3 == lp->id)
                     ip->inst[3] = lp->con;
               }
            }
         }
      }
   }
   KillAllLocinit(lbase);
}
#endif

BBLOCK *GetFallPath(BBLOCK *head, INT_BVI loopblks, INT_BVI inblks, 
                    INT_BVI tails, INT_BVI fallblks)
/*
 * Starting at head, finds the fall thru path, stopping when the fall-thru
 * block is a loop tail (in tails) or succ is not fall-thru
 * (a block we've already duped (inblks) is illegal).
 * Whole path may then be duped right after last block in path.
 * RETURNS: the block to add the path after
 */
{
   BBLOCK *bp;

   SetVecAll(fallblks, 0);
   if (!head)
      return(NULL);
   if (BitVecCheck(inblks, head->bnum-1) || !BitVecCheck(loopblks,head->bnum-1))
      return(NULL);
   for (bp = head; bp; bp = bp->down)
   {
/*
 *    If we've already added something in the path, we've got an inner
 *    loop, which is presently not allowed.
 */
      assert(!BitVecCheck(inblks, bp->bnum-1));
/*
 *    Add block to fall-thru path, and blocks that have been added
 */
      SetVecBit(fallblks, bp->bnum-1, 1);
      SetVecBit(inblks, bp->bnum-1, 1);
/*
 *    Path complete if we've added tail of non-unrolled loop or if fall-thru
 *    not a succ
 */
      if (BitVecCheck(tails, bp->bnum-1) ||
          (bp->usucc != bp->down && bp->csucc != bp->down))
         break;
      
   }
   assert(bp);
   return(bp);
}

void InsUnrolledCode(int unroll, BLIST **dupblks, BBLOCK *head,
                     INT_BVI loopblks, INT_BVI inblks, INT_BVI tails, 
                     INT_BVI fallblks)
/*
 * Majedul: FIXME:
 * This function can't explore and copy all the blks recursively. Only works 
 * for those where each conditional successor much have other csucc to create
 * a path. If an usucc of a csucc has csucc, it doesn't work!!! 
 */
{
   BBLOCK *prev, *bpN, *bp0;
   int i, k, n;

/*
 * Return if head not in loop or if we've already handled it
 */
   if (!BitVecCheck(loopblks, head->bnum-1) || BitVecCheck(inblks,head->bnum-1))
      return;
/*
 * Find fall-through path to duplicate
 */
/*
 * FIXME: What would be the fallpath? here only the code fallpath is considered
 * not explored all the paths 
 */
   prev = GetFallPath(head, loopblks, inblks, tails, fallblks);
   if (prev)
   {
/*
 *    Count blocks in path
 */
      for (bp0=head,n=0; bp0 != prev->down; n++,bp0 = bp0->down);
/*
 *    Add unrolled paths in order, one-by-one, to loop
 */
      for (i=1; i < unroll; i++)
      {
/*
 *       For each original block, find it's analogue in the unrolled block,
 *       and add them in order below the final block in the path
 */
         for (k=0,bp0=head; k != n; k++,bp0 = bp0->down)
         {
            bpN = FindBlockInListByNumber(dupblks[i-1], bp0->bnum);
            bpN->up = prev;
            bpN->down = prev->down;
            prev->down = bpN;
            prev = bpN;
         }
      }
   }
/*
 * Try to build paths based on successors
 */
   if (head->usucc)
      InsUnrolledCode(unroll, dupblks, head->usucc, loopblks,
                      inblks, tails, fallblks);
   if (head->csucc)
      InsUnrolledCode(unroll, dupblks, head->csucc, loopblks,
                      inblks, tails, fallblks);
}

void InsUnrolledBlksInCode(int unroll, BLIST **dupblks, BBLOCK *head,
                     INT_BVI loopblks, INT_BVI inblks, INT_BVI tails, 
                     INT_BVI fallblks, INT_BVI visitedblks)
/*
 * insert duplicated blks in code. This is a modified version based on the 
 * previous function to support complex CFG like: the CFG after SV.
 * Assumption:
 * -----------
 * called TransformFallThruPath before to reshape the code flow
 * GetFallpath function examine only the down blks as fall-thru blks(not usucc).
 * This assumption is always true only if reshape the code with my fall-thru
 * transformation. So, need to call the transformation before calling the 
 * UnrollLoop, at-least the LIL where SV is applied before.
 */
{
   BBLOCK *prev, *bpN, *bp0;
   int i, k, n;
/*
 * nothing to do if this blk is not inside the loop 
 */

   if (!BitVecCheck(loopblks, head->bnum-1) )
   {
      return;
   }

/*
 * If it is not considered yet, get fallpath and added the duplicated code.
 * NOTE: if the blk is placed already, we don't need to findout its fall-path; 
 * its fallpath should also be placed. But we need to explore all of its 
 * successor to visit the node which is not visited yet.
 */
   if (!BitVecCheck(inblks, head->bnum-1))
   {
/*
 *    Find fall-through path to duplicate
 *    NOTE: here fall-thru path is considered to be located at down.
 *    we need to insert code between prev and prev->down. Fall-path includes
 *    the tail block also
 */
      prev = GetFallPath(head, loopblks, inblks, tails, fallblks);
      if (prev)
      {
/*
 *       Count blocks in path
 */
         for (bp0=head,n=0; bp0 != prev->down; n++,bp0 = bp0->down);
/*
 *       Add unrolled paths in order, one-by-one, to loop
 */
         for (i=1; i < unroll; i++)
         {
/*
 *       For each original block, find it's analogue in the unrolled block,
 *       and add them in order below the final block in the path
 */
            for (k=0,bp0=head; k != n; k++,bp0 = bp0->down)
            {
               bpN = FindBlockInListByNumber(dupblks[i-1], bp0->bnum);
               bpN->up = prev;
               bpN->down = prev->down;
               prev->down = bpN;
               prev = bpN;
            }
         }
      }
   }
/*
 * If it is a tail blk, returns. No need to recurse further!
 * Otherwise, it will create infinite loop as tail has a csucc to the head.
 * NOTE: this terminating condition for recursion is used after coping the 
 * blks, otherwise no blk would be copied if there is only one blk inside the 
 * loop (head=tail)
 * NOTE: After implementing the visitedblk concept like: DFS, do we really need
 * this terminating concept??? Already visited blocks is skipped, should not 
 * fall into inifite loop, will check later when consider inner loop!!!
 */
   if (BitVecCheck(tails, head->bnum-1))
      return;
/*
 * to limit the recursion, need to check whether it is already visited
 * If it is already visited, we don't need to visit its successor; otherwise
 * it would recurse all its successor. Total function call would be 2^n
 */   
   SetVecBit(visitedblks, head->bnum-1, 1);
/*
 * Try to build paths based on successors
 */
   if (head->usucc && !BitVecCheck(visitedblks, head->usucc->bnum-1))
      InsUnrolledBlksInCode(unroll, dupblks, head->usucc, loopblks,
                      inblks, tails, fallblks, visitedblks);
   if (head->csucc && !BitVecCheck(visitedblks, head->csucc->bnum-1))
      InsUnrolledBlksInCode(unroll, dupblks, head->csucc, loopblks,
                      inblks, tails, fallblks, visitedblks);
   
}

void InsertUnrolledCode(LOOPQ *lp, int unroll, BLIST **dupblks)
{
   INT_BVI tails, fallblks, inblks, visitedblks;

   tails = BlockList2BitVec(lp->tails);
   fallblks = NewBitVec(32);
   inblks = NewBitVec(32);
   SetVecAll(inblks, 0);
#if 0
   InsUnrolledCode(unroll, dupblks, lp->header, lp->blkvec, 
                   inblks, tails, fallblks);
#else   
   visitedblks = NewBitVec(32);
   SetVecAll(visitedblks, 0);
   InsUnrolledBlksInCode(unroll, dupblks, lp->header, lp->blkvec, 
                         inblks, tails, fallblks, visitedblks);
   KillBitVec(visitedblks);
#endif
   KillBitVec(fallblks);
   KillBitVec(inblks);
}

int UpdateIndexRef(BLIST *scope, short I, int ur)
/*
 * Finds all LDs from I in given scope, and adds UR to the lded reg.
 * Majedul: new updates: If a const is already added, try to update it.
 */
{
   BLIST *bl;
   INSTQ *ip;
   int changes=0, val; 
   short deref, k;
   deref = SToff[I-1].sa[2];
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainstN; ip; ip = ip->prev)
      {
         if (ip->inst[0] == LD && ip->inst[2] == deref)
         {
#if 0            
            k = ip->inst[1];
            InsNewInst(bl->blk, ip, NULL, ADD, k, k, STiconstlookup(ur));
            changes++;
#else
/*
 * works with unrolling of scalar_restart but only for PINC right now.
 * Need to figure out the logic for NINC as well.
 */
            k = ip->next->inst[3];
            if (ip->next->inst[0] == ADD && IS_CONST(STflag[k-1]))
            {
               if (ur)
               {
                  val = SToff[k-1].i + ur; /* populate the value */
                  /*ip->next->inst[3] = STiconstlookup(val);*/
                  k = STiconstlookup(val);
                  ip->next->inst[3] = k;
/*
 *                FIXME: 
 *                OL_NEINC collide with the index of val !!!!!
 *                as OL_NEINC is changed so the val!!!
 */
#if 0                  
                  fprintf(stderr, "%s val = %d, k = [%d]%d, ol=%d\n", 
                          STname[ip->next->myblk->ilab-1], 
                          val, k, SToff[k-1].i, OL_NEINC);
#endif
                  changes++;
               }
            }
            else
            {
               k = ip->inst[1];
               InsNewInst(bl->blk, ip, NULL, ADD, k, k, STiconstlookup(ur));
               changes++;
            }
#endif
         }
      }
   }
   return(changes);
}

ILIST *FindIndexRef(BLIST *scope, short I)
/*
 * Finds all LDs from I in given scope.
 * This means all refs of I in code that is straight from h2l.
 */
{
   BLIST *bl;
   INSTQ *ip;
   ILIST *ilbase=NULL;
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainstN; ip; ip = ip->prev)
      {
         if (ip->inst[0] == LD && ip->inst[2] == I)
            ilbase = NewIlist(ip, ilbase);
      }
   }
   return(ilbase);
}

ILIST *FindIndexRefInArray(BLIST *scope, short I)
/*
 * Majedul: Finds all index refs to access memory in a given scope.
 * returns list of inst where this access occurs
 */
{
   BLIST *bl;
   INSTQ *ip, *ip1;
   short ireg, dt;
   ILIST *ilbase=NULL;
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainstN; ip; ip = ip->prev)
      {
         if (ip->inst[0] == LD && ip->inst[2] == I)
         {
            ireg = ip->inst[1];
            ip1 = ip->next;
            while(IS_LOAD(ip1->inst[0]))
            {
               dt = ip1->inst[2]; 
               if (NonLocalDeref(dt)) /* dt for array access */
               {
                  if (ireg == SToff[dt-1].sa[0] || ireg == SToff[dt-1].sa[1])
                     ilbase = NewIlist(ip, ilbase);
               }
               ip1 = ip1->next;
            }
         }
      }
   }
   return(ilbase);
}

struct ptrinfo *FindMovingPointers(BLIST *scope)
/*
 * Finds pointers that are inc/decremented in scope
 * INFO: where it is inc/dec, #of times inc/dec, whether inc by constant or var
 *       whether inc is contiguous, whether inc or dec
 * NOTE: assumes structure of code generated by HandlePtrArith(), so must be
 *       run before any code transforms.
 */
{
   struct ptrinfo *pbase=NULL, *p;
   BLIST *bl;
   INSTQ *ip;
   short k, j;
   int flag;
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainstN; ip; ip = ip->prev)
      {
/*
 *       Look for a store to a pointer, and then see how the pointer was
 *       changed to cause the store
 */
         if (ip->inst[0] == ST && ip->prev && 
             (ip->prev->inst[0] == ADD || ip->prev->inst[0] == SUB))
         {
            k = FindLocalFromDT(ip->inst[1]);
            if (k)
            {
/* ERROR : k is DT entry, not ST entry, need to find ST */
               flag = STflag[k-1];
               if (IS_PTR(flag))
               {
/*
 *                Remove these restrictions later to allow things like
 *                ptr0 += ptr1 or ptr0 = ptr0 + ptr1
 */
                  #if IFKO_DEBUG_LEVEL >= 1
                     assert(ip->prev->inst[1] == ip->inst[2]);
                  #endif
                  p = FindPtrinfo(pbase, k);
                  if (!p)
                  {
                     pbase = p = NewPtrinfo(k, 0, pbase);
                     p->nupdate = 1;
                     if (ip->prev->inst[0] == ADD)
                        p->flag |= PTRF_INC;
                     if (ip->inst[2] == ip->prev->inst[2])
                     {
                        j = ip->prev->inst[3];
                        if (j > 0 && IS_CONST(STflag[j-1]))
                        {
/*
 *                         save the ST index of the const updated by
 *                         NOTE: here we keep track of the element count, not
 *                         the actual distance (mulitplying with data size).
 */
#if 1                           
                           k = SToff[j-1].i 
                                 >> type2shift(FLAG2TYPE(STflag[p->ptr-1]));
                           /*fprintf(stderr, "i=%d, p->upst=%d\n", 
                                              SToff[j-1].i, k);*/
#endif
                           p->upst = STiconstlookup(k);;
                           p->flag |= PTRF_CONSTINC;
                           if (SToff[j-1].i == type2len(FLAG2TYPE(flag)))
                              p->flag |= PTRF_CONTIG;
                        }
                     }
                  }
                  else
                  {
                     p->nupdate++;
                     p->flag = 0;
                  }
                  p->ilist = NewIlist(ip, p->ilist);
               }
            }
         }
      }
   }
   return(pbase);
}

INSTQ *KillPointerUpdates(struct ptrinfo *pbase, int UR)
/*
 * This function kills all updates pointed to by pi, so they can be done
 * at the bottom of the loop
 * NOTE: requires that update is always performed, and that is performed
 *       a known number of times (i.e., not once in path A, and twice in 
 *       path B).  For now, just assume these are true.
 *       Also assumes that no access of pointer is done after update!
 * RETURNS: inst queue with update to add to end of loop if it worked, NULL
 *          on error.
 */
{
   struct ptrinfo *pi;
   INSTQ *ipbase=NULL, *ip, *ipN;
   short inc;

/*
 * For now, only unroll constant increments, and require that ptrs are
 * incremented only once staticly (even two seperate paths will disqualify).
 * Enforce these restrictions up-front.
 */
   for (pi=pbase; pi; pi = pi->next)
   {
      if (!(pi->flag & PTRF_CONSTINC) || pi->nupdate > 1)
         return(NULL);
   }
/*
 * If all moving pointers meet requirements, delete the updates, assuming
 * same format expected by FindMovingPointers().  Also, build list of
 * instructions to add to end of loop
 */
   for (pi=pbase; pi; pi = pi->next)
   {
/*
 *    Assert we have code in required format
 */
      ip = pi->ilist->inst;
      assert(ip->inst[0] == ST);
      if (pi->flag & PTRF_INC)
      {
         assert(ip->prev->inst[0] == ADD);
      }
      else
      {
         assert(ip->prev->inst[0] == SUB);
      }
      assert(ip->prev->prev->inst[0] == LD);
/*
 *    Figure new post-update code
 */
      inc = type2len(FLAG2TYPE(STflag[pi->ptr-1]));
      ipN = NewInst(NULL, NULL, NULL, LD, ip->prev->prev->inst[1], 
                    ip->prev->prev->inst[2], 0);
#if 1
/*
 *    we need to increment the ptr by the const. it is updated multiplying with
 *    data type:  HIL: ptr = ptr + val; 
 *                inc = val * type
 */
      inc = inc * SToff[pi->upst-1].i;
      /*fprintf(stderr, "inc=%d, UR=%d, udst=%d\n", inc, UR, 
                        SToff[pi->upst-1].i);*/
#endif
      ipN->next = NewInst(NULL, NULL, NULL, ip->prev->inst[0], 
                          ip->prev->inst[1], ip->prev->inst[2], 
                          STiconstlookup(inc*UR));
      ipN->next->next = NewInst(NULL, NULL, ipbase, ST, ip->inst[1],
                                ip->inst[2], 0);
      ipbase = ipN;
      ip = ip->prev->prev;
/*
 *    Delete inst and free deleted INSTQ* structs
 */
      ip = DelInst(ip);
      ip = DelInst(ip);
      ip = DelInst(ip);
      KillAllIlist(pi->ilist);
      pi->ilist = NULL;
   }
   return(ipbase);
}

short *UpdateDeref(INSTQ *ip, int ireg, int inc)
/*
 * Looks through inst ip for derefs using ireg, and adds inc to them
 * RETURNS: new instruction if inc can be added
 *          NULL if inc cannot be added
 */
{
   static short inst[4];
   int i;
   short ST, k;
   
   inst[0] = ip->inst[0];
   for (i=1; i < 4; i++)
   {
      ST = inst[i] = ip->inst[i];
      ST--;
      if (ST >= 0 && IS_DEREF(STflag[ST]))
      {
/*
 *       If machine doesn't support constant & register indexing, or if
 *       the index register is the changed one, give up
 */
         #ifndef ArchConstAndIndex
            if (SToff[ST].sa[1] < 0)
               return(NULL);
         #else
            if (ireg == -SToff[ST].sa[1])   /* disallow offset usage */
               return(NULL);
         #endif
         if (SToff[ST].sa[0] == -ireg)
         {
            k = SToff[ST].sa[3];
/*
 *       FIXED: 
 *       in l2a, SToff[].sa[3] is used directly as the offset. Here, why we 
 *       do look in SToff entry to figure out the value???
 */
#if 0
            k = k ? SToff[k-1].i+inc : inc;
#else
            k = k + inc;
#endif
            inst[i] = AddDerefEntry(-ireg, SToff[ST].sa[1], SToff[ST].sa[2], k,
                                    STpts2[ST]);
         }
      }
      else if (ST+1 == -ireg)  /* disallow explicit usage */
         return(NULL);
   }
   return(inst);
}
void UpdatePointerLoads(BLIST *scope, struct ptrinfo *pbase, int UR)
/*
 * Finds all loads of pointers in pbase, and adds UR*size to their deref
 */
{
   BLIST *bl;
   INSTQ *ip;
   struct ptrinfo *pi;
   short *pst, *sp;
   int i, n, inc;
   short k;

   if (!pbase || !scope)
      return;
   for (n=0,pi=pbase; pi; n++,pi=pi->next);
   pst = malloc(sizeof(short)*n);
   assert(pst);
   for (i=0,pi=pbase; pi; i++,pi=pi->next)
      pst[i] = pi->ptr;

   for (bl=scope; bl; bl = bl->next)
   {
/*
 *    Look for loads of pointers; since we have previously deleted all updates,
 *    all remaining loads should be for reads.  All reads of pointers must be
 *    either pointer arithmetic or dereferencing
 */
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (IS_LOAD(ip->inst[0]))
         {
            k = ip->inst[2];
            //assert(k > 0);
            if (k <= 0)
            {
               PrintThisInst(stderr,0,ip);
               assert(k > 0);
            }
            k = STpts2[k-1];
            if (!k) continue;
/*
 *          Find if load is of a moving pointer
 */
            for (i=0; i != n && pst[i] != k; i++);
            if (i == n) continue;
/*
 *          Now that we've got a moving pointer, determing unrolling increment
 *          NOTE: we are updating the inc with actual unroll factor * pi->upst
 *          the correctness of this code depeneds on the correct value of 'uri'
 *          parameter. 
 *          1) In unrolling, we have correct upst value. so, we need to use 
 *          unroll factor uri (not URmul)
 *          2) But during vectorization, we don't have correct pi->upst (which 
 *          is updated at the end of the process). So, uri should be dependent 
 *          of vlen.
 *          BTW, it doesn't depend on the load operation of vector-intrinsic 
 *          code. At the end, we will have correct 'upst' to multiply with
 */
            inc = UR * type2len(FLAG2TYPE(STflag[k-1]));
/*
 *          NOTE: i can be 0, then pi = pbase
 */
            for (pi=pbase; i && pi->ptr != k; pi=pi->next,i--)
               ;
            assert(pi);
#if 1
/*
 *          unrolling increment also depends on the const, the Ptr is updated
 *          by
 */   
            assert(pi->flag & PTRF_CONSTINC); /* haven't implemented other yet*/
            if (pi->flag & PTRF_CONSTINC)
               inc = inc * SToff[pi->upst-1].i;
#endif
            if (!(pi->flag & PTRF_INC))
               inc = -inc;
/*
 *          Find use of register we just loaded ptr to (since this step is done
 *          after initial code generation, there is always 1 and only 1 use)
 */
/* ===========================================================================
 * This is all wrong; doesn't take into account most important situation,
 * where register is then used via DT entry
 */
            k = -ip->inst[1];
            assert(k > 0);
            for (ip=ip->next; ip; ip = ip->next)
            {
               CalcThisUseSet(ip);
               if (BitVecCheck(ip->use, k-1))
                  break;
            }
            assert(ip && BitVecCheck(ip->use, k-1));
#if 0
            PrintThisInst(stderr, 1, ip);
#endif
/*
 *          Presently not allowing ptr0 = ptr1 + ptr2, so no explicit ref
 */
            assert(k != -ip->inst[2] && k != -ip->inst[3]);
            sp = UpdateDeref(ip, k, inc);
            if (sp)
            {
               for (k=0; k < 4; k++)
                  ip->inst[k] = sp[k];
            }
            else
               InsNewInst(bl->blk, NULL, ip, ADD, -k, -k, STiconstlookup(inc));
         }
      }
   }
   free(pst);
}

INSTQ *FindCompilerFlag(BBLOCK *bp, short flag)
{
   INSTQ *ip;
   for (ip=bp->inst1; ip; ip = ip->next)
      if (ip->inst[0] == CMPFLAG && ip->inst[1] == flag)
         break;
   return(ip);
}

void KillLoopControl(LOOPQ *lp)
/*
 * This function deletes all the loop control information from loop lp
 * (set, increment & test of index and goto top of loop body)
 */
{
   INSTQ *ip, *ip1;
   BLIST *bl;

   if (!lp) return;
/*
 * Delete index init that must be in preheader
 */
   ip = FindCompilerFlag(lp->preheader, CF_LOOP_INIT);
/*
 * NOTE: If AddLoopControl is already declared (suppose from GenLoopCleanup),
 * the preheader would be splited already and CF_LOOP_INIT would not be found
 * then. Extend the search to check predecessor of preheader. Need to change the 
 * AddLoopControl same way.
 */
#if 0   
   #if IFKO_DEBUG_LEVEL >= 1
      assert(ip);
   #endif
   ip = ip->next;
   while (ip && (ip->inst[0] != CMPFLAG || ip->inst[1] != CF_LOOP_BODY))
      ip = DelInst(ip);
   #if IFKO_DEBUG_LEVEL >= 1
      assert(ip);
   #endif
#else
/*
 * extend the search to pred of preheader and delete inst. CFG is messed up now
 */
   if (!ip)
   {
#if 0      
      fprintf(stderr, "EXTENDING SEARCH FOR KILLLOOPCONTROL!!\n");
#endif
/*
 *    must have only one predecessor of pre-header and it's for cleanup 
 *    checking
 */
      assert(lp->preheader->preds && !lp->preheader->preds->next);
      ip = FindCompilerFlag(lp->preheader->preds->blk, CF_LOOP_INIT);
      assert(ip); /* now, it must find the loop init */

      ip1 = FindCompilerFlag(lp->preheader, CF_LOOP_BODY);
      assert(ip1); /* loop body must be in preheader */
/*
 *    delete all inst of pred of preheader next to the CMPFLAG
 */
      ip = ip->next;
      while (ip) /* delete all inst in this block from CF_LOOP_INIT */  
         ip = DelInst(ip);
/*
 *    delete all inst up to the CF_LOOP_BODY in preheader 
 */
      ip = lp->preheader->inst1;
      while (ip && ip != ip1)
         ip = DelInst(ip);
      assert(ip);
   }
   else
   {
      ip = ip->next;
      while (ip && (ip->inst[0] != CMPFLAG || ip->inst[1] != CF_LOOP_BODY))
         ip = DelInst(ip);
      #if IFKO_DEBUG_LEVEL >= 1
         assert(ip);
      #endif
   }
#endif
/*
 * Delete index update, test and branch that must be in all tails
 */
   for (bl=lp->tails; bl; bl = bl->next)
   {
      ip = FindCompilerFlag(bl->blk, CF_LOOP_PTRUPDATE);
      if (ip)
         ip = DelInst(ip);
      ip = FindCompilerFlag(bl->blk, CF_LOOP_UPDATE);
      #if IFKO_DEBUG_LEVEL >= 1
         assert(ip);
      #endif
      ip = ip->next;
      while (ip && (ip->inst[0] != CMPFLAG || ip->inst[1] != CF_LOOP_END))
      {
/*
 *       Majedul: Need to keep LABEL which may be needed for Scalar Restart
 *       NOTE: Keep in mind that this will break the structure of tail 
 */
         if (ip->inst[0] == LABEL)
            ip = ip->next;
         else if (ip->inst[0] != CMPFLAG || ip->inst[1] != CF_LOOP_TEST)
            ip = DelInst(ip);
         else
            ip = ip->next;
      }
   }
}

void KillCompflagInRange(BLIST *base, enum comp_flag start, enum comp_flag end)
/*
 * Removes compiler flags in range [start,end] from instructions in BLIST bl
 */
{
   BLIST *bl;
   INSTQ *ip;

   for (bl=base; bl; bl = bl->next)
   {
      for (ip=bl->blk->inst1; ip; ip = ip->next)
      {
         if (ip->inst[0] == CMPFLAG && 
             ip->inst[1] >= start && ip->inst[1] <= end)
         {
            ip = DelInst(ip);
            if (!ip) break;
            ip = ip->prev;
         }
      }
   }
}

static void ForwardLoop(LOOPQ *lp, int unroll, INSTQ **ipinit, INSTQ **ipupdate,
                        INSTQ **iptest)
/*
 * This loop only used when index value is referenced inside loop.
 * NOTE: presently assumes constant lp->inc if unroll > 1
 *
 * NOTE: Forward Loop actually works for both NINC and PINC format, at least
 * for cleanup and loop peeling. 
 *
 */
{
   short r0, r1;
   INSTQ *ip;
   extern int FKO_UR;
   extern int SKIP_CLEANUP;

   r0 = GetReg(T_INT);
   r1 = GetReg(T_INT);

   if (unroll > 1)
   {
      assert(IS_CONST(STflag[lp->inc-1]));
      assert(SToff[lp->inc-1].i == 1);
/*
 *    Majedul: OL_NEINC is also used to update N by N+Unroll-1 for the 
 *    forward loop when it is directly jump to cleanup without the main
 *    loop. Look into the code of GenCleanupLoop. 
 */
      /*SToff[OL_NEINC-1].i = unroll-1;*/
      if (!(lp->LMU_flag & LMU_NO_CLEANUP)
            && !SKIP_CLEANUP
            && FKO_UR != -1) /* no need if no cleanup */
         SToff[OL_NEINC-1].i = unroll; /* it works. */
   }
   if (IS_CONST(STflag[lp->beg-1]))
      *ipinit = ip = NewInst(NULL, NULL, NULL, MOV, -r0, lp->beg, 0);
   else
      *ipinit=ip=NewInst(NULL, NULL, NULL, LD, -r0, SToff[lp->beg-1].sa[2], 0);
   ip->next = NewInst(NULL, NULL, NULL, ST, SToff[lp->I-1].sa[2], -r0, 0);
/*
 * If loop is unrolled,  and end is non-constant, subtract off UR-1 so we
 * don't go extra distance
 */

/*
 * Majedul: Need to add conditional branch to jump to cleanup
 * NOTE: for cleanup loop, as header is not set, this ipinit will not be added
 * for Loop peeling, as Unroll is 1, lp->end is not updated
 * NOTE: I haven't consider the case where lp->end is constant.
 */
   if (IS_CONST(STflag[lp->end-1]))
   {
      fprintf(stderr, "\nHaven't consider constant end for forward loop yet\n");
      assert(!IS_CONST(STflag[lp->end-1]));
   }
/*
 * Majedul: this is when used for main loop (in case of cleanup and peeling
 * unroll=1). I have consider that for PINC, need to check whether it works 
 * for NINC also!!! for iamax and for main loop, only runs this for PINC 
 * 
 * FIXME: consider MAIN LOOP with vector/unroll and NINC format...
 */
   if (unroll > 1 && !IS_CONST(STflag[lp->end-1]) 
         && !(lp->LMU_flag & LMU_NO_CLEANUP) 
         && !SKIP_CLEANUP
         && FKO_UR != -1)
   {
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, LD, -r0, SToff[lp->end-1].sa[2], 0);
      ip = ip->next;
      /*ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, 
                         STiconstlookup(unroll*SToff[lp->inc-1].i-1));*/
      ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, 
                         STiconstlookup(unroll*SToff[lp->inc-1].i));
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, ST, SToff[lp->end-1].sa[2], -r0, 0);
 /*
  *   add checking to jump to cleanup.
  */
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, LD, -r0, SToff[lp->I-1].sa[2], 0);
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, LD, -r1, SToff[lp->end-1].sa[2], 0);
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, CMP, -ICC0, -r0, -r1);
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, JGT, -PCREG, -ICC0, lp->NE_label);
/*    to keep preheader intact... deleted by cp */
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, MOV, -REG_SP, -REG_SP, 0);
   }
/*
 * NOTE: this loop update and loop test both work for NINC and PINC loop
 */ 
   *ipupdate = ip = NewInst(NULL, NULL, NULL, LD, -r0, SToff[lp->I-1].sa[2], 0);
   if (IS_CONST(STflag[lp->inc-1]))
   {
      if (unroll > 1)
         ip->next = NewInst(NULL, NULL, NULL, ADD, -r0, -r0, 
                            STiconstlookup(SToff[lp->inc-1].i*unroll));
      else
         ip->next = NewInst(NULL, NULL, NULL, ADD, -r0, -r0, lp->inc);
   }
   else
   {
      ip->next = NewInst(NULL, NULL, NULL, LD, -r1, SToff[lp->inc-1].sa[2], 0);
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, ADD, -r0, -r0, -r1);
   }
   ip = ip->next;
   ip->next = NewInst(NULL, NULL, NULL, ST, SToff[lp->I-1].sa[2], -r0, 0);

   *iptest = ip = NewInst(NULL, NULL, NULL, LD, -r0, SToff[lp->I-1].sa[2], 0);
   if (IS_CONST(STflag[lp->end-1]))
   {
      if (unroll < 2)
         ip->next = NewInst(NULL, NULL, NULL, CMP, -ICC0, -r0, lp->end);
      else
         ip->next = NewInst(NULL, NULL, NULL, CMP, -ICC0, -r0, 
                            STiconstlookup(SToff[lp->end-1].i-unroll+1));
   }
   else
   {
      ip->next = NewInst(NULL, NULL, NULL, LD, -r1, SToff[lp->end-1].sa[2], 0);
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, CMP, -ICC0, -r0, -r1);
   }
   ip = ip->next;
   if (lp->flag & L_MINC_BIT)
      ip->next = NewInst(NULL, NULL, NULL, JNE, -PCREG, -ICC0, lp->body_label);
   else if (lp->flag & L_NINC_BIT)
      ip->next = NewInst(NULL, NULL, NULL, JGT, -PCREG, -ICC0, lp->body_label);
   else
      ip->next = NewInst(NULL, NULL, NULL, JLT, -PCREG, -ICC0, lp->body_label);
   GetReg(-1);
}

static void SimpleLC(LOOPQ *lp, int unroll, INSTQ **ipinit, INSTQ **ipupdate,
                     INSTQ **iptest)
/*
 * Do simple N..0 loop.
 * NOTE: later specialize to use loop reg on PPC
 *       Assumes inc = 1
 *
 * NOTE: it works for all the cases: NINC, PINC. It uses LC optimization.
 * But if index ref is used inside the loop, it can't be used as it changes 
 * the index value for optimization. Then, we need to use ForwardLC
 */
{
   INSTQ *ip;
   short r0, r1;
   int I, I0, N, inc, Ioff, i;
   extern int FKO_UR;
   extern int SKIP_CLEANUP;

   I  = lp->I;
   I0 = lp->beg;
   N  = lp->end;
   inc = lp->inc;
   Ioff = SToff[I-1].sa[2];

   r0 = GetReg(T_INT);
   r1 = GetReg(T_INT);
/*
 * Loop already in SimpleLC form (i = N, 0, -1) 
 */
   if (lp->flag & L_NSIMPLELC_BIT)
   {
      if (IS_CONST(STflag[I0-1]))
         *ipinit = ip = NewInst(NULL, NULL, NULL, MOV, -r0, 
             STiconstlookup(SToff[I0-1].i), 0);
      else
         *ipinit = ip = NewInst(NULL, NULL, NULL, LD, -r0, SToff[I0-1].sa[2],0);
      inc = STiconstlookup(-SToff[inc-1].i);
   }
   else if (IS_CONST(STflag[N-1]) && IS_CONST(STflag[I0-1]))
   {
      i = SToff[N-1].i - SToff[I0-1].i;
      *ipinit = ip = NewInst(NULL, NULL, NULL, MOV, -r0, STiconstlookup(i), 0);
   }
   else
   {
      if (IS_CONST(STflag[I0-1]))
      {
         *ipinit = ip = NewInst(NULL, NULL, NULL, LD, -r0, SToff[N-1].sa[2], 0);
         if (SToff[I0-1].i)
         {
            ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, I0);
            ip = ip->next;
         }
      }
      else if (IS_CONST(STflag[N-1]))
      {
         *ipinit = ip = NewInst(NULL, NULL, NULL, LD, -r1, SToff[I0-1].sa[2],0);
         ip->next = NewInst(NULL, NULL, NULL, MOV, -r0, N, 0);
         ip = ip->next;
         ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, -r1);
         ip = ip->next;
      }
      else
      {
         *ipinit = ip = NewInst(NULL, NULL, NULL, LD, -r0, SToff[N-1].sa[2], 0);
         ip->next = NewInst(NULL, NULL, NULL, LD, -r1, SToff[I0-1].sa[2], 0);
         ip = ip->next;
         ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, -r1);
         ip = ip->next;
      }
   }
/*
 * NOTE: if NO_CLEANUP is used, we don't need the checking; but we need to init
 * the index variable
 */
   if (unroll < 2 || (lp->LMU_flag & LMU_NO_CLEANUP)
         || SKIP_CLEANUP
         || FKO_UR == -1) /* this is for unroll all the way */
      ip->next = NewInst(NULL, NULL, NULL, ST, Ioff, -r0, 0);
   else
   {
      SToff[OL_NEINC-1].i = unroll-1;
      ip->next = NewInst(NULL, NULL, NULL, SUBCC, -r0, -r0, OL_NEINC);
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, ST, Ioff, -r0, 0);
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, JLE, -PCREG, -ICC0, lp->NE_label);
/*
 *    Majedul: I'm not sure why it is necessary!!! this inst can't keep 
 *    preheader intact if new CFG is built!!!
 */
      ip = ip->next;
      /* Keep preheader intact; this inst will be deleted by CP */
      ip->next = NewInst(NULL, NULL, NULL, MOV, -REG_SP, -REG_SP, 0);
   }
   *ipupdate = ip = NewInst(NULL, NULL, NULL, LD, -r0, Ioff, 0);
   if (unroll)
      ip->next = NewInst(NULL, ip, NULL, SUBCC, -r0, -r0, 
                         STiconstlookup(SToff[inc-1].i*unroll));
   else
      ip->next = NewInst(NULL, ip, NULL, SUBCC, -r0, -r0, inc);
   ip = ip->next;
   ip->next = NewInst(NULL, NULL, NULL, ST, Ioff, -r0, 0);
   if (unroll)
      *iptest=ip=NewInst(NULL, NULL, NULL, JGT, -PCREG, -ICC0, lp->body_label);
   else
      *iptest=ip=NewInst(NULL, NULL, NULL, JNE, -PCREG, -ICC0, lp->body_label);
   GetReg(-1);
}

void AddLoopControl(LOOPQ *lp, INSTQ *ipinit, INSTQ *ipupdate, INSTQ *ippost,
                    INSTQ *iptest)
/*
 * Assumes loop preheader and tails info up-to-date
 */
{
   INSTQ *ip, *ipl;
   BLIST *bl;

   assert(lp->tails);
   if (ipinit) /* applied on original loop, not in cleanup loop */
   {
      ipl = FindCompilerFlag(lp->preheader, CF_LOOP_INIT);
/*
 *    IF CF_LOOP_INIT is not found in preheader, may be found on 
 *    predecessor of preheader. see the new implementation of 
 *    KillLoopControl
 */
#if 1 /* to support new loop control */
      if (!ipl)
      {
         /*fprintf(stderr, "EXTENDING SERACH IN ADDLOOPCONTROL \n");*/
         assert(lp->preheader && lp->preheader->preds && 
                !lp->preheader->preds->next);
         ipl = FindCompilerFlag(lp->preheader->preds->blk, CF_LOOP_INIT);
         //assert(ipl);
      }
#endif      
      assert(ipl);
      for (ip = ipinit; ip; ip = ip->next)
         ipl = InsNewInst(NULL, ipl, NULL, ip->inst[0], ip->inst[1], 
                          ip->inst[2], ip->inst[3]);

   }
   for (bl=lp->tails; bl; bl = bl->next)
   {
      ipl = FindCompilerFlag(bl->blk, CF_LOOP_UPDATE);
      #if IFKO_DEBUG_LEVEL >= 1
         assert(ipl);
      #endif
      InsNewInst(NULL, NULL, ipl, CMPFLAG, CF_LOOP_PTRUPDATE, 0, 0);
      for (ip = ippost; ip; ip = ip->next)
         InsNewInst(NULL, NULL, ipl, ip->inst[0], ip->inst[1],
                          ip->inst[2], ip->inst[3]);
      for (ip = ipupdate; ip; ip = ip->next)
         ipl = InsNewInst(NULL, ipl, NULL, ip->inst[0], ip->inst[1],
                          ip->inst[2], ip->inst[3]);

      ipl = FindCompilerFlag(bl->blk, CF_LOOP_TEST);
      #if IFKO_DEBUG_LEVEL >= 1
         assert(ipl);
      #endif
      for (ip = iptest; ip; ip = ip->next)
      {
         ipl = InsNewInst(NULL, ipl, NULL, ip->inst[0], ip->inst[1],
                          ip->inst[2], ip->inst[3]);
      }
   }
}

int AlreadySimpleLC(LOOPQ *lp)
{
   return( (IS_CONST(STflag[lp->inc-1]) && IS_CONST(STflag[lp->end-1])
            && SToff[lp->inc-1].i == -1 && SToff[lp->end-1].i == 0) );
}

void SetLoopControlFlag(LOOPQ *lp, int NeedKilling)
/*
 * Majedul: As this checking is performed sevaral times, I made it a function
 * KNOWN ISSUE: lp->blocks is changed but not updated yet. There is a inconsis-
 * tancy. We need to update the loop control flags once before changing the 
 * loop structure. This is done for speculative vector in the analysis.
 * NOTE: The main logic here is that even PINC can be made simpleLC if index 
 * ref is not used inside loop. 
 */
{
   ILIST *il;
   int isLcOpt;

   if (NeedKilling)
      KillLoopControl(lp);

/*
 * NOTE: L_FORWARDLC_BIT and L_SIMPLELC_BIT are one time decision, it is true
 * for every loop (main, cleanup and peel). as cleanup and peel loop control
 * (lpn) is created with original lp flag, both two pass through.
 * 
 * L_FORWARDLC: loop may be NINC/PINC but can't be optimized as there is index
 *              ref inside loop. forward loop works both for PINC and NINC
 * 
 * L_SIMPLELC: Loop may be inherently simple (L_NSIMPLELC_BIT) or can be
 *             optimizable (as no index ref) and use LC optimization.
 *
 * L_NSIMPLELC: this flag means loop is inherently simple format [N,0,-1].
 *              Keep in mind that it is one time decision. Adding loop
 *              peeling may create a NSIMPLELC into ~NSIMPLELC. So, we need
 *              to recheck this for every lp control.
 *
 */
   if (AlreadySimpleLC(lp))
      lp->flag |= (L_NSIMPLELC_BIT | L_SIMPLELC_BIT);
   else   
      lp->flag &= ~L_NSIMPLELC_BIT;
   
   if (!(lp->flag & (L_FORWARDLC_BIT | L_SIMPLELC_BIT)))
   {
/*
 *    Majedul: in SSV, we check only the vector path. for iamax, vector
 *    path doesn't contain Index ref. Hence, for optloop the loop control is 
 *    optimized where as the clean up doesn't follow it.
 *    I fixed this issue using existing L_IREF_BIT flag.
 *    
 *    HERE HERE, when this is true, it true for all loop. But what happens if
 *    we create transformation which changes the scenario!!!
 */
      /*fprintf(stderr, "\n\n\nlp->end = %s, lp->beg = %s \n", 
              STname[lp->end-1], STname[lp->beg-1]);*/

/*
 *    if SSV is used, LIREF_BIT is set if index is refered in loop
 */
      if (lp->flag & L_IREF_BIT)
      {
         isLcOpt = 0;    
      }
      else /* normal vector*/
      {
         il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
         if (il)
         {
            isLcOpt = 0;
            lp->flag |= L_IREF_BIT;
            KillIlist(il);
         }
         else isLcOpt = 1;
      }
/*
 *    We have already check the NSIMPLELC, if it is true, we will not enter 
 *    into this condition.
 */      
      /*if (!AlreadySimpleLC(lp) && !isLcOpt)*/
      if (!isLcOpt)
        lp->flag |= L_FORWARDLC_BIT;
      else lp->flag |= L_SIMPLELC_BIT;
      
      /*fprintf(stderr, "flag: %d, SIMPLELC = %d, FORWARDLC = %d\n\n\n", 
            lp->flag, lp->flag & L_SIMPLELC_BIT, lp->flag & L_FORWARDLC_BIT);*/
              
   }

}


void OptimizeLoopControl(LOOPQ *lp, /* Loop whose control should be opt */
                         int unroll, /* unroll factor to apply */
                         int NeedKilling, /* 0 if LC already removed */
                         INSTQ *ippost)   /* inst to add to end of loop */
/*
 * attempts to generate optimized loop control for the given loop
 * NOTE: if blind unrolling has been applied, expect that loop counter
 *       is incremented in duplicated bodies, and not in last dup, so
 *       we would call this routine with unroll=1
 */
{
   INSTQ *ipinit, *ipupdate, *iptest;
   /*ILIST *il;*/
   /*int I, beg, end, inc, i;*/

   if (unroll <= 1) unroll = 0;
/*
 * Avoid extra local creation by assuming constant increment for unrolled loops
 */
   if (unroll)
      assert(IS_CONST(STflag[lp->inc-1]));
#if 0
   if (NeedKilling)
      KillLoopControl(lp);
   if (AlreadySimpleLC(lp))
      lp->flag |= (L_NSIMPLELC_BIT | L_SIMPLELC_BIT);
/*
 * if we've not yet determined what kind of loop control to use, do so
 */
   if (!(lp->flag & (L_FORWARDLC_BIT | L_SIMPLELC_BIT)))
   {
/*
 *    Majedul: FIXME: in SSV, we check only the vector path. for iamax, vector
 *    path doesn't contain Index ref. Hence, for optloop the loop control is 
 *    optimized where as the clean up doesn't follow it. 
 */
      il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
      if (!AlreadySimpleLC(lp) && il)
        lp->flag |= L_FORWARDLC_BIT;
      else lp->flag |= L_SIMPLELC_BIT;
      if (il)
         KillIlist(il);
   }
#else
   SetLoopControlFlag(lp, NeedKilling); 
#endif

   if (lp->flag & L_FORWARDLC_BIT)
   {
      fko_warn(__LINE__, "\nIndex refs in loop prevent SimpleLC!!!\n\n");
      ForwardLoop(lp, unroll, &ipinit, &ipupdate, &iptest);
   }
   else
   {
      /*fprintf(stderr, "\nLoop good for SimpleLC!!!\n\n");*/
      SimpleLC(lp, unroll, &ipinit, &ipupdate, &iptest);
   }
#if 0
   fprintf(stderr, "Loop init:\n");
   if(ipinit) PrintThisInstQ(stderr, ipinit);
   fprintf(stderr, "Loop test:\n");
   if(iptest) PrintThisInstQ(stderr, iptest);
   else 
      fprintf(stderr, "NO Loop test!!!\n");
#endif
/*
 * FIXED: if user throughs NO_CLEANUP markup, we need to skip the cleanup 
 * test too. but need initialization of index variable. did it SimpleLC
 * and ForwardLoop function. same as unroll all the way
 */
   AddLoopControl(lp, lp->preheader ? ipinit : NULL, ipupdate, ippost, iptest);
   KillAllInst(ipinit);
   KillAllInst(ipupdate);
   KillAllInst(iptest);
}

char *DupedLabelName(int dupnum, int ilab)
/*
 * Given a label and the dupnum, returns duped label name
 * 
 */
{
   int i;
   static char ln[256];
   char *sp;
   char spnum[64];
   sp = STname[ilab-1];
/*
 * FIXED: If we need to duplicate any blk which is already duplicated, treat 
 * them as seperate blk. In Scalar Restart, _IFKOCD1_NEW and _IFKOCD2_NEW are 
 * the separate blks, duplicated blks of them should also be separated.
 */
#if 0   
   if (!strncmp(sp, "_IFKOCD", 7) && isdigit(sp[7]))
   {
      sp += 7;
      while (*sp && *sp != '_') sp++;
      assert(*sp == '_' && sp[1]);
      sp++;
   }
   sprintf(ln, "_IFKOCD%d_%s", dupnum, sp);
#else
/*
 * to keep the already duplicated blks separated, don't skip their dupnum
 * NOTE: to keep track the duplicated blks(from where it is being duplicated)
 * I added the original dupnum at the end using spnum, like:
 *    _IFKOCD1_NEWMAX, the duplicated one is : _IFKOCDx_NEWMAX1
 *    here, x is any number maintained by a static variable
 *    
 */
   if (!strncmp(sp, "_IFKOCD", 7) && isdigit(sp[7]))
   {
      sp += 7;
      i = 0;
      while (*sp && *sp != '_')
      {
        spnum[i++] = *(sp++);
      }
      assert(*sp == '_' && sp[1]);
      sp++;
      spnum[i]='\0';
   }
   else 
      spnum[0]='\0';
   sprintf(ln, "_IFKOCD%d_%s_%s", dupnum, sp, spnum);
#endif   
   
   return(ln);
}

static BBLOCK *DupCFScope0(INT_BVI ivscp0, /* original scope */
                   INT_BVI ivscp,   /* scope left to dupe */
                   int dupnum,  /* number of duplication, starting at 1 */
                   BBLOCK *head) /* block being duplicated */
/*
 * Duplicates CF starting at head.  Any block outside ivscp is not duplicated
 * NOTE: actual head of loop should not be in ivscop0, even though we dup it
 * FIXED: Majedul: it only works when we don't have loop control in tail blk.
 * But if we have back edge as the loop control, it won't change the branch 
 * traget of the loop control.
 * So, if you want to copy the loop control too (means not kill the loop control
 * before calling this function, you may want to call this function with ivscp0
 * and ivscp where ivscp0 is included the header but ivscp is not. It doesn't 
 * hurt the case of loop without loop control because, in that case we don't 
 * have the branch for back edge to change too.
 */
{
   BBLOCK *nhead, *bp;
   char *sp;
/*
 * Duplicate new block, and remove it from scope to be duplicated
 */
   nhead = DupBlock(head);
   SetVecBit(ivscp, head->bnum-1, 0);
/*
 * If this block has a label, individualize it by adding prefix
 */
   if (head->ilab)
   {
      assert(head->ainst1->inst[0] == LABEL && 
             head->ainst1->inst[1] == head->ilab);
      sp = DupedLabelName(dupnum, head->ilab);
      nhead->ainst1->inst[1] = nhead->ilab = STlabellookup(sp);
   }
/*
 * If block ends with a jump to a block in duplicated scope, change the block
 * target
 */
   if (IS_BRANCH(head->ainstN->inst[0]))
   {
      bp = head->csucc ? head->csucc : head->usucc;
/*
 *    NOTE: majedul: it only works with loop control when ivscp0 has header of 
 *    loop included.
 */
      if (BitVecCheck(ivscp0, bp->bnum-1))
      {
         assert(bp->ilab);
         sp = DupedLabelName(dupnum, bp->ilab);
         if (head->ainstN->inst[0] == JMP)
            nhead->ainstN->inst[2] = STlabellookup(sp);
         else
            nhead->ainstN->inst[3] = STlabellookup(sp);
      }
   }
   if (head->usucc && BitVecCheck(ivscp, head->usucc->bnum-1))
      nhead->usucc = DupCFScope0(ivscp0, ivscp, dupnum, head->usucc);
   if (head->csucc && BitVecCheck(ivscp, head->csucc->bnum-1))
      nhead->csucc = DupCFScope0(ivscp0, ivscp, dupnum, head->csucc);
   return(nhead);
}

BBLOCK *DupCFScope(INT_BVI ivscp0, /* original scope */
                   INT_BVI ivscp,  /* scope left to dupe */
                   /*int dupnum,*/   /* number of duplication, starting at 1 */
                   BBLOCK *head) /* block being duplicated */
{
#if 1
   static int dnum=0;
   return (DupCFScope0(ivscp0, ivscp, dnum++, head));
#else
   return (DupCFScope0(ivscp0, ivscp, dupnum, head));
#endif
}

BLIST *CF2BlockList(BLIST *bl, INT_BVI bvblks, BBLOCK *head)
/*
 * Given a list of blocks in control-flow headed by head, create a list of
 * Basic Blocks containing all blocks indicated in bvblks (zeroing bvblks)
 * This routine works when [up,down] pointers are not set correctly.
 */
{
   if (BitVecCheck(bvblks, head->bnum-1))
   {
      SetVecBit(bvblks, head->bnum-1, 0);
      bl = AddBlockToList(bl, head);
      if (head->usucc)
         bl = CF2BlockList(bl, bvblks, head->usucc);
      if (head->csucc)
         bl = CF2BlockList(bl, bvblks, head->csucc);
   }
   return(bl);
}

void UpdateUnrolledIndices(BLIST *scope, short I, int UR)
/*
 * Adds UR*sizeof to all index refs in scope
 */
{ 
   UpdateIndexRef(scope, I, UR);
}

BBLOCK *FindFallHead(BBLOCK *head, INT_BVI tails, INT_BVI inblks)
/*
 * Given a header in scope, return NULL if already added to inblks, head if
 * it is indeed the head of a previously unseen fall-thru path
 */
{
   BBLOCK *bp;
   if (!head)
      return(NULL);
   if (BitVecCheck(inblks, head->bnum-1))
      return(NULL);
/*
 * Mark all fall-thru blocks in scope from head
 */
   for (bp=head; bp; bp = bp->down)
   {
      SetVecBit(inblks, bp->bnum-1, 1);
/*
 *    path complete if we hit a tail or if fall-thru not successor
 */
     if (BitVecCheck(tails, bp->bnum-1) ||
         (bp->usucc != bp->down && bp->csucc != bp->down))
        break;
   }
   return(head);
}

static BLIST *FindAllFallHeads0(BLIST *ftheads, INT_BVI iscope, BBLOCK *head, 
                                INT_BVI tails, INT_BVI inblks, 
                                INT_BVI visitedblks)
{
   BBLOCK *bp;
#if 0
   fprintf(stderr, "enter blk = %d\n", head->bnum);
#endif   
/*
 * If we've added all blocks in scope, or head is already in, or head is not
 * in scope, stop recursion
 * NOTE: BitVecComp() is a costly operation.
 */
   if (!head || !BitVecComp(iscope, inblks))
      return(ftheads);
/*
 * FIXED: need to explore the node further.
 */
#if 0   
   if (BitVecCheck(inblks, head->bnum-1) || !BitVecCheck(iscope, head->bnum-1))
      return(ftheads); 
   bp = FindFallHead(head, tails, inblks);
#endif
/*
 * need no recursion if head isn't in scope
 */
   if (!BitVecCheck(iscope, head->bnum-1))
      return(ftheads);
/*
 * If it is already inblk (as fall-thru or any way), we don't need to find fall
 * head using it
 */
   if (!BitVecCheck(inblks, head->bnum-1))
   {
      bp = FindFallHead(head, tails, inblks);
      if (bp)
         ftheads = AddBlockToList(ftheads, bp);
   }

/*
 * FIXED: checking with inblks would prevent a node from being explored
 * further. We need to track the explored blk like: DFS to control the 
 * recursion.
 */
#if 0   
   if (head->usucc && !BitVecCheck(inblks, head->usucc->bnum-1))
      ftheads = FindAllFallHeads(ftheads, iscope, head->usucc, tails, inblks);
   if (head->csucc && !BitVecCheck(inblks, head->csucc->bnum-1))
      ftheads = FindAllFallHeads(ftheads, iscope, head->csucc, tails, inblks);
#endif
/*
 * make this node visited 
 */
   SetVecBit(visitedblks, head->bnum-1, 1);
/*
 * recurse only unvisited successors 
 */
   if (head->usucc && !BitVecCheck(visitedblks, head->usucc->bnum-1))
      ftheads = FindAllFallHeads0(ftheads, iscope, head->usucc, tails, inblks, 
                                  visitedblks);
   if (head->csucc && !BitVecCheck(visitedblks, head->csucc->bnum-1))
      ftheads = FindAllFallHeads0(ftheads, iscope, head->csucc, tails, inblks,
                                  visitedblks);
   return(ftheads);
}

BLIST *FindAllFallHeads(BLIST *ftheads, INT_BVI iscope, BBLOCK *head, 
                        INT_BVI tails, INT_BVI inblks)
/*
 * It's a wrapper funtion for the original function. I introduce an extra
 * parameter to manage the visited nodes like: DFS. To avoid the change in 
 * function call, this wrapper is used.
 */
{
   INT_BVI visitedblks;
   BLIST *bl;

   visitedblks = NewBitVec(32);
   SetVecAll(visitedblks, 0);
   bl = FindAllFallHeads0(ftheads, iscope, head, tails, inblks, visitedblks);
   KillBitVec(visitedblks);
   return(bl);
}

void GenCleanupLoop(LOOPQ *lp)
/*
 * Creates the cleanup loop for loop lp, so lp may be unrolled/vectorized
 */
{
   BLIST *ftheads, *bl, *dupblks;
   BBLOCK *bp0, *bp, *newCF;
   LOOPQ *lpn;
   char ln[512];
   INT_BVI iv, ivtails;
   short r0;
   extern BBLOCK *bbbase;
   extern INT_BVI FKO_BVTMP;

/*
 * If no cleanup is needed, return
 */
   if (lp->CU_label == -1)
      return;
/*
 * There's special code for getting N right when unrolled loop was never
 * traversed at all, and it gets _CUNE_ prefixed
 * Beginning of cleanup code is loop body label with _CU_ prefixed
 * post-tail cleanup-done lab is body label with _CUDONE_ prefixed
 */
   sprintf(ln, "_CUNE_%s", STname[lp->body_label-1]);
   lp->NE_label = STlabellookup(ln);
   sprintf(ln, "_CU_%s", STname[lp->body_label-1]);
   lp->CU_label = STlabellookup(ln);
   if (!lp->PTCU_label)
   {
      sprintf(ln, "_CUDONE_%s", STname[lp->body_label-1]);
      lp->PTCU_label = STlabellookup(ln);
   }
/*
 * Find last block, add cleanup after it
 */
   for (bp0=bbbase; bp0->down; bp0 = bp0->down);
   bp = NewBasicBlock(bp0, NULL);
   bp0->down = bp;
   bp0 = bp;
/*
 * First block is for getting N right when unrolled loop never entered
 */
/*
 * Majedul: This block is specially for cleanup loop, only when the program
 * will not enter into the vector/unroll loop.
 * But we need to handle this part for forward loop separately.
 * FIXED: for forward loop, index variable 'I' should not be changed. 
 * We need to update N by N+unroll as we updated the main loop_init but reverse
 * way in forward loop.
 * NOTE: forward loop only executed when there is a index ref inside the loop.
 * Normall forward loop is converted to backward using LC optimization.
 */
   SetLoopControlFlag(lp, 0);      
  
   if (lp->flag & L_FORWARDLC_BIT) /* update N by N+unroll*/
   {
      r0 = GetReg(T_INT);
      InsNewInst(bp, NULL, NULL, LABEL, lp->NE_label, 0, 0);
      InsNewInst(bp, NULL, NULL, LD, -r0, SToff[lp->end-1].sa[2], 0);
/*
 *    OL_NEINC will be changed to real unrolling when known
 *    ========================================================================
 *    NOTE: OL_NEINC can be updated multiple time as we may call the cleanup
 *    several times. It would create problem if other inst use this entry as
 *    a const. We need to consider this as special purpose constant entry and
 *    prevent the search of symbol table to return this entry for others.
 *    FIXED: changes made in STiconstlookup to skip this type of entry!!!
 *    check for side affect
 *    ========================================================================
 */
      /*OL_NEINC = STdef(NULL, CONST_BIT | T_INT, 1);*/
      OL_NEINC = STdef("OL_NEINC", CONST_BIT | T_INT, 1);
      InsNewInst(bp, NULL, NULL, ADD, -r0, -r0, OL_NEINC);
      InsNewInst(bp, NULL, NULL, ST, SToff[lp->end-1].sa[2], -r0, 0);
      GetReg(-1);
   }
   else  /* update I by I+unroll*/
   {
      r0 = GetReg(T_INT);
      InsNewInst(bp, NULL, NULL, LABEL, lp->NE_label, 0, 0);
      InsNewInst(bp, NULL, NULL, LD, -r0, SToff[lp->I-1].sa[2], 0);
/*
 *    OL_NEINC will be changed to real unrolling when known 
 */
      OL_NEINC = STdef(NULL, CONST_BIT | T_INT, 1);
      InsNewInst(bp, NULL, NULL, ADD, -r0, -r0, OL_NEINC);
      InsNewInst(bp, NULL, NULL, ST, SToff[lp->I-1].sa[2], -r0, 0);
      GetReg(-1);
   }
/*
 * Start new block with a cleanup label
 */
   InsNewInst(bp, NULL, NULL, LABEL, lp->CU_label, 0, 0);
/*
 * Duplicate original loop body
 */
   SetVecBit(lp->blkvec, lp->header->bnum-1, 0);
   FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
   /*newCF = DupCFScope(lp->blkvec, iv, 0, lp->header);*/
   newCF = DupCFScope(lp->blkvec, iv, lp->header);
   assert(newCF->ilab);
/*
 * Use CF to produce a block list of duped blocks
 */
   SetVecBit(lp->blkvec, lp->header->bnum-1, 1);
   iv = BitVecCopy(iv, lp->blkvec);
   dupblks = CF2BlockList(NULL, iv, newCF);
#if 0
   fprintf(stderr, "Dupblks = %s\n", PrintBlockList(dupblks));
#endif
/*
 * Find all fall-thru path headers in loop; iv becomes blocks we have already
 * added
 */
   SetVecAll(iv, 0);
   ivtails = BlockList2BitVec(lp->tails);
   ftheads = FindAllFallHeads(NULL, lp->blkvec, lp->header, ivtails, iv);
   ftheads = ReverseBlockList(ftheads);
#if 0
   fprintf(stderr, "ftheads = %s\n", PrintBlockList(ftheads));
#endif
/*
 * Add new cleanup loop (minus I init) at end of rout, one fall-thru path at
 * a time
 */
   for (bl=ftheads; bl; bl = bl->next)
   {
      for (bp=bl->blk; bp; bp = bp->down)
      {
         if (!BitVecCheck(lp->blkvec, bp->bnum-1))
            break;
         bp0->down = FindBlockInListByNumber(dupblks, bp->bnum);
#if 0
         //PrintThisBlockInst(stderr, bp0->down);
         fprintf(stderr, "blk added = %d\n", bp0->down->bnum);
         fflush(stderr);
#endif
         bp0->down->up = bp0;
         bp0 = bp0->down;
         if (BitVecCheck(ivtails, bp->bnum-1))
            break;
/*
 *       if bp->down is not a succesor of bp, we don't need to explore that
 *       NOTE: it's a safe guard. We ensure that everytime, only one path is
 *       copied. ftheads contains the head of each path.
 */
         if (bp->usucc != bp->down && bp->csucc != bp->down)
            break;
      }
      bp0->down = NULL;
   }
#if 0
   PrintLoop(stderr,lp);
   fflush(stderr);
#endif
/*
 * Form temporary new loop struct for cleanup loop
 */
   lpn = NewLoop(lp->flag);
   lpn->I = lp->I;
   lpn->beg = lp->beg;
   lpn->end = lp->end;
   lpn->inc = lp->inc;
   bp = FindBlockInListByNumber(dupblks, lp->header->bnum);
   assert(bp && bp->ilab);
   lpn->body_label = bp->ilab;
   for (bl=lp->tails; bl; bl = bl->next)
   {
      bp = FindBlockInListByNumber(dupblks, bl->blk->bnum);
      assert(bp);
      lpn->tails = AddBlockToList(lpn->tails, bp);
   }
   lpn->blocks = dupblks;
   OptimizeLoopControl(lpn, 1, 0, NULL);
/*
 * After all tails of cleanup loop, add jump back to posttail of unrolled loop
 */
   for (bl=lpn->tails; bl; bl = bl->next)
   {
      bp = NewBasicBlock(bl->blk, bl->blk->down);
      bl->blk->down = bp;
      InsNewInst(bp, NULL, NULL, JMP, -PCREG, lp->PTCU_label, 0);
   }
   KillLoop(lpn);
}

#if 1
void UnrollCleanup(LOOPQ *lp, int unroll)
/*
 * Creates a cleanup loop for loop lp at end of program.
 * Presently just reproduces entire loop without I initialization (assumed
 * to be done before jmp).
 * NOTE: assumes loop control has already been deleted.
 */
{
   BBLOCK *bp;
   INSTQ *ipnext;
   int FORWARDLOOP;
   short r0, r1;

   if (lp->CU_label == -1)
      return;
/*
 * Generate the actual code to do loop cleanup
 */
   if (lp->CU_label == 0)
      GenCleanupLoop(lp);

   r0 = GetReg(T_INT);
   r1 = GetReg(T_INT);
/*
 * If flag's loop control not set, compute it, then set boolean based on flag
 */
/*
 * Majedul: it is used in many places. So, I use that a function. 
 */
   SetLoopControlFlag(lp, 0);
   FORWARDLOOP = L_FORWARDLC_BIT & lp->flag;
   unroll *= Type2Vlen(lp->vflag);  /* need to update Type2Vlen for AVX*/
/*
 * Require one and only one post-tail; later do transformation to ensure this
 * for loops where it is not natively true
 */
   assert(lp->posttails && !lp->posttails->next);
/*
 * Put cleanup info before 1st non-label instruction in posttail, unless
 * we are doing vectorization, in which case put it after all live-out
 * vectors are reduced
 */
   bp = lp->posttails->blk;
   if (DO_VECT(FKO_FLAG))
   {
      ipnext = FindCompilerFlag(bp, CF_VRED_END);
      assert(ipnext);
      ipnext = ipnext->next;
   }
   else if (bp->ainst1 && bp->ainst1->inst[0] == LABEL)
      ipnext = bp->ainst1->next;
   else
      ipnext = bp->ainst1;
   
   if (FORWARDLOOP)
   {
/*
 *    If we've used unrolled forward loop, restore N to original value
 */
      /*fprintf(stderr, "\n\nForward loop !!!\n");*/
      if (!IS_CONST(STflag[lp->end-1]))
      {
         InsNewInst(bp, NULL, ipnext, LD, -r1, SToff[lp->end-1].sa[2], 0);
         /*InsNewInst(bp, NULL, ipnext, ADD, -r1, -r1, 
                            STiconstlookup(unroll*SToff[lp->inc-1].i-1));*/
         InsNewInst(bp, NULL, ipnext, ADD, -r1, -r1, 
                            STiconstlookup(unroll*SToff[lp->inc-1].i));
         InsNewInst(bp, NULL, ipnext, ST, SToff[lp->end-1].sa[2], -r1, 0);
      }
/*
 *    FIXME: if beg and end are both const, it goes to SimpleLC... we don't
 *    consider them here. need to extend incase they appear here.
 */
      else assert(0); /* not considering const !!!!*/
      InsNewInst(bp, NULL, ipnext, LD, -r0, SToff[lp->I-1].sa[2], 0);
      if (IS_CONST(STflag[lp->end-1]))
         InsNewInst(bp, NULL, ipnext, CMP, -ICC0, -r0, lp->end);
      else
      {
         InsNewInst(bp, NULL, ipnext, LD, -r1, SToff[lp->end-1].sa[2], 0);
         InsNewInst(bp, NULL, ipnext, CMP, -ICC0, -r0, -r1);
      }
   }
   else
   {
      InsNewInst(bp, NULL, ipnext, LD, -r0, SToff[lp->I-1].sa[2], 0);
      InsNewInst(bp, NULL, ipnext, SUBCC, -r0, -r0,
                 STiconstlookup(-(FKO_abs(SToff[lp->inc-1].i)*unroll-1)));
      InsNewInst(bp, NULL, ipnext, ST, SToff[lp->I-1].sa[2], -r0, 0);
   }
   InsNewInst(bp, NULL, ipnext, JNE, -PCREG, -ICC0, lp->CU_label);
/*
 * Add label to jump back to when cleanup is done (screws up block, of course)
 */
   InsNewInst(bp, NULL, ipnext, LABEL, lp->PTCU_label, 0, 0);
   GetReg(-1);
}
#endif


void UnrollCleanup2(LOOPQ *lp, int unroll); /* defined later */
int ListElemCount(BLIST *blist);

int UnrollLoop(LOOPQ *lp, int unroll)
{
   INT_BVI iv;
   BBLOCK *newCF;
   BLIST **dupblks, *bl, *ntails=NULL;
   ILIST *il;
   INSTQ *ippost=NULL, *ip, *ipn;
   struct ptrinfo *pi, *pi0;
   int i, UsesIndex=1, UsesPtrs=1, URbase=0, UR, URmul;
   enum comp_flag kbeg, kend;
   short *sp;
   int n;
   extern INT_BVI FKO_BVTMP;
   extern BBLOCK *bbbase;
   extern int VECT_FLAG;
   extern int FKO_UR;
/*
 * Kill previous loop control to simplify the analysis for unroll
 * NOTE: If vectorization is applied before, killing loop control may not 
 * be straight forward. Condition for cleanup loop will split the preheader
 */
   KillLoopControl(lp);
#if 0   
   URmul = Type2Vlen(FLAG2TYPE(lp->vflag)); 
#else
   /*fprintf(stderr, "SB = %d\n", FKO_SB);*/
   URmul = Type2Vlen(FLAG2TYPE(lp->vflag)); 
   if (FKO_SB && (VECT_FLAG & VECT_SV) )
      URmul *= FKO_SB;
#endif
   if (VECT_FLAG & VECT_INTRINSIC)
      UR = unroll;
   else
      UR = lp->vflag ? URmul*unroll : unroll;
#if 0
   fprintf(stdout, "\nAfter killing lp \n");
   PrintInst(stdout, bbbase);
   OptimizeLoopControl(lp, URmul, 0, ippost);
   fprintf(stdout, "\nAfter adding lp control \n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif
/* PrintInst(fopen("err.tmp", "w"), bbbase); */
   
   il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
   if (il) KillIlist(il);
   else UsesIndex = 0;
   
   pi0 = FindMovingPointers(lp->blocks);
   if (!pi0)
      UsesPtrs = 0;

/*
 * NOTE: after unrolling, cleanup is introduced. Analysis for moving ptr would 
 * be difficult. So, if vectorization is not applied and optloop->varrs is not 
 * updated, we can update this here.
 */
/*==========================================================================
 *     Update optloop->varrs .... 
 *==========================================================================*/
   if (!optloop->varrs) 
   {
      for (n=0, pi=pi0; pi; pi=pi->next,n++);
      sp = malloc(sizeof(short)*(n+1));
      assert(sp);
      sp[0] = n;
      for (i=1,pi=pi0; i <= n; i++, pi=pi->next)
         sp[i] = pi->ptr;
      optloop->varrs = sp;
   }

/*
 * Majedul: in new program states, logic of unroll cleanup is changed. 
 * So, need to use new UnrollCleanup function.
 * NOTE: We may skip cleanup if markup says so
 */
   /*UnrollCleanup(lp, unroll);*/
   if ( !(lp->LMU_flag & LMU_NO_CLEANUP)
         /*&& !SKIP_CLEANUP*/
         && FKO_UR != -1) /* unroll all the way */
   {
      if (FKO_SB && (VECT_FLAG & VECT_SV) )
         UnrollCleanup2(lp, unroll*FKO_SB);
      else 
         UnrollCleanup2(lp, unroll);
   }
   else
   {
      OL_NEINC = STdef("OL_NEINC", CONST_BIT | T_INT, 1);
      /*fprintf(stderr, "Force no cleanup!\n");*/
   }

   URbase = (lp->flag & L_FORWARDLC_BIT) ? 0 : UR-1;

   dupblks = malloc(sizeof(BLIST*)*unroll);
   assert(dupblks);
/*
 * Duplicate all of loop's CF
 */
   SetVecBit(lp->blkvec, lp->header->bnum-1, 0);
   kbeg = CF_LOOP_INIT;
   for (i=1; i < unroll; i++)
   {
/*
 *    Duplicate original loop body for unroll i
 */
      FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
/*
 *    FIXED: need to provide appropriate dupnum. 
 *    DupCFScope is already called in several places: cleanup, loop peeling,
 *    speculative vect!!
 *    Why don't use a static variable inside DupCFScope!!! done!
 */
      /*newCF = DupCFScope(lp->blkvec, iv, 100+i, lp->header);*/
      newCF = DupCFScope(lp->blkvec, iv, lp->header);
      iv = BitVecCopy(iv, lp->blkvec);
/*
 *    Use CF to produce a block list of duped blocks
 */
      SetVecBit(iv, lp->header->bnum-1, 1);
      dupblks[i-1] = CF2BlockList(NULL, iv, newCF);
/*
 *    Kill the appropriate loop markup in the blocks (so we don't increment
 *    i multiple times, test it multiple times, etc)
 */
      kend = (i != unroll-1) ? CF_LOOP_END : CF_LOOP_BODY;
      KillCompflagInRange(dupblks[i-1], kbeg, kend);
      if (UsesPtrs)
      {
/*
 *       Find the moving pointers used in unrolled loop
 */
         pi = FindMovingPointers(dupblks[i-1]);
         assert(pi);
/*
 *       Kill pointer updates in loop, and get ptr inc code to add to EOL
 */
         ip = KillPointerUpdates(pi, i);
         assert(ip);
         for (; ip; ip = ipn)
         {
            ipn = ip->next;
            free(ip);
         }
/*
 *       Find all lds of moving ptrs, and add unrolling factor to them
 */
         /*UpdatePointerLoads(dupblks[i-1], pi, i*URmul);*/
         UpdatePointerLoads(dupblks[i-1], pi, i);
         KillAllPtrinfo(pi);
      }
/*
 *    Update indices to add in unrolling factor
 *    NOTE: if SV is already applied, Index need to update with following rules
 */
      if (UsesIndex)
      {
         if (VECT_FLAG & VECT_SV) /* unrolling after SV*/
         {
            if (lp->flag & L_NINC_BIT) 
            {
/*
 *             haven't tested this for NINC yet!
 */
               UpdateUnrolledIndices(dupblks[i-1], lp->I, (unroll-i-1)*URmul); 
            }
            else
               UpdateUnrolledIndices(dupblks[i-1], lp->I, i*URmul); 
         }
         else
            UpdateUnrolledIndices(dupblks[i-1], lp->I, (lp->flag & L_NINC_BIT) ?
                                  URbase-i : URbase+i);
      }
   }
/*
 * NOTE: this is to update the original loop blks which would be placed
 * first before duplicated blks.
 */
   if (UsesIndex)
   {
      if (VECT_FLAG & VECT_SV)
      {
         if (lp->flag & L_NINC_BIT)
         {
            UpdateUnrolledIndices(lp->blocks, lp->I, (unroll-1)*URmul);
         }
         else
         {
            /* adding 0 means : don't need to call at all*/
            /*UpdateUnrolledIndices(lp->blocks, lp->I, 0);*/
         }
      }
      else
         UpdateUnrolledIndices(lp->blocks, lp->I, URbase);
   }
                            
   if (pi0)
   {
      /*ippost = KillPointerUpdates(pi0, UR);*/
      ippost = KillPointerUpdates(pi0, unroll);
      KillAllPtrinfo(pi0);
      assert(ippost);
   }
   SetVecBit(lp->blkvec, lp->header->bnum-1, 1);
   KillCompflagInRange(lp->blocks, CF_LOOP_UPDATE, CF_LOOP_END);
/*
 * Put duplicated blocks into program at correct location; this means that
 * the blocks [up,down] links are correct, but CF is messed up
 */
   InsertUnrolledCode(lp, unroll, dupblks);
#if 0
   for (i=1; i < unroll ; i++)
      fprintf(stderr, "\n\n Copied: id=%d BLKS[%d]: %s\n\n", i, 
              ListElemCount(dupblks[i-1]), PrintBlockList(dupblks[i-1]));
   fprintf(stdout, "Inserted code: \n");
   PrintInst(stdout, bbbase);
   fflush(stdout);
#endif   
/*
 * Fix the tails info for OptimizeLoopControl
 */
   iv = BlockList2BitVec(lp->tails);
   for (bl=dupblks[unroll-2]; bl; bl = bl->next)
   {
/*
 *    If last unrolling blk is a former tail, add it to new tails
 */
      if (BitVecCheck(iv, bl->blk->bnum-1))
         ntails = AddBlockToList(ntails, bl->blk);
   }
   KillBlockList(lp->tails);
   lp->tails = ntails;
/*
 * Put unrolled & optimized loop control back in loop
 * FIXME: outer loop unrolled vector-intrinsic code, UR should not be used.
 * should multiply previous increment with unroll factor "unroll"
 */
   OptimizeLoopControl(lp, UR, 0, ippost);
/*
 * Fix CF by recomputing it and all loop stuff
 */
   CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = 0;

#if 0
   fprintf(stdout, "LIL just after unroll\n");
   PrintInst(stdout, bbbase);
   ShowFlow("ursv.dot", bbbase);
   exit(0);
#endif

#if 0
   RemoveLoopFromQ(optloop);
   optloop->depth = 0;
   KillAllLoops();
#else
   InvalidateLoopInfo();
#endif
   NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
#if 0
   fprintf(stdout, "LIL after unroll\n");
   PrintInst(stdout, bbbase);
   ShowFlow("ur.dot", bbbase);
   exit(0);
#endif   
   FindLoops();  /* need to setup optloop for this */
   CheckFlow(bbbase, __FILE__, __LINE__);
   return(0);
}

void findDTentry(BLIST *scope, short ptr)
{
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip, *ipn;
   short op;
   
   for (bl=scope; bl; bl=bl->next)
   {
      bp = bl->blk;
      for (ip=bp->ainst1; ip; ip=ip->next)
      {
         if (IS_LOAD(ip->inst[0]) && ip->inst[2]==SToff[ptr-1].sa[2])
         {
/*
 *          How to diff : memory access vs variable load
 *          check SToff[].sa[1] ...
 *          variable load: sa[1] always point to the ST index of the var
 *          memory load: sa[1] always be <= 0 (depends on index/lda)
 *          used this tick in PrintIns func in inst.c
 *          NOTE: a better way to identify memory access, when base ptr sa[0] 
 *          is not REG_SP. It would work for any arch for sure.
 */
            ipn = ip;
            while (IS_LOAD(ipn->inst[0]))
            {
               PrintThisInst(stderr, 0, ipn);
               op = ipn->inst[2];
#if 0               
               if (SToff[op-1].sa[1] <= 0 && SToff[op-1].sa[0] != -REG_SP)
#else
               if (NonLocalDeref(op))
#endif
               {
                  fprintf(stderr, "DT(%d): %4d%4d%4d%4d\n",op, SToff[op-1].sa[0],
                        SToff[op-1].sa[1],
                        SToff[op-1].sa[2],
                        SToff[op-1].sa[3]);
               }
               ipn=ipn->next; 
            }
            fprintf(stderr, "%s: ", STname[ptr-1]);
            PrintThisInst(stderr, 0, ip);
         }
      }
   }

}

int CountMemDT(BLIST *scope, short sta)
/*
 * count memory access using the 2d array
 */
{
   int ct;
   BLIST *bl;
   INSTQ *ip;
   
   ct = 0;

   for (bl=scope; bl; bl=bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip=ip->next)
      {
#if 0         
         if (IS_LOAD(ip->inst[0]) 
               && SToff[ip->inst[2]-1].sa[1] <= 0  /* dt for mem access */
               && FindInShortList(STarr[sta-1].colptrs[0], 
                                  STarr[sta-1].colptrs+1, 
                                  STpts2[ip->inst[2]-1]))
#else
         if (IS_LOAD(ip->inst[0]) 
               && NonLocalDeref(ip->inst[2])  /* dt for mem access */
               && FindInShortList(STarr[sta-1].colptrs[0], 
                                  STarr[sta-1].colptrs+1, 
                                  STpts2[ip->inst[2]-1]))
#endif
            ct++;
      }
   }
   return ct;
}


ILIST **AddPrefInsIlistFromDT(LOOPQ *lp, short sta, int pid, int npf,
                        enum inst inst, short ireg1, short ireg2, int ur, 
                        int *ic)
/*
 * given the STarr index of the 2D array, findout all the column access and add
 * prefetch for each for them
 */
{
   int i, j, dtc;
   short colptr;
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ip1, *ipp;
   ILIST **ils2d;

   short op, dt, lvl;
/*
 * allocate space to save the prefetch inst.. 
 * NOTE: assuming one column is accessed only once, max number of 
 * mem access = unroll * outer-loop-unroll factor. is it a reasonable 
 * assumption??? we need to count the memory access at first!
 */
   dtc = CountMemDT(lp->blocks, sta);
#if 1
   dtc = dtc / ur; 
#endif
   ils2d = calloc(sizeof(ILIST*), dtc); /* logical assumption!!! */
   assert(ils2d);
   j=0; 

   lvl = lp->pfflag[pid] & 0x7;
   ipp = NULL;

   for (bl=lp->blocks; bl; bl=bl->next)
   {
      bp = bl->blk;
      for (ip=bp->ainst1; ip; ip=ip->next)
      {
/*
 *       checking ptr in load:
 *       SToff[ptr-1].sa[2] ==> refers DT enrty of a variable
 *       STpts2[dt-1] ==> refers ptr of all dt entry, even the dt for mem access
 *       Here, we are looking for only those dt entry for mem access
 */
#if 0         
         if (IS_LOAD(ip->inst[0]) 
               && SToff[ip->inst[2]-1].sa[1] <= 0 /* dt for mem access */
               && FindInShortList(STarr[sta-1].colptrs[0], 
                                  STarr[sta-1].colptrs+1, 
                                  STpts2[ip->inst[2]-1]))
#else
/*
 *       better to find mem access: sa[0] != -REG_SP
 *       use NonLocalDeref function.
 */
         if (IS_LOAD(ip->inst[0]) 
               && NonLocalDeref(ip->inst[2]) /* dt for array access */
               && FindInShortList(STarr[sta-1].colptrs[0], 
                                  STarr[sta-1].colptrs+1, 
                                  STpts2[ip->inst[2]-1]))

#endif
         {
            op = ip->inst[2];
            colptr = STpts2[ip->inst[2]-1];
#if 0
            PrintThisInst(stderr, ip);
            fprintf(stderr, "colptr=%s\n",STname[colptr-1]);
#endif
            /*ipn = ip;*/
            ip1 = ip->prev;
            ip0 = ip->prev->prev;
/*
 *          ip0 = load of _A_
 *          ip1 = load of ldas or NULL
 */
            if (IS_LOAD(ip1->inst[0]) 
                  && IS_LOAD(ip0->inst[0]))   /* we have lda to load */
            {
               /*for (i=0; i < npf; i++)*/
               for (i=npf-1; i >= 0; i--)
               {
                  ipp = NewInst(NULL, NULL, NULL, LD, -ireg1, ip0->inst[2],0);
                  ipp->next = NewInst(NULL, ipp, NULL, LD, -ireg2, 
                                      ip1->inst[2],0);
                  dt = AddDerefEntry(-ireg1, -ireg2, SToff[op-1].sa[2], 
                                     lp->pfdist[pid]+i*LINESIZE[lvl], colptr);
                  ipp->next->next = NewInst(NULL, ipp->next, NULL, inst, 0, dt, 
                                            STiconstlookup(lvl));
                  ils2d[j] = NewIlist(ipp, ils2d[j]);
               }
            }
            else if(IS_LOAD(ip1->inst[0]))
            {
               ip0 = ip1;
               /*for (i=0; i < npf; i++)*/
               for (i=npf-1; i >= 0; i--)
               {
                  ipp = NewInst(NULL, NULL, NULL, LD, -ireg1, ip0->inst[2],0);
                  dt = AddDerefEntry(-ireg1, 0, SToff[op-1].sa[2], 
                                     lp->pfdist[pid]+i*LINESIZE[lvl], colptr);
                  ipp->next = NewInst(NULL, ipp, NULL, inst, 0, dt, 
                                      STiconstlookup(lvl));
                  ils2d[j] = NewIlist(ipp, ils2d[j]);
               }
            }
            else assert(0);
            j++;
#if 1
            if ( j == dtc )
            {
               *ic = j;
               return ils2d;
            }
#endif
#if 0
            fprintf(stderr, "DT(%d): %4d%4d%4d%4d\n",op, SToff[dt-1].sa[0],
                        SToff[dt-1].sa[1],
                        SToff[dt-1].sa[2],
                        SToff[dt-1].sa[3]);
            PrintThisInstQ(stderr, ipp);
#endif
         }
      }
   }
   *ic = j;
   return ils2d;
}

ILIST *GetPrefetchInst(LOOPQ *lp, int unroll)
{
   BBLOCK *bp;
   INSTQ *ipp;
   BLIST *bl;
   ILIST *ilbase=NULL;
   ILIST **ils, **ils2d;
   short ir, ptr, lvl;
   short ir2, sta;   
   int i, j, k, p, n, m, npf, nils2d;
   /*int ncptr, STc;*/
   int flag;
   enum inst inst;
   struct ptrinfo *pbase, *pb; 

   bp = lp->header;
   assert(bp->ilab == lp->body_label);
   ir = GetReg(T_INT);
   ir2 = GetReg(T_INT);
/*
 * Find vars set in loop
 */
   if (!lp->sets) lp->sets = NewBitVec(TNREG+32);
   else SetVecAll(lp->sets, 0);
   for (bl=lp->blocks; bl; bl = bl->next)
   {
      if (!INUSETU2D)
         CalcUseSet(bl->blk); 
      for (ipp=bl->blk->inst1; ipp; ipp = ipp->next)
         if (ipp->set)
            BitVecComb(lp->sets, lp->sets, ipp->set, '|');
   }
/*
 * Get an ILIST for each array
 */
   n = lp->pfarrs[0];
#if 1
   /*m = n;*/
   m=0;
   for (i=1; i<=n; i++)
   {
      ptr = lp->pfarrs[i];
      sta = STarrlookup(ptr);  
      if (sta && STarr[sta-1].ndim > 1)
      {
         /*m += STarr[sta-1].colptrs[0];*/
         assert(STarr[sta-1].ndim == 2); /* considering only 2D array */
         m += SToff[STarr[sta-1].urlist[0]-1].i;
      }
      else m++;
   }
   ils = calloc(sizeof(ILIST*), m);
#else
   ils = calloc(sizeof(ILIST*), n);
#endif
   assert(ils);
/*
 * get ptr info from loop 
 */
   pbase = FindMovingPointers(lp->blocks); 
/*
 * generate prefetch insts 
 */
   for (i=1, p=1; i <= n; i++)
   {
      ptr = lp->pfarrs[i];
#if 1
      flag = STflag[ptr-1];
      inst = BitVecCheck(lp->sets, lp->pfarrs[i]-1+TNREG) ? PREFW : PREFR;
      lvl = lp->pfflag[i] & 0x7;
/*
 *    # of pref to issue is CEIL(unroll*sizeof(), LINESIZE)
 *    FIXME: incase of implicit unrolling, we can consider the const which is 
 *    used to update the pointer! if we consider that.. we need not 
 *    consider the unroll and vectorization... do we?????
 */
#if 0      
      npf = unroll > 1 ? unroll : 1;
      npf *= type2len(FLAG2TYPE(flag));
      if (!IS_VEC(flag) && IS_VEC(lp->vflag))
         npf *= Type2Vlen(lp->vflag);
#else
/*
 *    using pointer update....
 *    pointer updates should reflect both the unrolling and vectorization
 */
      pb = FindPtrinfo(pbase, ptr);
      assert(pb);
      npf = SToff[pb->upst-1].i; 
      npf *= type2len(FLAG2TYPE(flag));
#endif
#if 0
      fprintf(stderr, "npf = %d\n", npf);
#endif
      npf = (npf + LINESIZE[lvl]-1) / LINESIZE[lvl];
#if 0
      fprintf(stderr, "final npf = %d\n", npf);
#endif
      sta = STarrlookup(ptr);   
/*
 *    for opt 2D array, we don't have pointers for all columns but we want to
 *    add prefetch for all of them. So, *ils[] should keep prefetch inst for all
 *    the columns. Need to redesign to manage that.
 *    
 */
      if (sta && STarr[sta-1].ndim > 1)
      {
         ils2d = AddPrefInsIlistFromDT(lp, sta, i, npf, inst, ir, ir2, unroll, 
                                       &nils2d);  
/*
 *       keep only 1 set of prefetch inst from unrolled loop
 */
   #if 0         
         for(j=0; j < nils2d/unroll; j++)
         {
            for(k=0; k < npf; k++) 
            {
               ils[p-1] = NewIlist(ils2d[j]->inst, ils[p-1]);
               ils2d[j] = KillIlist(ils2d[j]);
            }
            p++;
         }
/*
 *       delete all other inst and Ilist
 */
         for (j=nils2d/unroll; j < nils2d; j++)
         {
            for (k=0; k < npf; k++)
            {
               KillAllInst(ils2d[j]->inst);
               ils2d[j] = KillIlist(ils2d[j]);
            }
         }
         free(ils2d);
   #else
/*
 *       right now, we don't consider more than one access of column in rolled
 *       loop. we allocate ils based on this!
 */
         assert(nils2d <= SToff[STarr[sta-1].urlist[0]-1].i);
/*
 *       copy the prefetch inst to main list, in ils.
 */
         for(j=0; j < nils2d; j++)
         {
            for(k=0; k < npf; k++) 
            {
               ils[p-1] = NewIlist(ils2d[j]->inst, ils[p-1]);
               ils2d[j] = KillIlist(ils2d[j]);
            }
            p++;
         }
         free(ils2d);
   #endif
      }
      else
      {
         for (j=0; j < npf; j++)
         {
            ipp = NewInst(NULL, NULL, NULL, LD, -ir, SToff[ptr-1].sa[2], 0);
            ipp->next = NewInst(NULL, ipp, NULL, inst, 0, 
                  AddDerefEntry(-ir, 0, 0, lp->pfdist[i]+j*LINESIZE[lvl], ptr),
                                STiconstlookup(lvl));
            ils[p-1] = NewIlist(ipp, ils[p-1]);
         }
         p++;
      }
#else
      flag = STflag[ptr-1];
      inst = BitVecCheck(lp->sets, lp->pfarrs[i]-1+TNREG) ? PREFW : PREFR;
      lvl = lp->pfflag[i] & 0x7;
/*
 *    # of pref to issue is CEIL(unroll*sizeof(), LINESIZE)
 */
      npf = unroll > 1 ? unroll : 1;
      npf *= type2len(FLAG2TYPE(flag));
      if (!IS_VEC(flag) && IS_VEC(lp->vflag))
         npf *= Type2Vlen(lp->vflag);
      npf = (npf + LINESIZE[lvl]-1) / LINESIZE[lvl];
      for (j=0; j < npf; j++)
      {
         ipp = NewInst(NULL, NULL, NULL, LD, -ir, SToff[ptr-1].sa[2], 0);
         ipp->next = NewInst(NULL, ipp, NULL, inst, 0, 
                  AddDerefEntry(-ir, 0, 0, lp->pfdist[i]+j*LINESIZE[lvl], ptr),
                             STiconstlookup(lvl));
         ils[i-1] = NewIlist(ipp, ils[i-1]);
      }
#endif      
   }
   GetReg(-1);

#if 0
/*
 *    print and check before copying them to final list
 */   
   fprintf(stderr, "Printing the prefetch insts: \n");
   fprintf(stderr, "=============================\n");
   fprintf(stderr, "m=%d, npf=%d \n", m, npf);
   for (i=0; i < m; i++)
   {
      ilbase = ils[i]; 
      fprintf(stderr, "mem-access=%d",i);
      while(ilbase)
      {
         PrintThisInstQ(stderr, ilbase->inst);
         ilbase=ilbase->next;
      }
   }
   //exit(0);
#endif

/*
 * Create master list of pref inst, ordering by taking one pref from
 * each array in ascending order
 */
#if 1
   for (j=0; j < npf; j++)
   {
      /*for (i=m-1; i >= 0; i--)*/
      for (i=p-2; i >= 0; i--)
      {
         ilbase = NewIlist(ils[i]->inst, ilbase);
         ils[i] = KillIlist(ils[i]);
      }
   }
   free(ils);
#else
   for (j=0; j < npf; j++)
   {
      for (i=n-1; i >= 0; i--)
      {
         ilbase = NewIlist(ils[i]->inst, ilbase);
         ils[i] = KillIlist(ils[i]);
      }
   }
   free(ils);
#endif   
   if (pbase) KillAllPtrinfo(pbase);
   return(ilbase);
}

BLIST *FindAlwaysTakenBlocks(LOOPQ *lp)
/*
 * Finds blocks that are always taken in the loop for use in spreading prefetch
 * inst around.  This algorithm takes only the straight-line code from header
 * and tail (one and only one of each of these in our loops), rather than
 * finding all always-taken blocks.  Can upgrade later if we ever get rout
 * that this doesn't capture all always-taken blocks (this works for all BLAS).
 * RETURNS: BLIST of always-taken blocks, in loop order
 */
{
   BLIST *base=NULL, *bl, *btail;
   BBLOCK *bp, *nbp;
/*
 * Take all straightline code from tail back to header
 */
   assert(lp->tails && !lp->tails->next);
   bp = lp->tails->blk;
   if (bp->usucc && bp->csucc)
      base = NewBlockList(bp, base);
   else
      for (; bp && bp != lp->header && !bp->preds->next; bp = bp->preds->blk)
         base = NewBlockList(bp, base);

   if (lp->header != lp->tails->blk)
   {
      if (bp == lp->header)
         base = NewBlockList(bp, base);
/*
 *    If we didn't make it all the way back to header, add blocks starting at
 *    header to start of list
 */
      else
      {
         btail = base;
         bp = lp->header;
         bl = base = NewBlockList(bp, base);
         if (!bp->csucc || !bp->usucc)
         {
/*
 *          Majedul: HERE HERE: 
 *          tail is added two times as tail is part of loop block in this case
 *          which increases the instruction count resulting the error in skip. 
 *          Another point: how to skip the actual tail part from the active 
 *          instruction count??? Is it needed at all?
 */
            for(; bp && (!bp->csucc || !bp->usucc);
                bp = bp->usucc ? bp->usucc : bp->csucc, bl = bl->next)
            {
               nbp = bp->usucc ? bp->usucc : bp->csucc;
               if (nbp != lp->tails->blk )
                  bl->next = NewBlockList(nbp, btail);
              /*bl->next = NewBlockList(bp->usucc?bp->usucc:bp->csucc,btail);*/
            }
         }
      }
   }
   return(base);
}

void SchedPrefInLoop0(LOOPQ *lp, ILIST *ilbase)
/*
 * Adds the inst in ilbase to loop, one ILIST chunk scheduled at a time.
 * This variant adds all instructions at beginning of header.
 */
{
   BBLOCK *bp;
   INSTQ *ipp, *ip, *ipn;

   bp = lp->header;
   ipp = bp->ainst1;
   while(ilbase)
   {
      for (ip=ilbase->inst; ip; ip = ipn)
      {
         ipp = InsNewInst(bp, ipp, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                          ip->inst[3]);
         ipn = ip->next;
         free(ip);
      }
      ilbase = KillIlist(ilbase);
   }
}

static int NumberOfThisInst(INSTQ *ip, short inst)
{
   int i;
   for (i=0; ip; ip = ip->next)
      if (ip->inst[0] == inst)
         i++;
   return(i);
}
static int NumberOfInst(INSTQ *ip)
{
   int i;
   for (i=0; ip; ip = ip->next)
      if (ACTIVE_INST(ip->inst[0]) && ip->inst[0] != LABEL)
         i++;
   return(i);
}

static int NumberOfThisInstInList(BLIST *bl, short inst)
{
   int i=0;

   while(bl)
   {
      i += NumberOfThisInst(bl->blk->ainst1, inst);
      bl = bl->next;
   }
   return(i);
}

static int NumberOfInstInList(BLIST *bl)
{
   int i=0;

   while(bl)
   {
      i += NumberOfInst(bl->blk->ainst1);
      bl = bl->next;
   }
   return(i);
}

static int FindSkip(int N, int npf, int chunk)
{
   npf = (npf+chunk-1)/chunk;
   return(N/npf);
}

static BLIST *GetSchedInfo(LOOPQ *lp, ILIST *ilbase, short inst, int *NPF, 
                           int *NINST)
{
   BLIST *atake;
   INSTQ *ip;
   ILIST *il;
   int npf, N;

   for (npf=0, il=ilbase; il; il = il->next, npf++);
   atake = FindAlwaysTakenBlocks(lp);
   if (inst < 0)
      N = NumberOfInstInList(atake);
   else
      N = NumberOfThisInstInList(atake, inst);
   ip = FindCompilerFlag(lp->tails->blk, CF_LOOP_PTRUPDATE);
   if (!ip)
      ip = FindCompilerFlag(lp->tails->blk, CF_LOOP_UPDATE);
   assert(ip);
   if (inst < 0)
      N -= NumberOfInst(ip);
   else
      N -= NumberOfThisInst(ip, inst);
   *NPF = npf;
   *NINST = N;
   return(atake);
}

INT_BVI FindUseSetInScope(BLIST *scope)
{
   BLIST *bl;
   INSTQ *ip;
   INT_BVI bv;
   extern INT_BVI FKO_BVTMP;
/*
 * Find all regs used in this range
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   else SetVecAll(FKO_BVTMP, 0);
   bv = FKO_BVTMP;
   for (bl=scope; bl; bl = bl->next)
   {
      if (!INUSETU2D)
         CalcUseSet(bl->blk); 
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (ip->set)
            BitVecComb(bv, bv, ip->set, '|');
         if (ip->use)
            BitVecComb(bv, bv, ip->use, '|');
      }
   }
   return(bv);
}

int FindUnusedIRegInList(BLIST *scope, int ir)
/*
 * Finds integer register not set or used in scope.  Returns ir if it has
 * not been used, and another, unused register, if it has
 */
{
   INT_BVI bv;
   extern INT_BVI FKO_BVTMP;
   /*INSTQ *ip;*/
   /*BLIST *bl;*/
#if 0
/*
 * Find all regs used in this range
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   else SetVecAll(FKO_BVTMP, 0);
   bv = FKO_BVTMP;
   for (bl=scope; bl; bl = bl->next)
   {
      if (!INUSETU2D)
         CalcUseSet(bl->blk); 
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (ip->set)
            BitVecComb(bv, bv, ip->set, '|');
         if (ip->use)
            BitVecComb(bv, bv, ip->use, '|');
      }
   }
#else
   bv = FindUseSetInScope(scope); 
#endif
/*
 * If previous register already used, get one that isn't
 */
   if (!ir)
      ir = GetReg(T_INT);
   while(BitVecCheckComb(bv, Reg2Regstate(ir), '&'))
      ir = GetReg(T_INT);
   return(ir);
}
void FindUnusedTwoIRegInList(BLIST *scope, int ir0, int irn0, int *ir, int *irn)
{
   INT_BVI bv;
   bv = FindUseSetInScope(scope);
   if (!ir0)
      ir0 = GetReg(T_INT);
   if (!irn0)
      irn0 = GetReg(T_INT);
   
   while(BitVecCheckComb(bv, Reg2Regstate(ir0), '&'))
      ir0 = GetReg(T_INT);
   *ir = ir0;

   while(BitVecCheckComb(bv, Reg2Regstate(irn0), '&'))
      irn0 = GetReg(T_INT);
   *irn = irn0;
   GetReg(-1);
}
static void ChangeRegInPF(int ir, ILIST *ilbase)
/*
 * Update all pref inst with new reg, if necessary
 */
{
   ILIST *il;

   if (ilbase->inst->inst[1] != -ir)
   {
      for (il=ilbase; il; il = il->next)
      {
         il->inst->inst[1] = -ir;
         SToff[il->inst->next->inst[2]-1].sa[0] = -ir;
      }
   }
}

static void ChangeBothRegInPF(int ir0, int ir1, ILIST *ilbase)
{
   ILIST *il;
   for (il=ilbase; il; il=il->next)
   {
      il->inst->inst[1] = -ir0;
      if (IS_LOAD(il->inst->next->inst[0])) /* has ldas load */
      {
         il->inst->next->inst[1] = -ir1;
         SToff[il->inst->next->next->inst[2]-1].sa[0] = -ir0;
         SToff[il->inst->next->next->inst[2]-1].sa[1] = -ir1;
      }
      else
      {
         SToff[il->inst->next->inst[2]-1].sa[0] = -ir0;
      }
   }
}

int IsPrefInsNeedTwoRegs(ILIST *ilbase, int *reg0, int *reg1)
/*
 * check whether pref inst loads lda and needs more than one regs; if true,
 * registers are saved in parameter
 */
{
   ILIST *il;
   *reg0 = *reg1 = 0;
   for(il=ilbase; il; il=il->next)
   {
/*
 *    first inst is always a load; load of pointer... 2nd load inst is to load 
 *    the ldas
 */
      if (il->inst->next && IS_LOAD(il->inst->next->inst[0]))
      {
         if (il->inst->inst[1] != il->inst->next->inst[1])
         {
            *reg0 = -il->inst->inst[1];
            *reg1 = -il->inst->next->inst[1];
            return(1);
         }
      }
   }
   return(0);
}

void SchedPrefInLoop1(LOOPQ *lp, ILIST *ilbase, int dist, int chunk)
/*
 * Adds the inst in ilbase to loop, one ILIST chunk scheduled at a time.
 * This variant puts the first pref chunk at dist inst from the start of
 * the header, and then adds another chunk every NINST/(np/chunk)
 */
{
   BLIST *atake, *bl;
   INSTQ *ip, *ipp, *ipn, *ipf, *ipfn;
   int N, skip, sk, i, j, k, npf, ir, ir0;
   int irn, irn0;
   extern INT_BVI FKO_BVTMP;

   atake = GetSchedInfo(lp, ilbase, -1, &npf, &N);

   N -= dist;
   assert(N > 0);
   skip = FindSkip(N, npf, chunk);
   while (!skip)
      skip = FindSkip(N, npf, ++chunk);
   i = dist ? 1 : 0;
   j = 0;
   sk = dist ? dist : skip;
/*
 * Update all pref inst with new inst, if necessary
 * FIXED: there may have two loads pior to the prefetch instruction. So, we
 * need to update regs for both of them
 */
   if (IsPrefInsNeedTwoRegs(ilbase, &ir0, &irn0))
   {
      FindUnusedTwoIRegInList(atake, ir0, irn0, &ir, &irn);
      if (ir0 != ir || irn0 != irn)
         ChangeBothRegInPF(ir, irn, ilbase);
   }
   else
   {
/*
 *    Update all pref inst with new inst, if necessary
 */
      ir0 = -ilbase->inst->inst[1];
      ir = FindUnusedIRegInList(atake, ir0);
      if (ir0 != ir)
         ChangeRegInPF(ir, ilbase);
   }
/*
 * Insert the prefetch inst
 */
   for (bl=atake; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (i%sk == 0)
         {
            ipn = ipp = NULL;
            if (ip->inst[0] == LABEL)
               ipp = ip;
            else
               ipn = ip;
            for (k=0; k < chunk; k++)
            {
               for (ipf=ilbase->inst; ipf; ipf = ipfn)
               {
                  ipp = InsNewInst(bl->blk, ipp, ipn, ipf->inst[0], 
                                   ipf->inst[1], ipf->inst[2], ipf->inst[3]);
                  ipn = NULL;
                  ipfn = ipf->next;
                  free(ipf);
               }
               ilbase = KillIlist(ilbase);
               if (++j == npf) goto EOL;
               sk = skip;
            }
            i++;
         }
         else if (ACTIVE_INST(ip->inst[0]) && ip->inst[0] != LABEL)
            i++;
      }
   }
EOL:
/*
 * Check that last inst added before EOL
 */

/*
 * Majedul: tails of loop is considered to calc inst count. It is very unlikely
 * but still possible that prefetch insts are placed after these CMPFLAGs. In 
 * that case, it may hang the program.
 */
   assert(bl && ip);
   if (bl->blk == lp->tails->blk)
   {
      for (; ip; ip = ip->next)
         if (ip->inst[0] == CMPFLAG && ip->inst[1] == CF_LOOP_PTRUPDATE) 
            break;
      assert(ip);
   }
   KillBlockList(atake);
}

INSTQ *FindNthInstInList(BLIST *scope, INSTQ *ip0, int inst, int N)
/*
 * Starting at ip0, find Nth occurance of instruction inst
 */
{
   BLIST *bln;
   INSTQ *ip;

   assert(ip0 && scope);
   bln = FindInList(scope, ip0->myblk);
   do
   {
      for (ip=ip0; ip; ip = ip->next)
      {
         if (ip->inst[0] == inst)
            if (--N == 0)
               return(ip);
      }
      bln = bln->next;
      ip0 = bln->blk->inst1;
   }
   while(N);

   return(NULL);
}

void SchedPrefInLoop2(LOOPQ *lp, ILIST *ilbase, int iskip, int chunk,
                      int BEFORE, short inst)
/*
 * Adds the inst in ilbase to loop, one ILIST chunk scheduled at a time.
 * This variant puts the first pref chunk at the iskip'th occurance of
 * inst, and then adds another chunk every NINST/(np/chunk)
 */
{
   BLIST *atake, *bl;
   INSTQ *ip, *ipp, *ipn, *ipf, *ipfn;
   int N, skip, sk, j, k, npf, ir, ir0;
   int irn, irn0;

   if (iskip < 0)
   {
     SchedPrefInLoop0(lp, ilbase);
     return;
   }
   if (inst < 0)
   {
     SchedPrefInLoop1(lp, ilbase, iskip, chunk);
     return;
   }

   atake = GetSchedInfo(lp, ilbase, inst, &npf, &N);
/*
 * Update all pref inst with new inst, if necessary
 * FIXED: there may have two loads pior to the prefetch instruction. So, we
 * need to update regs for both of them
 */
   if (IsPrefInsNeedTwoRegs(ilbase, &ir0, &irn0))
   {
      FindUnusedTwoIRegInList(atake, ir0, irn0, &ir, &irn);
      if (ir0 != ir || irn0 != irn)
         ChangeBothRegInPF(ir, irn, ilbase);
   }
   else
   {
      ir0 = -ilbase->inst->inst[1];
      ir = FindUnusedIRegInList(atake, ir0);
      if (ir0 != ir)
         ChangeRegInPF(ir, ilbase);
   }
   N -= iskip;
   iskip++;
   assert(N > 0);
   skip = FindSkip(N, npf, chunk);
   while (!skip)
      skip = FindSkip(N, npf, ++chunk);
   sk = iskip;
   ip = atake->blk->ainst1;
   j = 0;
   while(1)
   {
      ip = FindNthInstInList(atake, ip, inst, sk);
      ipn = ipp = NULL;
      if (BEFORE)
         ipn = ip;
      else
         ipp = ip;
      for (k=0; k < chunk; k++)
      {
         for (ipf=ilbase->inst; ipf; ipf = ipfn)
         {
            ipp = InsNewInst(ip->myblk, ipp, ipn, ipf->inst[0], 
                             ipf->inst[1], ipf->inst[2], ipf->inst[3]);
            ipn = NULL;
            ipfn = ipf->next;
            free(ipf);
         }
         ilbase = KillIlist(ilbase);
         if (++j == npf) goto EOL2;
      }
      sk = skip;
      if (ip->next)
         ip = ip->next;
      else
      {
         bl = FindInList(atake, ip->myblk);
         ip = bl->blk->inst1;
      }
   }
EOL2:
/*
 * Check that last inst added before EOL
 */
   assert(ip);
   bl = FindInList(atake, ip->myblk);
   if (bl->blk == lp->tails->blk)
   {
      for (; ip; ip = ip->next)
         if (ip->inst[0] == CMPFLAG && ip->inst[1] == CF_LOOP_PTRUPDATE) 
            break;
      assert(ip);
   }
   KillBlockList(atake);
}

void SchedPrefInLoop(LOOPQ *lp, ILIST *ilbase)
{
   extern int PFISKIP, PFINST, PFCHUNK;
   int BEFORE=0, DOUB=1, flag;
   enum inst inst;
   char ch;

   if (PFCHUNK < 0)
   {
      BEFORE = 1;
      PFCHUNK = -PFCHUNK;
   }
   ch = PFINST;
   if (lp->vflag)
   {
      if (IS_FLOAT(lp->vflag) || IS_VFLOAT(lp->vflag))
         DOUB=0;
   }
   else
   {
      flag = STflag[lp->pfarrs[1]-1];
      if (IS_FLOAT(flag));
         DOUB=0;
   }
   switch(ch)
   {
   case 'a':
      if (lp->vflag)
      {
         inst = DOUB ? VDADD : VFADD;
      }
      else
      {
         inst = DOUB ? FADD : FADDD;
      }
      break;
   case 'm':
      if (lp->vflag)
      {
         inst = DOUB ? VDMUL : VFMUL;
      }
      else
      {
         inst = DOUB ? FMUL : FMULD;
      }
      break;
   case 'l':
      if (lp->vflag)
      {
         inst = DOUB ? VDLD : VFLD;
      }
      else
      {
         inst = DOUB ? FLD : FLDD;
      }
      break;
   default:
      inst = -1;
   }
   SchedPrefInLoop2(lp, ilbase, PFISKIP, PFCHUNK, BEFORE, inst);
   CFUSETU2D = INUSETU2D = INDEADU2D = 0;
}

void AddPrefetch(LOOPQ *lp, int unroll)
/*
 * Inserts prefetch inst as first active inst in loop header
 * NOTE: assumes called after loop unrolling, but before repeatable opt
 */
{

#if 0
   ShowFlow("ShowFlow.txt",bbbase);
#endif

#if 1
   ILIST *il;
   il = GetPrefetchInst(lp, unroll);
   SchedPrefInLoop(lp, il);
#else
   BBLOCK *bp;
   INSTQ *ipp;
   BLIST *bl;
   short ir, ptr, lvl;
   int i, j, n, npf;
   int flag;
   enum inst inst;

   bp = lp->header;
   assert(bp->ilab == lp->body_label);
   ir = GetReg(T_INT);
/*
 * Find vars set in loop
 */
   if (!lp->sets) lp->sets = NewBitVec(TNREG+32);
   else SetVecAll(lp->sets, 0);
   for (bl=lp->blocks; bl; bl = bl->next)
   {
      if (!INUSETU2D)
         CalcUseSet(bl->blk); 
      for (ipp=bl->blk->inst1; ipp; ipp = ipp->next)
         if (ipp->set)
            BitVecComb(lp->sets, lp->sets, ipp->set, '|');
   }
   ipp = PrintComment(bp, bp->ainst1, NULL, "START prefetching");
   for (i=1,n=lp->pfarrs[0]; i <= n; i++)
   {
      ptr = lp->pfarrs[i];
      flag = STflag[ptr-1];
      inst = BitVecCheck(lp->sets, lp->pfarrs[i]-1+TNREG) ? PREFW : PREFR;
      lvl = lp->pfflag[i] & 0x7;
/*
 *    # of pref to issue is CEIL(unroll*sizeof(), LINESIZE)
 */
      npf = unroll > 1 ? unroll : 1;
      npf *= type2len(FLAG2TYPE(flag));
      if (!IS_VEC(flag) && IS_VEC(lp->vflag))
         npf *= Type2Vlen(lp->vflag);
      npf = (npf + LINESIZE[lvl]-1) / LINESIZE[lvl];
      ipp = PrintComment(bp, ipp, NULL, "\tprefetching %s", STname[ptr-1] ?
                         STname[ptr-1] : "NULL");
      for (j=0; j < npf; j++)
      {
         ipp = InsNewInst(bp, ipp, NULL, LD, -ir, SToff[ptr-1].sa[2], 0);
         ipp = InsNewInst(bp, ipp, NULL, inst, 0, 
                  AddDerefEntry(-ir, 0, 0, lp->pfdist[i]+j*LINESIZE[lvl],ptr),
                  STiconstlookup(lvl));
      }
   }
   ipp = PrintComment(bp, ipp, NULL, "DONE prefetching");
   CFUSETU2D = INUSETU2D = INDEADU2D = 0;
   GetReg(-1);
#endif
}

int VarIsAccumulator(BLIST *scope, int var)
/*
 * Determines of var is used exclusively as an accumulator in scope.
 * This is true iff: (1) All read followed by add inst using that read,
 * followed by write to same location of addition
 * NOTE: assumes h2l format, so no cross-block ops
 */
/*
 * Majedul: accumulator expansion can also be used for FMAC and FMACD. So,
 * I extend those as candidates to determine the acc vars.
 */

{
   BLIST *bl;
   INSTQ *ip;
   enum inst ld, st, add, mac;
   int i;

   i = FLAG2TYPE(STflag[var-1]);
   switch(i)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      add = FADD;
      mac = FMAC;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      add = FADDD;
      mac = FMACD;
      break;
   case T_INT:
      ld = LD;
      st = ST;
      add = ADD;
      mac = UNIMP; /* INT MAC is not implemented yet */
      break;
#if 0      
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__, "Unknown type=%d, file=%s", i, __FILE__);
#else
   case T_VFLOAT:
      ld = VFLD;
      st = VFST;
      add = VFADD;
      mac = VFMAC;
      break;
   case T_VDOUBLE:
      ld = VDLD;
      st = VDST;
      add = VDADD;
      mac = VDMAC;
      break;
#endif
   default:
      fko_error(__LINE__, "Unknown type=%d, file=%s", i, __FILE__);
   }
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (ip->inst[0] == ld && STpts2[ip->inst[2]-1] == var)
         {
            if (ip->next->inst[0] == ld)
               ip = ip->next;
            if (ip->next->inst[0] != add && ip->next->inst[0] != mac)
               return(0);            
            if (ip->next->inst[0] == mac && mac == UNIMP) /* INT MAC */
               return(0);
            ip = ip->next->next;
            if (!ip)
               return(0);
            if (ip->inst[0] != st || STpts2[ip->inst[1]-1] != var)
               return(0);
         }
/*
 *       Majedul: why STpts2[ip->inst[0]-1] instead of ...ip->inst[1]...
 *       FIXED. 
 */
         /*else if (ip->inst[0] == st && STpts2[ip->inst[0]-1] == var)*/
         else if (ip->inst[0] == st && STpts2[ip->inst[1]-1] == var)
            return(0);
      }
   }
   return(1);
}

int CheckMaxMinConstraints(BLIST *scope, short var, short label)
/*
 * This function checks whether a max/min var is only set inside the if-blk
 * and from no other location inside the loop.
 */
{
   int i;
   int isetcount, osetcount;
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip;
   enum inst st;

   i = FLAG2TYPE(STflag[var-1]);
   if (i == T_FLOAT)
      st = FST;
   else if (i == T_DOUBLE)
      st = FSTD;
   else if (i == T_INT)
      st = ST;
/*
 * not suppported for anyother type, like: vector
 */
   else
      return(0);
   
/*
 * check in the if blk, found by label. var should be set only once
 */
   isetcount = 0;
   osetcount = 0;

   for (bl = scope; bl; bl=bl->next)
   {
      bp = bl->blk;
      ip = bp->ainst1;
/*
 *    NOTE: assuming this if blk begins with ainst LABEL
 */
      if (ip && ip->inst[0] == LABEL && ip->inst[1] == label)
      {
         for ( ; ip; ip=ip->next)
         {
/*
 *          FIXED: amax/amin is set by x not after some operations
 */
            if (ip->inst[0] == st && STpts2[ip->inst[1]-1] == var 
                  && ip->prev && IS_LOAD(ip->prev->inst[0]))
               isetcount++;
         }
      }
      else
      {
         for ( ; ip; ip=ip->next)
         {
            if (ip->inst[0] == st && STpts2[ip->inst[1]-1] == var)
               osetcount++;
         }
      }
   }
/*
 * condition to be max/min: update only once inside if-blk, not in other blk
 */
   if (isetcount == 1 && !osetcount)
      return (1);

   return (0);
}

int VarIsMaxOrMin(BLIST *scope, short var, int maxcheck, int mincheck)
/*
 * want to use same function to verify max or min variable simultaneously 
 * Assumption: scope is the blist of a loop, we don't check loop here
 * maxcheck = 1, check for max variable
 * mincheck = 1, check for min variable
 * if both set, return 1 if either max or min
 */
{
   int i, istrue;
   short reg0, reg1, regv;
   enum inst br, cmp;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl;

   i = FLAG2TYPE(STflag[var-1]);
   switch(i)
   {
   case T_FLOAT:
      cmp = FCMP;
      break;
   case T_DOUBLE:
      cmp = FCMPD;
      break;
   case T_INT:
      cmp = CMP;
      break;
   default:
   /*case T_VFLOAT:
   case T_VDOUBLE:*/
/*
 *    not applicable for vector intrinsic code
 */
      return(0);
   }
  
   istrue = 0;
   for (bl = scope; bl; bl = bl->next)
   {
      bp = bl->blk;
      for (ip = bp-> ainst1; ip; ip = ip->next)
      {
/*
 *       FORMAT: cmp fcc, reg0, reg1
 *               JLT/JGT PC, fcc, LABEL
 *       There would be exactly two ld of 2 vars/const
 */
         if (IS_COND_BRANCH(ip->inst[0]) && ip->prev
               && (ip->prev->inst[0]== cmp))
         {
/*
 *          registers to compare
 */
            reg0 = ip->prev->inst[2];
            reg1 = ip->prev->inst[3];
            br = ip->inst[0];
/*
 *          FIXED: what if we use a condition with const!!! skipped that 
 */
            ip0 = NULL;
            ip1 = ip->prev->prev;           /* 2nd ld */
            if(ip1) 
                  ip0 = ip->prev->prev->prev;     /* 1st ld */
            if (ip0 && ip1 && IS_LOAD(ip0->inst[0]) && IS_LOAD(ip1->inst[0]))
            {
/*
 *             for load, inst[2] must be a var 
 */
               if (STpts2[ip0->inst[2]-1] == var)
                  regv = ip0->inst[1];
               else if (STpts2[ip1->inst[2]-1] == var)
                  regv = ip1->inst[1];
               else 
                  regv = 0;
/*
 *             checking for max var
 */
               if (maxcheck)
               {
/*
 *                check the branch condition for max var
 */
                  if ( ((regv == reg0) && (br == JLT)) ||  
                       ((regv == reg1) && (br == JGT)) )
                  {
/*
 *                   check whether var is only set in if-blk, not else blk
 */
                     if (CheckMaxMinConstraints(scope, var, ip->inst[3]))
                        istrue = 1;
                  }
               }
/*
 *             check for min var
 */
               if (mincheck)
               {
                  if ( ((regv == reg0) && (br == JGT)) ||  
                       ((regv == reg1) && (br == JLT)) )
                  {
                     if (CheckMaxMinConstraints(scope, var, ip->inst[3]))
                        istrue = 1;;
                  }

               }
               if (istrue) /* either max or min, no need to check further */
                  break; 
            }
         }
      }
   }
   return(istrue);
}

#if 0
int VarIsMax(BLIST *scope, short var)
/*
 * Figure out whether a var is Max varibale
 * shifted in appropriate location later.
 */
{
   int i, j;
   short reg0, reg1, regv;
   enum inst inst, ld, st, br, cmp;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl;

   i = FLAG2TYPE(STflag[var-1]);
   switch(i)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      cmp = FCMP;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      cmp = FCMPD;
      break;
   case T_INT:
      ld = LD;
      st = ST;
      cmp = CMP;
      break;
   default:
#if 0
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__,"Unknown type=%d, file=%s. Should be done before Vect",
                i, __FILE__);
#else
   case T_VFLOAT:
   case T_VDOUBLE:
/*
 *    not applicable for vector intrinsic code
 */
      return(0);
#endif
   }

   for (bl = scope; bl; bl = bl->next)
   {
      bp = bl->blk;
      for (ip = bp-> ainst1; ip; ip = ip->next)
      {
/*
 *       FORMAT: cmp fcc, reg0, reg1
 *               JLT/JGT PC, fcc, LABEL
 *       There would be exactly two ld of 2 vars/const
 */
         if (IS_COND_BRANCH(ip->inst[0]) && ip->prev
               && (ip->prev->inst[0]== cmp))
         {
            reg0 = ip->prev->inst[2];
            reg1 = ip->prev->inst[3];
            br = ip->inst[0];
/*
 *          FIXED: what if we use a condition with const!!! skipped that 
 */
            ip0 = NULL;
            ip1 = ip->prev->prev;           /* 2nd ld */
            if(ip1) 
                  ip0 = ip->prev->prev->prev;     /* 1st ld */
            /*assert((ip1->inst[0] == ld));
            assert((ip0->inst[0] == ld));*/
            
            if (ip0 && ip1 && IS_LOAD(ip0->inst[0]) && IS_LOAD(ip1->inst[0]))
            {
/*
 *             for load, inst[2] must be a var 
 */
               if (STpts2[ip0->inst[2]-1] == var)
                  regv = ip0->inst[1];
               else if (STpts2[ip1->inst[2]-1] == var)
                  regv = ip1->inst[1];
               else 
                  regv = 0;
               if ( ((regv == reg0) && (br == JLT)) ||  
                    ((regv == reg1) && (br == JGT)) )
               {
               /* check for single set inside if-blk and not set where else*/
                  if (CheckMaxMinConstraints(scope, var, ip->inst[3]))
                     return (1);
               }
            }
         }
      }
   }
   return (0);
}

int VarIsMin(BLIST *scope, short var)
/*
 * Figure out whether a var is Min varibale
 * shifted in appropriate location later.
 * NOTE: this function is same as the VarIsMax, it can be merged with that
 */
{
   int i, j;
   short reg0, reg1, regv;
   enum inst inst, ld, st, br, cmp;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl;

   i = FLAG2TYPE(STflag[var-1]);
   switch(i)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      cmp = FCMP;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      cmp = FCMPD;
      break;
   case T_INT:
      ld = LD;
      st = ST;
      cmp = CMP;
      break;
#if 0      
   default:
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__,"Unknown type=%d, file=%s. Should be done before Vect",
                i, __FILE__);
#else
   case T_VFLOAT:
   case T_VDOUBLE:
      return(0);
#endif
   }

   for (bl = scope; bl; bl = bl->next)
   {
      bp = bl->blk;
      for (ip = bp-> ainst1; ip; ip = ip->next)
      {
/*
 *       FORMAT: cmp fcc, reg0, reg1
 *               JLT/JGT PC, fcc, LABEL
 *       There would be exactly two ld of 2 vars/const
 */
         if (IS_COND_BRANCH(ip->inst[0]) && (ip->prev->inst[0]== cmp))
         {
            reg0 = ip->prev->inst[2];
            reg1 = ip->prev->inst[3];
            br = ip->inst[0];

            ip0 = ip->prev->prev->prev;     /* 1st ld */
            assert((ip0->inst[0] == ld));
            ip1 = ip->prev->prev;           /* 2nd ld */
            assert((ip1->inst[0] == ld));
/*
 *          for load, inst[2] must be a var
 *          STpts2[] vs. SToff[].sa[2] 
 */
            if (STpts2[ip0->inst[2]-1] == var)
               regv = ip0->inst[1];
            else if (STpts2[ip1->inst[2]-1] == var)
               regv = ip1->inst[1];
            else 
               regv = 0;
/*
 *          NOTE: cbr is opposit from Max
 */
            if ( ((regv == reg0) && (br == JGT)) ||  
                 ((regv == reg1) && (br == JLT)) )
            {
               /* check for single set inside if-blk and not set where else*/
               if (CheckMaxMinConstraints(scope, var, ip->inst[3]))
                  return (1);
            }
         }
      }
   }
   return (0);
}
#endif

#if 0
void UpdateOptLoopWithMaxMinVars()
/*
 * Note: it is not used anymore in new program states
 */
{
   int i, j, N, n, m;
   short *scal, *spmax, *spmin;
   LOOPQ *lp;
   
#if 0   
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__); 
   lp = optloop; 
#else
   GenPrologueEpilogueStubs(bbbase, 0);
   NewBasicBlocks(bbbase);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
   lp = optloop;
#endif
   
   scal = FindAllScalarVars(lp->blocks);   
   N = scal[0];
   spmax = malloc(sizeof(short)*(N+1));
   assert(spmax);
   spmin = malloc(sizeof(short)*(N+1));
   assert(spmin);

   for (i=1, m=0, n=0; i <= N; i++)
   {
      /*fprintf(stderr, " var = %s(%d)\n",STname[scal[i]-1], scal[i] );*/
      if (VarIsMax(lp->blocks, scal[i]))
      {
         spmax[++m] = scal[i];
         /*fprintf(stderr, " Max var = %s\n",STname[scal[i]-1] );*/
      }
      else if (VarIsMin(lp->blocks, scal[i]))
      {
         spmin[++n] = scal[i];
         /*fprintf(stderr, " Min var = %s\n",STname[scal[i]-1] );*/
      }
   }
/*
 * Update with max vars
 */
   if (m)
   {
      if (lp->maxvars) free(lp->maxvars);
      lp->maxvars = malloc(sizeof(short)*(m+1));
      assert(lp->maxvars);
      lp->maxvars[0] = m;
      for (i=1; i <=m ; i++)
         lp->maxvars[i] = spmax[i];
   }
   else
   {
      if (lp->maxvars) free(lp->maxvars);
      lp->maxvars = NULL;
   }
/*
 * update with min vars
 */
   if (n)
   {
      if (lp->minvars) free(lp->minvars);
      lp->minvars = malloc(sizeof(short)*(n+1));
      assert(lp->minvars);
      lp->minvars[0] = n;
      for (i=1; i <=n ; i++)
         lp->minvars[i] = spmin[i];
   }
   else
   {
      if (lp->minvars) free(lp->minvars);
      lp->minvars = NULL;
   }
/*
 * free all temporaries
 */
   if (scal) free(scal);
   if (spmax) free(spmax);
   if (spmin) free(spmin);
}
#endif

void UpdateOptLoopWithMaxMinVars()
/*
 * This function is used in state1 to keep track of all the max/min vars
 * there is an other version of it which is kept for backward compability and
 * will be deleted later.
 */
{
   int i, N, n, m;
   short *scal, *spmax, *spmin;
   LOOPQ *lp;
   
   lp = optloop;
   
   scal = FindAllScalarVars(lp->blocks);   
   N = scal[0];
   spmax = malloc(sizeof(short)*(N+1));
   assert(spmax);
   spmin = malloc(sizeof(short)*(N+1));
   assert(spmin);

   for (i=1, m=0, n=0; i <= N; i++)
   {
/*
 *    checking for maxvar 
 */
      if (VarIsMaxOrMin(lp->blocks, scal[i], 1, 0))
      {
         spmax[++m] = scal[i];
         /*fprintf(stderr, " Max var = %s\n",STname[scal[i]-1] );*/
      }
/*
 *    checking for min var 
 */
      else if (VarIsMaxOrMin(lp->blocks, scal[i], 0, 1))
      {
         spmin[++n] = scal[i];
         /*fprintf(stderr, " Min var = %s\n",STname[scal[i]-1] );*/
      }
   }
/*
 * Update with max vars
 */
   if (m)
   {
      if (lp->maxvars) free(lp->maxvars);
      lp->maxvars = malloc(sizeof(short)*(m+1));
      assert(lp->maxvars);
      lp->maxvars[0] = m;
      for (i=1; i <=m ; i++)
         lp->maxvars[i] = spmax[i];
   }
   else
   {
      if (lp->maxvars) free(lp->maxvars);
      lp->maxvars = NULL;
   }
/*
 * update with min vars
 */
   if (n)
   {
      if (lp->minvars) free(lp->minvars);
      lp->minvars = malloc(sizeof(short)*(n+1));
      assert(lp->minvars);
      lp->minvars[0] = n;
      for (i=1; i <=n ; i++)
         lp->minvars[i] = spmin[i];
   }
   else
   {
      if (lp->minvars) free(lp->minvars);
      lp->minvars = NULL;
   }
/*
 * free all temporaries
 */
   if (scal) free(scal);
   if (spmax) free(spmax);
   if (spmin) free(spmin);
}

void CountVarAccess(BLIST *scope, int ptr, int *nread, int *nwrite)
{
   BLIST *bl;
   INSTQ *ip;
   int nr=0, nw=0;
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
/*
 *       IF we count variable access, not mem
 */
#if 0         
         if (IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]-1] == ptr 
               && SToff[ip->inst[1]-1].sa[1] > 0 ) /* not mem */
            nw++;
         else if (IS_LOAD(ip->inst[0]) && STpts2[ip->inst[2]-1] == ptr 
               && SToff[ip->inst[2]-1].sa[1] > 0 ) /* not mem */
            nr++;
#else
/*
 *       better way to check var, but not mem access
 */
         if (IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]-1] == ptr 
               && SToff[ip->inst[1]-1].sa[0] == -REG_SP 
               && SToff[ip->inst[1]-1].sa[1] >= 0 ) /* not mem */
            nw++;
         else if (IS_LOAD(ip->inst[0]) && STpts2[ip->inst[2]-1] == ptr 
               && SToff[ip->inst[2]-1].sa[0] == -REG_SP 
               && SToff[ip->inst[2]-1].sa[1] >= -REG_SP ) /* not mem */
            nr++;
#endif
      }
   }
   *nread = nr;
   *nwrite = nw;
}


void CountArrayAccess(BLIST *scope, int ptr, int *nread, int *nwrite)
/*
 * Finds number of read/write to *ptr (memory access) in scope.
 */
{
   BLIST *bl;
   INSTQ *ip;
   int nr=0, nw=0;
   
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
#if 0         
 /*
  *       IF we count only the memory access
  *       for mem access sa[1] of DT entry would be a reg or 0 ( <= 0)
  *       Better way: sa[0] != -REG_SP
  */
         if (IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]-1] == ptr 
               && SToff[ip->inst[1]-1].sa[1] <= 0 ) /* mem */
            nw++;
         else if (IS_LOAD(ip->inst[0]) && STpts2[ip->inst[2]-1] == ptr 
               && SToff[ip->inst[2]-1].sa[1] <= 0 ) /* mem */
            nr++;
#else
/*
 *       better way to check : sa[0] != -REG_SP
 *       use NonLocalDeref function
 */
         if (IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]-1] == ptr 
               && NonLocalDeref(ip->inst[1]) ) /* array access */
            nw++;
         else if (IS_LOAD(ip->inst[0]) && STpts2[ip->inst[2]-1] == ptr 
               && NonLocalDeref(ip->inst[2]) ) /* array access */
            nr++;
#endif
      }
   }
   *nread = nr;
   *nwrite = nw;
}

short *FindAllScalarVars(BLIST *scope)
{
   struct ptrinfo *pbase, *p;
   short *sp, *s, *scal;
   int i, j, k, n, N;
   INT_BVI iv;
   BLIST *bl;
   extern INT_BVI FKO_BVTMP;
   extern short STderef;
/*
 * Find variable accessed in the path and store it in path
 */
   /*iv = NewBitVec(32);*/
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   SetVecAll(iv, 0);

   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
   for (bl=scope; bl; bl=bl->next)
   {
      iv = BitVecComb(iv, iv, bl->blk->uses, '|');
      iv = BitVecComb(iv, iv, bl->blk->defs, '|');
   }
   
/*
 * right now, skip all the regs from uses, defs
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
   SetVecBit(iv, STderef+TNREG-1, 0);

/*
 * Skip all non-fp variables, valid entires upto n (included) 
 * NOTE: No action for INT var in vector but need to consider complex case 
 * with index var update later!!!
 * NOTE: As our vector path never uses/sets integer variable (except index)
 * we don't have to worry about this in Backup/Recovery stage. So, we just 
 * skip int here.
 */
   sp = BitVec2Array(iv, 1-TNREG);
   for (N=sp[0],n=0,i=1; i <= N; i++)
   {
      if (IS_FP(STflag[sp[i]-1]))
         sp[n++] = sp[i];
   }   

/*
 * Moving pointer analysis for path
 */
   pbase = FindMovingPointers(scope);
   for (N=0, p=pbase; p; p = p->next)
      if (IS_FP(STflag[p->ptr-1]))
         N++;
/*
 * Copy all moving pointers to varrs
 */
   s = malloc(sizeof(short)*(N+1));
   assert(s);
   s[0] = N;
   for (j=0,i=1,p=pbase; p; p = p->next)
      if (IS_FP(STflag[p->ptr-1]))
         s[i++] = p->ptr;

   for (k=0,i=1; i < n; i++) /* BUG: why n is included??? changed to < n */
   {
      for (j=1; j <= N && s[j] != sp[i]; j++);
      if (j > N)
      {
         sp[k++] = sp[i]; /*sp elem starts with 0 pos*/
      }
   }
   n = k;   /* n is k+1 */
   KillAllPtrinfo(pbase); /* free mem */
/*
 * Store the scals for path->scals. we will update the flags later. 
 */
   scal = malloc(sizeof(short)*(n+1));
   assert(scal );
   scal[0] =  n;
   for (i=1; i <= n; i++)
      scal[i] = sp[i-1];
   
   if (s) free(s);
   if (sp) free(sp);

   return scal;

}

void PrintMovingPtrAnalysis(FILE *fpout)
{
   int i, j, k, m, n, ns, N, npt, nst, nus;
   int nlds, nsts;
   int n1d, n2d;
   INT_BVI set, use, var;
   short *sp, *aptr;
   char pre;
   struct ptrinfo *pi, *pi0;
   BLIST *bl;
   INSTQ *ip;
   LOOPQ *lp;
   short sta, ptr;
   extern short STderef;
   
/*
 *    Find all vars set & used in loop
 */
   lp = optloop;
   KillLoopControl(lp);

   set = NewBitVec(32);
   use = NewBitVec(32);
   for (bl=lp->blocks; bl; bl = bl->next)
   {
      CalcUseSet(bl->blk);
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (ip->set)
            set = BitVecComb(set, set, ip->set, '|');
         if (ip->use)
            use = BitVecComb(use, use, ip->use, '|');
      }
   }
   pi0 = FindMovingPointers(lp->blocks);
   for (N=0,pi=pi0; pi; pi = pi->next)
      if (IS_FP(STflag[pi->ptr-1]))
         N++;
/*
 * FIXED: if we have 2D array access, we won't return column ptr as
 * they are compiler's internal var/ptr. We will return the 2D ptr
 * but in prefetch opt, we will prefetch all the columns.
 */
   aptr = malloc(sizeof(short)*(N+1)); /* ptr from 0 */
   assert(aptr);
   npt = 0;

   for (pi=pi0; pi; pi=pi->next)
   {
      i = pi->ptr;
      if (IS_FP(STflag[i-1]))
      {
         sta = STarrColPtrlookup(i);
         if (sta && STarr[sta-1].ndim > 1)
            i = STarr[sta-1].ptr;
         if (!FindInShortList(npt, aptr, i))
         {
            npt = AddToShortList(npt, aptr, i);
         }
      }
   }
   n1d = 0; n2d =0;
   for (i=0; i < npt; i++)
   {
      ptr = aptr[i];
      sta = STarrlookup(ptr);
      if (!sta)
         n1d++;
      else if( sta && STarr[sta-1].ndim > 1)
         n2d++;
   }
   if (n2d)
   {
      fprintf(fpout, "   Moving 2D Pointers: %d\n", n2d);
      for (i=0; i < npt; i++)
      {
         ptr = aptr[i];
         sta = STarrlookup(ptr);
         if( sta && STarr[sta-1].ndim > 1)
         {
            fprintf(fpout, "      '%s': type=%c", 
                    STname[ptr-1]?STname[ptr-1]:"NULL",
                    IS_FLOAT(STflag[ptr-1]) ? 's' : 'd');
            if (lp->nopf && FindInShortList(lp->nopf[0], lp->nopf+1, ptr))
                  j = 0;
            else
            {
               j = 1;
               nst = 0; nus = 0; nsts = 0; nlds = 0;
/*
 *             compute from all internally created column pointers
 */
               for (k=1, N=STarr[sta-1].colptrs[0]; k <= N; k++ )
               {
                  ptr = STarr[sta-1].colptrs[k];
                  pi = FindPtrinfo(pi0, ptr);
                  j = ((pi->flag | PTRF_CONTIG | PTRF_INC) == pi->flag) & j;
                  CountArrayAccess(lp->blocks, ptr, &m, &n);
                  nsts += n;
                  nlds += m;
                  CountVarAccess(lp->blocks, ptr, &m, &n);
                  nst += n;
                  nus += m;
               }
            }
            fprintf(fpout, " uses=%d sets=%d", nus, nst);
            fprintf(fpout, " lds=%d sts=%d", nlds, nsts);
            fprintf(fpout, " prefetch=%d", j);
            fprintf(fpout, " ncol=%d nreg=%d nptr=%d\n", 
                    SToff[STarr[sta-1].urlist[0]-1].i,
                    STarr[sta-1].colptrs[0] + STarr[sta-1].cldas[0],
                    STarr[sta-1].colptrs[0]); 
         }
      }
   }
   if (n1d)
   {
      fprintf(fpout, "   Moving 1D Pointers: %d\n", n1d);
      for (i=0; i < npt; i++)
      {
         ptr = aptr[i];
         sta = STarrlookup(ptr);
         if (!sta)
         {
            fprintf(fpout, "      '%s': type=%c", 
                    STname[ptr-1]?STname[ptr-1]:"NULL",
                    IS_FLOAT(STflag[ptr-1]) ? 's' : 'd');
            pi = FindPtrinfo(pi0, ptr);
/*
 *          NOTE: prefetch not applicable for outer loop unrolled kernel by this 
 *          condition
 *          FIXED: we don't need contiguous ptr inc; const ptr movement is 
 *          enough for our cases. 
 *          NOTE: what if there are const strided? it doesn't hurt since 
 *          prefetch doesn't cause problem for correctness.. we will tune 
 *          prefetch distance anyway.
 */
            /*j = ((pi->flag | PTRF_CONTIG | PTRF_INC) == pi->flag);*/
            j = ((pi->flag | PTRF_CONSTINC | PTRF_INC) == pi->flag);
            if (lp->nopf)
               if (FindInShortList(lp->nopf[0], lp->nopf+1, ptr))
                  j = 0;
            CountVarAccess(lp->blocks, ptr, &m, &n);
            fprintf(fpout, " uses=%d sets=%d", m, n);
            CountArrayAccess(lp->blocks, ptr, &m, &n);
            fprintf(fpout, " lds=%d sts=%d", m, n);
            fprintf(fpout, " prefetch=%d\n", j);
         }
      }
   }
   free(aptr);
/*
 *    sets + use - regs - ptrs = scalars in loop
 */
   var = BitVecComb(0, set, use, '|');
/*
 *    Sub off all registers
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(var, i, 0);
/*
 *    Sub off all moving ptrs
 */
   for (pi=pi0; pi; pi = pi->next)
      SetVecBit(var, pi->ptr-1+TNREG, 0);
   KillAllPtrinfo(pi0);

/*
 *    Find all scalars used in loop
 */
   sp = BitVec2Array(var, 1-TNREG);
   for (j=0, ns=sp[0], i=1; i <= ns; i++)
   { 
      k = sp[i];
      if (k == STderef)
         continue;
      if (IS_DEREF(STflag[k-1]))
         k = STpts2[k-1];
      assert(STname[k-1]);
      if (!FindInShortList(j, sp+1, k))
         sp[j++] = k;
   }
   ns = j;
   fprintf(fpout, "   Scalars Used in Loop: %d\n", ns);
   for (i=0; i < ns; i++)
   { 
      k = sp[i]-1;
      j = STflag[k];
      j = FLAG2TYPE(j);
      switch(j)
      {
         case T_FLOAT:
         case T_VFLOAT:
            pre = 's';
            break;
         case T_DOUBLE:
         case T_VDOUBLE:
            pre = 'd';
            break;
         case T_INT:
            pre = 'i';
            break;
         default:
            fko_error(__LINE__, "Unknown type %d, file %s", j, __FILE__);
      }

      fprintf(fpout, "      '%s': type=%c", STname[k], pre);
      /*CountArrayAccess(lp->blocks, sp[i], &k, &j);*/
      CountVarAccess(lp->blocks, sp[i], &k, &j);
      fprintf(fpout, " uses=%d sets=%d", k, j);
#if 0      
      if (j == k)
         j = VarIsAccumulator(lp->blocks, sp[i]);
      else
         j = 0;
      fprintf(fpout, " accum=%d", j);
#else
      if (j) /* all SE vars is set inside loop */
      {
         if (VarIsAccumulator(lp->blocks, sp[i]))
            j = 1;
         else if (VarIsMaxOrMin(lp->blocks, sp[i], 1, 1))
            j = 1;
         else j = 0;   
      }
      else j = 0;

      fprintf(fpout, " ReduceExpandable=%d", j);
#endif
      fprintf(fpout, "\n");
   }
/*
 * Kill all tem data structure
 */
   KillBitVec(set);
   KillBitVec(use);
   KillBitVec(var);
   free(sp);
}
int FindNumIFs(BLIST *scope)
{
   int count;
   BLIST *bl;
   INSTQ *ip;

   count = 0;
   for (bl = scope; bl; bl = bl->next)
   {
      for (ip = bl->blk->ainst1; ip; ip = ip->next)
      {
         if (IS_COND_BRANCH(ip->inst[0]))
            count++;
      }
   }
   return(count);
}

void FeedbackLoopInfo()
/*=============================================================================
OPTLOOP=1
   NUMPATHS=2
      VECTORIZABLE: 1 0
      EliminateAllBranches: [NO,MaxMin,RedComp, MaxMin RedComp]
      NUMIFS=1
         MaxEliminatedIfs=1
         MinEliminatedIfs=0
         RedCompEliminatedIfs=1
   VECTORIZATION: [NONE, LoopLvl, SpecVec, LoopLvl SpecVec]
   Moving 2D Pointers: 1
      'A': type=d sets=1 uses=3 lds=X sts=Y prefetch=1 ncol=W nreg=X nptr=Y
   Moving 1D Pointers: 1
      'X': type=d sets=1 uses=3 lds=X sts=Y prefetch=1
   Scalars Used in Loop: 2
      'amax': type=d sets=1 uses=1 ReduceExpandable=1
      'x': type=d sets=2 uses=3 ReduceExpandable=0
 *============================================================================*/
{
   int i, npaths, nifs;
   FILE *fpout=stdout;
   int MaxR, MinR, RC;
   int *pvec;
   const int nelimbr = 3;
   char *cElimBr[] = {"NO", "MaxMin", "RedComp"};
   int iElimBr[] = {0, 0, 0};
   enum eElimBr {NO, MAXMIN, REDCOMP};
   int VmaxminR, Vrc, Vspec, Vn, Vslp;
   const int nvec = 4;
   char *cVecMethod[] = {"NONE", "LoopLvl", "SpecVec", "SLP"};
   enum eVecMethod {NONE, LOOPLVL, SPECVEC, SLP};
   int iVecMethod[] = {0, 0, 0, 0};
   extern FILE *fpLOOPINFO;
   extern short STderef;
   extern BBLOCK *bbbase;
   extern int FKO_MaxPaths;
   extern LOOPQ *optloop;
   extern int VECT_FLAG;
   /*int SimpleLC=0, UR;*/
   /*ILIST *il;*/
   
   MaxR=0; MinR=0; RC=0;
   VmaxminR=0; Vrc=0; Vspec=0; Vn=0;

   pvec = calloc(FKO_MaxPaths, sizeof(int));
   assert(pvec);

   if (fpLOOPINFO)
      fpout = fpLOOPINFO;
/*
 * NOTE: Saving and Restoring FKO's State won't re-initiate some global data
 * (like: bitvect), if we restore from any states other than state 0.
 * Majedul: I redesigned save and restore function for state 0 so that there 
 * would be no memory leak.
 */
   /*RestoreFKOState(0);*/
   RestoreFKOState0();
/*
 * find out optloop info  
 */   
   GenPrologueEpilogueStubs(bbbase,0);
   NewBasicBlocks(bbbase);
   FindLoops(); 
   CheckFlow(bbbase, __FILE__, __LINE__);
   if (optloop)
   {
      fprintf(fpout, "OPTLOOP=1\n");
#if 0
/*
 *    figure out the loop structure. 
 *    NOTE: Assume optloop as a simple loop using HIL LOOP syntrax     
 */
      lp = optloop;
      KillLoopControl(lp);
      il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
      if (il)
      {
         if (AlreadySimpleLC(lp))
            SimpleLC = 1;
         KillIlist(il);
      }
      else
         SimpleLC = 1;
      fprintf(fpout, "   LoopNormalForm=%d\n", SimpleLC);
/*
 *    Restoring state0 ... ... ...
 */
      RestoreFKOState0();
      GenPrologueEpilogueStubs(bbbase,0);
      NewBasicBlocks(bbbase);
      FindLoops(); 
      CheckFlow(bbbase, __FILE__, __LINE__);
#endif      
/*
 *    Findout Path information.
 *    NOTE: always kill path table after doing the path analysis. Here, it is 
 *    done inside FindNumPaths() function
 */
      npaths = FindNumPaths(optloop);
      nifs = FindNumIFs(optloop->blocks) - 1; /* 1 for loop itself */

      if (npaths > 1)
      {
/*
 *       Check Whether it is reducable by Max/Min
 */
         UpdateOptLoopWithMaxMinVars();
         MaxR = ElimMaxMinIf(1,0);
         MinR = ElimMaxMinIf(0,1); /*rule out all maxs before  */
/*
 *       does max/min eliminate all paths
 */   
/*
 *       NOTE: SpeculativeVectorAnalysis can be applied on single path too.  
 */
         iElimBr[MAXMIN] = MaxR | MinR;
      
         #if defined(ArchHasVec)
            if (FindNumPaths(optloop) == 1 && iElimBr[MAXMIN] )
            {
               VmaxminR = !(SpeculativeVectorAnalysis()); 
               iVecMethod[LOOPLVL] = VmaxminR; 
               KillPathTable();
            }
         #endif
/*
 *       Restore to state 0 to perform next transformation 
 */
         RestoreFKOState0();
         GenPrologueEpilogueStubs(bbbase,0);
         NewBasicBlocks(bbbase);
         FindLoops(); 
         CheckFlow(bbbase, __FILE__, __LINE__);
         
         if (!CFUSETU2D || 1)
         {
            CalcInsOuts(bbbase);
            CalcAllDeadVariables();
         }
/*
 *       update loop info 
 */
         UpdateOptLoopWithMaxMinVars();
/*
 *       Apply Redundant Computation to eliminate branches
 *       NOTE: we can apply shadow VRC for AVX2.
 */
         #ifdef AVX
            if (optloop->LMU_flag & LMU_UNSAFE_RC) /* UNSAFE_RC mark up*/
               RC = 0;
            else
               RC = !(IterativeRedCom());
         #else
/*
 *          Not supported on SSE now, although SSE4.1 introduce blend operation.
 */
            RC = 0;
#        endif
         iElimBr[REDCOMP] = RC;
         #if defined(ArchHasVec)
            if (RC && !(VECT_FLAG & VECT_INTRINSIC) )
            {
/*
 *             We make RcVec analysis relax to support shadow RC. Make sure to
 *             call RcVectorization too to check error
 *             NOTE: shadow RC only works in X86_64 now, not in 32 bit
 */
               Vrc = !(RcVectorAnalysis(optloop)); /* checking for shadow RC too */
               #ifdef X86_32
                  if (Vrc && (VECT_FLAG & VECT_SHADOW_VRC) )
                     Vrc = 0; /* can't vect on X86_32*/
                  else if (Vrc)
               #else
                  if (Vrc)
               #endif
                     Vrc = !RcVectorization(optloop);
               iVecMethod[LOOPLVL] = Vrc;
               KillPathTable();
            }
         #endif
      }
/*
 *    Now, we will check other vectorization methods 
 *    Restoring to state0
 */
      RestoreFKOState0();
      GenPrologueEpilogueStubs(bbbase,0);
      NewBasicBlocks(bbbase);
      FindLoops(); 
      CheckFlow(bbbase, __FILE__, __LINE__);
     
      #if defined(ArchHasVec)
         if (IsSpeculationNeeded())
         {
            if ( !(VECT_FLAG & VECT_INTRINSIC) )
            {
               Vspec = !(SpeculativeVectorAnalysis()); /* most general */
               iVecMethod[SPECVEC] = Vspec;
               if (Vspec)
                  for (i=0; i < npaths; i++)
                     pvec[i] = PathVectorizable(i+1);
               KillPathTable();
            }
         }
         else
         {  
            if ( !(VECT_FLAG & VECT_INTRINSIC) )
            {
/*
 *             FIXED: Restore the state 0 code. SpecVec analysis may change the 
 *             original code
 */
               RestoreFKOState0();
               GenPrologueEpilogueStubs(bbbase,0);
               NewBasicBlocks(bbbase);
               FindLoops(); 
               CheckFlow(bbbase, __FILE__, __LINE__);
/*
 *             use SpeculativeVectorAnalysis to analyse the loop with one path
 *             if you use RcVectorAnalysis, be sure to apply RcVectorization and
 *             check for error too.
 */
            /*Vn = !(RcVectorAnalysis());*/ /* checking for shadow RC too */
               Vn = !(SpeculativeVectorAnalysis()); /*more restrictive tesdt */
               iVecMethod[LOOPLVL] = Vn;
/*
 *             check for slp vectorization
 *             Restore state0 first
 */
               RestoreFKOState0();
               GenPrologueEpilogueStubs(bbbase,0);
               NewBasicBlocks(bbbase);
               FindLoops(); 
               CheckFlow(bbbase, __FILE__, __LINE__);

               /*Vslp = !SlpVectorization();*/
               VECT_FLAG |= VECT_SLP; /* need to specify the vect for optloop*/
               Vslp = !LoopNestVec();
               iVecMethod[SLP] = Vslp;

            }
         }
      
         if (Vn || Vrc || VmaxminR || Vslp)
         {
            if (!Vspec)
               pvec[0] = 1; /* vectorizable, so default 1 for first path */
         }
      #endif
/*
 *    set none, if no other is applicable
 */
      iElimBr[NO] = 1; /* NO is the first one */
      for (i=1; i < nelimbr; i++)
      {
         if (iElimBr[i])
         {
            iElimBr[NO] = 0;
            break;
         }
      }
      iVecMethod[NONE] = 1;
      for (i=1; i < nvec; i++)
      {
         if (iVecMethod[i]) 
         {
            iVecMethod[NONE] = 0; 
            break;
         }
      }
/*
 *    Now print all information related paths
 */
      fprintf(fpout, "   NUMPATHS=%d\n",npaths);
      if (npaths > 1)
      {
         fprintf(fpout, "      VECTORIZABLE:");
         for (i=0; i < npaths; i++)
            fprintf(fpout, " %d", pvec[i]);
         fprintf(fpout, "\n");
         fprintf(fpout, "      EliminatedAllBranches:");
         for (i=0; i < nelimbr; i++)
            if (iElimBr[i]) 
               fprintf(fpout, " %s", cElimBr[i]);
         fprintf(fpout, "\n");
         fprintf(fpout, "      NUMIFS=%d\n",nifs);
         if (nifs)
         {
            fprintf(fpout, "         MaxEliminatedIfs=%d\n",MaxR);
            fprintf(fpout, "         MinEliminatedIfs=%d\n",MinR);
            fprintf(fpout, "         RedCompEliminatedIfs=%d\n",RC);
         }
      }
      fprintf(fpout, "   VECTORIZATION:");
      for (i=0; i < nvec; i++)
         if (iVecMethod[i]) 
            fprintf(fpout, " %s", cVecMethod[i]);
      fprintf(fpout, "\n");
      /*
       *    Start checking for moving ptr
       */
      RestoreFKOState0();
      GenPrologueEpilogueStubs(bbbase,0);
      NewBasicBlocks(bbbase);
      FindLoops(); 
      CheckFlow(bbbase, __FILE__, __LINE__);
      PrintMovingPtrAnalysis(fpout);     
   }

   if (fpout != stdout && fpout != stderr)
      fclose(fpout);
#if 0   
   if (fpLOOPINFO)
   {
/*
 *    Just to kill every thing
 */
      /*KillAllGlobalData();*/
      exit(0);
   }
#endif
   return;   
}
#if 0
void PrintLoopInfo()
/*
 * Figures out Loop info needed for info file, always starting from stage 2
 */
{
   LOOPQ *lp;
   ILIST *il;
   BLIST *bl;
   INSTQ *ip;
   short *sp;
   struct ptrinfo *pi, *pi0;
   FILE *fpout=stdout;
   int SimpleLC=0, UR, N, Npf, i, j, k, vect, ns;
   INT_BVI set, use, var;
   char pre;
   extern FILE *fpLOOPINFO;
   extern short STderef;

   if (fpLOOPINFO)
      fpout = fpLOOPINFO;

   fprintf(fpout, "NCACHES=%d\n", NCACHE);
   fprintf(fpout, "   LINESIZES :");
   for (i=0; i < NCACHE; i++)
      fprintf(fpout, " %d", LINESIZE[i]);
   fprintf(fpout, "\n");
   RestoreFKOState(0);
   DoStage2(0, 0);
   if (optloop)
   {
      fprintf(fpout, "OPTLOOP=1\n");
      UR = optloop->maxunroll;
/*
 *    Right now, UnrollLoop always succeeds or dies, so don't query if it
 *    actually works or not
 */
#if 0
      if (!UR)
      {
         UR = UnrollLoop(optloop, 2);
         if (UR)
           UR = 1;
         RestoreFKOState(0);
         DoStage2(0, 0);
      }
#endif
      fprintf(fpout, "   MaxUnroll=%d\n", UR);
      if (!UR || UR > 1)
      {
         RestoreFKOState(0);
#if 0         
         vect = VectorizeStage1();
         if (!vect)
            vect = VectorizeStage3(0,0);
         vect = !vect;
#else
/*
 *       Need to apply appropriate analyzer..
 */
#if 1
         GenPrologueEpilogueStubs(bbbase, 0);
         NewBasicBlocks(bbbase);
         FindLoops();
         CheckFlow(bbbase,__FILE__,__LINE__);
#endif
         if (IsSpeculationNeeded())
         {
            vect = SpeculativeVectorAnalysis();
            vect = !vect;
         }
         else
         {
            vect = VectorizeStage1();
            if (!vect)
               vect = VectorizeStage3(0,0);
            vect = !vect;
         }
#endif
         RestoreFKOState(0);
         DoStage2(0, 0);
      }
      lp = optloop;
      KillLoopControl(lp);
      il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
      if (il)
      {
         if (AlreadySimpleLC(lp))
            SimpleLC = 1;
         KillIlist(il);
      }
      else
         SimpleLC = 1;
      fprintf(fpout, "   LoopNormalForm=%d\n   Vectorizable=%d\n", 
              SimpleLC, vect);
/*
 *    Find all vars set & used in loop
 */
      set = NewBitVec(32);
      use = NewBitVec(32);
      for (bl=lp->blocks; bl; bl = bl->next)
      {
         CalcUseSet(bl->blk);
         for (ip=bl->blk->ainst1; ip; ip = ip->next)
         {
            if (ip->set)
               set = BitVecComb(set, set, ip->set, '|');
            if (ip->use)
               use = BitVecComb(use, use, ip->use, '|');
         }
      }
      pi0 = FindMovingPointers(lp->blocks);
      for (N=0,pi=pi0; pi; pi = pi->next)
         if (IS_FP(STflag[pi->ptr-1]))
            N++;
      fprintf(fpout, "   Moving FP Pointers: %d\n", N);
      for (pi=pi0; pi; pi = pi->next)
      {
         i = pi->ptr-1;
         if (IS_FP(STflag[i]))
         {
            fprintf(fpout, "      '%s': type=%c", STname[i]?STname[i]:"NULL",
                    IS_FLOAT(STflag[i]) ? 's' : 'd');
            j = ((pi->flag | PTRF_CONTIG | PTRF_INC) == pi->flag);
            if (lp->nopf)
               if (FindInShortList(lp->nopf[0], lp->nopf+1, i+1))
                  j = 0;
            fprintf(fpout, " prefetch=%d", j);
            CountArrayAccess(lp->blocks, i+1, &k, &j);
            fprintf(fpout, " sets=%d uses=%d\n", j, k);
         }
      }
/*
 *    sets + use - regs - ptrs = scalars in loop
 */
      var = BitVecComb(0, set, use, '|');
/*
 *    Sub off all registers
 */
      for (i=0; i < TNREG; i++)
         SetVecBit(var, i, 0);
/*
 *    Sub off all moving ptrs
 */
      for (pi=pi0; pi; pi = pi->next)
         SetVecBit(var, pi->ptr-1+TNREG, 0);
      KillAllPtrinfo(pi0);

/*
 *    Find all scalars used in loop
 */
      sp = BitVec2Array(var, 1-TNREG);
      for (j=0, ns=sp[0], i=1; i <= ns; i++)
      {
         k = sp[i];
         if (k == STderef)
            continue;
         if (IS_DEREF(STflag[k-1]))
            k = STpts2[k-1];
         assert(STname[k-1]);
         if (!FindInShortList(j, sp+1, k))
            sp[j++] = k;
      }
      ns = j;
      fprintf(fpout, "   Scalars used in loop: %d\n", ns);
      for (i=0; i < ns; i++)
      {
         k = sp[i]-1;
         j = STflag[k];
         j = FLAG2TYPE(j);
         switch(j)
         {
         case T_FLOAT:
         case T_VFLOAT:
            pre = 's';
            break;
         case T_DOUBLE:
         case T_VDOUBLE:
            pre = 'd';
            break;
         case T_INT:
            pre = 'i';
            break;
         default:
            fko_error(__LINE__, "Unknown type %d, file %s", j, __FILE__);
         }

         fprintf(fpout, "      '%s': type=%c", STname[k], pre);
         CountArrayAccess(lp->blocks, sp[i], &k, &j);
         fprintf(fpout, " sets=%d uses=%d", j, k);
         if (j == k)
            j = VarIsAccumulator(lp->blocks, sp[i]);
         else
            j = 0;
         fprintf(fpout, " accum=%d", j);
         fprintf(fpout, "\n");
      }
      free(sp);
   }
   else
      fprintf(fpout, "OPTLOOP=0\n");

   if (fpout != stderr && fpout != stderr)
      fclose(fpout);
   if (fpLOOPINFO)
      exit(0);
}
#endif

short *DeclareAE(int VEC, int ne, short STi)
/*
 * Declare ne-1 extra vars for accum expans on var STi
 */
{
   int type, i, j;
   short *sp, k;
   char ln[1024];

   sp = malloc(ne*sizeof(short));
   sp[0] = ne-1;
   type = FLAG2TYPE(STflag[STi-1]);
   if (VEC)
   {
      if (type == T_FLOAT)
         type = T_VFLOAT;
      else if (type == T_DOUBLE)
         type = T_VDOUBLE;
   }
   for (i=1; i < ne; i++)
   {
      sprintf(ln, "_AE_%s_%d", STname[STi-1], i);
      k = sp[i] = STdef(ln, type | LOCAL_BIT | UNKILL_BIT, 0);
/*
 *    FIXME:  
 *    AddDerefEntry func will change the SToff pointer itselt. But compiler
 *    can't track this up and tried to store value using the old pinter!
 *    This can be solved by breaking the statement down1!
 *
 *    Interesting findings: 
 *    Why messed up here? Adding the previous _AE_... the SToff went to the 
 *    border line (k=256, means updated the [255] entry), now adding the DT 
 *    entry will overflow the table. This problem will only show up where 
 *    STdef() is called with the last entry and AddDerefEntry() is called 
 *    with the following syntax!!
 *    
 */
      /*SToff[k-1].sa[2] = AddDerefEntry(-REG_SP, k, -k, 0, k);*/
      j = AddDerefEntry(-REG_SP, k, -k, 0, k);
      SToff[k-1].sa[2] = j; 
   }
   return(sp);
}

void AddInstToPrehead(LOOPQ *lp, INSTQ *iadd, short type, short r0, short r1)
/*
 * Adds insts iadd to loop preheader without renaming registers
 * Adds them as last active instruction, but before any jump
 */
{
   BBLOCK *bp;
   INSTQ *ipn, *ip, *ipA;
   short s0, s1;
   int i;

   bp = lp->preheader;
   ipn = bp->ainstN;
      
/*
 * If we must rename registers
 */
   if (r0)
   {
      s0 = -GetReg(type);
      if (!r1)
      {
         r1 = r0;
         s1 = s0;
      }
      else
      {
         s1 = -GetReg(type);
      }
      for (ip=iadd; ip; ip = ip->next)
      {
         for (i=1; i < 4; i++)
         {
            if (ip->inst[i] == r0)
               ip->inst[i] = s0;
            else if (ip->inst[i] == r1)
               ip->inst[i] = s1;
         }
      }
   }
   if (ipn)
   {
      if (!IS_BRANCH(ipn->inst[0]))
         ipn = NULL;
   }
   for (ipA=iadd; ipA; ipA = ipA->next)
   {
      ip = InsNewInst(bp, NULL, ipn, ipA->inst[0], ipA->inst[1], ipA->inst[2], 
                      ipA->inst[3]);
   }
   GetReg(-1);
}

void AddInstToPosttail(LOOPQ *lp, INSTQ *iadd, short type, short r0, short r1)
/*
 * Adds insts iadd to loop posttail as the first active instruction, after label
 */
{
   INSTQ *ip, *ipp;
   BLIST *bl;
   BBLOCK *bp;
   short s0, s1;
   int i;
/* 
 * See if we need to rename registers
 */
   if (!iadd)
      return;
   if (r0)
   {
/*
 *    NOTE: later change this to calc live reg & choose dead
 */
      s0 = -GetReg(type);
      if (!r1)
      {
         r1 = r0;
         s1 = s0;
      }
      else
         s1 = -GetReg(type);
      for (ip=iadd; ip; ip = ip->next)
      {
         for (i=1; i < 4; i++)
         {
            if (ip->inst[i] == r0)
               ip->inst[i] = s0;
            else if (ip->inst[i] == r1)
               ip->inst[i] = s1;
         }
      }
   }
/*
 * Add inst to post-tails
 */
   for (bl=lp->posttails; bl; bl = bl->next)
   {
      bp = bl->blk;
      ip = iadd;
      if (bp->ilab)
         ipp = InsNewInst(bp, bp->ainst1, NULL, ip->inst[0], ip->inst[1], 
                          ip->inst[2], ip->inst[3]);
      else
         ipp = InsNewInst(bp, NULL, bp->ainst1, ip->inst[0], ip->inst[1], 
                          ip->inst[2], ip->inst[3]);
      CalcThisUseSet(ipp);
      for (ip=iadd->next; ip; ip = ip->next)
      {
         ipp = InsNewInst(bp, ipp, NULL, ip->inst[0], ip->inst[1], 
                          ip->inst[2], ip->inst[3]);
         CalcThisUseSet(ipp);
      }
   }
   GetReg(-1);
}

INSTQ *GetAEHeadTail(LOOPQ *lp, short ae, short ne, short *aes, int vec)
/*
 * RETURNS: dummy instq, where prev pts to inst to be added to the loop header,
 *          and next is the reduction to be added to loop tail
 *          inst[0] is type, inst[1] is dummy reg1, inst[2] dummy reg2
 */
{
   enum inst zero, add, ld, st;
   INSTQ *ibase, *ip;
   int i, j, k, i1, i2, type;
   short r0, r1;

   ibase = NewInst(NULL, NULL, NULL, 0, 0, 0, 0);
   type = FLAG2TYPE(STflag[ae]);
   if (vec)
   {
      if (type == T_FLOAT)
         type = T_VFLOAT;
      else if (type == T_DOUBLE)
         type = T_VDOUBLE;
   }
   ibase->inst[0] = type;
   switch(type)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      add = FADD;
      zero = FZERO;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      add = FADDD;
      zero = FZEROD;
      break;
   case T_VFLOAT:
      ld = VFLD;
      st = VFST;
      add = VFADD;
      zero = VFZERO;
      break;
   case T_VDOUBLE:
      ld = VDLD;
      st = VDST;
      add = VDADD;
      zero = VDZERO;
      break;
   default:
      fko_error(__LINE__, "Unknown type file %s", __FILE__);
      assert(0);
   }
   r0 = -TNREG-1;
   r1 = -TNREG-2;
/*
 * Zero shadow accumulators in loop header
 */
   ibase->prev = ip = NewInst(NULL, NULL, NULL, COMMENT, 
                      STstrconstlookup("Begin shadow accum init"), 0, 0);
   ip->next = NewInst(NULL, ip, NULL, zero, r0, 0, 0);
   ip = ip->next;
   for (i=1; i < ne; i++)
   {
      ip->next = NewInst(NULL, ip, NULL, st, SToff[aes[i]-1].sa[2], r0, 0);
      ip = ip->next;
   }
   ip->next = NewInst(NULL, ip, NULL, COMMENT, 
                      STstrconstlookup("End shadow accum init"), 0, 0);
/* 
 * These are the registers that should be changed to a dead reg during insertion
 */
   ibase->inst[1] = r0;
   ibase->inst[2] = r1;
/*
 * Use binary tree to reduce aes[:] to ae
 */
   assert(aes[0] == ne-1);
   aes[0] = ae;
   for (j=0, i=ne; i; i >>= 1) j++;
   if (1<<(j-1) == ne)
      j--;

   ibase->next = NewInst(NULL, NULL, NULL, COMMENT, 
                         STstrconstlookup("Begin accum reduce"), 0, 0);
   ip = ibase->next;
   for (i=1; i <= ne; i <<= 1, j--)
   {
      for (k=0; k < j; k++)
      {
         i1 = k*(i+i);
         i2 = i1 + i;
         if (i2 < ne)
         {
            i1 = aes[i1];
            i2 = aes[i2];
            ip->next = NewInst(NULL, ip, NULL, ld, r0, SToff[i1-1].sa[2], 0);
            ip = ip->next;
            ip->next = NewInst(NULL, ip, NULL, ld, r1, SToff[i2-1].sa[2], 0);
            ip = ip->next;
            ip->next = NewInst(NULL, ip, NULL, add, r0, r0, r1);
            ip = ip->next;
            ip->next = NewInst(NULL, ip, NULL, st, SToff[i1-1].sa[2], r0, 0);
            ip = ip->next;
         }
      }
   }
   ip->next = NewInst(NULL, ip, NULL, COMMENT, 
                      STstrconstlookup("End accum reduce"), 0, 0);
   aes[0] = ne-1;
   return(ibase);
}

int DoAccumExpansOnLoop(LOOPQ *lp, short type, short ae, short *aes)
{
   BLIST *bl;
   INSTQ *ip;
   int j, ne, nchanges=0;
   short dt;

   ne = aes[0]+1;
   dt = SToff[ae-1].sa[2];
/*
 * Note, this is not necessarily in loop order, but what the hell
 */
   for (j=0,bl=lp->blocks; bl; bl = bl->next)
   {
/*
 *    For AE to be legal, every load must be followed by an add, which must
 *    be followed by a store, so just change the loads & stores
 */
/*
 *    Majedul: It works for simple format. To make it work, keep the loopbody
 *    into a single block.
 *
 *    FIXME: in case of redundant vector computation, there may be more than
 *    one blks which need to be duplicated for unrolled. in that case, load and
 *    store may not be in same blk. 
 *    Question: need AE in that case? PrintLoopInfo provides status for state0
 *    code but after vectorization, it may change!!!
 */
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (IS_LOAD(ip->inst[0]) && ip->inst[2] == dt && j%ne)
         {
            ip->inst[2] = SToff[aes[j%ne]-1].sa[2];
            CalcThisUseSet(ip);
         }
         if (IS_STORE(ip->inst[0]) && ip->inst[1] == dt)
         {
            if (j%ne)
            {
               nchanges++;
               ip->inst[1] = SToff[aes[j%ne]-1].sa[2];
               CalcThisUseSet(ip);
            }
            j++;
         }
      }
   }
   return(nchanges);
}

int DoAllAccumExpansion(LOOPQ *lp, int unroll, int vec)
/*
 * Inserts inst required for Accumulator Expansion
 * NOTE: assumes called after loop unrolling, but before repeatable opt
 *       DeclareAE must have been called before Stage[1,3]
 */
{
   int n, i, k, ae, nchanges=0;
   INSTQ *ipb;

   for (n=lp->ae[0],i=1; i <= n; i++)
   {
      ae = lp->ae[i];
      if (vec)
      {
         k = FindInShortList(optloop->vscal[0], optloop->vscal+1, ae);
         assert(k);
         ae = optloop->vvscal[k];
      }
      if (unroll % (lp->aes[i-1][0]+1))
         fko_warn(__LINE__, "UNROLL=%d, but NACCEXP=%d!", unroll, 
                  (lp->aes[i-1][0]+1));
      ipb = GetAEHeadTail(lp, ae, lp->aes[i-1][0]+1, lp->aes[i-1], vec);
      AddInstToPrehead(lp, ipb->prev, ipb->inst[0], ipb->inst[1], 0);
      KillAllInst(ipb->prev);
      AddInstToPosttail(lp, ipb->next, ipb->inst[0],ipb->inst[1],ipb->inst[2]);
      KillAllInst(ipb->next);
      nchanges += DoAccumExpansOnLoop(lp, ipb->inst[0], ae, lp->aes[i-1]);
      ipb->prev = ipb->next = NULL;
      KillThisInst(ipb);
   }
   CFUSETU2D = INDEADU2D = 0;
/*   fprintf(stderr, "ACCEXP, nchanges=%d\n\n", nchanges); */
   return(nchanges);
}

/*
 * Majedul: Generalizing the AccumExpansion with VarExpansion
 * Will support Accumulator and Max/Min variable right now.
 * NOTE: I have implemented Expansion for Max/Min separately, later I will
 * merge it with Accum.
 */

short *DeclareMaxE(int VEC, int ne, short STi)
/*
 * Declare ne-1 extra vars for max expans on var STi
 */
{
   int type, i, j;
   short *sp, k;
   char ln[1024];
   sp = malloc(ne*sizeof(short));
   sp[0] = ne-1;
   type = FLAG2TYPE(STflag[STi-1]);
   if (VEC)
   {
      if (type == T_FLOAT)
         type = T_VFLOAT;
      else if (type == T_DOUBLE)
         type = T_VDOUBLE;
   }
   for (i=1; i < ne; i++)
   {
      sprintf(ln, "_MAXE_%s_%d", STname[STi-1], i);
      k = sp[i] = STdef(ln, type | LOCAL_BIT | UNKILL_BIT, 0);
/*
 *    Majedul: see bug report on DeclareAE function 
 */
      /*SToff[k-1].sa[2] = AddDerefEntry(-REG_SP, k, -k, 0, k);*/
      j = AddDerefEntry(-REG_SP, k, -k, 0, k);
      SToff[k-1].sa[2] = j;
   }
   return(sp);
}

short *DeclareMinE(int VEC, int ne, short STi)
/*
 * Declare ne-1 extra vars for max expans on var STi
 */
{
   int type, i, j;
   short *sp, k;
   char ln[1024];

   sp = malloc(ne*sizeof(short));
   sp[0] = ne-1;
   type = FLAG2TYPE(STflag[STi-1]);
   if (VEC)
   {
      if (type == T_FLOAT)
         type = T_VFLOAT;
      else if (type == T_DOUBLE)
         type = T_VDOUBLE;
   }
   for (i=1; i < ne; i++)
   {
      sprintf(ln, "_MINE_%s_%d", STname[STi-1], i);
      k = sp[i] = STdef(ln, type | LOCAL_BIT | UNKILL_BIT, 0);
/*
 *    Majedul: see bug report on DeclareAE function 
 */
      /*SToff[k-1].sa[2] = AddDerefEntry(-REG_SP, k, -k, 0, k);*/
      j = AddDerefEntry(-REG_SP, k, -k, 0, k);
      SToff[k-1].sa[2] = j;
   }
   return(sp);
}

/*============================================================================
 * Generalization of Scalar Expansion
 *
 * ==========================================================================*/

INSTQ *GetSEHeadTail(LOOPQ *lp, short se, short ne, short *ses, int vec, 
                     int sflag)
/*
 * RETURNS: dummy instq, where prev pts to inst to be added to the loop header,
 *          and next is the reduction to be added to loop tail
 *          inst[0] is type, inst[1] is dummy reg1, inst[2] dummy reg2
 */
{
   enum inst zero, inst, ld, st, vsld, vshuf;
   INSTQ *ibase, *ip;
   int i, j, k, i1, i2, type;
   short r0, r1;

   ibase = NewInst(NULL, NULL, NULL, 0, 0, 0, 0);
   type = FLAG2TYPE(STflag[se]);
   if (vec)
   {
      if (type == T_FLOAT)
         type = T_VFLOAT;
      else if (type == T_DOUBLE)
         type = T_VDOUBLE;
   }
   ibase->inst[0] = type;
   switch(type)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      if (sflag & SC_ACC) inst = FADD;
      else if (sflag & SC_MAX) inst = FMAX;
      else if (sflag & SC_MIN) inst = FMIN;
      else ;
      zero = FZERO;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      if (sflag & SC_ACC) inst = FADDD;
      else if (sflag & SC_MAX) inst = FMAXD;
      else if (sflag & SC_MIN) inst = FMIND;
      else ;
      zero = FZEROD;
      break;
   case T_VFLOAT:
      ld = VFLD;
      vsld = VFLDS;
      st = VFST;
      vshuf = VFSHUF;
      if (sflag & SC_ACC) inst = VFADD;
      else if (sflag & SC_MAX) inst = VFMAX;
      else if (sflag & SC_MIN) inst = VFMIN;
      else ;
      zero = VFZERO;
      break;
   case T_VDOUBLE:
      ld = VDLD;
      vsld = VDLDS;
      st = VDST;
      vshuf = VDSHUF;
      if (sflag & SC_ACC) inst = VDADD;
      else if (sflag & SC_MAX) inst = VDMAX;
      else if (sflag & SC_MIN) inst = VDMIN;
      else ;
      zero = VDZERO;
      break;
   default:
      fko_error(__LINE__, "Unknown type file %s", __FILE__);
      assert(0);
   }
/*
 * Dummy regs... ... !!!! 
 */
   r0 = -TNREG-1;
   r1 = -TNREG-2;
/*
 * Shadow initialization actuallly depends on instruction type 
 * ACCUM -> 0 but for Max/Min -> init value 
 */
#if 0   
   fprintf(stderr, "inst = %s\n", instmnem[inst]);
#endif
/*
 * Zero shadow accumulators in loop header
 */
   if (sflag & SC_ACC) 
   {
      ibase->prev = ip = NewInst(NULL, NULL, NULL, COMMENT, 
                         STstrconstlookup("Begin shadow accum init"), 0, 0);
      /*ip->next = NewInst(NULL, ip, NULL, zero, r0, 0, 0);*/
   }
   else /* option left: Max/Min */
   {
      if(sflag & SC_MAX)
         ibase->prev = ip = NewInst(NULL, NULL, NULL, COMMENT, 
                            STstrconstlookup("Begin shadow max init"), 0, 0);
      else if (sflag & SC_MIN)
         ibase->prev = ip = NewInst(NULL, NULL, NULL, COMMENT, 
                            STstrconstlookup("Begin shadow max init"), 0, 0);
/*
 *    load value of original max/min variable, shuf if vec
 *    NOTE: se is scalar var whether ses[i] may be vector 
 */
      if (vec)
      {
         ip->next = NewInst(NULL, ip, NULL, vsld, r0, SToff[se-1].sa[2], 0);
         ip = ip->next;
         ip->next = NewInst(NULL, ip, NULL, vshuf, r0, r0, STiconstlookup(0));
         ip = ip->next;
      }
      else
      {
         ip->next = NewInst(NULL, ip, NULL, ld, r0, SToff[se-1].sa[2], 0);
         ip = ip->next;
      }
   }

   for (i=1; i < ne; i++)
   {
      if (sflag & SC_ACC)
      {
         ip->next = NewInst(NULL, ip, NULL, zero, r0, 0, 0);
         ip = ip->next;
      }
      ip->next = NewInst(NULL, ip, NULL, st, SToff[ses[i]-1].sa[2], r0, 0);
      ip = ip->next;
   }
   
   if (sflag & SC_ACC)
      ip->next = NewInst(NULL, ip, NULL, COMMENT, 
                         STstrconstlookup("End shadow accum init"), 0, 0);
   else if (sflag & SC_MAX)
      ip->next = NewInst(NULL, ip, NULL, COMMENT, 
                         STstrconstlookup("End shadow max init"), 0, 0);
   else if (sflag & SC_MIN)
      ip->next = NewInst(NULL, ip, NULL, COMMENT, 
                         STstrconstlookup("End shadow min init"), 0, 0);
   else ;
   
/* 
 * These are the registers that should be changed to a dead reg during insertion
 */
   ibase->inst[1] = r0;
   ibase->inst[2] = r1;
/*
 * Use binary tree to reduce aes[:] to ae
 */
   assert(ses[0] == ne-1);
   ses[0] = se;

   if (sflag & SC_ACC) 
      ibase->next = NewInst(NULL, NULL, NULL, COMMENT, 
                           STstrconstlookup("Begin shadow accum reduce"), 0, 0);
   else if (sflag & SC_MAX) 
      ibase->next = NewInst(NULL, NULL, NULL, COMMENT, 
                           STstrconstlookup("Begin shadow max reduce"), 0, 0);
   else if (sflag & SC_MIN) 
      ibase->next = NewInst(NULL, NULL, NULL, COMMENT, 
                           STstrconstlookup("Begin shadow min reduce"), 0, 0);
   else;
/*
 * FIXED: Majedul: there was a problem with the reduction. It didn't work with
 * ne >= 8. I fixed that by changing the condition of inner loop. 
 */
#if 0
   for (j=0, i=ne; i; i >>= 1) j++;
   if (1<<(j-1) == ne)
      j--;
#endif
   ip = ibase->next;
   j = 1;
   /*for (i=1; i <= ne; i <<= 1, j--)*/
   for (i=1; i < ne; i <<= 1, j--)
   {
      /*for (k=0; k <= j; k++)*/
      j = j << 1;
      for (k=0; k < ne/j; k++)
      {
         i1 = k*(i+i);
         i2 = i1 + i;
         if (i2 < ne)
         {
            i1 = ses[i1];
            i2 = ses[i2];
            ip->next = NewInst(NULL, ip, NULL, ld, r0, SToff[i1-1].sa[2], 0);
            ip = ip->next;
            ip->next = NewInst(NULL, ip, NULL, ld, r1, SToff[i2-1].sa[2], 0);
            ip = ip->next;
            ip->next = NewInst(NULL, ip, NULL, inst, r0, r0, r1);
            ip = ip->next;
            ip->next = NewInst(NULL, ip, NULL, st, SToff[i1-1].sa[2], r0, 0);
            ip = ip->next;
         }
      }
   }
   if (sflag & SC_ACC) 
      ip->next = NewInst(NULL, ip, NULL, COMMENT, 
                         STstrconstlookup("End shadow accum reduce"), 0, 0);
   else if (sflag & SC_MAX) 
      ip->next = NewInst(NULL, ip, NULL, COMMENT, 
                         STstrconstlookup("End shadow max reduce"), 0, 0);
   else if (sflag & SC_MIN) 
      ip->next = NewInst(NULL, ip, NULL, COMMENT, 
                         STstrconstlookup("End shadow min reduce"), 0, 0);
   else ;

   ses[0] = ne-1;
   return(ibase);
}

int DoScalExpansOnLoop(LOOPQ *lp, short type, short se, short *ses)
{
   BLIST *bl;
   INSTQ *ip;
   int j, ne, nchanges=0;
   short dt;

   ne = ses[0]+1;
   dt = SToff[se-1].sa[2];
/*
 * Note, this is not necessarily in loop order, but what the hell
 */
   for (j=0,bl=lp->blocks; bl; bl = bl->next)
   {
/*
 *    For AE to be legal, every load must be followed by an add, which must
 *    be followed by a store, so just change the loads & stores
 */
/*
 *    Majedul: It works for simple format. To make it work, keep the loopbody
 *    into a single block.
 *
 *    FIXME: in case of redundant vector computation, there may be more than
 *    one blks which need to be duplicated for unrolled. in that case, load and
 *    store may not be in same blk. 
 *    Question: need AE in that case? PrintLoopInfo provides status for state0
 *    code but after vectorization, it may change!!!
 */
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (IS_LOAD(ip->inst[0]) && ip->inst[2] == dt && j%ne)
         {
            ip->inst[2] = SToff[ses[j%ne]-1].sa[2];
            CalcThisUseSet(ip);
         }
         if (IS_STORE(ip->inst[0]) && ip->inst[1] == dt)
         {
            if (j%ne)
            {
               nchanges++;
               ip->inst[1] = SToff[ses[j%ne]-1].sa[2];
               CalcThisUseSet(ip);
            }
            j++;
         }
      }
   }
   return(nchanges);
}


int DoAllScalarExpansion(LOOPQ *lp, int unroll, int vec)
/*
 * Inserts inst required for Scalar Expansion
 * NOTE: assumes called after loop unrolling, but before repeatable opt
 *       DeclareAE must have been called before Stage[1,3]
 */
{
   int n, i, k, se, nchanges=0;
   INSTQ *ipb;

   for (n=lp->se[0],i=1; i <= n; i++)
   {
      se = lp->se[i];
      if (vec)
      {
         k = FindInShortList(optloop->vscal[0], optloop->vscal+1, se);
         assert(k);
         se = optloop->vvscal[k];
      }
      if (unroll % (lp->ses[i-1][0]+1))
         fko_warn(__LINE__, "UNROLL=%d, but NSCLEXP=%d!", unroll, 
                  (lp->ses[i-1][0]+1));
      ipb = GetSEHeadTail(lp, se, lp->ses[i-1][0]+1, lp->ses[i-1], vec, 
                          lp->seflag[i]);
#if 0
      fprintf(stderr, "insts for SE (preheader )\n\n");
      PrintThisInstQ(stderr, ipb->prev);
      fprintf(stderr, "insts for SE (posttail)\n\n");
      PrintThisInstQ(stderr, ipb->next);
#endif      
      AddInstToPrehead(lp, ipb->prev, ipb->inst[0], ipb->inst[1], 0);
      KillAllInst(ipb->prev);
      AddInstToPosttail(lp, ipb->next, ipb->inst[0],ipb->inst[1],ipb->inst[2]);
      KillAllInst(ipb->next);
      nchanges += DoScalExpansOnLoop(lp, ipb->inst[0], se, lp->ses[i-1]);
      ipb->prev = ipb->next = NULL;
      KillThisInst(ipb);
   }
   CFUSETU2D = INDEADU2D = 0;
/*   fprintf(stderr, "SCLEXP, nchanges=%d\n\n", nchanges); */
   return(nchanges);
}

/*============================================================================
 * Majedul: 
 * Transformations of State1 is implemented here as primarily those are applied
 * on optloop now but they can be applied in anywhere.  
 *============================================================================*/

void FinalizeVectorCleanup(LOOPQ *lp, int unroll)
/*
 * this function will finalize the vector cleanup adding instructions to test
 * for cleanup after CF_VRED_END CMPFLAG. 
 * NOTE: Need to change UnrollCleanup to identify these and update the test 
 * accordingly.
 * NOTE: loop peeling might change the loop control information. So, both
 * GenCleanupLoop and FinalizeVectorCleanup should be called before Looppeeling.
 * NOTE: need to parametrize the cleanup for larger bet unrolling... now,
 * this function and UnrollCleanup bacomes similar....
 * HERE HERE why not use the same cleanup function for both vector and unroll!!
 */
{
   BBLOCK *bp;
   INSTQ *ipnext;
   int FORWARDLOOP;
   short r0, r1;

   if (unroll < 1) unroll = 1;
/*
 * Cleanup should already be generated before finalizing it
 */
#if 0   
   assert(lp->CU_label > 0);
#else
   if (lp->CU_label == -1)
      return;
#endif
   r0 = GetReg(T_INT);
   r1 = GetReg(T_INT);
/*
 * If flag's loop control not set, compute it, then set boolean based on flag
 * NOTE: loop peeling might change the loop control information. So, both
 * GenCleanupLoop and FinalizeVectorCleanup should be called before Looppeeling.
 */
   SetLoopControlFlag(lp, 0);
   FORWARDLOOP = L_FORWARDLC_BIT & lp->flag;
   unroll *= Type2Vlen(lp->vflag);  
/*
 * Require one and only one post-tail; later do transformation to ensure this
 * for loops where it is not natively true
 */
   assert(lp->posttails && !lp->posttails->next);
/*
 * put cleanup test after vector reduction
 */   
   bp = lp->posttails->blk;
   ipnext = FindCompilerFlag(bp, CF_VRED_END);
   assert(ipnext);
   ipnext = ipnext->next;
   
   if (FORWARDLOOP)
   {
/*
 *    If we've used unrolled forward loop, restore N to original value
 */
      /*fprintf(stderr, "\n\nForward loop !!!\n");*/
      if (!IS_CONST(STflag[lp->end-1]))
      {
         InsNewInst(bp, NULL, ipnext, LD, -r1, SToff[lp->end-1].sa[2], 0);
         InsNewInst(bp, NULL, ipnext, ADD, -r1, -r1, 
                            STiconstlookup(unroll*SToff[lp->inc-1].i));
         InsNewInst(bp, NULL, ipnext, ST, SToff[lp->end-1].sa[2], -r1, 0);
      }
      else assert(0); /* FIXME: need to consider const too */
      InsNewInst(bp, NULL, ipnext, LD, -r0, SToff[lp->I-1].sa[2], 0);
      if (IS_CONST(STflag[lp->end-1]))
         InsNewInst(bp, NULL, ipnext, CMP, -ICC0, -r0, lp->end);
      else
      {
         InsNewInst(bp, NULL, ipnext, LD, -r1, SToff[lp->end-1].sa[2], 0);
         InsNewInst(bp, NULL, ipnext, CMP, -ICC0, -r0, -r1);
      }
   }
   else
   {
      InsNewInst(bp, NULL, ipnext, LD, -r0, SToff[lp->I-1].sa[2], 0);
      InsNewInst(bp, NULL, ipnext, SUBCC, -r0, -r0,
                 STiconstlookup(-(FKO_abs(SToff[lp->inc-1].i)*unroll-1)));
      InsNewInst(bp, NULL, ipnext, ST, SToff[lp->I-1].sa[2], -r0, 0);
   }
   InsNewInst(bp, NULL, ipnext, JNE, -PCREG, -ICC0, lp->CU_label);
/*
 * Add label to jump back to when cleanup is done (screws up block, of course)
 */
   InsNewInst(bp, NULL, ipnext, LABEL, lp->PTCU_label, 0, 0);
   GetReg(-1);
}

void UnrollCleanup2(LOOPQ *lp, int unroll)
/*
 * this is actually a modified version of previous unroll cleanup function
 * NOTE: this cleanup can be merged with FinalizeVectorCleanup function later
 */
{    
   BBLOCK *bp;
   INSTQ *ipnext, *ip;
   int FORWARDLOOP;
   short r0, r1;
   extern int VECT_FLAG;

   if (lp->CU_label == -1)
      return;
/*
 * Generate the actual code to do loop cleanup
 */
   if (lp->CU_label == 0)
      GenCleanupLoop(lp);

   r0 = GetReg(T_INT);
   r1 = GetReg(T_INT);
/*
 * If flag's loop control not set, compute it, then set boolean based on flag
 */
/*
 * Majedul: it is used in many places. So, I use that as a function. 
 */
   SetLoopControlFlag(lp, 0);
   FORWARDLOOP = L_FORWARDLC_BIT & lp->flag;
   
   if (!(VECT_FLAG & VECT_INTRINSIC))
      unroll *= Type2Vlen(lp->vflag);  /* need to update Type2Vlen for AVX*/
/*
 * Require one and only one post-tail; later do transformation to ensure this
 * for loops where it is not natively true
 */
   assert(lp->posttails && !lp->posttails->next);
/*
 * Put cleanup info before 1st non-label instruction in posttail, unless
 * we are doing vectorization, in which case put it after all live-out
 * vectors are reduced
 */
   bp = lp->posttails->blk;
   if (DO_VECT(FKO_FLAG))
   {
      ipnext = FindCompilerFlag(bp, CF_VRED_END);
      assert(ipnext);
/*
 *    Majedul: need to delete previous cleanup test instruction
 *    HERE HERE 
 *    assuming : FinalizeVectorCleanup is already called  and
 *    instructions after CF_VRED_END upto branch is just for cleanup. 
 *    NOTE: keeping track with CMPFLAG will not work as adding branch may 
 *    screw up the block structure.
 */
#if 0
      extern BBLOCK *bbbase;
      fprintf(stdout, "LIL before removing the cu_tst\n");
      PrintInst(stdout, bbbase);
#endif      
      ip = ipnext->next;
      while (ip && !IS_COND_BRANCH(ip->inst[0]))
            ip = RemoveInstFromQ(ip);
      assert(ip && ip->inst[3] == lp->CU_label);
      ipnext = RemoveInstFromQ(ip); /* remove the branch too */      
   }
   else if (bp->ainst1 && bp->ainst1->inst[0] == LABEL)
      ipnext = bp->ainst1->next;
   else
      ipnext = bp->ainst1;
   
   if (FORWARDLOOP)
   {
/*
 *    If we've used unrolled forward loop, restore N to original value
 */
      /*fprintf(stderr, "\n\nForward loop !!!\n");*/
      if (!IS_CONST(STflag[lp->end-1]))
      {
         InsNewInst(bp, NULL, ipnext, LD, -r1, SToff[lp->end-1].sa[2], 0);
         /*InsNewInst(bp, NULL, ipnext, ADD, -r1, -r1, 
                            STiconstlookup(unroll*SToff[lp->inc-1].i-1));*/
         InsNewInst(bp, NULL, ipnext, ADD, -r1, -r1, 
                            STiconstlookup(unroll*SToff[lp->inc-1].i));
         InsNewInst(bp, NULL, ipnext, ST, SToff[lp->end-1].sa[2], -r1, 0);
      }
      InsNewInst(bp, NULL, ipnext, LD, -r0, SToff[lp->I-1].sa[2], 0);
      if (IS_CONST(STflag[lp->end-1]))
         InsNewInst(bp, NULL, ipnext, CMP, -ICC0, -r0, lp->end);
      else
         InsNewInst(bp, NULL, ipnext, CMP, -ICC0, -r0, -r1);
   }
   else
   {
      InsNewInst(bp, NULL, ipnext, LD, -r0, SToff[lp->I-1].sa[2], 0);
      InsNewInst(bp, NULL, ipnext, SUBCC, -r0, -r0,
                 STiconstlookup(-(FKO_abs(SToff[lp->inc-1].i)*unroll-1)));
      InsNewInst(bp, NULL, ipnext, ST, SToff[lp->I-1].sa[2], -r0, 0);
   }
   InsNewInst(bp, NULL, ipnext, JNE, -PCREG, -ICC0, lp->CU_label);
/*
 * Add label to jump back to when cleanup is done (screws up block, of course)
 * NOTE: If FinalizeVectorCleanup is called before, it is already added.
 */

   if (!DO_VECT(FKO_FLAG))
      InsNewInst(bp, NULL, ipnext, LABEL, lp->PTCU_label, 0, 0);
   
   GetReg(-1);
}

int ListElemCount(BLIST *blist)
{
   BLIST *bl;
   int i;
   for (i=0, bl = blist; bl; bl = bl->next) i++;
   return i; 
}

void PrintLoop(FILE *fpout, LOOPQ *lp)
/*
 * Print necessary basic info for loop
 */
{
   int i, nlp;
   extern LOOPQ *loopq;
   LOOPQ *lpq; 
   
   lpq = loopq;
   nlp = 0;
   while (lpq)
   {  
      nlp++;
      lpq = lpq->next;
   }

   fprintf(fpout, "\nNumber of Loop: %d\n", nlp);
   fprintf(fpout, "Optloop = LOOP-%d\n", lp->loopnum);
   for (i=0, lpq=loopq; lpq; i++, lpq=lpq->next)
   {
      fprintf(fpout, "LOOP-%d:\n", i+1); 
      fprintf(fpout, "\tloopnum = %d\n", lpq->loopnum);
      fprintf(fpout, "\tloop depth = %d\n", lpq->depth);
      fprintf(fpout, "\tloop_body = %s\n", 
            STname[lpq->body_label-1]?  STname[lpq->body_label-1]: "NULL");
      fprintf(fpout, "\tHead: %d\n", lpq->header->bnum);
      fprintf(fpout, "\tTails: %s\n", PrintBlockList(lpq->tails));
      if(lpq->preheader)  
         fprintf(fpout, "\tPreHeader: %d\n", lpq->preheader->bnum);
      else
         fprintf(fpout, "\tPreHeader: NULL\n");
      fprintf(fpout, "\tPostTails: %s\n", PrintBlockList(lpq->posttails));
      fprintf(fpout, "\tBLOCKS[%d]: %s\n",ListElemCount(lpq->blocks), 
           PrintBlockList(lpq->blocks));

      fprintf(fpout, "\tloop index var = %s\n", 
            lpq->I?(STname[lpq->I-1]?  STname[lpq->I-1]: "NULL"): "0");

   }
#if 0
   fprintf(fpout, "OPTLOOP INFO: \n");
   fprintf(fpout, "========================================================\n");
   fprintf(fpout, "Loop #%d\n", lp->loopnum);
   fprintf(fpout, "Depth: %d\n", lp->depth);
   fprintf(fpout, "Duplication: %d\n", lp->ndup);
   fprintf(fpout, "BLOCKS[%d]: %s\n",ListElemCount(lp->blocks), 
           PrintBlockList(lp->blocks));
   fprintf(fpout, "PreHeader: %d\n", lp->preheader->bnum);
   fprintf(fpout, "Head: %d\n", lp->header->bnum);
   fprintf(fpout, "Tails: %s\n", PrintBlockList(lp->tails));
   fprintf(fpout, "PostTails: %s\n", PrintBlockList(lp->posttails));
   fprintf(fpout, "========================================================\n");
#endif
}


/*============================================================================
 *    MAX/MIN REDUCTION:
 *
 *
 * ==========================================================================*/

int CheckMaxMinReduceConstraints(BLIST *scope, short var, short label)
/*
 * this function checks the ifblk whether anyother variable is set inside that
 * blk. That means, this if blk is only to compute the max/min var; then we can
 * reduce the ifblk with single max/min inst. And also no nested if-blk  
 */
{
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip, *ip0;

 /*fprintf(stderr, "check for maxvar=%s, lable=%d\n", STname[maxvar-1],label);*/

   for (bl=scope; bl; bl=bl->next)
   {
      bp = bl->blk;
/*
 *    Assumption: if blk starts with active inst LABEL
 */
      ip0 = bp->ainst1;
      if (ip0 && ip0->inst[0] == LABEL && ip0->inst[1] == label)
      {
         for (ip = ip0; ip; ip=ip->next)
         {
/*
 *          Need to check for all stores, not just one type      
 */
            if ( IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]-1]!= var)
               return(0);
/*
 *          if-blk shouldn't have nested if; check for the conditional if-blk
 */
            if (IS_COND_BRANCH(ip->inst[0]))
               return(0);
         }
      }
   }
   return 1;
}

void RemoveInstFromLabel2Br(BLIST *scope, short label)
{
   BLIST *bl;
   BBLOCK *bp;
   INSTQ *ip;

   for (bl=scope; bl; bl=bl->next)
   {
      bp = bl->blk;
      for (ip=bp->inst1; ip; ip=ip->next)
      {
         if (ip->inst[0] == LABEL && ip->inst[1] == label)
         {
            while (ip && ip->inst[0]!=JMP)
            {  
               assert(!IS_COND_BRANCH(ip->inst[0]));
               ip = RemoveInstFromQ(ip);
            }
            assert(ip->inst[0] == JMP);
            RemoveInstFromQ(ip);
         }
      }
   }
   
}

int ElimIFBlkWithMaxMin(BLIST *scope, short var, int ismax)
/*
 * var can be maxvar or minvar depending on ismax. 
 * eliminate the if-blk with max/min inst; returns 1 if successful, 0 otherwise
 * ASSUMPTION: scope is the blist of the loop
 */
{
   int i;
   short regv, regx, label;
   enum inst inst, st, cmp;
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl;
   BBLOCK *bp;

   i = FLAG2TYPE(STflag[var-1]);
   switch(i)
   {
      case T_FLOAT:
         st = FST;
         cmp = FCMP;
         inst = ismax? FMAX : FMIN;
         break;
      case T_DOUBLE:
         st = FSTD;
         cmp = FCMPD;
         inst = ismax? FMAXD : FMIND;
         break;
/*
 *    not support for vector intrinsic
 */
      case T_VFLOAT:
      case T_VDOUBLE:
         fko_warn(__LINE__, 
            "MAX/MIN var can't be vector type: not supported for intrinsic\n");
         return(0);
   }
/*
 * NOTE:
 * we have already tested whether the var is max/min. So, we don't need to 
 * retest it. But we need to test one extra condition to reduce the if-blk: 
 * no other var cannot be set in if-blk; that means, this if-blk is only to
 * compute the max/min. 
 */
   for (bl = scope; bl; bl = bl->next)
   {
      bp = bl->blk;
      for (ip = bp->ainst1; ip; ip = ip->next)
      {
         if (IS_COND_BRANCH(ip->inst[0]) 
               && ip->prev && ip->prev->inst[0] == cmp)
         {
            ip0 = NULL;
            ip1 = ip->prev->prev;           /* 2nd ld */
            if(ip1) 
                  ip0 = ip->prev->prev->prev;     /* 1st ld */
/*
 *          the cond branch is to check x and max/min var without any arithmatic
 *          in between them
 */
            if (ip0 && ip1 && IS_LOAD(ip0->inst[0]) && IS_LOAD(ip1->inst[0]))
            {
               if (STpts2[ip0->inst[2]-1] == var)
               {
                  regv = ip0->inst[1];
                  regx = ip1->inst[1];
               }
               else if (STpts2[ip1->inst[2]-1] == var)
               {
                  regv = ip1->inst[1];
                  regx = ip0->inst[1];
               }
               else 
                  continue; /* not a cond with var, continue to next ip */
               label =  ip->inst[3]; 
               if (CheckMaxMinReduceConstraints(scope, var, label)) 
               {
/*
 *                insert inst after ip1 but before ip1->next
 */
                  ip1 = InsNewInst(bp, ip1, NULL, inst, regv, regv, regx);
                  ip1 = InsNewInst(bp, ip1, NULL, st, SToff[var-1].sa[2],
                                   regv, 0);
/*
 *                delete inst upto the cond branch
 */
                  ip1 = ip1->next; /* old ip1->next */
                  while (ip1 && !IS_COND_BRANCH(ip1->inst[0])) 
                     ip1 = RemoveInstFromQ(ip1);
                  assert(IS_COND_BRANCH(ip1->inst[0]));
                  ip1 = RemoveInstFromQ(ip1); /* delete the branch itself */
/*
 *                delete the if-blk altogether
 */
                  RemoveInstFromLabel2Br(scope, label);
                  return(1); 
               }
            }
         }
      }
   }
   return(0);
}

int ElimMaxMinIf(int maxelim, int minelim)
/*
 * Depending on the ismaxvar/isminvar, we will eliminate 'if' for that var. 
 * if both set, we will try to eliminate 'if' for both type of var. 
 */
{
   int i, N;
   short *scal;
   LOOPQ *lp;
   int changes;

   changes = 0;
   lp = optloop;
/*
 * NOTE: find all fp scalar variables
 */
   scal = FindAllScalarVars(lp->blocks);   

/*
 *  check for max elim
 */
   if (maxelim)
   {
      for (N = scal[0], i=1; i <= N; i++)
         if ( VarIsMaxOrMin(lp->blocks, scal[i], 1, 0) )
            changes += ElimIFBlkWithMaxMin(lp->blocks, scal[i], 1);
   }

/*
 *  check for min elim
 */
   if (minelim)
   {
      for (N = scal[0], i=1; i <= N; i++)
         if ( VarIsMaxOrMin(lp->blocks, scal[i], 0, 1) )
            changes += ElimIFBlkWithMaxMin(lp->blocks, scal[i], 0);
   }

   if (scal) free(scal);
/*
 * re-construct the CFG 
 */
   if (changes)
   {
      CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = CFLOOP = 0;
      InvalidateLoopInfo();
      bbbase = NewBasicBlocks(bbbase);
      CheckFlow(bbbase, __FILE__, __LINE__);
      FindLoops();
      CheckFlow(bbbase, __FILE__, __LINE__);
      return (changes);
   }

   return (0);  
}

int AddMaxMinInst(BBLOCK *bp, INSTQ *ipnext, short mvar, short xvar, int ismax)
{
   int type;
   short reg0, reg1;
   enum inst ld, st, inst;

   type = FLAG2TYPE(STflag[mvar-1]);
   switch(type)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      inst = ismax ? FMAX: FMIN;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      inst = ismax ? FMAXD: FMIND;
      break;
   default:
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_warn(__LINE__,"Unknown type=%d, file=%s. Should be done before Vect",
                type, __FILE__);
      return(0);
   }
   
   reg0 = GetReg(type);
   reg1 = GetReg(type);
  
   InsNewInst(bp, NULL, ipnext, ld, -reg0, 
         SToff[mvar-1].sa[2], 0); 
   InsNewInst(bp, NULL, ipnext, ld, -reg1, 
         SToff[xvar-1].sa[2], 0);
   InsNewInst(bp, NULL, ipnext, inst, -reg0, -reg0, -reg1);
   InsNewInst(bp, NULL, ipnext, st, SToff[mvar-1].sa[2],
         -reg0, 0);
   GetReg(-1);
   return(1);
}

int RemoveVarFromIFBlk(BBLOCK *ifblk, short var)
{
   INSTQ *ip, *ip1, *ip2;
   
   for (ip = ifblk->ainst1; ip; ip = ip->next)
   {
      if (IS_STORE(ip->inst[0]) && STpts2[ip->inst[1]-1] == var)
      {
/* 
 *       Just a plain assignment, no other operation
 */
         if (ip->prev && IS_LOAD(ip->prev->inst[0]))
         {
            ip1 = ip->prev;
            while (IS_LOAD(ip1->inst[0]))
            {
               ip2 = ip1->prev;
               RemoveInstFromQ(ip1);
               ip1 = ip2;
            }
/*
 *          remove this assignment inst
 */
            ip = RemoveInstFromQ(ip);
            return(1);
         }
      }
   }
   return(0);
}

int MovOneMaxMinVarOut(BLIST *scope, short mvar, int ismax)
{
   int i;
   short xvar;
   /*short label;*/
   enum inst cmp;
   /*enum inst inst, ld, st;*/
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl;
   BBLOCK *bp, *splitblk, *ifblk, *elseblk, *mergeblk;

   i = FLAG2TYPE(STflag[mvar-1]);
   switch(i)
   {
      case T_FLOAT:
         cmp = FCMP;
         /*ld = FLD;
         st = FST;
         inst = ismax? FMAX : FMIN;*/
         break;
      case T_DOUBLE:
         cmp = FCMPD;
         /*ld = FLDD;
         st = FSTD;
         inst = ismax? FMAXD : FMIND;*/
         break;
/*
 *    not support for vector intrinsic
 */
      case T_VFLOAT:
      case T_VDOUBLE:
         fko_warn(__LINE__, 
            "MAX/MIN var can't be vector type: not supported for intrinsic\n");
         return(0);
   }
/*
 * NOTE:
 * we have already tested whether the var is max/min. So, we don't need to 
 * retest it. But we need to test one extra condition to reduce the if-blk: 
 * no other var cannot be set in if-blk; that means, this if-blk is only to
 * compute the max/min. 
 */
   for (bl = scope; bl; bl = bl->next)
   {
      bp = bl->blk;
      for (ip = bp->ainst1; ip; ip = ip->next)
      {
         if (IS_COND_BRANCH(ip->inst[0]) 
               && ip->prev && ip->prev->inst[0] == cmp)
         {
            ip0 = NULL;
            ip1 = ip->prev->prev;           /* 2nd ld */
            if(ip1) 
                  ip0 = ip->prev->prev->prev;     /* 1st ld */
/*
 *          the cond branch is to check x and max/min var without any arithmatic
 *          in between them
 */
            if (ip0 && ip1 && IS_LOAD(ip0->inst[0]) && IS_LOAD(ip1->inst[0]))
            {
               if (STpts2[ip0->inst[2]-1] == mvar)
               {
                  xvar = ip1->inst[2];
               }
               else if (STpts2[ip1->inst[2]-1] == mvar)
               {
                  xvar = ip0->inst[2];
               }
               else 
                  continue; /* not a cond with var, continue to next ip */
               /*label =  ip->inst[3];*/
/*
 *             max/min var is already tested before. so, we don't test here 
 *             find conditional blk structure
 */
               splitblk = bp;
               ifblk = splitblk->csucc;
               if (ifblk->usucc != splitblk->usucc)
               {
                  elseblk = splitblk->usucc;
                  mergeblk = elseblk->usucc;
               }
               else 
               {
                  elseblk = NULL;
                  mergeblk = splitblk->usucc;
               }
               if (mergeblk != ifblk->usucc)
               {
                  fko_warn(__LINE__,
                        "Doesn't follow required if-else structure\n");
                  continue;
               }
/*
 *             have required structure. now time to remove the var
 */
               if (RemoveVarFromIFBlk(ifblk, mvar))
               {
                  if (AddMaxMinInst(mergeblk, 
                                    (mergeblk->ainst1->inst[0]==LABEL)?
                                    mergeblk->ainst1->next : mergeblk->ainst1 , 
                                    mvar, STpts2[xvar-1], ismax))
                     return(1);
                  
               }
            }
         }
      }
   }
   return(0);

}

int MovMaxMinVarsOut(int movmax, int movmin)
{
   int i, N;
   short *scal;
   LOOPQ *lp;
   int changes;

   changes = 0;
   lp = optloop;
   
   scal = FindAllScalarVars(lp->blocks);   
   for (N = scal[0], i=1; i <= N; i++)
   {
      if (movmax && VarIsMaxOrMin(lp->blocks, scal[i], 1, 0))
      {
         #if 0
            fprintf(stderr, "Max var = %s\n", STname[scal[i]-1]);  
         #endif
         changes += MovOneMaxMinVarOut(lp->blocks, scal[i], 1);
      }
      else if (movmin && VarIsMaxOrMin(lp->blocks, scal[i], 0, 1))
      {
         #if 0
            fprintf(stderr, "Min var = %s\n", STname[scal[i]-1]);
         #endif
         changes += MovOneMaxMinVarOut(lp->blocks, scal[i], 0);
      }
   }
   if (scal) free(scal);
/*
 * re-construct the CFG 
 */
   if (changes)
   {
      CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = CFLOOP = 0;
      InvalidateLoopInfo();
      bbbase = NewBasicBlocks(bbbase);
      CheckFlow(bbbase, __FILE__, __LINE__);
      FindLoops();
      CheckFlow(bbbase, __FILE__, __LINE__);
#if 0
      fprintf(stderr, "LIL: \n");
      PrintInst(stderr, bbbase);
      ShowFlow("cfg.dot", bbbase);
#endif
      return (changes);
   }

   return (0);  

}

/*=============================================================================
 *    REDUNDANT COMPUTATION:
 *
 *  
 *============================================================================*/

short *UpdateBlkWithNewVar(BBLOCK *bp0, int vid, short *sp)
/*
 * take a var list (format: N, v1, v2,..) and blk; rename and update with
 * new var and returns the newvar list in same format
 */
{
   int i, j, k, N;
   short sv, nv, op;
   short *newvars; 
   char *strnvar;
   INSTQ *ip;
   
/*
 * allocate space for newvars
 */
#if 0
   newvars = calloc(sp[0]+1, sizeof(short));
#else
   newvars = malloc(sizeof(short)*(sp[0]+1));
   assert(newvars);
   for (i=0; i < (sp[0]+1); i++)
      newvars[i] = 0;
#endif
/*
 * Findout the first set of var, update the destination and update all later
 * use of the var as dest/src
 */

   for ( N=sp[0], i=1; i <= N; i++ )
   {
      sv = sp[i];
      for (ip = bp0->ainst1, j=1; ip; ip=ip->next, j++)
      {
         op = ip->inst[1];
         if (op > 0)
         {
            k = SToff[op-1].sa[1]; /* index of original ST */
            /*fprintf(stderr, "%d: %d\n", j, k);*/
            if (k == sv)
            {
               /*fprintf(stderr, " got 1st set for %s at inst %d\n", 
                       STname[sv-1], j);*/
               strnvar = malloc(sizeof(char)*(strlen(STname[sv-1])+4));
               assert(strnvar);
               sprintf(strnvar,"%s_%d",STname[sv-1],vid);
               nv = InsertNewLocal(strnvar,STflag[sv-1]);
               free(strnvar); /* string copied and stored in ST */
               newvars[i] = nv; /* update the param */
               /*ip->inst[1] = nv;*/
               ip->inst[1] = SToff[nv-1].sa[2];
               break;
            }
         }
      }
      /* findout the use of this var in remaining instruction */
      if (!ip) continue;
      for (ip=ip->next; ip; ip=ip->next)
      {
         for (j=1; j < 4; j++)
         {
            op = ip->inst[j];
            if (op > 0)
            {
               k = SToff[op-1].sa[1]; /* index of original ST */
               if (k == sv)
               {
                  /*ip->inst[j] = nv;*/
                  ip->inst[j] = SToff[nv-1].sa[2];
               }
            }
         }
      }
   }
   return newvars;
}

short RemoveBranchWithMask(BBLOCK *sblk)
/*
 * removes conditional branches from a split blk with appropriate CMP and mask 
 */
{
   int k;
   INSTQ *ip, *ip0;
   short mask;
   static int maskid = 0;
   char *cmask;
   int type;
   enum inst fst;
/*
 * NOTE: Although SSE doesn't support GT and GE, we can implement those with
 * NLT, NLE
 */
   static enum inst
      brinsts[] = {JEQ, JNE, JLT, JLE, JGT, JGE},
      fcmpwinsts[] = {FCMPWEQ, FCMPWNE, FCMPWLT, FCMPWLE, FCMPWGT, FCMPWGE},
      dcmpwinsts[] = {FCMPDWEQ, FCMPDWNE, FCMPDWLT,FCMPDWLE, FCMPDWGT, 
                      FCMPDWGE};
#if 0      
      #if defined(AVX)
         fcmpwinsts[] = {FCMPWEQ, FCMPWNE, FCMPWLT, FCMPWLE, FCMPWGT, FCMPWGE},
         dcmpwinsts[] = {FCMPDWEQ, FCMPDWNE, FCMPDWLT,FCMPDWLE, FCMPDWGT, 
                         FCMPDWGE};
      #else
/*
 *    SSE supports : EQ, NE, LT, LE, NLT, NLE
 *    not supports : GT, GE, NGT, NGE
 *    So, we need to replace GT and GE with NLE and NLT
 */
         fcmpwinsts[] = {FCMPWEQ, FCMPWNE, FCMPWLT, FCMPWLE, FCMPWNLE, 
                         FCMPWNLT},
         dcmpwinsts[] = {FCMPDWEQ, FCMPDWNE, FCMPDWLT, FCMPDWLE, FCMPDWNLE, 
                         FCMPDWNLT};
      #endif
#endif

   enum inst *cmpinsts;      
   int nbr;

   nbr = 6; mask = 0;  
   for (ip=sblk->ainst1; ip; ip=ip->next)
   {
      if (IS_COND_BRANCH(ip->inst[0]))
      {
/*
 *       Right now, we don't expect cmp without float/double
 *       format: 
 *                CMP -FCC0, -freg0, -freg1
 *                BR -PCREG, -FCC0, LABEL
 */
         assert((ip->inst[2] == -FCC0));
         ip0 = ip->prev;
/*
 *       Assuming that this conditional branch is not optimized (unlike 
 *       loopcontrol). So, there must be a CMP inst. We need that to modify 
 *       this to CMPW, writing result on mask. 
 */
         assert(IS_CMP(ip0->inst[0]) && (ip0->inst[1]==-FCC0));
         if (ip0->inst[0] == FCMP)
         {
            type = T_FLOAT;
            cmpinsts = fcmpwinsts;
            fst = FST;
         }
         else if (ip0->inst[0] == FCMPD)
         {
            type = T_DOUBLE;
            cmpinsts = dcmpwinsts;
            fst = FSTD;
         }
         else assert(0);
/*
 *       creating mask local variable to store the result of CMP
 */
         cmask = (char*)malloc(sizeof(char)*128);
         assert(cmask);
         sprintf(cmask,"mask_%d",++maskid);
         mask = InsertNewLocal(cmask,type);
         free(cmask); /* string copied and stored in ST */
/*
 *       changing the CMP with CMPW, remove the branch
 *       NOTE: of course, it will messed up the CFG  but we will change the CFG
 *       anyway after this translation.
 */
         for (k=0; k < nbr; k++)
            if (brinsts[k] == ip->inst[0])
               break;
         assert(k!=nbr);
         
         ip0->inst[0] = cmpinsts[k];
/*
 *       NOTE: we keep op1 and op2 same which will work for both AVX and SSE
 */
         /*ip0->inst[1] = SToff[mask-1].sa[2];*/
         ip0->inst[1] = ip0->inst[2]; /* update value in a reg */
         CalcThisUseSet(ip0);
         
         ip0 = InsNewInst(sblk, ip0, NULL, fst, SToff[mask-1].sa[2], 
                          ip0->inst[1], 0);
         CalcThisUseSet(ip0);
#if 1 
         DelInst(ip); 
#else
/*
 *       Changing the conditional branch into unconditinal
 *       format: 
 *             JMP PCREG, LABEL, 0
 *             BR  PCREG, cc#, LABEL
 */
         ip->inst[0] = JMP;
         ip->inst[2] = ip->inst[3];
         ip->inst[3] = 0;

#endif
/*
 *       HERE HERE, we consider only one conditinal branch in a block
 *       So, no need to check other instructions after getting that.
 */
         break;
      }
   }
   return mask;
}

short *FindVarsCMOVNeeded(BBLOCK *bp0)
/*
 * returns array of vars in standard format (N,s1,s2...) which needed the
 * CMOV for redundant vector transformation.
 * Assumption: All the ip->set and bp->outs are already up to date.
 */
{
   int i;
   INT_BVI iv;
   short *sp;
   INSTQ *ip;
   extern INT_BVI FKO_BVTMP;
   extern short STderef;
   extern BBLOCK *bbbase;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;   /* avoiding init of temp vec rather reuse */
   SetVecAll(iv, 0);
/*
 * NOTE: we can't recalculate the ins and outs as it may change the blk ptr
 * itself. So, make sure ip->set and bp->outs are updated before calling this
 * function.
 */
#if 0   
   if (!CFUSETU2D )
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#endif   
/*
 * figure out all vars and regs which is set in this blk
 * Take those which is liveout at the exiting of this blk
 * NOTE: only fully tested but works for the default testcase.
 */
   for (ip = bp0->inst1; ip; ip=ip->next)
   {
      iv = BitVecComb(iv, iv, ip->set, '|');
   }
#if 0
   fprintf(stderr, "bp0 = %d, bp0->outs=%d\n", bp0->bnum, bp0->outs);
   PrintThisBlockInst(stderr, bp0);
#endif
/*
 * NOTE: never bypass a bug like this, always find the cause of it
 */
   /*if(bp0->outs)
      iv = BitVecComb(iv, iv, bp0->outs, '&');*/
   iv = BitVecComb(iv, iv, bp0->outs, '&');
/*
 * clear all the regs in bvec to find out vars only
 */
   for (i=0; i < TNREG; i++)
   {
      SetVecBit(iv, i, 0);
   }
   SetVecBit(iv, STderef+TNREG-1,0);
/*
 * create array of vars (st-index) which needed CMOV
 */
   sp = BitVec2Array(iv, 1-TNREG);
   return sp;
}

short *FindVarsSetInBlock(BBLOCK *bp0)
{
   int i;
   INT_BVI iv;
   short *sp;
   INSTQ *ip;
   extern INT_BVI FKO_BVTMP;
   extern short STderef;
   extern BBLOCK *bbbase;

   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;   /* avoiding init of temp vec rather reuse */
   SetVecAll(iv, 0);
/*
 * Recalculate ins and outs, dead vars if not updated yet.
 * HERE HERE CalcInsOuts may change the bp0 itself ...
 * As we only need to check the ip->set and CalcUseSet() will not change the 
 * blk, we can safely use CalcUseSet().
 */
#if 0   
   if (!CFUSETU2D )
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
#endif

   if (!INUSETU2D)
      CalcUseSet(bp0);
/*
 * figure out all vars and regs which is set in this blk
 */
   for (ip = bp0->inst1; ip; ip=ip->next)
   {
#if 0
      if (!ip->set)
      {
         fprintf(stderr, "\nERROR: %s %d %d %d\n", instmnem[ip->inst[0]], 
                 ip->inst[1], ip->inst[2], ip->inst[3] );
         PrintThisBlockInst(stderr, bp0);
         //CalcThisUseSet(ip);
         assert(ip->set);
      }
#else
      assert(ip->set);
#endif
      iv = BitVecComb(iv, iv, ip->set, '|');
   }
/*
 * clear all the regs in bvec to find out vars only
 */
   for (i=0; i < TNREG; i++)
   {
      SetVecBit(iv, i, 0);
   }
   SetVecBit(iv, STderef+TNREG-1,0);
/*
 * create array of vars (st-index) which needed CMOV
 */
   sp = BitVec2Array(iv, 1-TNREG);
   return sp;
}

void MovInstFromBlkToBlk(BBLOCK *fromblk, BBLOCK *toblk)
/*
 * delete all inst and move inst except JMP and LABEL from that blk to a another
 * blk (at the end but before JMP). 
 * HERE HERE, don't consider conditional branch yet
 */
{
   BBLOCK *bp;
   INSTQ *ip, *ip0;

   bp = toblk;
   ip0 = toblk->instN;
   if (ip0->inst[0] == JMP) ip0 = ip0->prev; /* JMP, keep that at the end */
   
   ip = fromblk->inst1; /* inst to be moved */

   while (ip)
   {
      if (ip->inst[0] != JMP && ip->inst[0] != LABEL) 
      {
         ip0 = InsNewInst(bp, ip0, NULL, ip->inst[0], ip->inst[1], ip->inst[2],
                          ip->inst[3]);
         ip = RemoveInstFromQ(ip);
      }
      else
         ip = ip->next;
         /*ip = RemoveInstFromQ(ip);*/ /* delete all */
   }
}

int CreateIMaskVar(int Mask)
/*
 * this function creates integer mask variable from original mask 
 */
{
   int imask;
   char *cmask, *stmask;
   int type;
#if 0
   fprintf(stderr, "mask = %d: %s\n", Mask, STname[Mask-1]);
#endif
   type = T_INT;
   stmask = STname[Mask-1];
   assert(stmask);
   cmask = (char*)malloc(sizeof(char)*(strlen(stmask)+2));
   assert(cmask);
   sprintf(cmask,"i%s",stmask);
   imask = InsertNewLocal(cmask,type);
   free(cmask); /* string copied and stored in ST */
   return(imask);
}
#if 0
int RedundantScalarComputation(LOOPQ *lp)
/*
 * Assuming CFG is already constructed.
 * NOTE: Works only for single if-else blk, not nested one
 */
{
   int i, j, k, n, N;
   int type, err;
   int *vpos;
   INT_BVI iv, iv1;
   short *sp;
   
   short *isetvars, *esetvars, *icmvars, *ecmvars; /* i=if, e=else */
   short *inewvars, *enewvars;

   short nv, sv, nv1, nv2, mask;
   short freg0, freg1, freg2;
   enum inst cmov1, cmov2, fld, fst; 
   BBLOCK *bp, *bp0;
   INSTQ *ip, *ip0;
   BLIST *ifblks, *elseblks, *splitblks, *mergeblks;
   BLIST *bl;
   extern short STderef;
   extern INT_BVI FKO_BVTMP;

   splitblks = NULL;
   ifblks = NULL;
   elseblks = NULL;
   mergeblks = NULL;
   
   isetvars = esetvars = icmvars = ecmvars = inewvars = enewvars = NULL;
   sp = NULL;
   err = 0;
/*
 * identify the ifblks, elseblks and common blks
 * NOTE: loopcontrol is killed already. So, there should not be any loop branch
 * Only branch should be the branches responsiblefor  the control flow inside 
 * loop.
 * 
 * Right now, I just consider the single if-else block
 */

/*
 * Finding all split blks inside the optloop
 * NOTE: call KillLoopControl before it, otherwise catch that one also 
 */
   for (bl = lp->blocks; bl; bl = bl->next  )
   {
      bp = bl->blk;
      for (ip = bp->ainst1; ip; ip=ip->next) 
      {
         if (IS_COND_BRANCH(ip->inst[0]))
         {
            splitblks = AddBlockToList(splitblks, bp);
         }
      }
   }
/*
 * Right now, we consider single if-else block, will generalize later
 */
   assert(!splitblks->next);
/*
 * find out if and corresponding else block
 * NOTE: right now, ifblks / elseblks are the single blk. need to traverse
 * the bbbase otherwise.
 */
   ifblks = AddBlockToList(ifblks, splitblks->blk->csucc); 

   if (ifblks->blk->usucc != splitblks->blk->usucc)
      elseblks = AddBlockToList(elseblks, splitblks->blk->usucc);
   
   if (elseblks)
      bp = elseblks->blk->usucc;
   else
      bp = splitblks->blk->usucc;
   assert(bp);
   
   if (bp == ifblks->blk->usucc)
      mergeblks = AddBlockToList(mergeblks, bp);
   else assert(0);

#if 0
   fprintf(stderr, "Split Blocks: \n");
   for (bl = splitblks; bl; bl = bl->next)
      fprintf(stderr, "%d ", bl->blk->bnum);
   fprintf(stderr, "\n");
   fprintf(stderr, " if-block = %d\n", ifblks->blk->bnum);
   fprintf(stderr, " else-block = %d\n", elseblks->blk->bnum);
   fprintf(stderr, " commonblks = %d\n", mergeblks->blk->bnum);
#endif

/*
 * NOTE: We need to rename all the variables which are set inside the blks
 * even if it is not a candidate for select operation: 
 * Example: 
 * 
 * [using if-else]
 *
 * if (x < 0.0)
 * { 
 *    x = -x;
 *    sum +=x;
 * }
 * else
 *    sum+=x;
 *
 * Transformation:
 *
 * x = X[0];
 * mask1 = x < 0.0 
 *    x1 = -x;          # if x is not renamed, it doesn't work
 *    sum1 = sum + x1;
 *    sum2 = sum + x
 *    sum = mask1 ? sum1: sum2
 *
 * RULE: rename if it is set in both blks or, it is live out
 */

/*
 * finding all the variables which are set inside blks
 */
   if (ifblks) isetvars = FindVarsSetInBlock(ifblks->blk);
   if (elseblks) esetvars = FindVarsSetInBlock(elseblks->blk);
/*
 * identify all vars which need select operation
 * 1) which is set inside if/else blks
 * 2) which is liveout at the exit of the block
 */
   if (ifblks) icmvars = FindVarsCMOVNeeded(ifblks->blk);
   if (elseblks) ecmvars = FindVarsCMOVNeeded(elseblks->blk);
/*
 * Update with new vars, return new vars' st index in corresponding location
 * that means, if x var is in pos 2 in setvars, new var st will be found on the
 * same index of nvars
 */
   if(ifblks) inewvars = UpdateBlkWithNewVar(ifblks->blk, 1, isetvars);
   if (elseblks) enewvars = UpdateBlkWithNewVar(elseblks->blk, 2, esetvars);

#if 0
   sp = isetvars;
   fprintf(stderr, "vars set [ifblk]: ");
   for (N=sp[0], i=1; i <=N; i++)
      fprintf(stderr, "%s[%d] ", STname[sp[i]-1], sp[i]-1);
   fprintf(stderr, "\n");
#endif

/*
 * Update the conditional branch of split blk with VCMPW updating a mask. 
 * Conditional branch is deleted.
 */
   mask = RemoveBranchWithMask(splitblks->blk);  
/*
 * copy the if/else block at end of split block but before JMP, if any
 */
   if (ifblks) MovInstFromBlkToBlk(ifblks->blk, splitblks->blk);
   if (elseblks) MovInstFromBlkToBlk(elseblks->blk, splitblks->blk);

/*
 * find out the union of reguired vars form if/else blks
 * NOTE: avoid creating new bvec rather use tmp
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;   /* avoiding init of temp vec rather reuse */
   SetVecAll(iv, 0); 
/*
 * NOTE: BitVec2Array returns formated array[N,s1,s2...] but Array2BitVec
 * takes unformatted array without the count.
 */
   iv = BitVecCopy(iv, Array2BitVec(icmvars[0], icmvars+1, TNREG-1));
   if (elseblks)
      iv = BitVecComb(iv, iv, Array2BitVec(ecmvars[0], ecmvars+1, TNREG-1),'|');
   sp = BitVec2Array(iv, 1-TNREG);

#if 0
   fprintf(stderr, "vars set [union]: ");
   for (N=sp[0], i=1; i <=N; i++)
      fprintf(stderr, "%s[%d] ", STname[sp[i]-1], sp[i]-1);
   fprintf(stderr, "\n");
#endif
/*
 * Figure out which var is in which blk
 */
   N = sp[0]; 
   vpos = malloc(sizeof(int)*(N+1));
   vpos[0] = N;
   
   for (i=1; i <= N; i++)
   {
      if (FindInShortList(icmvars[0], icmvars+1, sp[i]))
      {
         if (elseblks)
         {
            if (FindInShortList(ecmvars[0], ecmvars+1, sp[i]))
               vpos[i] = 3;
            else
               vpos[i] = 1;
         }
         else
            vpos[i] = 1;
      }
      else
         vpos[i] = 2;
   }

#if 0
   fprintf(stderr, "vars set [union]: ");
   for (N=sp[0], i=1; i <=N; i++)
      fprintf(stderr, "%s[%d] = %d ", STname[sp[i]-1], sp[i]-1, vpos[i]);
   fprintf(stderr, "\n");
#endif

/*
 * Add select operations. two cases:
 * CASE-1: var is set in both if and else blks
 * CASE-2: var is set either of the blks but not in both
 *
 * for case-1, add select instruction at the first of the common blocks.
 * for case-2, add select inst at the end of the working blk.
 * 
 * NOTE: before inserting the select inst, the conditinal jump should be change
 * update the mask variable.....!!!!!!!!!
 *
 * KEEP IN MIND: 
 * convention of select: 
 * dest = (mask==0) ? src1 : src2   
 * that means, dest = mask? scr1: src2
 *
 * LIL:
 * FCMOV1   :  dest = (mask==0) ? dest : src   # dest is aliased with src1 
 * FCMOV2   :  dest = (mask==0) ? src : dest   # dest is aliased with src2
 *
 */

/*
 * NOTE: We should merge everything into single blk so that other optimization
 * can be performed, like: AE 
 */
            
   bp = splitblks->blk;
   ip = bp->ainstN;
   if (ip->inst[0] == JMP) ip=ip->prev;

   for (N=sp[0], i=1; i <= N; i++)
   {
      if (IS_FLOAT(STflag[sp[i]-1]))
      {
         cmov1 = FCMOV1;
         cmov2 = FCMOV2;
         fld = FLD;
         fst = FST;
         type = T_FLOAT;
      }
      else if (IS_DOUBLE(STflag[sp[i]-1]))
      {
         cmov1 = FCMOVD1;
         cmov2 = FCMOVD2;
         fld = FLDD;
         fst = FSTD;
         type = T_DOUBLE;
      }
      else  /* can't handle integer yet */
      {
         err = 1;
         break;
      }
      switch(vpos[i])
      {
         case 1:  /* if blks*/ 
/*
 *          findout corrsponding new vars
 */
            k = FindInShortList(isetvars[0], isetvars+1, sp[i]);
            nv1 = inewvars[k];
/*
 *          Now time to insert select inst
 *          new var in if: nv1 
 */
            freg0 = GetReg(type);
            freg1 = GetReg(type);
            freg2 = GetReg(type);
            ip = InsNewInst(bp, ip, NULL, fld, -freg0, SToff[sp[i]-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg1, SToff[nv1-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg2, SToff[mask-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, cmov1, -freg0, -freg1, -freg2 );
            ip = InsNewInst(bp, ip, NULL, fst, SToff[sp[i]-1].sa[2], -freg0, 0);
            GetReg(-1);
            break;
         case 2:  /* else blks.... only else possible??? */
            /*bp = elseblks->blk;*/
/*
 *          findout corrsponding new vars
 */
            k = FindInShortList(esetvars[0], esetvars+1, sp[i]);
            nv2 = inewvars[k];
/*
 *          Now time to insert select inst
 *          New var in else: nv2
 */
            freg0 = GetReg(type);
            freg1 = GetReg(type);
            freg2 = GetReg(type);
            ip = InsNewInst(bp, ip, NULL, fld, -freg0, SToff[sp[i]-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg1, SToff[nv2-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg2, SToff[mask-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, cmov1, -freg0, -freg1, -freg2 );
            ip = InsNewInst(bp, ip, NULL, fst, SToff[sp[i]-1].sa[2], -freg0, 0);
            GetReg(-1);
            break;
         case 3:  /* both in if and else blk */
            
            /*bp = mergeblks->blk;
            ip = bp->ainst1;
            if (ip->inst[0] == LABEL) ip=ip->next;
            */
/*
 *          findout corrsponding new vars
 */
            k = FindInShortList(isetvars[0], isetvars+1, sp[i]);
            nv1 = inewvars[k];
            k = FindInShortList(esetvars[0], esetvars+1, sp[i]);
            nv2 = enewvars[k];
/*
 *          Now time to insert select inst
 *          new var in if: nv1, New var in else: nv2
 */
            freg0 = GetReg(type);
            freg1 = GetReg(type);
            freg2 = GetReg(type);
            /*ip = InsNewInst(bp, NULL, ip, fld, -freg0, SToff[nv1-1].sa[2],0);*/
            ip = InsNewInst(bp, ip, NULL, fld, -freg0, SToff[nv1-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg1, SToff[nv2-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, fld, -freg2, SToff[mask-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, cmov2, -freg0, -freg1, -freg2 );
            ip = InsNewInst(bp, ip, NULL, fst, SToff[sp[i]-1].sa[2], -freg0, 0);
            GetReg(-1);
            break;
         default: ;
     }
   }
/*
 * Free all the memory which is alloated for analysis
 * NOTE: optimized the code to minimize the use of temp mem later.
 */
   if (sp) free(sp);
   if (vpos) free(vpos);
   if (isetvars) free(isetvars);
   if (esetvars) free(esetvars);
   if (icmvars) free(icmvars);
   if (ecmvars) free(ecmvars);
   if (inewvars) free(inewvars);
   if (enewvars) free(enewvars);
0   KillBlockList(ifblks);
   KillBlockList(elseblks);
   KillBlockList(splitblks);
   KillBlockList(mergeblks);
   return err; 
}
#endif

int ReduceBlkWithSelect(BBLOCK *ifblk, BBLOCK *elseblk, BBLOCK *splitblk)
/*
 * Providing Ifblk, elseblk, splitblk and mergeblk, this function will reduce
 * If/else blk. If there is no elseblk, elseblk should be NULL.
 * We parameterize this function so that we can use this function repeatably 
 * to reduce nested if-else.
 */
{
   int i, k, N;
   int type, err;
   int *vpos;
   INT_BVI iv;
   short *sp;
   
   short *isetvars, *esetvars, *icmvars, *ecmvars; /* i=if, e=else */
   short *inewvars, *enewvars;

   short nv1, nv2, mask, codemask;
   short reg0, reg1, reg2, ireg;
   enum inst cmov1, cmov2, ld, st, mskld, movmsk; 
   BBLOCK *bp;
   /*BBLOCK *mergeblk;*/
   INSTQ *ip;
   extern short STderef;
   extern INT_BVI FKO_BVTMP;

   isetvars = esetvars = icmvars = ecmvars = inewvars = enewvars = NULL;
   sp = NULL;
   err = 0;
   assert(ifblk); /* ifblk can't be NULL */

#if 0
   fprintf(stderr, "splitblk = %d, ifblk = %d, elseblk = %d\n",
         splitblk->bnum, ifblk->bnum, elseblk?elseblk->bnum:-1);
#endif   
   
/*
 * figure out the merge blk 
 */
   if (elseblk)
      bp = elseblk->usucc;
   else
      bp = splitblk->usucc;
   assert(bp); 
   if (bp == ifblk->usucc)
   {
      /*mergeblk = bp;*/
#if 0
      fprintf(stderr, "mergeblk=%d\n", mergeblk->bnum);
#endif
   }
   else assert(0);

/*
 * NOTE: We need to rename all the variables which are set inside the blks
 * even if it is not a candidate for select operation: 
 * Example: 
 * 
 * [using if-else]
 *
 * if (x < 0.0)
 * { 
 *    x = -x;
 *    sum +=x;
 * }
 * else
 *    sum+=x;
 *
 * Transformation:
 *
 * x = X[0];
 * mask1 = x < 0.0 
 *    x1 = -x;          # if x is not renamed, it doesn't work
 *    sum1 = sum + x1;
 *    sum2 = sum + x
 *    sum = mask1 ? sum1: sum2
 *
 * RULE: rename if it is set in both blks or, it is live out
 */
#if 0
   fprintf(stderr, "\nREDUCE: split=%d, if=%d, else=%d, merge=%d\n", 
           splitblk->bnum, ifblk->bnum, elseblk?elseblk->bnum:-1, 
           mergeblk->bnum);
#endif   
/*
 * finding all the variables which are set inside blks
 */
   if (ifblk) isetvars = FindVarsSetInBlock(ifblk);
   if (elseblk) esetvars = FindVarsSetInBlock(elseblk);
/*
 * identify all vars which need select operation
 * 1) which is set inside if/else blks
 * 2) which is liveout at the exit of the block
 */
   if (ifblk) icmvars = FindVarsCMOVNeeded(ifblk);
   if (elseblk) ecmvars = FindVarsCMOVNeeded(elseblk);
/*
 * Update with new vars, return new vars' st index in corresponding location
 * that means, if x var is in pos 2 in setvars, new var st will be found on the
 * same index of nvars
 */
   if(ifblk) inewvars = UpdateBlkWithNewVar(ifblk, 1, isetvars);
   if (elseblk) enewvars = UpdateBlkWithNewVar(elseblk, 2, esetvars);

#if 0
   //sp = isetvars;
   //sp = esetvars;
   sp = icmvars;
   fprintf(stderr, "vars set [ifblk]: ");
   for (N=sp[0], i=1; i <=N; i++)
      fprintf(stderr, "%s[%d] ", STname[sp[i]-1], sp[i]-1);
   fprintf(stderr, "\n");
#endif

/*
 * Update the conditional branch of split blk with VCMPW updating a mask. 
 * Conditional branch is deleted.
 */
   mask = RemoveBranchWithMask(splitblk);  
/*
 * copy the if/else block at end of split block but before JMP, if any
 */
   if (ifblk) MovInstFromBlkToBlk(ifblk, splitblk);
   if (elseblk) MovInstFromBlkToBlk(elseblk, splitblk);

/*
 * find out the union of reguired vars form if/else blks
 * NOTE: avoid creating new bvec rather use tmp
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;   /* avoiding init of temp vec rather reuse */
   SetVecAll(iv, 0); 
/*
 * NOTE: BitVec2Array returns formated array[N,s1,s2...] but Array2BitVec
 * takes unformatted array without the count (first element).
 */
   iv = BitVecCopy(iv, Array2BitVec(icmvars[0], icmvars+1, TNREG-1));
   if (elseblk)
      iv = BitVecComb(iv, iv, Array2BitVec(ecmvars[0], ecmvars+1, TNREG-1),'|');
   sp = BitVec2Array(iv, 1-TNREG);

#if 0
   fprintf(stderr, "vars set [union]: ");
   for (N=sp[0], i=1; i <=N; i++)
      fprintf(stderr, "%s[%d] ", STname[sp[i]-1], sp[i]-1);
   fprintf(stderr, "\n");
#endif
/*
 * Figure out which var is in which blk
 * NOTE: 1 => if only blk  2=> else only blk    3=> in both blks
 */
   N = sp[0]; 
   vpos = malloc(sizeof(int)*(N+1));
   vpos[0] = N;
   
   for (i=1; i <= N; i++)
   {
      if (FindInShortList(icmvars[0], icmvars+1, sp[i])) /* in if blk */
      {
         if (elseblk) /* is else blk*/
         {
            if (FindInShortList(ecmvars[0], ecmvars+1, sp[i])) /* in else blk*/
               vpos[i] = IN_BOTH_IF_ELSE;
            else
               vpos[i] = IN_IF_ONLY;
         }
         else
            vpos[i] = IN_IF_ONLY;
      }
      else
         vpos[i] = IN_ELSE_ONLY;
   }

#if 0
   fprintf(stderr, "vars set [union]: ");
   for (N=sp[0], i=1; i <=N; i++)
      fprintf(stderr, "%s[%d] = %d ", STname[sp[i]-1], sp[i]-1, vpos[i]);
   fprintf(stderr, "\n");
   exit(0);
#endif

/*
 * Add select operations. two cases:
 * CASE-1: var is set in both if and else blks
 * CASE-2: var is set either of the blks but not in both
 *
 * for case-1, add select instruction at the first of the common blocks.
 * for case-2, add select inst at the end of the working blk.
 * 
 * NOTE: before inserting the select inst, the conditinal jump should be changed
 * update the mask variable.....!!!!!!!!!
 *
 * KEEP IN MIND: 
 * convention of select: 
 * dest = (mask==0) ? src1 : src2   
 * that means, dest = mask? scr2 src1
 *
 * LIL:
 * FCMOV1   :  dest = (mask==0) ? dest : src   # dest is aliased with src1 
 * FCMOV2   :  dest = (mask==0) ? src : dest   # dest is aliased with src2
 *
 */

/*
 * NOTE: We should merge everything into single blk so that other optimization
 * can be performed, like: AE 
 */
            
   bp = splitblk;
   ip = bp->ainstN;
   if (ip->inst[0] == JMP) ip=ip->prev;

   for (N=sp[0], i=1; i <= N; i++)
   {
      if (IS_FLOAT(STflag[sp[i]-1]))
      {
         cmov1 = FCMOV1;
         cmov2 = FCMOV2;
         ld = mskld = FLD;
         st = FST;
         type = T_FLOAT;
         codemask = mask;
      }
      else if (IS_DOUBLE(STflag[sp[i]-1]))
      {
         cmov1 = FCMOVD1;
         cmov2 = FCMOVD2;
         ld = mskld = FLDD;
         st = FSTD;
         type = T_DOUBLE;
         codemask = mask;
      }
      else if (IS_INT(STflag[sp[i]-1]))/* for int */
      {
/*
 *       statement with index variable can be 
 *       new int inst needed: CMOV1, CMOV2
 */
#if 0
         err = 1;
         break;
#else
         cmov1 = CMOV1;
         cmov2 = CMOV2;
         ld = LD;
         st = ST;
         type = T_INT;

   #if 0         
/*
 *       we need to convert mask into imask (something like: vmovmskps in X86)
 *       so that it can be used in CMOV instruction.... need to find out 
 *       suitable implementation of CNOV instruction too.
 */
/*
 *       STEPS:
 *       1. find type of mask variable
 *       2. create a new mask for integer
 *       3. load orginal mask into appropriate reg
 *       4. covert to imask 
 *       5. st the imask ... use this mask to operate cmov for integer
 *
 *       FIXME: it would create problem for scalar version. Scalar cmov inst
 *       does depend on EFLAGS, not any reg/var in X86. converting the mask is 
 *       another issue... it would be complicated
 */
         imask = CreateIMaskVar(mask);
         codemask = imask;
/*
 *       insert inst to convert mask to imask
 */
         reg0 = GetReg(type);
         if (IS_FLOAT(STflag[mask-1]))
         {
            reg1 = GetReg(T_FLOAT);
            cvtmask = CVTMASKFI;
         }
         else if (IS_DOUBLE(STflag[mask-1]))
         {
            reg1 = GetReg(T_DOUBLE);
            cvtmask = CVTMASKDI; 
         }
         else assert(0);
         
         /*ip = InsNewInst(bp, ip, NULL, ld, -reg0, SToff[codemask-1].sa[2],0);*/
         ip = InsNewInst(bp, ip, NULL, ld, -reg1, SToff[mask-1].sa[2], 0);
         ip = InsNewInst(bp, ip, NULL, cvtmask, -reg0, -reg1, 0 );
         ip = InsNewInst(bp, ip, NULL, st, SToff[codemask-1].sa[2], -reg0, 0);
         GetReg(-1);
         //assert(0);
/*
 *       NOTE: Alternative idea: we have cmov for integer depending on EFLAGS
 *       in X86. The CMP of original scalar code already effects the EFLAGS. So,
 *       we can use the same CMP and CMOV instrcution.... problem then, if there 
 *       exists other CMP which also effects the EFLAGS... where should we place
 *       the code!!!
 */
   #else
/*
 *       Without converting the mask at all
 */
         if (IS_FLOAT(STflag[mask-1]))
         {
            reg0 = GetReg(T_FLOAT);
            mskld = FLD;
            movmsk = CVTBFI;
         }
         else if (IS_DOUBLE(STflag[mask-1]))
         {
            reg0 = GetReg(T_DOUBLE);
            mskld = FLDD;
            movmsk = CVTBDI;
         }
         else assert(0);
         codemask = mask;
/*
 *       insert mask test which updates FCC0
 *       FIXME: right now, we use following cmp instruction for each int var 
 *       update. But we can and should use only one cmp instruction for all the
 *       updates; will update so later.
 */
         ip = InsNewInst(bp, ip, NULL, mskld, -reg0, SToff[mask-1].sa[2], 0);
#if 0
         ip = InsNewInst(bp, ip, NULL, MASKTEST, -FCC0, -reg0, 0 );
#else
/*
 *       CVTBFI / CVTBDI ireg, freg/dreg 
 *       BT ireg, $0
 *       CMOV1/CMOV2 ireg1, ireg2, ireg3
 */
         ireg = GetReg(T_INT);
         ip = InsNewInst(bp, ip, NULL, movmsk, -ireg, -reg0, 0);
         ip = InsNewInst(bp, ip, NULL, BTC, -ICC0, -ireg, STiconstlookup(0) );
#endif
         GetReg(-1); 
   #endif
#endif
      }
      else
         fko_error(__LINE__, "Unsupported type for RC: var=%s, flag=%d\n", 
               STname[sp[i]-1], STflag[sp[i]-1]);
/*
 *    Insert blend/insert inst
 */
      switch(vpos[i])
      {
         case IN_IF_ONLY:  /* if blks*/ 
/*
 *          findout corrsponding new vars
 */
            k = FindInShortList(isetvars[0], isetvars+1, sp[i]);
            assert(k);
            nv1 = inewvars[k];
/*
 *          Now time to insert select inst
 *          new var in if: nv1 
 */
            reg0 = GetReg(type);
            reg1 = GetReg(type);
            reg2 = GetReg(type);
            ip = InsNewInst(bp, ip, NULL, ld, -reg0, SToff[sp[i]-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, ld, -reg1, SToff[nv1-1].sa[2], 0);
            if (IS_INT(type))
               ip = InsNewInst(bp, ip, NULL, cmov1, -reg0, -reg1, -ICC0 );
            else
            {
               ip = InsNewInst(bp, ip, NULL, mskld, -reg2, SToff[codemask-1].sa[2], 0);
               ip = InsNewInst(bp, ip, NULL, cmov1, -reg0, -reg1, -reg2 );
            }
            ip = InsNewInst(bp, ip, NULL, st, SToff[sp[i]-1].sa[2], -reg0, 0);
            GetReg(-1);
            break;
         case IN_ELSE_ONLY:  /* else blks....*/
            /*bp = elseblks->blk;*/
/*
 *          findout corrsponding new vars
 */
            k = FindInShortList(esetvars[0], esetvars+1, sp[i]);
            assert(k);
            nv2 = enewvars[k];
/*
 *          Now time to insert select inst
 *          New var in else: nv2
 */
            reg0 = GetReg(type);
            reg1 = GetReg(type);
            reg2 = GetReg(type);
            ip = InsNewInst(bp, ip, NULL, ld, -reg0, SToff[sp[i]-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, ld, -reg1, SToff[nv2-1].sa[2], 0);
/*
 *             FIXED: cmov1 vs cmov2: 
 *             cmov1: 
 *                   dest = (mask==0)? dest: src
 *                   desk = mask? src: dest
 *             cmov2: 
 *                   dest = (mask==0)? src: dest
 *                   desk = mask? dest: src
 *
 */
            if (IS_INT(type))
               ip = InsNewInst(bp, ip, NULL, cmov2, -reg0, -reg1, -ICC0 );
            else
            {
               ip = InsNewInst(bp, ip, NULL, mskld, -reg2, SToff[codemask-1].sa[2], 0);
               ip = InsNewInst(bp, ip, NULL, cmov2, -reg0, -reg1, -reg2 );
            }
            ip = InsNewInst(bp, ip, NULL, st, SToff[sp[i]-1].sa[2], -reg0, 0);
            GetReg(-1);
            break;
         case IN_BOTH_IF_ELSE:  /* both in if and else blk */
            
            /*bp = mergeblks->blk;
            ip = bp->ainst1;
            if (ip->inst[0] == LABEL) ip=ip->next;
            */
/*
 *          findout corrsponding new vars
 */
            k = FindInShortList(isetvars[0], isetvars+1, sp[i]);
            nv1 = inewvars[k];
            assert(k);
            k = FindInShortList(esetvars[0], esetvars+1, sp[i]);
            assert(k);
            nv2 = enewvars[k];
/*
 *          Now time to insert select inst
 *          new var in if: nv1, New var in else: nv2
 */
            reg0 = GetReg(type);
            reg1 = GetReg(type);
            reg2 = GetReg(type);
            /*ip = InsNewInst(bp, NULL, ip, ld, -reg0, SToff[nv1-1].sa[2],0);*/
            ip = InsNewInst(bp, ip, NULL, ld, -reg0, SToff[nv1-1].sa[2], 0);
            ip = InsNewInst(bp, ip, NULL, ld, -reg1, SToff[nv2-1].sa[2], 0);
            if (IS_INT(type))
               ip = InsNewInst(bp, ip, NULL, cmov2, -reg0, -reg1, -ICC0 );
            else
            {
               ip = InsNewInst(bp, ip, NULL, mskld, -reg2, SToff[codemask-1].sa[2], 0);
               ip = InsNewInst(bp, ip, NULL, cmov2, -reg0, -reg1, -reg2 );
            }
            ip = InsNewInst(bp, ip, NULL, st, SToff[sp[i]-1].sa[2], -reg0, 0);
            GetReg(-1);
            break;
         default: ;
     }
   }
/*
 * Free all the memory which is alloated for analysis
 * NOTE: optimized the code to minimize the use of temp mem later.
 */
   if (sp) free(sp);
   if (vpos) free(vpos);
   if (isetvars) free(isetvars);
   if (esetvars) free(esetvars);
   if (icmvars) free(icmvars);
   if (ecmvars) free(ecmvars);
   if (inewvars) free(inewvars);
   if (enewvars) free(enewvars);

   return(err);
}

static BLIST *FindCondHeaders(BLIST *hblks, INT_BVI iscope, BBLOCK *head, 
                              INT_BVI tails, INT_BVI visitedblks)
{
/*
 * Terminating conditions
 */
   if (!head || !BitVecCheck(iscope, head->bnum-1)) 
      return(hblks);
/*
 * add blk in the list when it has both csucc and usucc
 * NOTE: tail blks not considered!
 */
   if (!BitVecCheck(tails, head->bnum-1))
   {
      if (head->usucc && head->csucc)
      {
         assert(!FindBlockInList(hblks, head));
         hblks = AddBlockToList(hblks, head);
      }
   }
/*
 * mark the blk as visited
 */
   SetVecBit(visitedblks, head->bnum-1, 1);
/*
 * recurse the unvisited successors
 */
   if (head->usucc && !BitVecCheck(visitedblks, head->usucc->bnum-1))
      hblks = FindCondHeaders(hblks, iscope, head->usucc, tails, visitedblks);
   
   if (head->csucc && !BitVecCheck(visitedblks, head->csucc->bnum-1))
      hblks = FindCondHeaders(hblks, iscope, head->csucc, tails, visitedblks);
   
   return(hblks);
}

BLIST *FindConditionalHeaders(BBLOCK *head, INT_BVI iscope, INT_BVI tails)
{
   BLIST *headers;
   INT_BVI visitedblks;

   headers = NULL;
   visitedblks = NewBitVec(32);
   SetVecAll(visitedblks, 0);
   headers = FindCondHeaders(headers, iscope, head, tails, visitedblks);
   KillBitVec(visitedblks);
   return(headers);
}
#if 0
/*
 * This function is replaced by DelRcBlk() 
 */
void DelIfElseBlk(BBLOCK *ifblk, BBLOCK *elseblk, BBLOCK *splitblk)
/*
 * this function deletes the if and else blk from the CFG keeping the 
 * up, dpwn, usucc, csucc updated.
 */
{
   BLIST *delblks, *dbl;
   BBLOCK *bp;
   extern BBLOCK *bbbase;
   extern LOOPQ *optloop;
/*
 * delete the ifblk and elseblk from CFG 
 * NOTE: 
 * 1. Make sure if/elseblk doesn't have predecessor other  than cond header
 *    It can only happen if irregular GOTO statement is used. Normal if/else
 *    should not create that problem.
 */
   delblks = NULL;
   delblks = AddBlockToList(delblks, ifblk);
   if (elseblk)
      delblks = AddBlockToList(delblks, elseblk);

   for (dbl = delblks; dbl; dbl = dbl->next)
   {
      bp = dbl->blk;
/*
 *    preds of this blk should only be the cond header /splitblk
 */
      assert((splitblk == bp->preds->blk) && !bp->preds->next);
/*
 *    if it is a usucc (elseblk), make the usucc of elseblk as the usucc of 
 *    split blk. It it is a ifblk, make the csucc of split blk as NULL
 *    NOTE: Updating the usucc is not enough, need to change the inst also.
 */
      if (splitblk->usucc == bp)
      {
#if 0           
         fprintf(stderr, "split=%d, bp=%d, bp->usucc=%d\n", splitblk->bnum, 
               bp->bnum, bp->usucc->bnum);
#endif   
         if (splitblk->down != bp->usucc)
         {
/*
 *          Normally mergeblk should have a label, otherwise how can the 
 *          if blk jump to it
 */
            assert(GET_INST(bp->usucc->ainst1->inst[0]) == LABEL);
            InsNewInst(splitblk, splitblk->ainstN, NULL, JMP, -PCREG, 
                       bp->usucc->ainst1->inst[1], 0);
         }
         splitblk->usucc = bp->usucc; 
         bp->usucc->preds = AddBlockToList(bp->usucc->preds, splitblk);
#if 0
         fprintf(stderr, "add %d as preds to %d\n", splitblk->bnum, 
                 bp->usucc->bnum);
#endif
      }
      else
      {
/*
 *       NOTE: RC already changed the conditional branch
 */
         splitblk->csucc = NULL;
      }
/*
 *    remove blk fro CFG
 */
      if (bp->up)
      {
         bp->up->down = bp->down;
         if (bp->down)
            bp->down->up = bp->up;
      }
/*
 *    Update the preds of other blks 
 */
#if 0
      fprintf(stderr, "blist=%s \nbp = %d bp->usucc=%d\n", 
              PrintBlockList(bp->usucc->preds), bp->bnum, bp->usucc->bnum);
#endif
      if (bp->usucc)
         bp->usucc->preds = RemoveBlockFromList(bp->usucc->preds, bp);
      if (bp->csucc)
         bp->csucc->preds = RemoveBlockFromList(bp->csucc->preds, bp);
      assert(bp->usucc->preds);
      
/*
 *    delete this if it is in optloop->blocks, optloop->tails
 *    NOTE: for a single tail of loop, tail should never be deleted
 *    NOTE: must re-assign the list as list itself may changed.
 *    NOTE: still looking ... whether blk ref is used any where else!
 */
     optloop->blocks = RemoveBlockFromList(optloop->blocks, bp); 
     /*RemoveBlockFromList(optloop->tails, bp);*/
/*
 *    Now delete the blk
 */
      KillAllInst(bp->inst1);
      KillBlockList(bp->preds);
#if 0
      fprintf(stderr, "DELBLK = %d\n", bp->bnum);
#endif
      free(bp);
   }
   KillBlockList(delblks);
}
#endif
/*
 * Generalize delblk for if/else/merge blk
 */
void DelRcBlk(BBLOCK *delblk, BBLOCK *splitblk)
{
   BBLOCK *bp;
/*
 * there must be a delblk and splitblk 
 */
   if( !delblk || !splitblk) return; 

#if 0
   fprintf(stderr, "delblk = %d\n", delblk->bnum);
#endif
/*
 * preds of the blk should only be cond header/ split blk
 */
   assert( (delblk->preds->blk == splitblk) && !delblk->preds->next );
/*
 * 1. update the predecessor of the delblks
 * if delblk is usucc of its preds, change it to delblk->usucc
 * otherwise, change the csucc to NULL. There will be no csucc after RC
 */
   bp = delblk->preds->blk; /* splitblk */
/*
 * incase of else and merge blk
 */
   if (bp->usucc == delblk)
   {
      if (bp->down != delblk->usucc)
      {  
/*
 *       we assume there is a label at the delblk->usucc, otherwise we need to
 *       create one.
 */
         assert(GET_INST(delblk->usucc->ainst1->inst[0]) == LABEL);
         if (GET_INST(bp->ainstN->inst[0]) != JMP)
         {
            InsNewInst(bp, bp->ainstN, NULL, JMP, -PCREG, 
                       delblk->usucc->ainst1->inst[1], 0);
         }
         else
         {
            bp->ainstN->inst[2] = delblk->usucc->ainst1->inst[1];
         }
      } /* else we may assume no JMP */
      bp->usucc = delblk->usucc;
      delblk->usucc->preds = AddBlockToList(delblk->usucc->preds, bp);
   }
   else /* incase of if blk */
   {
      bp->csucc = NULL; /* RC would already change the code */
      assert( !IS_COND_BRANCH(bp->ainstN->inst[0]) );
   }

/*
 * 2. Remove delblk from code 
 */
   if (delblk->up) /* there must be one */
   {
      delblk->up->down = delblk->down;
      if (delblk->down)
         delblk->down->up = delblk->up;
   }
   else assert(0);
/*
 * 3. update preds of other blks
 */
   if (delblk->usucc)
      delblk->usucc->preds = RemoveBlockFromList(delblk->usucc->preds, delblk);
   if (delblk->csucc)
      delblk->csucc->preds = RemoveBlockFromList(delblk->csucc->preds, delblk);
/*
 * 4. update global lists 
 */
   optloop->blocks = RemoveBlockFromList(optloop->blocks, delblk);
   assert(!FindBlockInList(optloop->tails, delblk));
/*
 * Now, it's time to delete the block
 */
   KillAllInst(delblk->inst1);
   KillBlockList(delblk->preds);
   free(delblk);
}

int IterativeRedCom()
/*
 * Applied RC transformation repeatablely until all the conditional blks are 
 * reduce to single blk.
 * FIXME: This xfrom can't handle  memory write inside the conditional blocks 
 * yet. All memory writes need to be shifted at the tail of loop. 
 */
{
   int err, CHANGES;
   int ivtails;
   int isdel;
   LOOPQ *lp;
   INSTQ *ippu;
   BLIST *bl, *splitblks;
   BBLOCK *ifblk, *elseblk, *mergeblk;
   struct ptrinfo *pi0;
   extern BBLOCK *bbbase;
   extern LOOPQ *optloop;
/*
 * use set needs to be updated as we use those in this transformation.
 */
   if (!CFUSETU2D)
   {
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
   }
/*
 * Apply RC until there is no conditional block inside the loop
 * NOTE: RC xform requires variable analysis but in each iteration it would
 * change the CFG. So, to be U2D, we must recompute the CFG and all variable
 * analysis.
 */
   do
   {
      CHANGES = 0; 
      lp = optloop;
      splitblks = NULL;
      KillLoopControl(lp);
      ivtails = BlockList2BitVec(lp->tails); /* no need to kill ivtails */
      splitblks = FindConditionalHeaders(lp->header, lp->blkvec, ivtails);
      /*assert(splitblks);*/ /* must have atleast one cond header */
/*
 *    use the first splitblk to reduce. it would be the deepest one.
 *    NOTE: if usucc of ifblk is the usucc of splitblk, there is no elseblk
 */
      if (splitblks)
      {   
         ifblk = splitblks->blk->csucc;
         if (ifblk->usucc != splitblks->blk->usucc)
         {
            elseblk = splitblks->blk->usucc;
            mergeblk = elseblk->usucc;
         }
         else 
         {
            elseblk = NULL;
            mergeblk = splitblks->blk->usucc;
         }
         assert(mergeblk == ifblk->usucc);

/*
 *       call RC with this if-cond, this function may delete all inst from 
 *       if/else blk but would not delete the blks.
 */
         err = ReduceBlkWithSelect(ifblk, elseblk, splitblks->blk);
#if 0         
         assert(!err);   
#else
         if (err) 
            break;
#endif
/*
 *       NOTE: to do rc iteratively, splitblk and mergeblk should be merged into
 *       single blk if megreblk is not successor of other blks.
 *       NOTE: As predecessor info may not be updated after DelIfElseBlk(), 
 *       perform checking before that.
 */
         isdel = 0;
         if (!FindBlockInList(lp->tails, mergeblk)) /* if not a tail of loop */
         {
            isdel = 1;
            for (bl=mergeblk->preds; bl; bl=bl->next)
            {
               if (bl->blk != ifblk && bl->blk != elseblk 
                     && bl->blk != splitblks->blk)
               {
                  isdel = 0; /* used by other blks, can't delete */
                  break;
               }
            }
         }
/*
 *       delete the if/else blk.
 *       NOTE: I will write more generalize on later
 */
         #if 0         
            DelIfElseBlk(ifblk, elseblk, splitblks->blk); 
         #else
            DelRcBlk(ifblk, splitblks->blk);
            DelRcBlk(elseblk, splitblks->blk);
         #endif
         if (isdel)
         {
/*
 *          merge blk can be merged with splitblk        
 */
#if 0
            fprintf(stderr, "mergeblk=%d\n", mergeblk->bnum);
#endif
            MovInstFromBlkToBlk(mergeblk, splitblks->blk);
            DelRcBlk(mergeblk, splitblks->blk); /* generalize later */
         }

         CHANGES = 1;
         CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = CFLOOP = 0;
      }
/*
 *    Update Loop control with ptr update
 *    NOTE: if there is a single tail of loop, tail never be deleted as tail 
 *    must be a merge blk.
 *
 *    FIXME: Chicken and egg problem: lp->blocks no longer consistant as blk
 *    may be deleted, but OptimizeLoopControl would use that!!! can't call
 *    invalidate loop control before fixing the loop control!!!
 *    FIXED: update optloop->blocks just after deleting the blks in DelRcBlk
 */
      lp = optloop; /* optloop info may be changed! */
#if 0
      PrintLoop(stderr, optloop);
#endif      
      /*pi0 = FindMovingPointers(lp->tails);*/
      pi0 = FindMovingPointers(optloop->tails);
      ippu = KillPointerUpdates(pi0,1);
      /*OptimizeLoopControl(lp, 1, 0, NULL);*/
      //OptimizeLoopControl(lp, 1, 0, ippu);
      OptimizeLoopControl(optloop, 1, 0, ippu);
      KillAllPtrinfo(pi0);
/*
 *    Now time to update all the var analysis, next iteration will require that
 *    NOTE: 
 *       NewBasicBlocks       --> CFU2D 
 *       FindLoops            --> CFDOMU2D, CFLOOP
 *       CalcInsOuts          --> INUSETU2D, CFUSETU2D
 *       CalcAllDeadVariables --> INDEADU2D
 */
      InvalidateLoopInfo();
      bbbase = NewBasicBlocks(bbbase);
      CheckFlow(bbbase, __FILE__, __LINE__);
      FindLoops();
      CheckFlow(bbbase, __FILE__, __LINE__);
      CalcInsOuts(bbbase);
      CalcAllDeadVariables();
#if 0
      fprintf(stdout, "Iteration %d: \n", i);
      PrintInst(stdout, bbbase);
      fflush(stdout);
      sprintf(ln, "rc%d.dot", i);
      i++;
      ShowFlow(ln, bbbase);
#endif      
   }
   while (CHANGES);
#if 0
   fprintf(stdout, "Final LIL after RC\n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif
   return(err);
}

/*
 * this function is replaced by IterativeRedCom()
 */
#if 0
int IfConvWithRedundantComp()
/*
 * this function will kill the loop control and call the function to RC xform
 * and add back the loopcontrol before return.
 */
{
   int err;
   LOOPQ *lp;
   INSTQ *ippu;
   struct ptrinfo *pi0;
   extern BBLOCK *bbbase;
   
   lp = optloop;
   KillLoopControl(lp);
   err = RedundantScalarComputation(lp);
#if 1
   pi0 = FindMovingPointers(lp->tails);
   ippu = KillPointerUpdates(pi0,1);
   /*OptimizeLoopControl(lp, 1, 0, NULL);*/
   OptimizeLoopControl(lp, 1, 0, ippu);
   KillAllPtrinfo(pi0);
#endif

#if 1
   CFU2D = CFDOMU2D = CFUSETU2D = INUSETU2D = INDEADU2D = CFLOOP = 0;
   InvalidateLoopInfo();
   bbbase = NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();
   CheckFlow(bbbase, __FILE__, __LINE__);
#endif

#if 0
   ShowFlow("rc.dot", bbbase);
   fprintf(stdout,"LIL after RC\n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif

   return(err);
}
#endif

int Get_OL_NEINC()
{
   return OL_NEINC;
}

void Set_OL_NEINC_One()
{
   OL_NEINC = STdef("OL_NEINC", CONST_BIT | T_INT, 1);
}

/* ===========================================================================
 *       Unroll All the way
 *
 *============================================================================*/

int CountUnrollFactor(LOOPQ *lp)
{
   int i;
   int ur=0;
   int beg, end, inc;
/*
 * loop index must be const to make the loop iter countable
 */
   if (!IS_CONST(STflag[lp->beg-1]) || !IS_CONST(STflag[lp->end-1]) 
         || !IS_CONST(STflag[lp->inc-1])) 
      return(0);
/*
 * return the iteration count
 */
   inc = SToff[lp->inc-1].i;
   beg = SToff[lp->beg-1].i;
   end = SToff[lp->end-1].i;
   assert(inc);
#if 0   
   if (inc > 0)
      ur = (end - beg + inc - 1 ) / inc;
   else
      ur = (beg - end + inc - 1) / (-inc);
#else
/*
 * to aviod off-by-one error, i do run it here!
 */
   if (inc > 0)
      for (i=beg; i < end; i += inc)
         ur++;
   else
      for (i=beg; i > end; i += inc)
         ur++;
#endif

#if 1
   fprintf(stderr, "unroll = %d\n", ur);
#endif
   return(ur);
}
#if 0
int UnrollAll(LOOPQ *lp, int unroll)
{
   int i;
   INT_BVI iv;
   BBLOCK *newCF;
   ILIST *il;
   INSTQ *ip, *ipn, *ippost;
   struct ptrinfo *pi, *pi0;
   int UsesPtrs=1;
   BLIST **dupblks, *bl, *ntails=NULL;
   enum comp_flag kbeg, kend;
   extern INT_BVI FKO_BVTMP;
   extern BBLOCK *bbbase;
/*
 * kill all loop control, won't need them later
 */
   KillLoopControl(lp);
/*
 * We don't allow use of index right now
 */
   il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
   if (il) 
   {
      KillIlist(il);
      return(0);
   }
/*
 * analyze moving ptrs
 */
   pi0 = FindMovingPointers(lp->blocks);
   if (!pi0)
      UsesPtrs = 0;

   dupblks = malloc(sizeof(BLIST*)*unroll);
   assert(dupblks);   
/*
 * Duplicate all of loop's CF
 */
   SetVecBit(lp->blkvec, lp->header->bnum-1, 0);
   kbeg = CF_LOOP_INIT;
   for (i=1; i < unroll; i++)
   {
/*
 *    Duplicate original loop body for unroll i
 */
      FKO_BVTMP = iv = BitVecCopy(FKO_BVTMP, lp->blkvec);
      
      newCF = DupCFScope(lp->blkvec, iv, lp->header);
      iv = BitVecCopy(iv, lp->blkvec);
/*
 *    Use CF to produce a block list of duped blocks
 */
      SetVecBit(iv, lp->header->bnum-1, 1);
      dupblks[i-1] = CF2BlockList(NULL, iv, newCF);
/*
 *    Kill the appropriate loop markup in the blocks (so we don't increment
 *    i multiple times, test it multiple times, etc)
 */
      kend = (i != unroll-1) ? CF_LOOP_END : CF_LOOP_BODY;
      KillCompflagInRange(dupblks[i-1], kbeg, kend);
      if (UsesPtrs)
      {
/*
 *       Find the moving pointers used in unrolled loop
 */
         pi = FindMovingPointers(dupblks[i-1]);
         assert(pi);
/*
 *       Kill pointer updates in loop, and get ptr inc code to add to EOL
 */
         ip = KillPointerUpdates(pi, i);
         assert(ip);
         for (; ip; ip = ipn)
         {
            ipn = ip->next;
            free(ip);
         }
/*
 *       Find all lds of moving ptrs, and add unrolling factor to them
 */
         UpdatePointerLoads(dupblks[i-1], pi, i);
         KillAllPtrinfo(pi);
      }
   }
   
   if (pi0)
   {
      /*ippost = KillPointerUpdates(pi0, UR);*/
      ippost = KillPointerUpdates(pi0, unroll);
      KillAllPtrinfo(pi0);
      assert(ippost);
   }
   SetVecBit(lp->blkvec, lp->header->bnum-1, 1);
   KillCompflagInRange(lp->blocks, CF_LOOP_UPDATE, CF_LOOP_END);
/*
 * Put duplicated blocks into program at correct location; this means that
 * the blocks [up,down] links are correct, but CF is messed up
 */
   InsertUnrolledCode(lp, unroll, dupblks);
/*
 * Fix the tails info for OptimizeLoopControl
 */
   iv = BlockList2BitVec(lp->tails);
   for (bl=dupblks[unroll-2]; bl; bl = bl->next)
   {
/*
 *    If last unrolling blk is a former tail, add it to new tails
 */
      if (BitVecCheck(iv, bl->blk->bnum-1))
         ntails = AddBlockToList(ntails, bl->blk);
   }
   KillBlockList(lp->tails);
   lp->tails = ntails;

   for (bl=lp->tails; bl; bl=bl->next)
   {
      ipn = bl->blk->instN;
      for (ip = ippost; ip; ip=ip->next)
      {
         ipn = InsNewInst(bl->blk, ipn, NULL, ip->inst[0], ip->inst[1], 
               ip->inst[2], ip->inst[3]);
      }
   }

   //InvalidateLoopInfo();
   NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   //FindLoops();  /* need to setup optloop for this */
   CheckFlow(bbbase, __FILE__, __LINE__);

#if 0
   fprintf(stdout, "unrolled loop: \n");
   PrintInst(stdout, bbbase);
   exit(0);
#endif
   
   return(1);
}

int UnrollAllTheWay()
{
   int ur;
   extern LOOPQ *optloop;

   ur = CountUnrollFactor(optloop);
#if 1
   fprintf(stderr, "all the way unroll factor = %d\n", ur);
   //exit(0);
#endif
   if (!ur) 
      return(0);
   
   UnrollAll(optloop, ur);

}
#endif

int IsIndexRefInBody(LOOPQ *lp)
{
   BBLOCK *bp;
   BLIST *bl;
   INSTQ *ip;

   for (bl=lp->blocks; bl; bl=bl->next)
   {
      if (FindInList(lp->tails, bl->blk))
      {
         for (ip = bl->blk->inst1; ip; ip = ip->next)
         {
            if (ip->inst[0] == LD && ip->inst[2] == SToff[lp->I-1].sa[2])
               return(1);
/*
 *          below the CF_LOOP_UPDATE, index is used to control the loop
 *          so, skip those
 */
            if (ip->inst[0] == CMPFLAG && ip->inst[1] == CF_LOOP_UPDATE)
               break;
         }
      }
      else
         for (ip = bl->blk->ainst1; ip; ip=ip->next)
            if (ip->inst[0] == LD && ip->inst[2] == SToff[lp->I-1].sa[2])
               return(1);
   }
   return(0);
}

int DelLoopControl(LOOPQ *lp)
{
   BBLOCK *bp;
   INSTQ *ip;
   extern BBLOCK *bbbase;
/*
 * Main idea: we will delete loop init and loop control (lp->I) if index
 * is never used inside the loop body, only branching otherwise
 */
#if 0
   fprintf(stderr, "before delloop\n");
   PrintInst(stderr, bbbase);
#endif
/*
 * NOTE: we can't use FindIndexRef here.. it works only after killing the 
 * loop control... we are not willing to do that here
 */
   if (IsIndexRefInBody(lp))
   {
/*
 *    index variable is used.. delete only the loop testing 
 */
      assert(lp->tails && !lp->tails->next);
      bp = lp->tails->blk;
      ip = FindCompilerFlag(bp, CF_LOOP_TEST);
      assert(ip);
/*
 *    FIXME: is it possible to be messed up with any fundamental optimizations?
 *    there shouldn't be any other inst after CF_LOOP_TEST
 */
      while(ip)
         ip = DelInst(ip);
   }
   else /* loop index is only used to control loop*/
   {
/*
 *    we need to delete loop init, loop update and loop test 
 */
      ip = FindCompilerFlag(lp->preheader, CF_LOOP_INIT);
      assert(ip);
/*
 *    FIXME: need a checking, it may delete some misplaced inst 
 */
      while(ip)
         ip = DelInst(ip);
      
      assert(lp->tails && !lp->tails->next);
      bp = lp->tails->blk;
      ip = FindCompilerFlag(bp, CF_LOOP_UPDATE);
      while(ip)
         ip = DelInst(ip);
   }
/*
 * recompute cfg 
 */
   InvalidateLoopInfo();
   NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
   FindLoops();  /* need to setup optloop for this */
   CheckFlow(bbbase, __FILE__, __LINE__);
#if 0 
   fprintf(stderr, "before delloop\n");
   PrintInst(stderr, bbbase);
#endif
   return(0);
}


