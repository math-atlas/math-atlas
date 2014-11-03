#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,648,1272,1896,2520
 * N : 25,648,1272,1896,2520
 * NB : 12,12,24,24,36
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 960) (nb_) = 12; \
   else if ((n_) < 2208) (nb_) = 24; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
