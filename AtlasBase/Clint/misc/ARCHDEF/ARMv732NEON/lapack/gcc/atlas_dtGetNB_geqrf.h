#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,480,720,1440
 * N : 25,160,240,320,400,480,720,1440
 * NB : 2,8,12,24,96,80,80,80
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 2; \
   else if ((n_) < 200) (nb_) = 8; \
   else if ((n_) < 280) (nb_) = 12; \
   else if ((n_) < 360) (nb_) = 24; \
   else if ((n_) < 440) (nb_) = 96; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
