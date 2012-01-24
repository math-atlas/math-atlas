#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,168,336,672
 * N : 25,96,168,336,672
 * NB : 4,24,24,24,24
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else (nb_) = 24; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
