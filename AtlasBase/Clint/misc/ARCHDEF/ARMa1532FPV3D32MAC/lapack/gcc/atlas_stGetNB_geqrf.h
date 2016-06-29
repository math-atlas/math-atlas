#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,800,880,960,1200,1600,3280
 * N : 25,160,240,320,400,800,880,960,1200,1600,3280
 * NB : 2,16,16,32,24,24,80,80,80,80,80
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 2; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 360) (nb_) = 32; \
   else if ((n_) < 840) (nb_) = 24; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
