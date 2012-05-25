#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,240,480,1020,1560,1800,1920,1980,2040,2100
 * N : 25,120,240,480,1020,1560,1800,1920,1980,2040,2100
 * NB : 9,60,60,60,60,60,60,60,60,60,120
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 9; \
   else if ((n_) < 2070) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
