#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,360,780,1620
 * N : 25,180,360,780,1620
 * NB : 9,60,60,60,60
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 9; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
