#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,270,594,1188,2376,4806
 * N : 25,108,270,594,1188,2376,4806
 * NB : 1,54,54,54,54,108,108
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 1; \
   else if ((n_) < 1782) (nb_) = 54; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
