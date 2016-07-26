#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,280,336,392,840,1736,3528
 * N : 25,168,280,336,392,840,1736,3528
 * NB : 4,28,28,56,56,56,56,112
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 4; \
   else if ((n_) < 308) (nb_) = 28; \
   else if ((n_) < 2632) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
