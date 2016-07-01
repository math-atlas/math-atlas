#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,224,448,896
 * N : 25,96,224,448,896
 * NB : 12,32,32,32,32
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 12; \
   else (nb_) = 32; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
