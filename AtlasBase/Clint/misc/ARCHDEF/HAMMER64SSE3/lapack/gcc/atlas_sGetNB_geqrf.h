#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,252,588,1260,2520
 * N : 25,252,588,1260,2520
 * NB : 4,84,84,84,84
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 138) (nb_) = 4; \
   else (nb_) = 84; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
