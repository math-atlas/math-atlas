#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,192,256,512,1088,2176,3264,4416
 * N : 25,128,192,256,512,1088,2176,3264,4416
 * NB : 9,12,64,64,64,64,64,64,192
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 9; \
   else if ((n_) < 160) (nb_) = 12; \
   else if ((n_) < 3840) (nb_) = 64; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
