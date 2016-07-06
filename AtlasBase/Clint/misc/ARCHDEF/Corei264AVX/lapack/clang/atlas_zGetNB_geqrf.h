#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,104,156,208,260,520,1092,2236
 * N : 25,104,156,208,260,520,1092,2236
 * NB : 4,12,12,52,52,52,52,52
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 64) (nb_) = 4; \
   else if ((n_) < 182) (nb_) = 12; \
   else (nb_) = 52; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
