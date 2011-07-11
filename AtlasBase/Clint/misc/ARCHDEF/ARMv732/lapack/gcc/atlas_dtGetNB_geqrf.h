#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,480,560,640,720,1040,1200,1280,1440
 * N : 25,160,240,320,400,480,560,640,720,1040,1200,1280,1440
 * NB : 1,8,8,20,24,24,32,32,80,80,80,80,160
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 1; \
   else if ((n_) < 280) (nb_) = 8; \
   else if ((n_) < 360) (nb_) = 20; \
   else if ((n_) < 520) (nb_) = 24; \
   else if ((n_) < 680) (nb_) = 32; \
   else if ((n_) < 1360) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
