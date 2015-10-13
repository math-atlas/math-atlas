#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,256,384,448,512,1088
 * N : 25,128,256,384,448,512,1088
 * NB : 4,12,16,16,16,64,64
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 4; \
   else if ((n_) < 192) (nb_) = 12; \
   else if ((n_) < 480) (nb_) = 16; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
