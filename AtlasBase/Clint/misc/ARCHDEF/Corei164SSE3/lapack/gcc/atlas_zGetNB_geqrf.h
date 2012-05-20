#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,312,624,1300
 * N : 25,156,312,624,1300
 * NB : 4,52,52,52,52
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 4; \
   else (nb_) = 52; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
