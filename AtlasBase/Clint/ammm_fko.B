@define BETA1 @1@
@PRE S C
   @define typ @FLOAT@
   @define sz @4@
@PRE D Z
   @define sz @8@
   @define typ @DOUBLE@
@PRE !
@beginskip
Should be called with VEC=[MDIM,KDIM], PRE=[S,D,C,Z] and

following defines:
   mu : m (scalar) unrolling
   nu : n (scalar) unrolling
   ku : k (scalar) unrolling
   vl : vector length to use
The following can be optionally defined:
   kb : compile-time constant K loop bound to use
   kp : # of kits to peel, must be a multiple of vku!
   pf : bit vec describing prefetch strategy
   pfLS : line size to assume for prefetch (64 bytes by default)

pf bit location meanings:
   prefC always done as just next mu*nu block
   pfA/B : can prefetch next mu/nu A/B within K-loop
   nA/nB : can prefetch next block outside K-loop 
   take pf integer bitvec bit/additive means:
      0/1  : prefetch C before K-loop
      1/2  : prefetch next block of A before K-loop
      2/4  : prefetch next block of B before K-loop
      3/8  : prefetch next mu*K iter of A inside K-loop
      4/16 : prefetch next nu*K iter of B inside K-loop
      5/32 : pref of C should use ATL_pfl1 instead of ATL_pfl2
      6/64 : pref of next blk of A should use ATL_pfl1 not ATL_pfl2
      7/128: pref of next blk of B should use ATL_pfl1 not ATL_pfl2
      8/256: K-loop pref of A use ATL_pfl1 not ATL_pfl2
      9/512: K-loop pref of B use ATL_pfl1 not ATL_pfl2
   We'll put pf bitvec in rout name, and then the search will find that
   we want to pref everything to L1 for small NB, only C &  block of A for
   medium size, and no pref for large, for instance.

During tuning, think about several regions for prefetch:
1. pref pfnA&B to L1:  m*n + 2*k*(m+n) < L1
   -> n^2 + 4n^2 < L2 ==> nb <= sqrt(L1/5)
2. pref B to L1, A to L2: m*n + 2*k*n + m*k < L1
   -> n^2 + 2n^2 + n^2 < L1 ==> nb <= sqrt(L1/4)
3. pref A&B to L2 so long as all 5 blocks fit (L2 size not known)
4. pref only one of nA/B to L2
5. No prefetch of next blocks (maybe internal prefetch)
@endskip
@ifdef ! pf
   @define pf @1@
@endifdef
@ifdef ! pfLS
   @define pfLS @64@
@endifdef
@iexp pfLS @(sz) @(pfLS) /
@iexp kk @(pf) 32 &
@iif kk = 0
   @define pfC @ATL_pfl2W@
@endiif
@ifdef ! pfC
   @define pfC @ATL_pfl1W@
@endifdef
@iexp kk @(pf) 64 &
@iif kk ! 0
   @define pfA @ATL_pfl1R@
@endiif
@ifdef ! pfA
   @define pfA @ATL_pfl2R@
@endifdef
@iexp kk @(pf) 128 &
@iif kk ! 0
   @define pfB @ATL_pfl1R@
@endiif
@ifdef ! pfB
   @define pfB @ATL_pfl2R@
@endifdef
@iexp kk @(pf) 8 &
@iif kk ! 0
   @define pfAk @ATL_pfl2R@
   @iexp kk @(pf) 256 &
   @iif kk ! 0
      @undef pfAk
      @define pfAk @ATL_pfl1R@
   @endiif
@endiif
@iexp kk @(pf) 16 &
@iif kk ! 0
   @define pfBk @ATL_pfl2R@
   @iexp kk @(pf) 512 &
   @iif kk ! 0
      @undef pfBk
      @define pfBk @ATL_pfl1R@
   @endiif
@endiif
@iexp kk @(mu) @(nu) 1 @(pf) & * *
@iif kk > pfLS
   @iexp npfC @(pfLS) @(kk) /
@endiif
@ifdef ! npfC
   @iif kk = 0
      @iexp npfC 0 0 +
   @endiif
   @iif kk ! 0
      @iexp npfC 1 0 +
   @endiif
@endifdef
@iexp npfA @(pfLS) 1 @(pf) r 1 & @(mu) @(nu) * * /
@iexp npfB @(pfLS) 2 @(pf) r 1 & @(mu) @(nu) * * /
@iexp npf @(npfC) @(npfA) +
@iif npfA ! 0
   @iexp npfA @(npfA) @(npfC) +
@endiif
@iif npfB ! 0
   @iexp npf @(npf) @(npfB) +
   @iexp npfB @(npf) 0 +
@endiif
@iexp pf @(pf) 31 &
@ifdef ! vl
   @abort "vl must be defined!"
@endifdef
@ifdef ! mu
   @abort "mu must be defined!"
@endifdef
@ifdef ! nu
   @abort "nu must be defined!"
@endifdef
@ifdef ! ku
   @abort "ku must be defined!"
@endifdef
@ifdef ! kb
   @define kb @0@
@endifdef
@iif kb = 0
   @addkeys KCON=no
@endiif
@iif kb ! 0
   @addkeys KCON=yes
@endiif
@VEC MDIM
   @iexp vmu @(vl) @(mu) /
   @iexp kk @(vmu) @(vl) *
   @iif kk ! mu
      @abort "MU=@(mu) illegal with VLEN=@(vl)!"
   @endiif
@VEC KDIM
   @iexp vku @(vl) @(ku) /
   @iexp kk @(vku) @(vl) *
   @iif kk ! ku
      @abort "KU=@(ku) illegal with VLEN=@(vl)!"
   @endiif
   @iexp kk @(vl) @(mu) @(nu) * %
   @iif kk ! 0
      @abort "MU*NU (@(mu)*@(nu)) must be a multiple of VLEN (@(vl))!"
   @endiif
@VEC !
@ifdef ! BETA1
   @ifdef ! BETA0
       @define ibet @-1@
   @endifdef
@endifdef
@ifdef BETA1
    @define ibet @1@
@endifdef
@ifdef BETA0
   @define ibet @0@
@endifdef
@BEGINPROC DoPref
@ENDPROC
@BEGINPROC DoIter0
@SKIP define internal vars, so they can popped off to leave caller unchanged
   @define DN @0@
   @define i @0@
   @define j @0@
   @define kk @0@ 
@SKIP 1-D with NU=1
   @iif nu = 1
         rB0 = pB[@(ib)];
         @iexp ib @(ib) 1 +
         @iexp i 0 0 +
         @iwhile i < @(mu)
         rC@(i)_0 = pA[@ia];
            @iexp ia @(ia) @(1) +
         rC@(i)_0 = rC@(i)_0 * rB0;
            @iexp mo 0 0 +
            @callproc DoPref
            @iexp i @(i) 1 +
         @endiwhile
   @endiif
   @iif nu > 1
@SKIP 1-D with MU=1, NU > 1
      @iif mu = 1
         rA0 = pA[@(ia)];
         pA += 1;
         @iexp j 0 0 +
         @iwhile j < @(nu)
            rC0_@(j) = pB[@(j)];
            rC0_@(j) = rC0_@(j) * rA0;
            @iexp mo 0 0 +
            @callproc DoPref
         @endiwhile
         pB += @(nu);
      @endiif
@SKIP 2-D case assumes all but last rB already loaded
      @iif mu > 1
         @iexp i 0 0 +
         @iwhile i < @(mu)
            rA@(i) = pA[@(ia)];
            @iexp ia @(ia) 1 +
            @iexp i @(i) 1 +
         @endiwhile
            pA += @(mu);
            rB@(jl) = pB[@(jl)];
            pB += @(nu);
         @iexp j 0 0 +
         @iwhile j < @(nu)
            @iexp i 0 0 +
            @iwhile i < @(mu)
         rC@(i)_@(j) = rA@(i) * rB@(j);
               @iexp mo 0 0 +
               @iif j = jl
               rA@(i) = pA[@(ia)];
                  @iexp ia @(ia) 1 +
                  @iexp mo @(mo) 1 +
               @endiif
               @iif i = il
                  @iif j ! jl
               rB@(j) = pB[@(j)];
                  @endiif
               @endiif
            @callproc DoPref
               @iexp i @(i) 1 +
            @endiwhile
            @iexp j @(j) 1 +
         @endiwhile
      @endiif
@SKIP pop our defs so caller's macros of same name aren't changed
   @undef i
   @undef j
   @undef kk
@ENDPROC

@iexp ia 0 0 +
@iexp ib 0 0 +
ROUTINE ATL_USERMM;
   PARAMS :: nmus, nnus, K, pA, pB, pC, pAn, pBn, pCn;
   INT :: nmus, nnus, K;
   @(typ)_PTR :: pA, pB, pC, pAn, pBn, pCn;
//
// Performs a GEMM with M,N,K unrolling (& jam) of (@(mu),@(nu),@(ku)).
@VEC KDIM `// Can be vectorized (VLEN=@(vl)) along K dimension`
@VEC MDIM `// Can be vectorized (VLEN=@(vl)) along M dimension`
@iif kb ! 0
// K-loop us fully unrolled, so K must be @(kb).
@endiif
//
ROUT_LOCALS 
   INT :: i, j, k, incAm, incBn;
   @(typ)_PTR :: pB0, pA0;
   @declare "   @(typ) :: " y n ";"
      @iexp i 0 0 +
      @iwhile i < @(mu)
         rA@(i)
         @iexp i @(i) 1 +
      @endiwhile
      @iexp j 0 0 +
      @iwhile j < @(nu)
         rB@(j)
         @iexp j @(j) 1 +
      @endiwhile
      @iexp j 0 0 +
      @iwhile j < @(nu)
         @iexp i 0 0 +
         @iwhile i < @(mu)
            rC@(i)_@(j)
            @iif ibet ! 0
            rC@(i)_@(j)m
            @endiif
            @iexp i @(i) 1 +
         @endiwhile
         @iexp j @(j) 1 +
      @endiwhile
   @enddeclare
ROUT_BEGIN
   @iexp mo 0 0 +
   @iexp ipf 0 0 +
   @iexp jl @(nu) -1 +
   @iexp il @(mu) -1 +
   @iexp jpf 0 -1 +
   pB0 = pB;
   pA0 = pA;
   incAm = K * @(mu);
   incBn = K * @(nu);
   K = K - 1;
   i = nmus;
   MLOOP:
   @iif mu > 1
      @iexp j 0 0 +
      @iwhile j < @(jl)
      rB@(j) = pB[@(j)];
         @iexp j @(j) 1 +
      @endiwhile
   @endiif
      j = nnus;
      NLOOP:
//       Peel K=0 iteration to avoid zero of rCxx and extra add
   @iexp IN_K 0 0 +
   @iexp npeel 1 0 +
   @callproc DoIter0
   @BEGINSKIP
         LOOP k = 0, K
         LOOP_BODY
         LOOP_END
   @ENDSKIP
   @iexp k 0 0 +
   @iexp j 0 0 +
   @iwhile j < @(nu)
      @iexp i 0 0 +
      @iwhile i < @(mu)
         @iif ibet ! 0
            rC@(i)_@(j)m = pC[@(k)];
            @iif ibet = 1
            rC@(i)_@(j) += rC@(i)_@(j)m;
            @endiif
            @iif ibet = -1
            rC@(i)_@(j) = rC@(i)_@(j) - rC@(i)_@(j)m;
            @endiif
         @endiif
            pC[@(k)] = rC@(i)_@(j);
         @iexp k @(k) 1 +
         @iexp i @(i) 1 +
      @endiwhile
      @iexp j @(j) 1 +
   @endiwhile
         pC += @(k);
   @iif ku = kb
         pB += incBn;
   @endiif
   @iif ku ! kb
         pA = pA0;
   @endiif
         j = j - 1;
      IF (j > 0) GOTO NLOOP;
      pB = pB0;
      pA0 += incAm;
      pA = pA0;
      i = i - 1;   
   IF (i > 0) GOTO MLOOP;

ROUT_END
