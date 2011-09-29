#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,234,390,780,1170,1404,1638,3276
 * N : 25,156,234,390,780,1170,1404,1638,3276
 * NB : 4,12,12,24,16,24,78,78,78
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 4; \
   else if ((n_) < 312) (nb_) = 12; \
   else if ((n_) < 585) (nb_) = 24; \
   else if ((n_) < 975) (nb_) = 16; \
   else if ((n_) < 1287) (nb_) = 24; \
   else (nb_) = 78; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
