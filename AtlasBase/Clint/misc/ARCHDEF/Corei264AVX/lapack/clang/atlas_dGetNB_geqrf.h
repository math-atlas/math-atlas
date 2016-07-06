#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,104,156,208,468,936,1872,3796
 * N : 25,104,156,208,468,936,1872,3796
 * NB : 7,12,12,52,52,52,104,104
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 64) (nb_) = 7; \
   else if ((n_) < 182) (nb_) = 12; \
   else if ((n_) < 1404) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
