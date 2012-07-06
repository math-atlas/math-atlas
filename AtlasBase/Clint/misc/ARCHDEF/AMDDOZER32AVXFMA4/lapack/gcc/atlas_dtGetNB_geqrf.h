#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,144,180,216,252,504,1044,2124,4284
 * N : 25,108,144,180,216,252,504,1044,2124,4284
 * NB : 2,6,36,36,54,72,72,72,72,72
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 2; \
   else if ((n_) < 126) (nb_) = 6; \
   else if ((n_) < 198) (nb_) = 36; \
   else if ((n_) < 234) (nb_) = 54; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
