#include "fko.h"

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
fprintf(stderr, "tails = %s\n", PrintVecList(tails, 1));
   for (bp = head; bp; bp = bp->down)
   {
fprintf(stderr, "scoping blk=%d\n", bp->bnum);
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

void InsertUnrolledCode(LOOPQ *lp, int unroll, BLIST **dupblks)
{
   int tails, fallblks, inblks;

   tails = BlockList2BitVec(lp->tails);
   fallblks = NewBitVec(32);
   inblks = NewBitVec(32);
   SetVecAll(inblks, 0);
   InsUnrolledCode(unroll, dupblks, lp->header, lp->blkvec, 
                   inblks, tails, fallblks);
   KillBitVec(fallblks);
   KillBitVec(inblks);
}

int UpdateIndexRef(BLIST *scope, short I, int ur)
/*
 * Finds all LDs from I in given scope, and adds UR to the lded reg.
 */
{
   BLIST *bl;
   INSTQ *ip;
   int changes=0; 
   short deref, k;
   deref = SToff[I-1].sa[2];
   for (bl=scope; bl; bl = bl->next)
   {
      for (ip=bl->blk->ainstN; ip; ip = ip->prev)
      {
         if (ip->inst[0] == LD && ip->inst[2] == deref)
         {
            k = ip->inst[1];
            InsNewInst(bl->blk, ip, NULL, ADD, k, k, STiconstlookup(ur));
            changes++;
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
            k = k ? SToff[k-1].i+inc : inc;
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
   INSTQ *iret=NULL, *ip;
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
   INSTQ *ip;
   BLIST *bl;

   if (!lp) return;
/*
 * Delete index init that must be in preheader
 */
   ip = FindCompilerFlag(lp->preheader, CF_LOOP_INIT);
   #if IFKO_DEBUG_LEVEL >= 1
      assert(ip);
   #endif
   ip = ip->next;
   while (ip && (ip->inst[0] != CMPFLAG || ip->inst[1] != CF_LOOP_BODY))
      ip = DelInst(ip);
   #if IFKO_DEBUG_LEVEL >= 1
      assert(ip);
   #endif
/*
 * Delete index update, test and branch that must be in all tails
 */
   for (bl=lp->tails; bl; bl = bl->next)
   {
      ip = FindCompilerFlag(bl->blk, CF_LOOP_UPDATE);
      #if IFKO_DEBUG_LEVEL >= 1
         assert(ip);
      #endif
      ip = ip->next;
      while (ip && (ip->inst[0] != CMPFLAG || ip->inst[1] != CF_LOOP_END))
      {
         if (ip->inst[0] != CMPFLAG || ip->inst[1] != CF_LOOP_TEST)
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
   if (unroll > 1 && !IS_CONST(STflag[lp->end-1]))
   {
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, LD, -r0, SToff[lp->end-1].sa[2], 0);
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, 
                         STiconstlookup(unroll*SToff[lp->inc-1].i-1));
      ip = ip->next;
      ip->next = NewInst(NULL, NULL, NULL, ST, SToff[lp->end-1].sa[2], -r0, 0);
   }

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
             STiconstlookup(SToff[I0-1].i - (unroll>1) ? unroll-1 : 0), 0);
      else
      {
         *ipinit = ip = NewInst(NULL, NULL, NULL, LD, -r0, SToff[I0-1].sa[2],0);
         if (unroll > 1)
         {
            ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, 
                               STiconstlookup(unroll-1));
            ip = ip->next;
         }
      }
      inc = STiconstlookup(-SToff[inc-1].i);
   }
   else if (IS_CONST(STflag[N-1]) && IS_CONST(STflag[I0-1]))
   {
      i = SToff[N-1].i - SToff[I0-1].i - (unroll>1) ? unroll-1 : 0;
      *ipinit = ip = NewInst(NULL, NULL, NULL, MOV, -r0, STiconstlookup(i), 0);
   }
   else
   {
      if (IS_CONST(STflag[I0-1]))
      {
         *ipinit = ip = NewInst(NULL, NULL, NULL, LD, -r0, SToff[N-1].sa[2], 0);
         if (SToff[I0-1].i)
         {
            if (unroll < 2)
               ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, I0);
            else
               ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, 
                                  STiconstlookup(SToff[I0].i-unroll+1));
            ip = ip->next;
         }
         else if (unroll > 1)
         {
            ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, 
                               STiconstlookup(unroll-1));
            ip = ip->next;
         }
      }
      else if (IS_CONST(STflag[N-1]))
      {
         *ipinit = ip = NewInst(NULL, NULL, NULL, LD, -r1, SToff[I0-1].sa[2],0);
         if (unroll < 2)
            ip->next = NewInst(NULL, NULL, NULL, MOV, -r0, N, 0);
         else
            ip->next = NewInst(NULL, NULL, NULL, MOV, -r0, 
                               STiconstlookup(SToff[N-1].i-unroll+1), 0);
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
         if (unroll > 1)
         {
            ip->next = NewInst(NULL, NULL, NULL, SUB, -r0, -r0, 
                               STiconstlookup(unroll-1));
            ip = ip->next;
         }
      }
   }
   ip->next = NewInst(NULL, NULL, NULL, ST, Ioff, -r0, 0);
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

fprintf(stderr, "%s(%d), %d,%d,%d tails=%d\n", __FILE__, __LINE__, ipinit, ipupdate, iptest, lp->tails);
   assert(lp->tails);
   if (ipinit)
   {
      ipl = FindCompilerFlag(lp->preheader, CF_LOOP_INIT);
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
PrintThisInst(stderr, 0, ip);
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
   if (NeedKilling)
      KillLoopControl(lp);
   if (AlreadySimpleLC(lp))
      lp->flag |= (L_NSIMPLELC_BIT | L_SIMPLELC_BIT);
/*
 * if we've not yet determined what kind of loop control to use, do so
 */
   if (!(lp->flag & (L_FORWARDLC_BIT | L_SIMPLELC_BIT)))
   {
      il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
      if (!AlreadySimpleLC(lp) && il)
        lp->flag |= L_FORWARDLC_BIT;
      else lp->flag |= L_SIMPLELC_BIT;
      if (il)
         KillIlist(il);
   }
   if (lp->flag & L_FORWARDLC_BIT)
   {
      fprintf(stderr, "\nIndex refs in loop prevent SimpleLC!!!\n\n");
      ForwardLoop(lp, unroll, &ipinit, &ipupdate, &iptest);
   }
   else
   {
      fprintf(stderr, "\nLoop good for SimpleLC!!!\n\n");
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
 */
{
   static char ln[256];
   char *sp;
   sp = STname[ilab-1];
   if (!strncmp(sp, "_IFKOCD", 7) && isdigit(sp[7]))
   {
      sp += 7;
      while (*sp && *sp != '_') sp++;
      assert(*sp == '_' && sp[1]);
      sp++;
   }
   sprintf(ln, "_IFKOCD%d_%s", dupnum, sp);
   return(ln);
}

BBLOCK *DupCFScope(short ivscp0, /* original scope */
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
      nhead->usucc = DupCFScope(ivscp0, ivscp, dupnum, head->usucc);
   if (head->csucc && BitVecCheck(ivscp, head->csucc->bnum-1))
      nhead->csucc = DupCFScope(ivscp0, ivscp, dupnum, head->csucc);
   return(nhead);
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

BLIST *FindAllFallHeads(BLIST *ftheads, int iscope, BBLOCK *head, int tails,
                        int inblks)
{
   BBLOCK *bp;
/*
 * If we've added all blocks in scope, or head is already in, or head is not
 * in scope, stop recursion
 */
   if (!head || !BitVecComp(iscope, inblks))
      return(ftheads);
   if (BitVecCheck(inblks, head->bnum-1) || !BitVecCheck(iscope, head->bnum-1))
      return(ftheads);
   
   bp = FindFallHead(head, tails, inblks);
   if (bp)
      ftheads = AddBlockToList(ftheads, bp);
   if (head->usucc && !BitVecCheck(inblks, head->usucc->bnum-1))
      ftheads = FindAllFallHeads(ftheads, iscope, head->usucc, tails, inblks);
   if (head->csucc && !BitVecCheck(inblks, head->csucc->bnum-1))
      ftheads = FindAllFallHeads(ftheads, iscope, head->csucc, tails, inblks);
   return(ftheads);
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
   extern BBLOCK *bbbase;
   extern int FKO_BVTMP;

/*
 * If no cleanup is needed, return
 */
   if (lp->CU_label == -1)
      return;
/*
 * Beginning of cleanup code is loop body label with _CU_ prefixed
 * post-tail cleanup-done lab is body label with _CUDONE_ prefixed
 */
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

/*
 * Find all fall-thru path headers in loop; iv becomes blocks we have already
 * added
 */
   SetVecAll(iv, 0);
   ivtails = BlockList2BitVec(lp->tails);
   ftheads = FindAllFallHeads(NULL, lp->blkvec, lp->header, ivtails, iv);
   ftheads = ReverseBlockList(ftheads);
fprintf(stderr, "ftheads = %s\n", PrintBlockList(ftheads));
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
         bp0->down->up = bp0;
         bp0 = bp0->down;
         if (BitVecCheck(ivtails, bp->bnum-1))
            break;
      }
      bp0->down = NULL;
   }
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
   if (!(lp->flag & (L_FORWARDLC_BIT | L_SIMPLELC_BIT)))
   {
      if (AlreadySimpleLC(lp))
         lp->flag |= (L_NSIMPLELC_BIT | L_SIMPLELC_BIT);
      else
      {
         il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
         if (!AlreadySimpleLC(lp) && il)
            lp->flag |= L_FORWARDLC_BIT;
         else
            lp->flag |= L_SIMPLELC_BIT;
         if (il) KillIlist(il);
      }
   }
   FORWARDLOOP = L_FORWARDLC_BIT & lp->flag;
   unroll *= Type2Vlen(lp->vflag);
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
      if (!IS_CONST(STflag[lp->end-1]))
      {
         InsNewInst(bp, NULL, ipnext, LD, -r1, SToff[lp->end-1].sa[2], 0);
         InsNewInst(bp, NULL, ipnext, ADD, -r1, -r1, 
                            STiconstlookup(unroll*SToff[lp->inc-1].i-1));
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
   extern int FKO_BVTMP;
   extern BBLOCK *bbbase;

   URmul = Type2Vlen(FLAG2TYPE(lp->vflag));
   UR = lp->vflag ? URmul*unroll : unroll;
   KillLoopControl(lp);
PrintInst(fopen("err.tmp", "w"), bbbase);
   il = FindIndexRef(lp->blocks, SToff[lp->I-1].sa[2]);
   if (il) KillIlist(il);
   else UsesIndex = 0;
   pi0 = FindMovingPointers(lp->blocks);
   if (!pi0)
      UsesPtrs = 0;
   UnrollCleanup(lp, unroll);
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
      newCF = DupCFScope(lp->blkvec, iv, i, lp->header);
      iv = BitVecCopy(iv, lp->blkvec);
/*
 *    Use CF to produce a block list of duped blocks
 */
      SetVecBit(iv, lp->header->bnum-1, 1);
      dupblks[i-1] = CF2BlockList(NULL, iv, newCF);
fprintf(stderr, "dupblks[%d] = %s\n", i-1, PrintBlockList(dupblks[i-1]));
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
 */
      if (UsesIndex)
         UpdateUnrolledIndices(dupblks[i-1], lp->I, (lp->flag & L_NINC_BIT) ?
                               URbase-i : URbase+i);
   }
   if (UsesIndex)
      UpdateUnrolledIndices(lp->blocks, lp->I, URbase);
                            
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
   RemoveLoopFromQ(optloop);
   optloop->depth = 0;
   KillAllLoops();
#else
   InvalidateLoopInfo();
#endif
   NewBasicBlocks(bbbase);
   CheckFlow(bbbase, __FILE__, __LINE__);
// ShowFlow("dot.err", bbbase);
   FindLoops();  /* need to setup optloop for this */
   CheckFlow(bbbase, __FILE__, __LINE__);
   return(0);
}

void AddPrefetch(LOOPQ *lp, int unroll)
/*
 * Inserts prefetch inst as first active inst in loop header
 * NOTE: assumes called after loop unrolling, but before repeatable opt
 */
{
   BBLOCK *bp;
   INSTQ *ipp;
   short ir, ptr, lvl;
   int i, j, n, npf;
   enum inst inst;

   bp = lp->header;
   assert(bp->ilab == lp->body_label);
   ir = GetReg(T_INT);
   for (i=1,n=lp->pfarrs[0]; i <= n; i++)
   {
      ptr = lp->pfarrs[i];
#if 0  /* sets/uses not yet figured */
      inst = BitVecCheck(lp->sets, lp->pfarrs[i]) ? PREFW : PREFR;
#else
      inst = PREFR;
#endif
      lvl = lp->pfflag[i] & 0x7;
/*
 *    # of pref to issue is CEIL(unroll*sizeof(), LINESIZE)
 */
      npf = unroll > 1 ? unroll : 1;
      npf *= type2len(STflag[lp->pfarrs[i]-1]);
      if (!IS_VEC(lp->pfarrs[i]) && IS_VEC(lp->vflag))
         npf *= Type2Vlen(lp->vflag);
      npf = (npf + LINESIZE[lvl]-1) / LINESIZE[lvl];
      ipp = bp->ainst1;
      for (j=0; j < npf; j++)
      {
         ipp = InsNewInst(bp, ipp, NULL, LD, -ir, SToff[ptr-1].sa[2], 0);
         ipp = InsNewInst(bp, ipp, NULL, inst, 0, AddDerefEntry(-ir, 0, 0, 
                          lp->pfdist[i]+j*LINESIZE[j],ptr),STiconstlookup(lvl));
      }
   }
   GetReg(-1);
}
