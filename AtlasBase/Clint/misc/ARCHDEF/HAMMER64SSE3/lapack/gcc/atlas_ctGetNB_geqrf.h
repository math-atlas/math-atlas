#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,252,336,504,1008,2016
 * N : 25,252,336,504,1008,2016
 * NB : 4,42,84,84,84,84
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 138) (nb_) = 4; \
   else if ((n_) < 294) (nb_) = 42; \
   else (nb_) = 84; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
