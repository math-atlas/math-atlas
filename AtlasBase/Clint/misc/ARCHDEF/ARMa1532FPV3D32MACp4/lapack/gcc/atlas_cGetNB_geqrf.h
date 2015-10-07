#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,320,384,448,896
 * N : 25,192,320,384,448,896
 * NB : 4,16,16,32,64,64
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 4; \
   else if ((n_) < 352) (nb_) = 16; \
   else if ((n_) < 416) (nb_) = 32; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
