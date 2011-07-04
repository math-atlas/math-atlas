#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,576,1152,2376
 * N : 25,144,288,576,1152,2376
 * NB : 11,72,72,72,72,72
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 11; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
