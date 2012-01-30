#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,88,220,440,924,1892
 * N : 25,88,220,440,924,1892
 * NB : 9,44,44,44,44,44
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 9; \
   else (nb_) = 44; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
