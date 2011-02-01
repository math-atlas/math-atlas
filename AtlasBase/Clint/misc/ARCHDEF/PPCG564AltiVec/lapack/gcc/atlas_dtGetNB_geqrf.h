#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,224,280,336,392,448,504,616,728,1120,1512,3080
 * N : 25,168,224,280,336,392,448,504,616,728,1120,1512,3080
 * NB : 2,12,28,28,32,32,32,84,72,88,112,112,168
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 2; \
   else if ((n_) < 196) (nb_) = 12; \
   else if ((n_) < 308) (nb_) = 28; \
   else if ((n_) < 476) (nb_) = 32; \
   else if ((n_) < 560) (nb_) = 84; \
   else if ((n_) < 672) (nb_) = 72; \
   else if ((n_) < 924) (nb_) = 88; \
   else if ((n_) < 2296) (nb_) = 112; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
