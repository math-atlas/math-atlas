#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,336,728,1120,1512,2296,3080,6160
 * N : 25,168,336,728,1120,1512,2296,3080,6160
 * NB : 4,56,56,56,112,112,112,168,168
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 4; \
   else if ((n_) < 924) (nb_) = 56; \
   else if ((n_) < 2688) (nb_) = 112; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
