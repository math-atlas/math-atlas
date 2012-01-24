#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,280,600
 * N : 25,120,280,600
 * NB : 4,40,40,40
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 4; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
