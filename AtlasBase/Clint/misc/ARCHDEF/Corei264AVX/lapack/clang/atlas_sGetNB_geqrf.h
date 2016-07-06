#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,192,256,512,1088,1664,2240,4544
 * N : 25,128,192,256,512,1088,1664,2240,4544
 * NB : 9,12,8,64,64,64,64,128,128
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 9; \
   else if ((n_) < 160) (nb_) = 12; \
   else if ((n_) < 224) (nb_) = 8; \
   else if ((n_) < 1952) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
