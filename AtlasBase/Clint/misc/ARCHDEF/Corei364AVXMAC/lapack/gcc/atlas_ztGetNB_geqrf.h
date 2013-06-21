#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,448,640,768,832,896,1856,3712
 * N : 25,192,448,640,768,832,896,1856,3712
 * NB : 9,64,64,64,64,64,128,128,128
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 9; \
   else if ((n_) < 864) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
