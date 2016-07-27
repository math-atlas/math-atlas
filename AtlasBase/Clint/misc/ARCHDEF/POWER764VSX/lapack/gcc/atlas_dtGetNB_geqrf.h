#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,480,960,1440,1680,1840,1920,2000,2960,4000,8080
 * N : 25,240,320,400,480,960,1440,1680,1840,1920,2000,2960,4000,8080
 * NB : 9,80,80,80,160,160,160,160,160,160,224,168,240,320
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 9; \
   else if ((n_) < 440) (nb_) = 80; \
   else if ((n_) < 1960) (nb_) = 160; \
   else if ((n_) < 2480) (nb_) = 224; \
   else if ((n_) < 3480) (nb_) = 168; \
   else if ((n_) < 6040) (nb_) = 240; \
   else (nb_) = 320; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
