#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,320,640,960,1280
 * N : 25,128,320,640,960,1280
 * NB : 4,16,16,16,64,64
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 4; \
   else if ((n_) < 800) (nb_) = 16; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
