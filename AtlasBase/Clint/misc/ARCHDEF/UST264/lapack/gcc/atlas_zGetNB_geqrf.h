#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,48,96,176,352,704
 * N : 25,48,96,176,352,704
 * NB : 9,16,16,16,16,16
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 36) (nb_) = 9; \
   else (nb_) = 16; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
