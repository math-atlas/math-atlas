#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,300,660,1320,2640
 * N : 25,120,300,660,1320,2640
 * NB : 2,60,60,60,60,60
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 2; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
