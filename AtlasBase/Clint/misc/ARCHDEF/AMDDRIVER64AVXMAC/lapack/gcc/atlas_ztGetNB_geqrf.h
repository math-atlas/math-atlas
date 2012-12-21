#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,228,532,1064,2128,4332
 * N : 25,228,532,1064,2128,4332
 * NB : 3,76,76,76,76,76
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 126) (nb_) = 3; \
   else (nb_) = 76; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
