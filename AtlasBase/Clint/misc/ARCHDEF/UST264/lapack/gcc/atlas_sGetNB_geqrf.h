#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,64,128,160,192,224,256,544,1088
 * N : 25,64,128,160,192,224,256,544,1088
 * NB : 2,8,8,12,12,12,32,32,32
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 44) (nb_) = 2; \
   else if ((n_) < 144) (nb_) = 8; \
   else if ((n_) < 240) (nb_) = 12; \
   else (nb_) = 32; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
