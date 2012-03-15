#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,312,624,1300,2652
 * N : 25,156,312,624,1300,2652
 * NB : 4,52,52,52,52,104
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 4; \
   else if ((n_) < 1976) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
