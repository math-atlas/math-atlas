#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,200,400,840,1720
 * N : 25,80,200,400,840,1720
 * NB : 9,40,40,40,40,40
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 9; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
