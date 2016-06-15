#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,320,640,1280,1920,2240,2400,2480,2560
 * N : 25,160,320,640,1280,1920,2240,2400,2480,2560
 * NB : 12,80,44,40,40,40,80,40,80,160
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 12; \
   else if ((n_) < 240) (nb_) = 80; \
   else if ((n_) < 480) (nb_) = 44; \
   else if ((n_) < 2080) (nb_) = 40; \
   else if ((n_) < 2320) (nb_) = 80; \
   else if ((n_) < 2440) (nb_) = 40; \
   else if ((n_) < 2520) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
