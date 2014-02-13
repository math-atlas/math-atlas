#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,256,384,448,576,1152,2304
 * N : 25,128,256,384,448,576,1152,2304
 * NB : 9,64,16,16,64,64,64,128
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 9; \
   else if ((n_) < 192) (nb_) = 64; \
   else if ((n_) < 416) (nb_) = 16; \
   else if ((n_) < 1728) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
