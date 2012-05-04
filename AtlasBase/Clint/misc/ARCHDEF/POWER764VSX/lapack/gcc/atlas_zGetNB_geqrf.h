#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,448,704,832,960,1984
 * N : 25,192,448,704,832,960,1984
 * NB : 4,64,64,64,64,128,128
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 4; \
   else if ((n_) < 896) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
