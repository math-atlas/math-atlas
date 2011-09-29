#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,468,1014,1482,2028
 * N : 25,468,1014,1482,2028
 * NB : 12,12,24,78,78
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 741) (nb_) = 12; \
   else if ((n_) < 1248) (nb_) = 24; \
   else (nb_) = 78; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
