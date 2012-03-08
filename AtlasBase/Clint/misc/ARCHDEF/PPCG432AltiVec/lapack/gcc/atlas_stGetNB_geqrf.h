#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,504,1008,2016
 * N : 25,216,504,1008,2016
 * NB : 2,24,24,72,72
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 2; \
   else if ((n_) < 756) (nb_) = 24; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
