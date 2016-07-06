#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,104,156,208,416,884,1768,3536
 * N : 25,104,156,208,416,884,1768,3536
 * NB : 9,12,12,52,52,52,52,52
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 64) (nb_) = 9; \
   else if ((n_) < 182) (nb_) = 12; \
   else (nb_) = 52; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
