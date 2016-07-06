#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,192,256,320,640,1344,2048,2752
 * N : 25,128,192,256,320,640,1344,2048,2752
 * NB : 9,12,24,64,64,64,64,64,192
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 9; \
   else if ((n_) < 160) (nb_) = 12; \
   else if ((n_) < 224) (nb_) = 24; \
   else if ((n_) < 2400) (nb_) = 64; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
