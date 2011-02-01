#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,168,224,336,504,728,1512,3080
 * N : 25,168,224,336,504,728,1512,3080
 * NB : 8,8,16,16,56,56,56,56
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 196) (nb_) = 8; \
   else if ((n_) < 420) (nb_) = 16; \
   else (nb_) = 56; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
