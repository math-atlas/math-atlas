#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,390,858,1716,3510
 * N : 25,156,390,858,1716,3510
 * NB : 4,78,78,78,78,78
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 4; \
   else (nb_) = 78; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
