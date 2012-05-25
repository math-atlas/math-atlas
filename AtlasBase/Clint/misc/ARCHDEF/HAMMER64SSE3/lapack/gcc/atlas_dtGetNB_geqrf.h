#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,168,280,336,392,448,616,1288,1904,2576
 * N : 25,112,168,280,336,392,448,616,1288,1904,2576
 * NB : 4,16,14,24,24,56,56,56,56,56,112
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 4; \
   else if ((n_) < 140) (nb_) = 16; \
   else if ((n_) < 224) (nb_) = 14; \
   else if ((n_) < 364) (nb_) = 24; \
   else if ((n_) < 2240) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
