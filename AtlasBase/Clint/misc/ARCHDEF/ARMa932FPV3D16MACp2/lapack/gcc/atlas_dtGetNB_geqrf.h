#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,108,144,180,216,252,324,684,1404
 * N : 25,72,108,144,180,216,252,324,684,1404
 * NB : 2,6,6,12,18,24,36,36,36,36
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 2; \
   else if ((n_) < 126) (nb_) = 6; \
   else if ((n_) < 162) (nb_) = 12; \
   else if ((n_) < 198) (nb_) = 18; \
   else if ((n_) < 234) (nb_) = 24; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
