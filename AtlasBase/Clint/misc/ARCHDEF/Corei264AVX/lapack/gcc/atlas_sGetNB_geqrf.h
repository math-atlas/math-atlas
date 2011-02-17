#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,420,840,1680,3360
 * N : 25,168,420,840,1680,3360
 * NB : 5,84,84,84,84,84
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 5; \
   else (nb_) = 84; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
