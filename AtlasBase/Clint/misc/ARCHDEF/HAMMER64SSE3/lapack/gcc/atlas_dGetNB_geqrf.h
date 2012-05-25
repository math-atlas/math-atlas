#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,224,280,336,392,504,1008,2072
 * N : 25,112,224,280,336,392,504,1008,2072
 * NB : 2,16,14,20,28,56,56,56,112
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 2; \
   else if ((n_) < 168) (nb_) = 16; \
   else if ((n_) < 252) (nb_) = 14; \
   else if ((n_) < 308) (nb_) = 20; \
   else if ((n_) < 364) (nb_) = 28; \
   else if ((n_) < 1540) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
