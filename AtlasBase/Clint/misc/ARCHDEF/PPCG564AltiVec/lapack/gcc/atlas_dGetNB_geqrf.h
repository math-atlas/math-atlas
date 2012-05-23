#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,168,280,392,560,672,840,1176,2408
 * N : 25,112,168,280,392,560,672,840,1176,2408
 * NB : 4,12,12,28,56,56,56,112,112,168
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 4; \
   else if ((n_) < 224) (nb_) = 12; \
   else if ((n_) < 336) (nb_) = 28; \
   else if ((n_) < 756) (nb_) = 56; \
   else if ((n_) < 1792) (nb_) = 112; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
