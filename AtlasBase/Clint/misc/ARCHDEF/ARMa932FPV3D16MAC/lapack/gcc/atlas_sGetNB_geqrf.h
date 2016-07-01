#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,320,704,1472
 * N : 25,320,704,1472
 * NB : 12,12,32,64
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 512) (nb_) = 12; \
   else if ((n_) < 1088) (nb_) = 32; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
