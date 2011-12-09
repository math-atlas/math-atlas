#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,336,672,1400,2800
 * N : 25,168,336,672,1400,2800
 * NB : 4,56,56,56,56,112
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 4; \
   else if ((n_) < 2100) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
