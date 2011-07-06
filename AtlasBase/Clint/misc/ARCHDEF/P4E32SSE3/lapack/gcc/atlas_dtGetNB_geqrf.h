#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,144,324,684,1368,2736
 * N : 25,72,144,324,684,1368,2736
 * NB : 1,36,36,36,36,72,108
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 1; \
   else if ((n_) < 1026) (nb_) = 36; \
   else if ((n_) < 2052) (nb_) = 72; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
