#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,126,294,588,714,882,1218,2436
 * N : 25,126,294,588,714,882,1218,2436
 * NB : 2,12,12,12,42,42,42,42
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 75) (nb_) = 2; \
   else if ((n_) < 651) (nb_) = 12; \
   else (nb_) = 42; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
