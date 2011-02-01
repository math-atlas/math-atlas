#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,64,96,128,160,224,256,320,672
 * N : 25,64,96,128,160,224,256,320,672
 * NB : 9,8,12,16,16,16,32,32,32
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 44) (nb_) = 9; \
   else if ((n_) < 80) (nb_) = 8; \
   else if ((n_) < 112) (nb_) = 12; \
   else if ((n_) < 240) (nb_) = 16; \
   else (nb_) = 32; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
