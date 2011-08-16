#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,320,640,1280
 * N : 25,128,320,640,1280
 * NB : 4,64,64,64,64
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 4; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
