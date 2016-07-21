#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,360,528,600,696,1368,2736
 * N : 25,360,528,600,696,1368,2736
 * NB : 2,2,2,48,72,72,72
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 564) (nb_) = 2; \
   else if ((n_) < 648) (nb_) = 48; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
