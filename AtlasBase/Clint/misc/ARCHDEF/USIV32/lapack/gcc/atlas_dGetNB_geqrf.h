#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,336,840,1680
 * N : 25,336,840,1680
 * NB : 4,24,24,168
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 180) (nb_) = 4; \
   else if ((n_) < 1260) (nb_) = 24; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
