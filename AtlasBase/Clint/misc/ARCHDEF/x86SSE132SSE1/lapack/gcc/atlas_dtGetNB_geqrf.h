#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,84,126,168,210,462,966,1932,3864
 * N : 25,84,126,168,210,462,966,1932,3864
 * NB : 2,6,6,18,42,42,42,42,42
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 54) (nb_) = 2; \
   else if ((n_) < 147) (nb_) = 6; \
   else if ((n_) < 189) (nb_) = 18; \
   else (nb_) = 42; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
