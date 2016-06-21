#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,224,448,896,1344,1792,3640,7280
 * N : 25,112,224,448,896,1344,1792,3640,7280
 * NB : 2,56,56,56,56,112,112,112,168
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 2; \
   else if ((n_) < 1120) (nb_) = 56; \
   else if ((n_) < 5460) (nb_) = 112; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
