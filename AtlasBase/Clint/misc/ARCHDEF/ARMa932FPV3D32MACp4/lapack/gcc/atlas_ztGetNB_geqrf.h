#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,521,1018,2012,4000
 * N : 25,87,149,273,521,1018,2012,4000
 * NB : 4,24,24,24,24,30,30,30
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 769) (nb_) = 24; \
   else (nb_) = 30; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
