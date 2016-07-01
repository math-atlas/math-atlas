#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,320,640,720,800,960,1360,2000,2320,2480,2560,2640,2720
 * N : 25,160,320,640,720,800,960,1360,2000,2320,2480,2560,2640,2720
 * NB : 2,80,80,24,128,88,80,80,80,80,80,80,80,400
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 2; \
   else if ((n_) < 480) (nb_) = 80; \
   else if ((n_) < 680) (nb_) = 24; \
   else if ((n_) < 760) (nb_) = 128; \
   else if ((n_) < 880) (nb_) = 88; \
   else if ((n_) < 2680) (nb_) = 80; \
   else (nb_) = 400; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
