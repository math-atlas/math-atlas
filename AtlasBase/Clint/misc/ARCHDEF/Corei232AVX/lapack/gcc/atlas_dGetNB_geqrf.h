#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,162,378,756,1512,3024
 * N : 25,162,378,756,1512,3024
 * NB : 2,54,54,54,54,108
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 93) (nb_) = 2; \
   else if ((n_) < 2268) (nb_) = 54; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
