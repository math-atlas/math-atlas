#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,576,1224,2520
 * N : 25,144,288,576,1224,2520
 * NB : 4,72,72,72,72,72
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
