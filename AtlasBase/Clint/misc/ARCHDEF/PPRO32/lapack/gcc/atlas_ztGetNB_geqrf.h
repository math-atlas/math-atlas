#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,280,600
 * N : 25,120,280,600
 * NB : 3,40,40,40
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 3; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
