#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,136,340,748,1564,3128
 * N : 25,136,340,748,1564,3128
 * NB : 5,68,68,68,68,68
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 80) (nb_) = 5; \
   else (nb_) = 68; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
