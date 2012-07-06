#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,360,780,1620,3240
 * N : 25,180,360,780,1620,3240
 * NB : 2,60,60,60,60,120
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 2; \
   else if ((n_) < 2430) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
