#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,150,180,240,480,960,1920,3840
 * N : 25,120,150,180,240,480,960,1920,3840
 * NB : 4,4,60,60,60,60,60,60,60
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 135) (nb_) = 4; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
