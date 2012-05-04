#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,448,960,1472,1984,3968,8000
 * N : 25,192,448,960,1472,1984,3968,8000
 * NB : 4,64,64,64,128,128,128,128
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 4; \
   else if ((n_) < 1216) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
