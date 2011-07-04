#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,240,480,1020,2040,4140
 * N : 25,120,240,480,1020,2040,4140
 * NB : 1,60,60,60,60,60,60
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 1; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
