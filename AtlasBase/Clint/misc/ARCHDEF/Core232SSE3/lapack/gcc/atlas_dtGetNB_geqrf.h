#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,192,384,816,1200,1392,1632,3264
 * N : 25,96,192,384,816,1200,1392,1632,3264
 * NB : 1,48,48,48,48,48,48,96,96
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 1; \
   else if ((n_) < 1512) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
