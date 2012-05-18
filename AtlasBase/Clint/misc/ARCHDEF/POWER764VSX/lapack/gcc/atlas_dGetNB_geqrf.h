#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,256,384,768,1536,3136
 * N : 25,192,256,384,768,1536,3136
 * NB : 5,16,64,64,64,64,128
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 5; \
   else if ((n_) < 224) (nb_) = 16; \
   else if ((n_) < 2336) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
