#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,224,448,896,1792,3640,7336
 * N : 25,112,224,448,896,1792,3640,7336
 * NB : 4,56,56,56,56,112,112,112
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 4; \
   else if ((n_) < 1344) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
