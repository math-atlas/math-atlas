#include <ifko.h>
#include <fko_arch.h>

static IGLIST *iglist=NULL;

char *Int2Reg(int i)
/*
 * Translates integral encoding to machine-specific registers
 */
{
   static char ln[128];

   assert (i < 0);
   i = -i;
   if (i >= IREGBEG && i < IREGEND)
      sprintf(ln, "%s", archiregs[i-IREGBEG]);
   else if (i >= FREGBEG && i < FREGEND)
      sprintf(ln, "%s", archfregs[i-FREGBEG]);
   else if (i >= DREGBEG && i < DREGEND)
      sprintf(ln, "%s", archdregs[i-DREGBEG]);
   else if (i >= ICCBEG && i < ICCEND)
      sprintf(ln, "%s", ICCREGS[i-ICCBEG]);
   else if (i >= FCCBEG && i < FCCEND)
      sprintf(ln, "%s", FCCREGS[i-FCCBEG]);
   else if (i == PCREG)
      sprintf(ln, "%s", "PC");
   else
   {
   fprintf(stderr, 
      "I=[%d,%d); F=[%d,%d); D=[%d,%d); ICC=[%d,%d); FCC=[%d,%d); PC=%d\n",
           IREGBEG, IREGEND,FREGBEG, FREGEND,DREGBEG, DREGEND,
           ICCBEG, ICCEND, FCCBEG, FCCEND, PCREG);
      fko_error(__LINE__, "Unknown register index %d, file=%s\n",
                i, __FILE__);
   }
   return(ln);
}

short Reg2Int(char *regname)
/*
 * Given a register of regname, returns integer number
 */
{
   int i;
   if (regname[0] == 'P' && regname[1] == 'C' && regname[2] == '\0')
      return(-PCREG);
   if (regname[0] == 'S' && regname[1] == 'P' && regname[2] == '\0')
      return(-REG_SP);
   for (i=IREGBEG; i < IREGEND; i++)
      if (!strcmp(archiregs[i], regname)) return(-i-1);
   for (i=FREGBEG; i < FREGEND; i++)
      if (!strcmp(archfregs[i], regname)) return(-i-1);
   for (i=DREGBEG; i < DREGEND; i++)
      if (!strcmp(archdregs[i], regname)) return(-i-1);
   for (i=ICC0; i < NICC; i++)
      if (!strcmp(ICCREGS[i], regname)) return(-i-1);
   for (i=FCC0; i < NFCC; i++)
      if (!strcmp(FCCREGS[i], regname)) return(-i-1);
   return(0);
}

int NumberArchRegs()
{
   return(TNREG);
}

IGLIST *NewIGNodeList(IGNODE *ig, IGLIST *next)
{
   IGLIST *ip;
   ip = malloc(sizeof(IGLIST));
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

IGLIST *AddIGNodeToListCheck(IGLIST *list, IGNODE *node)
/* 
 * Adds a ignode to list only if it is not already there
 * RETURNS: ptr to list
 */
{
   IGLIST *lp;
   if (list) list->next = NewIGNodeList(node, list->next);
   else 
   {
      for (lp=list; lp && lp->ignode != node; lp = lp->next);
      if (!lp)
         list = NewIGNodeList(node, NULL);
   }
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

IGLIST *RemoveIGNodeFromList(IGLIST *list, IGNODE *node)
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
   KillIGListEntry(lp);
   return(list);
}

IGNODE *NewIGNode(short var, BBLOCK *blk)
{
   IGNODE *ig;

   ig = malloc(sizeof(IGNODE));
   if (blk) ig->myblocks = NewBlockList(blk, NULL);
   else ig->myblocks = NULL;
   ig->myblkreg = NewBitVec(32);
   if (blk) SetVecBit(ig->myblkreg, blk->bnum-1, 1);
   ig->conflicts = NULL;
   ig->freq = 0;
   ig->regstate = NewBitVec(32);
   ig->var = var;
   iglist = NewIGNodeList(ig, iglist);
   return(ig);
}

void KillIGNode(IGNODE *kill)
{
   if (kill)
   {
      if (kill->myblocks) KillBlockList(kill->myblocks);
      if (kill->conflicts) KillAllIGList(kill->conflicts);
      if (kill->regstate) KillBitVec(kill->regstate);
      if (kill->myblkreg) KillBitVec(kill->myblkreg);
      free(kill);
   }
}


IGNODE *FindIGNodeByVar(IGLIST *list, short var)
{
   for (; list && list->ignode->var != var; list = list->next)
   return(list ? list->ignode : NULL);
}

IGLIST *IGNodeConflictAll(IGLIST *list, IGNODE *node)
/*
 * Creates a new list from list, containing all nodes except node
 * RETURNS: new list
 */
{
   IGLIST *new=NULL, *lp, *lo;

   for (lo=list; lo; lo = lo->next)
      if (lo->ignode != node)
         new = AddIGNodeToList(new, lo->ignode);
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
      if (bp->ignodes) KillAllIGList(bp->ignodes);
   }
}

void KillAllIG()
{
   IGLIST *lp;
   BBLOCK *bp;
   extern BBLOCK *bbbase;

   for (bp=bbbase; bp; bp = bp->down)
      KillAllBlockIG(bp);
   for(lp=iglist; lp; lp = lp->next)
      KillIGNode(lp->ignode);
   KillAllIGList(iglist);
   iglist = NULL;
}

void CalcBlockIG(BBLOCK *bp)
/*
 * Calculates register interference graph for block bp
 */
{
   short *vals;
   int i, n, k, iv;
   IGNODE *node;
   INSTQ *ip;
   int blkstate;
   extern int FKO_BVTMP;

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
         node = NewIGNode(vals[i]-TNREG+1, bp);
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
            node = FindIGNodeByVar(bp->ignodes, k);
            if (!node)
            {
               node = NewIGNode(vals[i]-TNREG+1, bp);
               node->conflicts = IGNodeConflictAll(bp->conout, node);
               node->regstate = BitVecCopy(node->regstate, blkstate);
               bp->conout = AddIGNodeToList(bp->conout, node);
            }
            else node->freq++;
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

IGNODE *CombineIGNodes(IGNODE *succ, IGNODE *pred)
/*
 * Combines interference graph node of successor and predecessor block,
 * RETURNS: the combined IGNODE
 */
{
   IGNODE *ig;
   IGLIST *igl;
   BLIST *bl;

   assert(succ->var == pred->var);
   ig = NewIGNode(succ->var, NULL);
   BitVecComb(ig->myblkreg, succ->myblkreg, pred->myblkreg, '|');
   ig->myblocks = BitVec2BlockList(ig->myblkreg);
   for (igl=succ->conflicts; igl; igl = igl->next)
      ig->conflicts = AddIGNodeToList(ig->conflicts, igl->ignode);
   for (igl=pred->conflicts; igl; igl = igl->next)
      ig->conflicts = AddIGNodeToListCheck(ig->conflicts, igl->ignode);
   ig->LRbeg = pred->LRbeg;
   ig->LRend = succ->LRend;
   ig->freq  = pred->freq + succ->freq;
   ig->regstate = BitVecComb(0, pred->regstate, succ->regstate, '|');
   ig->var = succ->var;
   return(ig);
}

void CalcIG(BLIST *blist)
/*
 * Calculates the interference graph for list of blocks given in blist
 * NOTE: live ranges are connected only between blocks in this list
 */
{
   BLIST *lp, *preds;
   extern BBLOCK *bbbase;
   IGLIST *in, *out, *inprev, *outprev;
   IGNODE *ignew;
   int blkvec;

   if (!CFUSETU2D)
      CalcInsOuts(bbbase);
   if (!INDEADU2D)
      CalcAllDeadVariables();
   for (lp=blist; lp; lp = lp->next)
      CalcBlockIG(lp->blk);
/*
 * Create a block list so we can easily check whether to join live ranges
 */
   blkvec = BlockList2BitVec(blist);
   for (lp=blist; lp; lp = lp->next)
   {
      for (preds = lp->blk->preds; preds; preds = preds->next)
      {
         if (BitVecCheck(blkvec, preds->blk->bnum-1))
         {
            for (in = lp->blk->conin; in; in = in->next)
            {
               for (out=lp->blk->preds->blk->conout; out; out = out->next)
               {
                  if (in->ignode->var == out->ignode->var)
                  {
                     ignew = CombineIGNodes(in->ignode, out->ignode);
                     KillIGNode(in->ignode);
                     KillIGNode(out->ignode);
                     in->ignode = out->ignode = ignew;
                  }
               }
            }
         }
      }
   }
}
