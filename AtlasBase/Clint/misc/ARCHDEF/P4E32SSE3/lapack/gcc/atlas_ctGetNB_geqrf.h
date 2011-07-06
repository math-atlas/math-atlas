#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,240,480,1020,2040
 * N : 25,120,240,480,1020,2040
 * NB : 11,60,60,60,60,60
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 11; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
