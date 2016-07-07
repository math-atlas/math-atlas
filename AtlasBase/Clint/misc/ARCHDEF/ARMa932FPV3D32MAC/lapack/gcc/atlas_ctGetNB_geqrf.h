#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,64,96,128,192,224,256,544,1120
 * N : 25,64,96,128,192,224,256,544,1120
 * NB : 4,8,8,12,16,32,32,32,32
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 44) (nb_) = 4; \
   else if ((n_) < 112) (nb_) = 8; \
   else if ((n_) < 160) (nb_) = 12; \
   else if ((n_) < 208) (nb_) = 16; \
   else (nb_) = 32; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
