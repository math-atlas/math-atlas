#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,224,448,896,1848,3752
 * N : 25,112,224,448,896,1848,3752
 * NB : 10,56,56,56,56,56,56
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 10; \
   else (nb_) = 56; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
