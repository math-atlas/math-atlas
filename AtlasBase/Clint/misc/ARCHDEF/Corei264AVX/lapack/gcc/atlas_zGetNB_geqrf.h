#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,56,112,224,448,896,1792
 * N : 25,56,112,224,448,896,1792
 * NB : 9,28,28,28,28,28,28
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 40) (nb_) = 9; \
   else (nb_) = 28; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
