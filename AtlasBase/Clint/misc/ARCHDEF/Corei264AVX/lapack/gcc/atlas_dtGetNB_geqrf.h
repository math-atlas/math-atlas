#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,364,728,1092,1456,2912,5876
 * N : 25,156,364,728,1092,1456,2912,5876
 * NB : 5,52,52,52,52,104,104,104
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 5; \
   else if ((n_) < 1274) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
