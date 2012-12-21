#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,228,532,1064,2128
 * N : 25,228,532,1064,2128
 * NB : 4,76,76,76,76
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 126) (nb_) = 4; \
   else (nb_) = 76; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
