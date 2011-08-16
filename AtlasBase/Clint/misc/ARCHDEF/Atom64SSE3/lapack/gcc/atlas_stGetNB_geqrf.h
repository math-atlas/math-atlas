#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,136,340,748,1564,3128
 * N : 25,136,340,748,1564,3128
 * NB : 4,68,68,68,68,68
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 80) (nb_) = 4; \
   else (nb_) = 68; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
