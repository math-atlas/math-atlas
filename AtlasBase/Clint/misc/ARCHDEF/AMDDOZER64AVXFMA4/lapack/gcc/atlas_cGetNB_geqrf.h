#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,216,288,576,1152,2304
 * N : 25,144,216,288,576,1152,2304
 * NB : 5,12,72,72,72,72,72
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 5; \
   else if ((n_) < 180) (nb_) = 12; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
