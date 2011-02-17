#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,360,480,600,1320,1920,2280,2400,2640
 * N : 25,240,360,480,600,1320,1920,2280,2400,2640
 * NB : 4,16,16,120,120,120,120,120,120,240
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 420) (nb_) = 16; \
   else if ((n_) < 2520) (nb_) = 120; \
   else (nb_) = 240; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
