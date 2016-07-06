#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,256,320,448,896,1792,3584,7232
 * N : 25,192,256,320,448,896,1792,3584,7232
 * NB : 6,8,8,64,64,64,128,128,128
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 6; \
   else if ((n_) < 288) (nb_) = 8; \
   else if ((n_) < 1344) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
