int DoInitLoopAnal(void)
/*
 * Does some analysis on unoptimized code to determine how to vectorize loop
 * This will be later thrown away, and we will start again with either
 * vectorized/unrolled loop, or unvectorized code.
 * RETURNS: 0 if loop cannot be vectorized, 1 if it can
 */
{
   struct ptrinfo *pbase, *p;
   short *sp, *s;
   int iv, iv1;
   int i, j, k, n, N;
   extern int FKO_BVTMP;
/*
 * Get code into standard form for analysis
 */
   GenPrologueEpilogueStubs(bbbase, 0);
   NewBasicBlocks(bbbase);
   FindLoops();
   CheckFlow(bbbase,__FILE__,__LINE__);
   if (!optloop)
      return(0);
   CalcInsOuts(bbbase);
   CalcAllDeadVariables();
/*
 * Find all variables accessed in the loop
 */
   if (!FKO_BVTMP) FKO_BVTMP = NewBitVec(32);
   iv = FKO_BVTMP;
   for (bl=blocks; bl; bl = bl->next)
   {
      iv = BitVecComb(iv, bl->blk->uses, '|');
      iv = BitVecComb(iv, bl->blk->defs, '|');
   }
/*
 * Subtract off all registers
 */
   for (i=0; i < TNREG; i++)
      SetVecBit(iv, i, 0);
/*
 * Subtract off all non-fp vars
 */
   sp = BitVec2Array(iv, 1-TNREG);
   for (N=sp[0],n=0,i=1; i <= N; i++)
   {
      if (IS_FP(STflag[sp[i]-1]))
         sp[n++] = sp[i];
/*
 *    For non-fp var, if it's not the index var, give up
 */
      else if (sp[i] != optloop->I && sp[i] != optloop->end && 
               sp[i] != optloop->inc)
      {
         fko_warn(__LINE__, "Bailing on vect due to var %d,%s\n", sp[i]
                  STname[sp[i]-1] ? STname[sp[i]-1] : "NULL");
         free(sp);
         return(0);
      }
   }
/*
 * If no fp vars, nothing to vectorize
 */
   if (!n)
   {
      free(sp);
      return(0);
   }
/*
 * Find arrays to vectorize
 */
   pbase = FindMovingPointers(optloop->blocks);
   for (N=0,p=pbase; p; p = p->next)
   {
      if (IS_FP(STflag[p->ptr-1]))
      {
         N++;
         if ((p->flag | PTRF_CONTIG | PTRF_INC) != p->flag)
         {
            fko_warn(__LINE__, "Ptr movement prevents vectorization!!\n");
            free(sp);
            return(0);
         }
      }
   }
/*
 * Copy all moving pointers to optloop->varrs
 */
   s = malloc(sizeof(short)*(N+1));
   assert(s);
   s[0] = N;
   for (j=0,i=1,p=pbase; p; p = p->next)
      if (IS_FP(STflag[p->ptr-1]))
         s[i++] = p->ptr;
   optloop->varrs = s;
/*
 * Remove the moving arrays from scalar vals
 */
   for (k=i=0; i < n; i++)
   {
      for (j=0; j < N && s[j] != sp[i]; j++);
      if (j == N)
         sp[k++] = sp[i];
   }
   n -= k;
   assert(n >= 0);
/*
 * Sort scalar vals into livein,liveout, and tmp
 */
   if (n)
   {
/*
 *    Find fp scalars live on loop input
 */
      iv1 = Array2BitVec(n, sp, TNREG-1);
      BitVecComb(iv1, optloop->header->ins, '&');
      optloop->vslivein = BitVec2Array(iv, 1-TNREG);
/*
 *    Find vars live on loop exit, that are also accessed in post-tail
 *    NOTE: if a var is live on one posttail, we'll write it to all posttail,
 *          but later phase of dead assignment elim can clean this up
 *
 */
      SetVecAll(iv1, 0);
      SetVecAll(iv, 0);
      for (bl=tails; bl; bl = bl->next)
      {
         BitVecDup(iv1, bl->blk->outs, '=');
         if (bl->blk->usucc && 
             !BitVecCheck(optloop->blkvec, bl->usucc->blk->bnum-1))
            BitVecComb(iv1, bl->blk->usucc->ins, '&');
         else
         {
            assert(bl->blk->csucc &&
                   !BitVecCheck(optloop->blkvec, bl->csucc->blk->bnum-1));
            BitVecComb(iv1, bl->blk->csucc->ins, '&');
         }
         BitVecComb(iv, iv1, '|');
      }
      iv1 = Array2BitVec(n, sp, TNREG-1);
      BitVecComb(iv, iv1, '&');
      optloop->vsliveout =  BitVec2Array(iv, 1-TNREG);
/*
 *    Find scalars not live on entry or exit
 */
      BitVecComb(iv1, iv, '-');
      BitVecDup(iv, iv1, '=');
      iv1 = Array2BitVec(optloop->slivein[0], optloop->slivein+1, TNREG-1);
      BitVecComb(iv, iv1, '-');
      optloop->vstmp =  BitVec2Array(iv, 1-TNREG);
   }
   free(sp);
/*
 * Find out how to init livein vars
 */
   if (optloop->vslivein[0])
   {
      optloop->vsflagin = calloc(optloop->vslivein[0], sizeof(short));
      assert(optloop->vsflagin);
   }
/*
 * Find out how to reduce liveout vars
 */
   if (optloop->vsliveout[0])
   {
      optloop->vsflagout = calloc(optloop->vsliveout[0], sizeof(short));
      assert(optloop->vsflagout);
   }
   return(1);
}
