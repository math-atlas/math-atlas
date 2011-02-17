#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,480,1040,1280,1600,2160,4400
 * N : 25,240,320,480,1040,1280,1600,2160,4400
 * NB : 2,16,32,32,32,80,80,80,80
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 2; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 1160) (nb_) = 32; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
