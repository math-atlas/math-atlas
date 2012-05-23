#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,144,324,684,1368,2772
 * N : 25,72,144,324,684,1368,2772
 * NB : 9,36,36,36,36,72,72
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 9; \
   else if ((n_) < 1026) (nb_) = 36; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
