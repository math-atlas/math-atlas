#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,252,336,756,1596
 * N : 25,168,252,336,756,1596
 * NB : 5,16,16,24,84,84
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 5; \
   else if ((n_) < 294) (nb_) = 16; \
   else if ((n_) < 546) (nb_) = 24; \
   else (nb_) = 84; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
