#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,160,320,680,1360
 * N : 25,80,160,320,680,1360
 * NB : 5,40,40,40,40,40
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 5; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
