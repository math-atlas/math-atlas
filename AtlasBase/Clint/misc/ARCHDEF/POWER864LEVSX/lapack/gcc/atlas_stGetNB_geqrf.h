#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,720,1440,2960,4400,5920
 * N : 25,160,240,320,720,1440,2960,4400,5920
 * NB : 2,16,20,80,80,80,80,80,160
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 2; \
   else if ((n_) < 200) (nb_) = 16; \
   else if ((n_) < 280) (nb_) = 20; \
   else if ((n_) < 5160) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
