#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,104,260,572,1196,1768,2392
 * N : 25,104,260,572,1196,1768,2392
 * NB : 4,52,52,52,52,52,104
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 64) (nb_) = 4; \
   else if ((n_) < 2080) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
