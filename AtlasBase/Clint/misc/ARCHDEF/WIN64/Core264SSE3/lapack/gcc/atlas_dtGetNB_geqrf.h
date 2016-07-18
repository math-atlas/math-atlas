#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,256,384,768,1536,1600,1728,1920,2304,3136
 * N : 25,192,256,384,768,1536,1600,1728,1920,2304,3136
 * NB : 2,16,64,64,64,64,128,128,128,128,192
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 2; \
   else if ((n_) < 224) (nb_) = 16; \
   else if ((n_) < 1568) (nb_) = 64; \
   else if ((n_) < 2720) (nb_) = 128; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
