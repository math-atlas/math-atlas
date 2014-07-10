#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,960,1920,3840,4080,4160,4240,4320,4800,5760,7680
 * N : 25,240,480,960,1920,3840,4080,4160,4240,4320,4800,5760,7680
 * NB : 5,80,80,80,80,80,80,80,80,160,160,160,160
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 5; \
   else if ((n_) < 4280) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
