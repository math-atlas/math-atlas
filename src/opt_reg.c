#include <ifko.h>
#include <arch.h>

IGLIST *NewIGNodeList(IGNODE *ig, IGNODE *next)
{
   IGNODE *ip;
   ip = malloc(sizeof(IGNODE));
   ip->ignode = ig;
   ip->next = next;
   return(ip);
}

IGLIST *AddIGNodeToList(IGLIST *list, IGNODE *node)
/* 
 * Adds a ignode to list (in second position)
 * RETURNS: ptr to list
 */
{
   if (list) list->next = NewIGNodeList(node, list->next);
   else list = NewIGNodeList(node, NULL);
   return(list);
}

IGLIST *KillIGListEntry(IGLIST *lp)
{
   IGLIST *ln;
   if (lp)
   {
      ln = lp->next;
      free(lp);
   }
   else ln = NULL;
   return(ln);
}

void KillAllIGList(IGLIST *lp)
{
   while (lp) lp = KillIGListEntry(lp);
}

IGLIST *RemoveIGNodeFromList(BLIST list, IGNODE *node)
/*
 * Removes node from list if it exists
 * RETURNS: (possibly new) list
 */
{
   IGLIST *lprev, *lp;
   if (!list) return(NULL);
   if (list->ignode == node) list = list->next;
   else
   {
      for (lprev=list; lp && lp->ignode != node; lp = lp->next) lprev=lp;
      if (lp) lprev->next = lp->next;
   }
   KillIGNodeListEntry(lp);
   return(list);
}

IGNODE *NewIGNode(IGNODE *prev, IGNODE *next, short var, BBLOCK *blk)
{
   IGNODE *ig;

   ig = malloc(sizeof(IGNODE));
   if (blk) ig->myblocks = NewBlockList(blk, NULL);
   else ig->myblocks = NULL;
   ig->conflicts = NULL;
   ig->prev = prev;
   ig->next = next;
   ig->freq = 0;
   ig->regstate = NewBitVec(32);
   ig->var = var;
   return(ig);
}

void KillIGNode(IGNODE *kill)
{
   if (kill)
   {
      if (kill->myblocks) KillBlockList(kill->myblocks);
      if (kill->conflicts) KillIGList(kill->conflicts);
      if (ig->regstate) KillBitVec(ig->regstate);
   }
}

void KillAllIG(IGLIST *lp)
{
   for (; lp; lp = lp->next)
      KillIGNode(lp->ignode);
}

IGNODE *FindIGNodeByVar(IGLIST *list, short var)
{
   for (; list && list->ignode->var != var; list = list->next)
   return(list);
}

IGNODE *IGNodeConflictAll(IGLIST *list, IGNODE *node)
/*
 * Creates a new list from list, containing all nodes except node
 * RETURNS: new list
 */
{
   IGLIST *new=NULL, *lp, *lo;

   for (lo=list; lo; lo = lo->next)
      if (lo->ignode != node)
         new = AddIGNodeToList(new, lo->ignode)
   return(new);
}

int Reg2Regstate(int k)
/*
 * Given register k, set regstate so that all registers used by k are
 * represented (i.e., on some archs, float and double regs are aliased)
 * RETURNS: bitvec with appropriate register numbers set
 */

{
   static int iv=0;
   int i;

   if (!iv) iv = NewBitVec(TNREG);
   SetVecAll(iv, 0);
   SetVecBit(iv, k-1, 1);
   if (k >= FREGBEG && k < FREGEND)
   {
      #if defined(X86) || defined(PPC)
         SetVecBit(iv, k-FREGBEG+DREGBEG, 1);
      #elif defined(SPARC)
         SetVecBit(iv, ((k-FREGBEG)>>1)+DREGBEG, 1);
      #endif
   }
   else if (k >= DREGBEG && k < DREGEND)
   {
      #if defined(X86) || defined(PPC)
         SetVecBit(iv, k-DREGBEG+FREGBEG, 1);
      #elif defined(SPARC)
         i = k - DREGBEG;
         if (i < 32)
         {
            SetVecBit(iv, FREGBEG+i, 1);
            SetVecBit(iv, FREGBEG+i+1, 1);
         }
      #endif
   }
   else if (k <= 0)
   {
      if (iv) KillBitVec(iv);
      iv = 0;
   }
   else fko_error(__LINE__, "Unknown register index %d, file=%s\n",
                  k, __FILE__);
   return(iv);
}

void UpdateIGListsRegstate(IGLIST *list, int iv, char op)
{
   for (; list; list = list->next)
      BitVecComb(list->ignode->regstate, list->ignode->regstate, iv, op);
}

void KillAllBlockIG(BBLOCK *bp)
{
   for (; bp; bp = bp->down)
   {
      if (bp->conin) KillAllIGList(bp->conin);
      if (bp->conout) KillAllIGList(bp->conout);
      if (bp->ignodes)  /* HERE HERE HERE */
void KillAllIG(IGLIST *lp)
   }
}
void CalcBlockIG(BBLOCK *bp)
/*
 * Calculates register interference graph for block bp
 */
{
   short *vals;
   int i, n, k, iv;
   IGNODE *node;
   int blkstate;
   extern int FK_BVTMP;

   if (FKO_BVTMP)
   {
      blkstate = FKO_BVTMP;
      SetVecAll(blkstate, 0);
   }
   else FKO_BVTMP = blkstate = NewBitVec(TNREG);
   vals = BitVec2StaticArray(bp->ins);
   for (n=vals[0], i=1; i <= n; i++)
   {
      if (vals[i] >= TNREG)
      {
         node = NewIGNode(NULL, NULL, vals[i]-TNREG+1, bp);
         bp->conin  = AddIGNodeToList(bp->conin,  node);
         bp->conout = AddIGNodeToList(bp->conout, node);
         bp->ignodes = AddIGNodeToList(bp->ignodes, node);
         node->conflicts = IGNodeConflictAll(bp->conin, node);
      }
      else
      {
         k = vals[i] + 1;
         iv = Reg2Regstate(k);
         UpdateIGListsRegstate(bp->conin, iv, '|');
         BitVecComb(blkstate, blkstate, iv, '|');
      }
   }
   for (ip=bp->ainst1; ip; ip = ip->next)
   {
      vals = BitVec2StaticArray(ip->use);
      for (n=vals[0], i=1; i <= n; i++)
      {
         if (vals[i] >= TNREG)
         {
            k = vals[i] - TNREG + 1;
            node = FindIGNodeByVar(bp->ignodes, k);
            assert(node);
            node->freq++;
         }
      }
      vals = BitVec2StaticArray(ip->deads);
      for (n=vals[0], i=1; i <= n; i++)
      {
         k = vals[i];
         if (k >= TNREG)
            bp->conout = RemoveIGNodeFromList(bp->conout, 
                            FindIGNodeByVar(bp->conout, k-TNREG+1));
         else SetVecBit(blkstate, k, 0);
      }
      vals = BitVec2StaticArray(ip->set);
      for (n=vals[0], i=1; i <= n; i++)
      {
         k = vals[i];
         if (k >= TNREG)
         {
            k = k - TNREG + 1;
            if (!FindIGNodeByVar(bp->ignodes, k))
            {
               node = NewIGNode(NULL, NULL, vals[i]-TNREG+1, bp);
               node->conflicts = IGNodeConflictAll(bp->conout, node);
               node->regstate = BitVecCopy(node->regstate, blkstate);
               bp->conout = AddIGNodeToList(bp->conout, node);
            }
         }
         else
         {
            k = vals[i] + 1;
            iv = Reg2Regstate(k);
            UpdateIGListsRegstate(bp->conin, iv, '|');
            BitVecComb(blkstate, blkstate, iv, '|');
         }
      }
   }
}

void 
