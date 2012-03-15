#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,162,324,648,1296,2646
 * N : 25,162,324,648,1296,2646
 * NB : 4,54,54,54,54,108
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 93) (nb_) = 4; \
   else if ((n_) < 1971) (nb_) = 54; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
