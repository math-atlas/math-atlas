#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,420,840,1680,3420
 * N : 25,180,420,840,1680,3420
 * NB : 4,60,60,60,60,120
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 4; \
   else if ((n_) < 2550) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
