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
   @define pCl @2@
@endiif
@ifdef ! pCl
   @define pCl @1@
@endifdef
@iexp kk @(pf) 64 &
@iif kk ! 0
   @define pAl @1@
@endiif
@ifdef ! pAl
   @define pAl @2@
@endifdef
@iexp kk @(pf) 128 &
@iif kk ! 0
   @define pBl @1@
@endiif
@ifdef ! pBl
   @define pBl @2@
@endifdef
@iexp kk @(pf) 8 &
@iif kk ! 0
   @define pAlk @2@
   @iexp kk @(pf) 256 &
   @iif kk ! 0
      @undef pAlk
      @define pAlk @1@
   @endiif
@endiif
@iexp kk @(pf) 16 &
@iif kk ! 0
   @define pBlk @2@
   @iexp kk @(pf) 512 &
   @iif kk ! 0
      @undef pBlk
      @define pBlk @1@
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
@echo @ifdef ! BETA1
@echo    @ifdef ! BETA0
@echo        @define ibet @-1@
@echo    @endifdef
@echo @endifdef
@echo @ifdef BETA1
@echo     @define ibet @1@
@echo @endifdef
@echo @ifdef BETA0
@echo    @define ibet @0@
@echo @endifdef
@skip Helper func for DoIter[0].
@skip IN: pAl,pBl,pCl,pfLS,npfA,npfB, npfC, mu, nu, IN_K;
@skip IN/OUT: mo, ipf
@BEGINPROC DoPref
   @define kk @dum@
   @skip kk = (mo == 0 || (mu == 2 && nu == 2))
   @iexp kk @(nu) 2 = @(mu) 2 = & @(mo) 0 = |
   @iif kk ! 0
      @iif ipf < npfC
         @iexp kk @(ipf) @(pfLS) *
                     _PREFETCHW(@(pCl), pC[@(kk)]);
         @iexp ipf @(ipf) 1 +
         @iexp mo @(mo) 1 +
      @endiif
      @skip if (mo = 0 && ipf < npfA)
      @iexp kk @(mo) 0 = @(npfA) @(ipf) < &
      @iif kk ! 0
         @iexp kk @(npfC) @(ipf) - @(pfLS) *
               _PREFETCHR(@(pAl),pAn[@(kk)]);
         @iexp ipf @(ipf) 1 +
         @iif ipf = npfA
               pAn += incAN;
         @endiif
         @iexp mo @(mo) 1 +
      @endiif
      @skip if (mo == 0 && ipf < npfB)
      @iexp kk @(mo) 0 = @(npfB) @(ipf) < &
      @iif kk ! 0
         @iexp kk @(npfA) @(ipf) - @(pfLS) *
               _PREFETCHR(@(pBl), pBn[@(kk)]);
         @iexp ipf @(ipf) 1 +
         @iif ipf = npfB
               pBn += incBN;
         @endiif
         @iexp mo @(mo) 1 +
      @endiif
      @iif IN_K ! 0
         @ifdef pBlk
            @iif mo = 0
            _PREFETCHR(@(pBlk), pB[incBn]);
               @undef pfBk
               @iexp mo @(mo) 1 +
            @endiif
         @endifdef
         @ifdef pAlk
            @iif mo = 0
            _PREFETCHR(@(pAlk), pA[incAm]);
               @undef pfAk
               @iexp mo @(mo) 1 +
            @endiif
         @endifdef
      @endiif
   @endiif
   @define kk @dum@
   @undef kk
@ENDPROC
@BEGINPROC DoIter0
@SKIP define internal vars, so they can popped off to leave caller unchanged
   @define i @0@
   @define j @0@
   @define kk @0@ 
@SKIP 1-D with NU=1
   @iif nu = 1
         rB0 = pB[@(ib)];
         pB += 1;
         @iexp ib @(ib) 1 +
         @iexp i 0 0 +
         @iwhile i < @(mu)
         rC@(i)_0 = pA[@(ia)];
            @iexp ia @(ia) 1 +
         rC@(i)_0 = rC@(i)_0 * rB0;
            @iexp mo 0 0 +
            @callproc DoPref
            @iexp i @(i) 1 +
         @endiwhile
         pA += @(ia);
         @iexp ia 0 0 +
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
            @iexp j @(j) 1 +
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
            @iexp ia 0 0 +
            rB@(jl) = pB[@(jl)];
            pB += @(nu);
            @iexp ib 0 0 +
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
@BEGINPROC DoIter
@SKIP define internal vars, so they can popped off to leave caller unchanged
   @define i @0@
   @define j @0@
   @define kk @0@ 
@SKIP 1-D with NU=1
   @iif nu = 1
            rB0 = pB[0];
            pB += 1;
         @iexp i 0 0 +
         @iwhile i < @(mu)
            rA@(i) = pA[@(i)];
            rC@(i)_0 += rA@(i) * rB0;
            @iexp mo 0 0 +
            @callproc DoPref
            @iexp i @(i) 1 +
         @endiwhile
            pA += @(mu);
   @endiif
   @iif nu > 1
      @iif mu = 1
            rA0 = pA[0];
            pA += 1;
         @iexp j 0 0 +
         @iwhile j < @(nu)
            rB@(j) = pB[@(j)];
            rC0_@(j) += rA0 * rB@(j);
            @iexp mo 0 0 +
            @callproc DoPref
            @iexp j @(j) 1 +
         @endiwhile
         pB += @(nu);
      @endiif
@SKIP 2-D case assumes all but last rB already loaded, pA & pB must be updated
      @iif mu > 1
         @iexp mo 0 0 +
         @iexp i 0 0 +
         @iexp j 0 0 +
         @iwhile j < @(nu)
            @iexp i 0 0 +
            @iwhile i < @(mu)
            rC@(i)_@(j) += rA@(i) * rB@(j);
         @iexp kk @(j) @(i) |
         @iif kk = 0
               rB@(jl) = pB[@(jl)];
               pA += @(mu);
            @iexp mo @(mo) 1 +
         @endiif
         @iexp kk @(j) @(nu) * @(i) +
         @iif kk = 1
               pB += @(nu);
         @endiif
         @iif j = jl
               rA@(i) = pA[@(i)];
            @iexp mo @(mo) 1 +
         @endiif
         @iif i = il
            @iif j ! jl
               rB@(j) = pB[@(j)];
               @iexp mo @(mo) 1 +
            @endiif
         @endiif
            @callproc DoPref
               @iexp i @(i) 1 +
            @endiwhile
            @iexp j @(j) 1 +
         @endiwhile
      @endiif
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
   @enddeclare
   @whiledef i
   @endwhile
   @whiledef j
   @endwhile
@echo   @declare "   @(typ) :: " y n ";"
@echo      @iexp j 0 0 +
@echo      @iwhile j < @(nu)
@echo         @iexp i 0 0 +
@echo         @iwhile i < @(mu)
@echo            rC@(i)_@(j)
@echo             @iif ibet ! 0
@echo             rC@(i)_@(j)m
@echo             @endiif
@echo            @iexp i @(i) 1 +
@echo         @endiwhile
@echo         @iexp j @(j) 1 +
@echo      @endiwhile
@echo   @enddeclare
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
         IF (K < 1) GOTO KDONE;
         LOOP k = 0, K
         LOOP_BODY
            @callproc DoIter
         LOOP_END
KDONE:
   @iexp k 0 0 +
   @iexp j 0 0 +
   @iwhile j < @(nu)
      @iexp i 0 0 +
      @iwhile i < @(mu)
@echo         @iif ibet ! 0
@echo         rC@(i)_@(j)m = pC[@(k)];
@echo            @iif ibet = 1
@echo         rC@(i)_@(j) += rC@(i)_@(j)m;
@echo            @endiif
@echo            @iif ibet = -1
@echo         rC@(i)_@(j) = rC@(i)_@(j) - rC@(i)_@(j)m;
@echo            @endiif
@echo         @endiif
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
//              pA = pA - incAm;
   @endiif
         j = j - 1;
      IF (j > 0) GOTO NLOOP;
      pB = pB0;
      pA0 += incAm;
      pA = pA0;
      i = i - 1;   
   IF (i > 0) GOTO MLOOP;

ROUT_END
