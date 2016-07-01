#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,252,540,1116
 * N : 25,252,540,1116
 * NB : 12,36,36,36
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 138) (nb_) = 12; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
