#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,360,432,504,1080,2232
 * N : 25,216,360,432,504,1080,2232
 * NB : 12,24,24,72,72,72,72
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 12; \
   else if ((n_) < 396) (nb_) = 24; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
