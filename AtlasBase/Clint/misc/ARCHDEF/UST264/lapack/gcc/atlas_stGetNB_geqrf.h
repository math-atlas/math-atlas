#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,64,160,320,480,672,1376,2752,2784,2816,2912,3072,3232,3424,4128,5504
 * N : 25,64,160,320,480,672,1376,2752,2784,2816,2912,3072,3232,3424,4128,5504
 * NB : 2,8,8,16,32,32,32,32,128,128,128,192,192,224,224,288
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 44) (nb_) = 2; \
   else if ((n_) < 240) (nb_) = 8; \
   else if ((n_) < 400) (nb_) = 16; \
   else if ((n_) < 2768) (nb_) = 32; \
   else if ((n_) < 2992) (nb_) = 128; \
   else if ((n_) < 3328) (nb_) = 192; \
   else if ((n_) < 4816) (nb_) = 224; \
   else (nb_) = 288; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
