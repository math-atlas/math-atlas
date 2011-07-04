#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,360,720,1560,3120
 * N : 25,360,720,1560,3120
 * NB : 11,120,120,120,120
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 192) (nb_) = 11; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
