#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,132,264,572,1144,2288
 * N : 25,132,264,572,1144,2288
 * NB : 5,44,44,44,44,44
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 78) (nb_) = 5; \
   else (nb_) = 44; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
