#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,1200,1280,1360,1440,1520,1840,2480,4960,10000
 * N : 25,1200,1280,1360,1440,1520,1840,2480,4960,10000
 * NB : 8,8,8,144,144,152,152,104,80,72
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 1320) (nb_) = 8; \
   else if ((n_) < 1480) (nb_) = 144; \
   else if ((n_) < 2160) (nb_) = 152; \
   else if ((n_) < 3720) (nb_) = 104; \
   else if ((n_) < 7480) (nb_) = 80; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
