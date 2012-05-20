#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,364,520,624,728,1508,3016
 * N : 25,156,364,520,624,728,1508,3016
 * NB : 4,52,52,52,52,104,104,104
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 4; \
   else if ((n_) < 676) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
