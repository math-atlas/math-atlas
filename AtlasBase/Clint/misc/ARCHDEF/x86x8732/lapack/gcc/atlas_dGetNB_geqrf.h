#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,88,132,220,352,484,1012,1496,1760,2024
 * N : 25,88,132,220,352,484,1012,1496,1760,2024
 * NB : 4,8,8,12,44,44,44,44,44,132
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 176) (nb_) = 8; \
   else if ((n_) < 286) (nb_) = 12; \
   else if ((n_) < 1892) (nb_) = 44; \
   else (nb_) = 132; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
