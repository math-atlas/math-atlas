#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,224,336,448,672,896,1344,1568,1792
 * N : 25,112,224,336,448,672,896,1344,1568,1792
 * NB : 12,24,16,56,56,56,64,64,64,112
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 12; \
   else if ((n_) < 168) (nb_) = 24; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 784) (nb_) = 56; \
   else if ((n_) < 1680) (nb_) = 64; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
