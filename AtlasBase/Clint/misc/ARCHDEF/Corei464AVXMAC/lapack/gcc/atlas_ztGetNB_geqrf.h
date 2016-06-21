#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,180,240,540,1080,2220,4440
 * N : 25,120,180,240,540,1080,2220,4440
 * NB : 5,4,60,60,60,60,120,120
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 5; \
   else if ((n_) < 150) (nb_) = 4; \
   else if ((n_) < 1650) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
