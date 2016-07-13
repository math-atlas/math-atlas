#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,128,256,576,1216,1856,2176,2304,2368,2496
 * N : 25,128,256,576,1216,1856,2176,2304,2368,2496
 * NB : 2,64,64,64,64,64,64,64,72,72
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 76) (nb_) = 2; \
   else if ((n_) < 2336) (nb_) = 64; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
