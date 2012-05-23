#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,224,336,728,1512,3080
 * N : 25,168,224,336,728,1512,3080
 * NB : 2,12,56,56,56,112,168
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 2; \
   else if ((n_) < 196) (nb_) = 12; \
   else if ((n_) < 1120) (nb_) = 56; \
   else if ((n_) < 2296) (nb_) = 112; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
