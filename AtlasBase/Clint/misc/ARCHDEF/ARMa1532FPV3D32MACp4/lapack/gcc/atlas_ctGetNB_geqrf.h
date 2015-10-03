#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,192,256,384,448,512,1088
 * N : 25,128,192,256,384,448,512,1088
 * NB : 4,8,32,24,16,64,64,64
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 4; \
   else if ((n_) < 160) (nb_) = 8; \
   else if ((n_) < 224) (nb_) = 32; \
   else if ((n_) < 320) (nb_) = 24; \
   else if ((n_) < 416) (nb_) = 16; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
