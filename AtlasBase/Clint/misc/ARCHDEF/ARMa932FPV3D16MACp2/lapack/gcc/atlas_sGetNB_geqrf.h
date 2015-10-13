#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,192,320,384,512,704,1472
 * N : 25,128,192,320,384,512,704,1472
 * NB : 4,12,12,24,64,64,64,128
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 4; \
   else if ((n_) < 256) (nb_) = 12; \
   else if ((n_) < 352) (nb_) = 24; \
   else if ((n_) < 1088) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
