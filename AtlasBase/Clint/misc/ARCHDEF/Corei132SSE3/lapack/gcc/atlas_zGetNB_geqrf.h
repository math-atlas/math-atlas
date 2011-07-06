#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,104,208,416,832,1716
 * N : 25,104,208,416,832,1716
 * NB : 9,52,52,52,52,52
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 64) (nb_) = 9; \
   else (nb_) = 52; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
