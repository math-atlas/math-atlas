#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,1040,2160
 * N : 25,240,480,1040,2160
 * NB : 2,80,80,80,80
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 2; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
