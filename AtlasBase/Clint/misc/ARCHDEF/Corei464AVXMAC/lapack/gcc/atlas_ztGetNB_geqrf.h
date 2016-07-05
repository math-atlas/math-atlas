#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,180,240,540,600,660,780,1080,2220,4500
 * N : 25,120,180,240,540,600,660,780,1080,2220,4500
 * NB : 4,4,60,24,12,60,60,60,60,120,180
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 150) (nb_) = 4; \
   else if ((n_) < 210) (nb_) = 60; \
   else if ((n_) < 390) (nb_) = 24; \
   else if ((n_) < 570) (nb_) = 12; \
   else if ((n_) < 1650) (nb_) = 60; \
   else if ((n_) < 3360) (nb_) = 120; \
   else (nb_) = 180; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
