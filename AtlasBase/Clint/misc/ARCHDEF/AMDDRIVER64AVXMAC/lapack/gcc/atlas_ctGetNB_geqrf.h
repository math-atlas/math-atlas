#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,576,1224,2520,5040
 * N : 25,144,288,576,1224,2520,5040
 * NB : 9,72,72,72,72,72,72
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 9; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
