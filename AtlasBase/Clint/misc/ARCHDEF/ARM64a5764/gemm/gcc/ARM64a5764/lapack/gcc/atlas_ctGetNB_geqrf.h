#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,192,384,816,1680
 * N : 25,96,192,384,816,1680
 * NB : 4,48,48,48,48,48
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
