#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,204,476,952,1972
 * N : 25,204,476,952,1972
 * NB : 4,68,68,68,68
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 114) (nb_) = 4; \
   else (nb_) = 68; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
