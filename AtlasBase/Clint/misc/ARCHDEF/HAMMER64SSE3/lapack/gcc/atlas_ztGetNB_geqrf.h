#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,120,180,240,540,660,840,1140,1680,2280
 * N : 25,120,180,240,540,660,840,1140,1680,2280
 * NB : 4,12,20,20,16,60,60,60,60,120
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 4; \
   else if ((n_) < 150) (nb_) = 12; \
   else if ((n_) < 390) (nb_) = 20; \
   else if ((n_) < 600) (nb_) = 16; \
   else if ((n_) < 1980) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
