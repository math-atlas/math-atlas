#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,208,260,364,728,1092,1508,3016,6032
 * N : 25,156,208,260,364,728,1092,1508,3016,6032
 * NB : 9,8,56,52,52,52,52,104,104,104
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 9; \
   else if ((n_) < 182) (nb_) = 8; \
   else if ((n_) < 234) (nb_) = 56; \
   else if ((n_) < 1300) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
