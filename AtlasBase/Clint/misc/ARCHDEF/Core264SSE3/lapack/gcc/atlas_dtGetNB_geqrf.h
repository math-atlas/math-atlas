#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,168,224,336,504,560,672,1344,2016,2744,5488
 * N : 25,168,224,336,504,560,672,1344,2016,2744,5488
 * NB : 4,12,60,60,56,112,112,112,112,168,168
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 4; \
   else if ((n_) < 196) (nb_) = 12; \
   else if ((n_) < 420) (nb_) = 60; \
   else if ((n_) < 532) (nb_) = 56; \
   else if ((n_) < 2380) (nb_) = 112; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
