#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,160,320,680,1400,2800
 * N : 25,80,160,320,680,1400,2800
 * NB : 2,40,40,40,40,80,80
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 2; \
   else if ((n_) < 1040) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
