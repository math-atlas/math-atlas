#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,256,320,448,896,1344,1856
 * N : 25,192,256,320,448,896,1344,1856
 * NB : 2,12,12,84,56,40,64,128
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 2; \
   else if ((n_) < 288) (nb_) = 12; \
   else if ((n_) < 384) (nb_) = 84; \
   else if ((n_) < 672) (nb_) = 56; \
   else if ((n_) < 1120) (nb_) = 40; \
   else if ((n_) < 1600) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
