#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,336,672,1008,1176,1260,1344,1428,2940
 * N : 25,168,336,672,1008,1176,1260,1344,1428,2940
 * NB : 9,84,84,84,84,84,84,84,252,252
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 9; \
   else if ((n_) < 1386) (nb_) = 84; \
   else (nb_) = 252; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
