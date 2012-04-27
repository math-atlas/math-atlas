#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,270,594,864,1188,2430,4860
 * N : 25,108,270,594,864,1188,2430,4860
 * NB : 5,54,54,54,54,108,108,108
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 5; \
   else if ((n_) < 1026) (nb_) = 54; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
