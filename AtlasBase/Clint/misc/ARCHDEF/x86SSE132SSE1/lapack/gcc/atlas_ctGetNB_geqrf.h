#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,420,900,1800,3660
 * N : 25,180,420,900,1800,3660
 * NB : 9,60,60,60,60,60
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 9; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
