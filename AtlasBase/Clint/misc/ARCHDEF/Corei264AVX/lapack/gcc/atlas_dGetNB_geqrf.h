#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,104,208,416,884,1820,3692
 * N : 25,104,208,416,884,1820,3692
 * NB : 5,52,52,52,52,52,104
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 64) (nb_) = 5; \
   else if ((n_) < 2756) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
