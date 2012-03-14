#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,228,532,1140,2356
 * N : 25,228,532,1140,2356
 * NB : 7,76,76,76,152
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 126) (nb_) = 7; \
   else if ((n_) < 1748) (nb_) = 76; \
   else (nb_) = 152; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
