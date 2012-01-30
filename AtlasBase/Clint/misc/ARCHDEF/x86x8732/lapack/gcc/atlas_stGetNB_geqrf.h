#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,180,240,300,360,420,480,960,1980,4020
 * N : 25,120,180,240,300,360,420,480,960,1980,4020
 * NB : 4,12,12,24,60,72,120,120,120,120,120
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 4; \
   else if ((n_) < 210) (nb_) = 12; \
   else if ((n_) < 270) (nb_) = 24; \
   else if ((n_) < 330) (nb_) = 60; \
   else if ((n_) < 390) (nb_) = 72; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
