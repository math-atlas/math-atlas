#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,224,504,1064,2128,4312
 * N : 25,112,224,504,1064,2128,4312
 * NB : 4,56,56,56,56,56,56
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 4; \
   else (nb_) = 56; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
