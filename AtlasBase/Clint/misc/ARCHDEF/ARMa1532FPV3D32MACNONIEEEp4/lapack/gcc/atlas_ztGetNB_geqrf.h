#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,88,132,220,440,924
 * N : 25,88,132,220,440,924
 * NB : 5,8,44,44,44,44
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 5; \
   else if ((n_) < 110) (nb_) = 8; \
   else (nb_) = 44; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
