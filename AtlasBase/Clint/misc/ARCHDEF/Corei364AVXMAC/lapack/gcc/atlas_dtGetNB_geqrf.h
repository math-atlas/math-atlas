#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,224,448,896,1848,3696,5544,6440,6888,7112,7224,7280,7336,7392
 * N : 25,112,224,448,896,1848,3696,5544,6440,6888,7112,7224,7280,7336,7392
 * NB : 10,112,112,112,112,112,112,224,224,224,224,224,224,224,560
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 10; \
   else if ((n_) < 4620) (nb_) = 112; \
   else if ((n_) < 7364) (nb_) = 224; \
   else (nb_) = 560; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
