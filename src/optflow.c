/*
 * NOTE: make all funcs return int of # of changes, so we can see if
 *       the phase has done anything
 */
int UselessJumpElim(BBLOCK *base)
/*
 * Finds unconditional jumps to bp->down, and removes them
 */
{
}
int BranchChaining(BBLOCK *base)
/*
 * Replaces jumps to unconditional jump(s) with final target
 */
{
}

int DeadCodeElim(BBLOCK *base)
/*
 * Finds blocks other than base that have no predecessors, and delete them.
 */
{
   if (!base) return;
   for (bp=base->down; bp; bp = bp->down)
   {
      if (!AnyBitsSet(bp->preds))
      {
      }
   }
}

int LabelFusion(BBLOCK *base)
/*
 * Finds blocks consisting only of a label, and either moves that label down
 * to the successor block, or if the successor block already has a label,
 * replaces all jumps to the useless label with jumps to the successor label.
 */
{
}
