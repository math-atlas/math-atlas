#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,256,512,1024,2048
 * N : 25,128,256,512,1024,2048
 * NB : 4,64,64,64,64,64
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 4; \
   else (nb_) = 64; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
