#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,1012,2000
 * N : 25,1012,2000
 * NB : 24,36,36
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 518) (nb_) = 24; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
