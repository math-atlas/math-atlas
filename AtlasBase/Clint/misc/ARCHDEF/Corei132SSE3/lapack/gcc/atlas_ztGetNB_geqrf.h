#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,312,676,1352,2704
 * N : 25,156,312,676,1352,2704
 * NB : 12,52,52,52,52,52
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 12; \
   else (nb_) = 52; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
