#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,480,720,1040,1200,1280,1360,1440,2160,2560,2720,2960
 * N : 25,160,240,320,400,480,720,1040,1200,1280,1360,1440,2160,2560,2720,2960
 * NB : 4,12,12,24,80,80,80,80,80,80,80,160,160,160,320,400
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 280) (nb_) = 12; \
   else if ((n_) < 360) (nb_) = 24; \
   else if ((n_) < 1400) (nb_) = 80; \
   else if ((n_) < 2640) (nb_) = 160; \
   else if ((n_) < 2840) (nb_) = 320; \
   else (nb_) = 400; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
