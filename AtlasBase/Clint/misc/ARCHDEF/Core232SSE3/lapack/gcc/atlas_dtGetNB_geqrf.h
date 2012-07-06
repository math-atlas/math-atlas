#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,208,312,624,936,1300,2652,5304
 * N : 25,156,208,312,624,936,1300,2652,5304
 * NB : 2,12,104,104,104,104,156,156,156
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 2; \
   else if ((n_) < 182) (nb_) = 12; \
   else if ((n_) < 1118) (nb_) = 104; \
   else (nb_) = 156; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
