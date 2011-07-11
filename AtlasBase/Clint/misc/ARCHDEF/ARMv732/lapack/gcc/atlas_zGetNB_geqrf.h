#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,192,256,320,384,512,704
 * N : 25,128,192,256,320,384,512,704
 * NB : 2,12,16,32,32,64,64,64
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 2; \
   else if ((n_) < 160) (nb_) = 12; \
   else if ((n_) < 224) (nb_) = 16; \
   else if ((n_) < 352) (nb_) = 32; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
