#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,360,720,1500,3000
 * N : 25,180,360,720,1500,3000
 * NB : 1,60,60,60,60,60
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 1; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
