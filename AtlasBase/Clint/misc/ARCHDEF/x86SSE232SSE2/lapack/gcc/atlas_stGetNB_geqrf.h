#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,136,340,748,1496,2992,5984
 * N : 25,136,340,748,1496,2992,5984
 * NB : 2,68,68,68,68,68,136
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 80) (nb_) = 2; \
   else if ((n_) < 4488) (nb_) = 68; \
   else (nb_) = 136; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
