#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,208,260,312,364,416,468,624,1300,2652,5304
 * N : 25,156,208,260,312,364,416,468,624,1300,2652,5304
 * NB : 2,12,56,56,64,72,80,104,104,104,104,104
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 2; \
   else if ((n_) < 182) (nb_) = 12; \
   else if ((n_) < 286) (nb_) = 56; \
   else if ((n_) < 338) (nb_) = 64; \
   else if ((n_) < 390) (nb_) = 72; \
   else if ((n_) < 442) (nb_) = 80; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
