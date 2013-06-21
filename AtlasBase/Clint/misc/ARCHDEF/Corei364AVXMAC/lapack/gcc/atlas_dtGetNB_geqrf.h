#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,224,336,728,896,952,1008,1120,1512,1680,1848,2240,3024,6104
 * N : 25,168,224,336,728,896,952,1008,1120,1512,1680,1848,2240,3024,6104
 * NB : 4,12,56,56,56,56,56,112,112,112,112,280,280,280,336
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 4; \
   else if ((n_) < 196) (nb_) = 12; \
   else if ((n_) < 980) (nb_) = 56; \
   else if ((n_) < 1764) (nb_) = 112; \
   else if ((n_) < 4564) (nb_) = 280; \
   else (nb_) = 336; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
