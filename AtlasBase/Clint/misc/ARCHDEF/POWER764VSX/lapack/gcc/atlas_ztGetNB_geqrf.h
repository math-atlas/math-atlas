#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,420,480,600,840,1020,1260,1740,1920,2040,2160,2220,2280,2400,2520,2640,3540,7140
 * N : 25,180,420,480,600,840,1020,1260,1740,1920,2040,2160,2220,2280,2400,2520,2640,3540,7140
 * NB : 6,24,20,24,24,24,60,60,60,60,60,80,88,96,96,120,120,120,240
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 6; \
   else if ((n_) < 300) (nb_) = 24; \
   else if ((n_) < 450) (nb_) = 20; \
   else if ((n_) < 930) (nb_) = 24; \
   else if ((n_) < 2100) (nb_) = 60; \
   else if ((n_) < 2190) (nb_) = 80; \
   else if ((n_) < 2250) (nb_) = 88; \
   else if ((n_) < 2460) (nb_) = 96; \
   else if ((n_) < 5340) (nb_) = 120; \
   else (nb_) = 240; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
