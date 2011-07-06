#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,360,720,1512,2232,3024,6048
 * N : 25,144,360,720,1512,2232,3024,6048
 * NB : 1,72,72,72,72,72,144,144
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 1; \
   else if ((n_) < 2628) (nb_) = 72; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
