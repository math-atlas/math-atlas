#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,224,448,896,1792
 * N : 25,112,224,448,896,1792
 * NB : 5,56,56,56,56,56
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 5; \
   else (nb_) = 56; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
