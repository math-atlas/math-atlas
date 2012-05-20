#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,648,1296,2664
 * N : 25,144,288,648,1296,2664
 * NB : 2,72,72,72,72,144
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 2; \
   else if ((n_) < 1980) (nb_) = 72; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
