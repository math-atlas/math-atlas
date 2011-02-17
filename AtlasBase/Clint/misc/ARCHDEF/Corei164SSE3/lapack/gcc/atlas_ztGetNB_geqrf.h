#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,312,676,988,1144,1352,2756
 * N : 25,156,312,676,988,1144,1352,2756
 * NB : 4,52,52,52,52,52,104,104
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 4; \
   else if ((n_) < 1248) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
