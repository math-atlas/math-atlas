#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,162,378,756,1512,3024
 * N : 25,162,378,756,1512,3024
 * NB : 9,54,54,54,54,54
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 93) (nb_) = 9; \
   else (nb_) = 54; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
