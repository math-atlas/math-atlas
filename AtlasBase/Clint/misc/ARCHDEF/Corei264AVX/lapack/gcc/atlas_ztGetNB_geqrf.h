#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,88,220,440,880,1804,3608
 * N : 25,88,220,440,880,1804,3608
 * NB : 5,44,44,44,44,44,44
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 5; \
   else (nb_) = 44; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
