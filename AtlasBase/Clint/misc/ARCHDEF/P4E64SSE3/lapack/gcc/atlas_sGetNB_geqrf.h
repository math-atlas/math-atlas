#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,300,660,1320,2700
 * N : 25,120,300,660,1320,2700
 * NB : 4,60,60,60,60,120
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 4; \
   else if ((n_) < 2010) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
