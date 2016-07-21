#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,256,512,768,1024,2048
 * N : 25,128,256,512,768,1024,2048
 * NB : 2,8,12,16,64,64,64
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 2; \
   else if ((n_) < 192) (nb_) = 8; \
   else if ((n_) < 384) (nb_) = 12; \
   else if ((n_) < 640) (nb_) = 16; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
