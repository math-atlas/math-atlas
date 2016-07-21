#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,192,256,512,768,1024,2048
 * N : 25,128,192,256,512,768,1024,2048
 * NB : 4,16,32,28,16,64,64,64
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 4; \
   else if ((n_) < 160) (nb_) = 16; \
   else if ((n_) < 224) (nb_) = 32; \
   else if ((n_) < 384) (nb_) = 28; \
   else if ((n_) < 640) (nb_) = 16; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
