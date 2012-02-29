#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,168,224,280,392,560,672,728,784,840,1176,2408
 * N : 25,112,168,224,280,392,560,672,728,784,840,1176,2408
 * NB : 2,12,12,12,28,56,56,56,56,56,112,112,112
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 2; \
   else if ((n_) < 252) (nb_) = 12; \
   else if ((n_) < 336) (nb_) = 28; \
   else if ((n_) < 812) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
