#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,540,780,1080,1620,1680,1740,1920,2220
 * N : 25,240,540,780,1080,1620,1680,1740,1920,2220
 * NB : 12,8,24,60,60,48,60,60,60,120
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 12; \
   else if ((n_) < 390) (nb_) = 8; \
   else if ((n_) < 660) (nb_) = 24; \
   else if ((n_) < 1350) (nb_) = 60; \
   else if ((n_) < 1650) (nb_) = 48; \
   else if ((n_) < 2070) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
