#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,152,380,760,1596,3192
 * N : 25,152,380,760,1596,3192
 * NB : 4,76,76,76,76,76
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 88) (nb_) = 4; \
   else (nb_) = 76; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
