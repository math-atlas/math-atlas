#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,400,800,1200,1440,1680,2480,2880,3360
 * N : 25,160,240,400,800,1200,1440,1680,2480,2880,3360
 * NB : 2,12,40,24,16,24,80,80,80,80,160
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 2; \
   else if ((n_) < 200) (nb_) = 12; \
   else if ((n_) < 320) (nb_) = 40; \
   else if ((n_) < 600) (nb_) = 24; \
   else if ((n_) < 1000) (nb_) = 16; \
   else if ((n_) < 1320) (nb_) = 24; \
   else if ((n_) < 3120) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
