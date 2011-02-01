#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,168,336,672,840,1008,1344,2772
 * N : 25,168,336,672,840,1008,1344,2772
 * NB : 4,14,24,24,84,84,84,84
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 4; \
   else if ((n_) < 252) (nb_) = 14; \
   else if ((n_) < 756) (nb_) = 24; \
   else (nb_) = 84; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
