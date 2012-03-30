#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,160,320,480,680,1400,2120,2480,2640,2720,2840
 * N : 25,80,160,320,480,680,1400,2120,2480,2640,2720,2840
 * NB : 2,40,40,40,40,80,80,80,96,80,80,160
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 2; \
   else if ((n_) < 580) (nb_) = 40; \
   else if ((n_) < 2300) (nb_) = 80; \
   else if ((n_) < 2560) (nb_) = 96; \
   else if ((n_) < 2780) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
