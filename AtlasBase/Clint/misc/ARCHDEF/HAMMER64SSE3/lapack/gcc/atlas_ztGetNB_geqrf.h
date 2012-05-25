#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,280,336,392,784,1624
 * N : 25,168,280,336,392,784,1624
 * NB : 4,28,16,56,56,56,56
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 4; \
   else if ((n_) < 224) (nb_) = 28; \
   else if ((n_) < 308) (nb_) = 16; \
   else (nb_) = 56; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
