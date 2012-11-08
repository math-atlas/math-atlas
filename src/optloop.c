#include "fko.h"

/*FIXED: considered OL_NEINC as special purpose const in symbol table */
static short OL_NEINC=0;

int NonLocalDeref(short dt)
{
   dt--;
   if (!IS_DEREF(STflag[dt]))
      return(0);
   if (SToff[dt].sa[0] == -REG_SP && SToff[dt].sa[1] >= 0)
      return(0);
   return(1);
}

BBLOCK *DupBlock(BBLOCK *bold)
{
   BBLOCK *nb;
   INSTQ *ip;
   short *sp, i, k;

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

BLIST *DupBlockList(BLIST *scope, int ivscope)
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
   INSTQ *ip;

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

BBLOCK *GetFallPath(BBLOCK *head, int loopblks, int inblks, int tails,
                    int fallblks)
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
                     int loopblks, int inblks, int tails, int fallblks)
/*
 * Majedul: FIXME:
 * This function can't explore and copy all the blks recursively. Only works 
 * for those where each conditional successor much have other csucc to create
 * a path. If an usucc of a csucc has csucc, it doesn't work!!! 
 */
{
   BBLOCK *prev, *bpN, *bp0, bplast;
   int i, k, n;
   static int II=0;

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
                     int loopblks, int inblks, int tails, int fallblks, 
                     int visitedblks)
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
   BBLOCK *prev, *bpN, *bp0, bplast;
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
   int tails, fallblks, inblks, visitedblks;

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
                  //ip->next->inst[3] = STiconstlookup(val);
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
   short k, i, j;
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
   ILIST *il;
   INSTQ *ipbase=NULL, *ip, *ipN;
   int reg;
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
   reg = GetReg(T_INT);
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
   GetReg(-1);
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
   short k, kk;

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
            assert(k > 0);
            k = STpts2[k-1];
            if (!k) continue;
/*
 *          Find if load is of a moving pointer
 */
            for (i=0; i != n && pst[i] != k; i++);
            if (i == n) continue;
/*
 *          Now that we've got a moving pointer, determing unrolling increment
 */
            inc = UR * type2len(FLAG2TYPE(STflag[k-1]));
/*
 *          NOTE: i can be 0 ...
 */
            for (pi=pbase; i && pi->ptr != k; pi=pi->next,i--);
            assert(pi);
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
   if (unroll > 1 && !IS_CONST(STflag[lp->end-1]))
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
   if (unroll < 2)
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
         assert(ipl);
      }
#endif      
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
   INSTQ *ipinit, *ipupdate, *iptest, *ip;
   ILIST *il;
   int I, beg, end, inc, i;
   int CHANGE=0;
   short ur;

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
      //if (!AlreadySimpleLC(lp) && il)
      if (!AlreadySimpleLC(lp) )
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
/*      fprintf(stderr, "\nLoop good for SimpleLC!!!\n\n"); */
      SimpleLC(lp, unroll, &ipinit, &ipupdate, &iptest);
   }
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
   sprintf(ln, "_IFKOCD%d_%s%s", dupnum, sp, spnum);
#endif   
   
   return(ln);
}

BBLOCK *DupCFScope0(short ivscp0, /* original scope */
                   short ivscp,  /* scope left to dupe */
                   int dupnum,   /* number of duplication, starting at 1 */
                   BBLOCK *head) /* block being duplicated */
/*
 * Duplicates CF starting at head.  Any block outside ivscp is not duplicated
 * NOTE: actual head of loop should not be in ivscop0, even though we dup it
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

BBLOCK *DupCFScope(short ivscp0, /* original scope */
                   short ivscp,  /* scope left to dupe */
                   int dupnum,   /* number of duplication, starting at 1 */
                   BBLOCK *head) /* block being duplicated */
{
#if 1
   static int dnum=0;
   return (DupCFScope0(ivscp0, ivscp, dnum++, head));
#else
   return (DupCFScope0(ivscp0, ivscp, dupnum, head));
#endif
}

BLIST *CF2BlockList(BLIST *bl, short bvblks, BBLOCK *head)
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

BBLOCK *FindFallHead(BBLOCK *head, int tails, int inblks)
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

static BLIST *FindAllFallHeads0(BLIST *ftheads, int iscope, BBLOCK *head, 
                                int tails, int inblks, int visitedblks)
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
 * If it is tail blk, we don't need to recurse; otherwise, we will fall into
 * infinit loop. 
 * FIXME: if there is a nested loop inside the scope, it will fail. 
 * Need to handle specially for nested loop and will do it later.
 */
#if 0   
   if (BitVecCheck(tails, head->bnum-1))
      return(ftheads);
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

BLIST *FindAllFallHeads(BLIST *ftheads, int iscope, BBLOCK *head, int tails,
                        int inblks)
/*
 * It's a wrapper funtion for the original function. I introduce an extra
 * parameter to manage the visited nodes like: DFS. To avoid the change in 
 * function call, this wrapper is used.
 */
{
   int visitedblks;
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
   int iv, ivtails;
   short r0;
   extern BBLOCK *bbbase;
   extern int FKO_BVTMP;

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
 *    FIXME: OL_NEINC can be updated multiple time as we may call the cleanup
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
   newCF = DupCFScope(lp->blkvec, iv, 0, lp->header);
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
   ILIST *il;
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
 */
   InsNewInst(bp, NULL, ipnext, LABEL, lp->PTCU_label, 0, 0);
   GetReg(-1);
}
#endif


void UnrollCleanup2(LOOPQ *lp, int unroll); /* defined later */
int ListElemCount(BLIST *blist);

int UnrollLoop(LOOPQ *lp, int unroll)
{
   short iv;
   BBLOCK *newCF;
   BLIST **dupblks, *bl, *ntails=NULL;
   ILIST *il;
   INSTQ *ippost=NULL, *ip, *ipn;
   struct ptrinfo *pi, *pi0;
   int i, UsesIndex=1, UsesPtrs=1, URbase=0, UR, URmul;
   enum comp_flag kbeg, kend;
   short *sp;
   int n;
   extern int FKO_BVTMP;
   extern BBLOCK *bbbase;
   extern int VECT_FLAG;
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
 */
   /*UnrollCleanup(lp, unroll);*/
   if (FKO_SB && (VECT_FLAG & VECT_SV) )
      UnrollCleanup2(lp, unroll*FKO_SB);
   else 
      UnrollCleanup2(lp, unroll);

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
 *    FIXME: need to provide appropriate dupnum. 
 *    DupCFScope is already called in several places: cleanup, loop peeling,
 *    speculative vect!!
 *    Why don't use a static variable inside DupCFScope!!!
 */
#if 0      
      newCF = DupCFScope(lp->blkvec, iv, i, lp->header);
#else
      newCF = DupCFScope(lp->blkvec, iv, 100+i, lp->header);
#endif      
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
         UpdatePointerLoads(dupblks[i-1], pi, i*URmul);
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
      ippost = KillPointerUpdates(pi0, UR);
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

ILIST *GetPrefetchInst(LOOPQ *lp, int unroll)
{
   BBLOCK *bp;
   INSTQ *ipp;
   BLIST *bl;
   ILIST *il, *ilbase=NULL;
   ILIST **ils;
   short ir, ptr, lvl;
   int i, j, n, npf, STc;
   int flag;
   enum inst inst;
   char ln[1024];

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
/*
 * Get an ILIST for each array
 */
   n = lp->pfarrs[0];
   ils = calloc(sizeof(ILIST*), n);
   assert(ils);
   for (i=1; i <= n; i++)
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
      for (j=0; j < npf; j++)
      {
         ipp = NewInst(NULL, NULL, NULL, LD, -ir, SToff[ptr-1].sa[2], 0);
         ipp->next = NewInst(NULL, ipp, NULL, inst, 0, 
                  AddDerefEntry(-ir, 0, 0, lp->pfdist[i]+j*LINESIZE[lvl], ptr),
                             STiconstlookup(lvl));
         ils[i-1] = NewIlist(ipp, ils[i-1]);
      }
   }
   GetReg(-1);
/*
 * Create master list of pref inst, ordering by taking one pref from
 * each array in ascending order
 */
   for (j=0; j < npf; j++)
   {
      for (i=n-1; i >= 0; i--)
      {
         ilbase = NewIlist(ils[i]->inst, ilbase);
         ils[i] = KillIlist(ils[i]);
      }
   }
   free(ils);
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
 *          which incrases the instruction count resulting the error in skip. 
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
   ILIST *il;
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

int FindUnusedIRegInList(BLIST *scope, int ir)
/*
 * Finds integer register not set or used in scope.  Returns ir if it has
 * not been used, and another, unused register, if it has
 */
{
   BLIST *bl;
   INSTQ *ip;
   int bv;
   extern int FKO_BVTMP;
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
/*
 * If previous register already used, get one that isn't
 */
   if (!ir)
      ir = GetReg(T_INT);
   while(BitVecCheckComb(bv, Reg2Regstate(ir), '&'))
      ir = GetReg(T_INT);
   return(ir);
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

void SchedPrefInLoop1(LOOPQ *lp, ILIST *ilbase, int dist, int chunk)
/*
 * Adds the inst in ilbase to loop, one ILIST chunk scheduled at a time.
 * This variant puts the first pref chunk at dist inst from the start of
 * the header, and then adds another chunk every NINST/(np/chunk)
 */
{
   BLIST *atake, *bl;
   INSTQ *ip, *ipp, *ipn, *ipf, *ipfn;
   int N, skip, sk, i, j, k, npf, ir, ir0, dt;
   int bv;
   ILIST *il;
   extern int FKO_BVTMP;

   atake = GetSchedInfo(lp, ilbase, -1, &npf, &N);
   ir0 = -ilbase->inst->inst[1];
   ir = FindUnusedIRegInList(atake, ir0);

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
 */
   if (ir0 != ir)
      ChangeRegInPF(ir, ilbase);
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
   int i;
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
   int N, skip, sk, i, j, k, npf, ir, ir0, dt;
   int bv;
   ILIST *il;

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
 */
   ir0 = -ilbase->inst->inst[1];
   ir = FindUnusedIRegInList(atake, ir0);
   if (ir0 != ir)
      ChangeRegInPF(ir, ilbase);

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
   enum inst inst, ld, st, add, mac;
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
   default:
   case T_VFLOAT:
   case T_VDOUBLE:
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

void CountArrayAccess(BLIST *scope, int ptr, int *nread, int *nwrite)
/*
 * Finds number of read/write to *ptr in scope, 
 * *ptr is float or double (not vec)
 * NOTE: assumes LD/ST only array access, may want to run DoEnforceLoadStore()
 *       to make sure this is true
 */
{
   BLIST *bl;
   INSTQ *ip;
   int nr=0, nw=0, i;
   enum inst st, ld;

   i = STflag[ptr-1];
   i = FLAG2TYPE(i);
   if (IS_FLOAT(i))
   {
      st = FST;
      ld = FLD;
   }
   else if (IS_INT(i))
   {
      st = ST;
      ld = LD;
   }
   else
   {
      assert(IS_DOUBLE(i));
      st = FSTD;
      ld = FLDD;
   }
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainst1; ip; ip = ip->next)
      {
         if (ip->inst[0] == st && STpts2[ip->inst[1]-1] == ptr)
            nw++;
         else if (ip->inst[0] == ld && STpts2[ip->inst[2]-1] == ptr)
            nr++;
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
   int iv;
   BLIST *bl;
   extern int FKO_BVTMP;
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
   int i, j, k, ns, N;
   short set, use, var;
   short *sp;
   char pre;
   struct ptrinfo *pi, *pi0;
   BLIST *bl;
   INSTQ *ip;
   LOOPQ *lp;
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
      CountArrayAccess(lp->blocks, sp[i], &k, &j);
      fprintf(fpout, " sets=%d uses=%d", j, k);
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
         else if (VarIsMax(lp->blocks, sp[i]))
            j = 1;
         else if (VarIsMin(lp->blocks, sp[i]))
            j = 1;
         else j = 0;   
      }
      else j = 0;

      fprintf(fpout, " ScalarExpandable=%d", j);
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

void FeedbackLoopInfo()
/*=============================================================================
NCACHES=N
   LINESIZES: X1 X2 ... XN
OPTLOOP=1
   MaxUnroll=0
   LoopNormalForm=1/0
   NUMPATHS=2
      MaxMinReducesToOnePath=1
      MaxReducesToOnePath=0
      MinReducesToOnePath=0
      RedCompReducesToOnePath=1
   VECTORIZABLE=1
      MaxMinOK=0/1
      MaxOK=0/1
      MinOK=0/1
      RedCompOK=0/1
      SpeculationOK=1
         path-1Vect=1/0
         path-2Vect=1/0
   Moving FP Pointers: N
      'X': type=s/d prefetch=1/0 sets=1/0 uses=1/0
      ... ... ...
   Scalars Used in Loop: N
      'X': type=s/d sets=1/0 uses=1/0 ScalarExpandable=1/0
      ... ... ... 
 *============================================================================*/
{
   int i, j, N, npaths;
   LOOPQ *lp;
   ILIST *il;
   BLIST *bl;
   FILE *fpout=stdout;
   int SimpleLC=0, UR;
   short *scal;
   int MaxR, MinR, MaxMinR, RC, pv;
   int VmaxR, VminR, VmaxminR, Vrc, Vspec, Vn;
   extern FILE *fpLOOPINFO;
   extern short STderef;
   extern BBLOCK *bbbase;

   MaxR=0; MinR=0; MaxMinR=0; RC=0; pv =0;
   VmaxR=0; VminR=0; VmaxminR=0; Vrc=0; Vspec=0; Vn=0;
   if (fpLOOPINFO)
      fpout = fpLOOPINFO;
/*
 * Print cache information  
 */
   fprintf(fpout, "NCACHES=%d\n", NCACHE);
   fprintf(fpout, "   LINESIZES :");
   for (i=0; i < NCACHE; i++)
      fprintf(fpout, " %d", LINESIZE[i]);
   fprintf(fpout, "\n");
/*
 * NOTE: Saving and Restoring FKO State don't re-initiate some global data
 * like: bitvect. So, if we restore any states (other than just after state0)
 * there would be existing bit vectors. 
 * FIXME: there can be 2 ways:
 *    1. Save all global data structures and restore all of them while 
 *       restoring states (need to keep track and delete some).
 *    2. Delete/free all the global data structures and re-calculate them while
 *       restoring states (need to free some);
 */

/* 
 * Right now, will call this function just after state0, we don't need to call
 * the restore function. 
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
/*
 *    HERE HERE, what is the significance of MaxUnroll ??? 
 *    It is derived from the annotation 
 */
      UR = optloop->maxunroll;
      fprintf(fpout, "   MaxUnroll=%d\n", UR);
/*
 *    figure out the loop structure    
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
#if 0
/*
 *    Check all loop info 
 */
      lp = optloop;
      fprintf(stderr, "\n LOOP BLOCKS: \n");
      if(!lp->blocks) fprintf(stderr, "NO LOOP BLK!!!\n");
      for (bl = lp->blocks; bl ; bl = bl->next)
      {
         assert(bl->blk);
         fprintf(stderr, "%d ",bl->blk->bnum);
      }
      fprintf(stderr,"\n");
      fprintf(stderr, "\n LOOP BLOCKS: \n");
      if(lp->header) fprintf(stderr, "loop header: %d\n",lp->header->bnum);
      if(lp->preheader) fprintf(stderr, "loop preheader: %d\n",
                                lp->preheader->bnum);
   
      fprintf(stderr,"loop tails: ");
      for (bl = lp->tails; bl ; bl = bl->next)
      {
         assert(bl->blk);
         fprintf(stderr, "%d ",bl->blk->bnum);
      }
      fprintf(stderr,"\n");
   
      fprintf(stderr,"loop posttails: ");
      for (bl = lp->posttails; bl ; bl = bl->next)
      {
         assert(bl->blk);
         fprintf(stderr, "%d ",bl->blk->bnum);
      }
      fprintf(stderr,"\n");
#endif      

/*
 *    Findout Path information.
 *    NOTE: always kill path table after doing the analysis. Here, it is 
 *    done inside FindNumPaths() function
 */
      npaths = FindNumPaths(optloop);
      fprintf(fpout, "   NUMPATHS=%d\n",npaths);

      if (npaths > 1)
      {
#if 0         
         RestoreFKOState0();
         GenPrologueEpilogueStubs(bbbase,0);
         NewBasicBlocks(bbbase);
         FindLoops(); 
         CheckFlow(bbbase, __FILE__, __LINE__);
#endif         
/*
 *       Check Whether it is reducable by Max/Min
 */

         UpdateOptLoopWithMaxMinVars1(optloop);

         scal = FindAllScalarVars(optloop->blocks);
         for (N = scal[0], i=1; i <= N; i++)
         {
            if (VarIsMax(optloop->blocks, scal[i]))
            {
               MaxR = ElimMaxMinIf(scal[i]);
               if (MaxR) break;
            }
         }         
         for (N = scal[0], i=1; i <= N; i++)
         {
            if (VarIsMin(optloop->blocks, scal[i]))
            {
               MinR = ElimMaxMinIf(scal[i]); /* this only support max now */
               if (MinR) break;
            }
         }
         if(scal) free(scal);
/*
 *       haven't checked MaxMin... no imp yet 
 *       NOTE: SpeculativeVectorAnalysis also works if there are multiple 
 *       paths still. So, it's a design decision whether we need to apply 
 *       SpeculativeVectorAnalysis ... ... 
 */
         if (MaxMinR)
         {
            VmaxminR = !(SpeculativeVectorAnalysis()); /* the most general */
/*
 *          Normally, during vectorization, we killed the path tables. 
 *          Must kill all before applying path based analysis again
 */
            KillPathTable();
         }
         else if (MaxR)
         {
            VmaxR = !(SpeculativeVectorAnalysis()); /*it is the most general */
            KillPathTable();
         }
         else if (MinR)
         {
            VminR = !(SpeculativeVectorAnalysis()); /*it is the most general */
            KillPathTable();
         }
         else;

         fprintf(fpout, "      MaxMinReducesToOnePath=%d\n",MaxMinR);/*not yet*/
         fprintf(fpout, "      MaxReducesToOnePath=%d\n",MaxR);
         fprintf(fpout, "      MinReducesToOnePath=%d\n",MinR);
/*
 *       Check whether RedundantComputation Transformation can be applied
 *       NOTE: right now, we can apply RC only for 2 paths! if or if-else
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

         if (npaths > 2)
         {
            /*fprintf(fpout, "      RedCompReducesToOnePath=%d\n",0);*/
            RC = 0;
         }
         else
         {
            /*fprintf(fpout, "      RedCompReducesToOnePath=%d\n",1);*/
            UpdateOptLoopWithMaxMinVars1(optloop);
            RC = !(IfConvWithRedundantComp());
         }
         fprintf(fpout, "      RedCompReducesToOnePath=%d\n",RC);
         
         if (RC)
         {
            Vrc = !(SpeculativeVectorAnalysis()); /*it is the most general */
            KillPathTable();
         }
      }
/*
 *    Now, we will check the vectorization 
 *    Restoring to state0
 */
      RestoreFKOState0();
      GenPrologueEpilogueStubs(bbbase,0);
      NewBasicBlocks(bbbase);
      FindLoops(); 
      CheckFlow(bbbase, __FILE__, __LINE__);
      
      if (IsSpeculationNeeded())
      {
         Vspec = !(SpeculativeVectorAnalysis()); /*it is the most general */
/*
 *       Kill path after getting vectorization info
 */
         /*KillPathTable();*/
      }
      else
      {
         Vspec = 0;
         Vn = !(SpeculativeVectorAnalysis()); /*it is the most general */ 
         KillPathTable();
      }
      
      if (Vn || Vrc || VmaxR || VminR || VmaxminR || Vspec)
      {
         fprintf(fpout, "   VECTORIZABLE=%d\n",1);
         if (npaths > 1)
         { 
            fprintf(fpout, "      MaxMinOK=%d\n",VmaxminR);
            fprintf(fpout, "      MaxOK=%d\n",VmaxR);
            fprintf(fpout, "      MinOK=%d\n",VminR);
            fprintf(fpout, "      RedCompOK=%d\n",Vrc);
            fprintf(fpout, "      SpeculationOK=%d\n",Vspec);
            if (Vspec)
            {
               for (i=0; i < npaths; i++)
               {
                  pv = PathVectorizable(i+1);
                  fprintf(fpout, "         path-%dVect=%d\n", i+1, pv);
               }               
               KillPathTable();
            }
         }
      }
      else
         fprintf(fpout, "   VECTORIZABLE=%d\n",0);
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

   if (fpLOOPINFO)
   {
/*
 *    Just to kill every thing
 */
      KillAllGlobalData();
      exit(0);
   }
}

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
   int set, use, var;
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
   INSTQ *ibase, *ip, *ipb;
   int i, j, k, i1, i2, n, type;
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
   enum inst zero, inst, ld, st, vsld, vsst, vshuf;
   INSTQ *ibase, *ip, *ipb;
   int i, j, k, i1, i2, n, type;
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
      vsst = VFSTS;
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
      vsst = VDSTS;
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
      ip->next = NewInst(NULL, ip, NULL, zero, r0, 0, 0);
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
      }
      else
      {
         ip->next = NewInst(NULL, ip, NULL, ld, r0, SToff[se-1].sa[2], 0);
      }
   }

   ip = ip->next;
   for (i=1; i < ne; i++)
   {
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
   for (j=0, i=ne; i; i >>= 1) j++;
   if (1<<(j-1) == ne)
      j--;

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

   ip = ibase->next;
   for (i=1; i <= ne; i <<= 1, j--)
   {
      for (k=0; k < j; k++)
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

int ElimIFBlkWithMin(short minvar)
/*
 * Assuming single occurrance first
 * NOTE: we will combine the func with Max later!!
 */
{
   int i, j;
   short reg0, reg1, regv, regx;
   short xvar, label;
   enum inst inst, ld, st, br, cmp, min;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl, *scope;

   i = FLAG2TYPE(STflag[minvar-1]);
   switch(i)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      cmp = FCMP;
      min = FMIN;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      cmp = FCMPD;
      min = FMIND;
      break;
   default:
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__,"Unknown type=%d, file=%s. Should be done before Vect",
                i, __FILE__);
   }
/*
 * Primarily, we are concern about the optloop blks, but need to extend to
 * all blocks in bbbase later
 */   
   scope = optloop->blocks;

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
 */
            if (STpts2[ip0->inst[2]-1] == minvar)
            {
               regv = ip0->inst[1];
               xvar = ip1->inst[2];
               regx = ip1->inst[1];
            }
            else if (STpts2[ip1->inst[2]-1] == minvar)
            {
               regv = ip1->inst[1];
               xvar = ip0->inst[2];
               regx = ip0->inst[1];
            }
            else 
               regv = 0;
            if ( ((regv == reg0) && (br == JGT)) ||  
                 ((regv == reg1) && (br == JLT)) )
            {
               label = ip->inst[3];
               /* check for single set inside if-blk and not set where else*/
/*
 *             There are two checks:
 *                1. Max/Min var is only set inside the ifblk, nowhere else. It
 *                   needed for the reduction at posttails
 *                2. No other var but max/min is set inside ifblk
 */
               if (CheckMaxMinConstraints(scope, minvar, label) && 
                   CheckMaxMinReduceConstraints(scope, minvar, label))
               {
                  // Now, it's time to eliminiate ifblk inserting max inst
                  /*fprintf(stderr, "elim blks for max var = %s\n", 
                          STname[maxvar-1]);*/
                  assert(ip0->prev->inst[0] != ld);
                  ip1 = InsNewInst(bp, ip1, NULL, min, regv, regv, regx);
                  ip1 = InsNewInst(bp, ip1, NULL, st, SToff[minvar-1].sa[2],
                                   regv, 0);
                  ip1 = ip1->next;
                  while (ip1 && !IS_COND_BRANCH(ip1->inst[0])) 
                     ip1 = RemoveInstFromQ(ip1);
                  assert(IS_COND_BRANCH(ip1->inst[0]));
                  ip1 = RemoveInstFromQ(ip1); /* delete the branch itself */
                  // it's time to delete the if blk
                  RemoveInstFromLabel2Br(scope, label);
                  return(1);
               }
            }
         }
      }
   }
   return (0);
   
}


int ElimIFBlkWithMax(short maxvar)
/*
 * Assuming single occurrance first
 *
 * NOTE: need to restructure the codes of this function for better 
 * understanding
 */
{
   int i, j;
   short reg0, reg1, regv, regx;
   short xvar, label;
   enum inst inst, ld, st, br, cmp, max;
   BBLOCK *bp;
   INSTQ *ip, *ip0, *ip1;
   BLIST *bl, *scope;

   i = FLAG2TYPE(STflag[maxvar-1]);
   switch(i)
   {
   case T_FLOAT:
      ld = FLD;
      st = FST;
      cmp = FCMP;
      max = FMAX;
      break;
   case T_DOUBLE:
      ld = FLDD;
      st = FSTD;
      cmp = FCMPD;
      max = FMAXD;
      break;
   default:
   case T_VFLOAT:
   case T_VDOUBLE:
      fko_error(__LINE__,"Unknown type=%d, file=%s. Should be done before Vect",
                i, __FILE__);
   }
/*
 * Primarily, we are concern about the optloop blks, but need to extend to
 * all blocks in bbbase later
 */   
   scope = optloop->blocks;

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
 */
            if (STpts2[ip0->inst[2]-1] == maxvar)
            {
               regv = ip0->inst[1];
               xvar = ip1->inst[2];
               regx = ip1->inst[1];
            }
            else if (STpts2[ip1->inst[2]-1] == maxvar)
            {
               regv = ip1->inst[1];
               xvar = ip0->inst[2];
               regx = ip0->inst[1];
            }
            else 
               regv = 0;
            if ( ((regv == reg0) && (br == JLT)) ||  
                 ((regv == reg1) && (br == JGT)) )
            {
               label = ip->inst[3];
               /* check for single set inside if-blk and not set where else*/
/*
 *             There are two checks:
 *                1. Max/Min var is only set inside the ifblk, nowhere else. It
 *                   needed for the reduction at posttails
 *                2. No other var but max/min is set inside ifblk
 */
               if (CheckMaxMinConstraints(scope, maxvar, label) && 
                   CheckMaxMinReduceConstraints(scope, maxvar, label))
               {
                  // Now, it's time to eliminiate ifblk inserting max inst
                  /*fprintf(stderr, "elim blks for max var = %s\n", 
                          STname[maxvar-1]);*/
                  assert(ip0->prev->inst[0] != ld);
                  ip1 = InsNewInst(bp, ip1, NULL, max, regv, regv, regx);
                  ip1 = InsNewInst(bp, ip1, NULL, st, SToff[maxvar-1].sa[2],
                                   regv, 0);
                  ip1 = ip1->next;
                  while (ip1 && !IS_COND_BRANCH(ip1->inst[0])) 
                     ip1 = RemoveInstFromQ(ip1);
                  assert(IS_COND_BRANCH(ip1->inst[0]));
                  ip1 = RemoveInstFromQ(ip1); /* delete the branch itself */
                  // it's time to delete the if blk
                  RemoveInstFromLabel2Br(scope, label);
                  return(1);
               }
            }
         }
      }
   }
   return (0);
   
}
int ElimMaxMinIf()
{
   int i, j, N;
   short *scal;
   LOOPQ *lp;
   int changes;

   changes = 0;
   lp = optloop;
   
   scal = FindAllScalarVars(lp->blocks);   
   for (N = scal[0], i=1; i <= N; i++)
   {
      if (VarIsMax(lp->blocks, scal[i]))
      {
         #if 0
            fprintf(stderr, "Max var = %s\n", STname[scal[i]-1]);  
         #endif
         changes += ElimIFBlkWithMax(scal[i]);
      }
      else if (VarIsMin(lp->blocks, scal[i]))
      {
         #if 0
            fprintf(stderr, "Min var = %s\n", STname[scal[i]-1]);
         #endif
         changes += ElimIFBlkWithMin(scal[i]);
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
      return (changes);
   }

   return (0);  
}

int IfConvWithRedundantComp()
{
   int err;
   LOOPQ *lp;

   INSTQ *ippu;
   struct ptrinfo *pi0;
   
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
   return(err);
}

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
   ILIST *il;
   int FORWARDLOOP;
   short r0, r1;

   if (unroll < 1) unroll = 1;
/*
 * Cleanup should already be generated before finalizing it
 */
   assert(lp->CU_label > 0);

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
   ILIST *il;
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
   BLIST *bl;
   fprintf(fpout, "LOOP INFO: \n");
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
}


