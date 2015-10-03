#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,240,300,420,480,540,660,900,1320,1800
 * N : 25,180,240,300,420,480,540,660,900,1320,1800
 * NB : 1,12,16,16,16,24,60,60,60,60,120
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 1; \
   else if ((n_) < 210) (nb_) = 12; \
   else if ((n_) < 450) (nb_) = 16; \
   else if ((n_) < 510) (nb_) = 24; \
   else if ((n_) < 1560) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
