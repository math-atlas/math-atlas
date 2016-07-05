#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,168,224,448,952,1400,1904,3864,7784
 * N : 25,112,168,224,448,952,1400,1904,3864,7784
 * NB : 4,8,56,56,56,56,56,112,112,112
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 4; \
   else if ((n_) < 140) (nb_) = 8; \
   else if ((n_) < 1652) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
