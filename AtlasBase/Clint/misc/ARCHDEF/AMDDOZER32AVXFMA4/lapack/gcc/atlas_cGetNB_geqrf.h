#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,180,240,480,960,1980
 * N : 25,120,180,240,480,960,1980
 * NB : 4,12,12,60,60,60,60
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 4; \
   else if ((n_) < 210) (nb_) = 12; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
