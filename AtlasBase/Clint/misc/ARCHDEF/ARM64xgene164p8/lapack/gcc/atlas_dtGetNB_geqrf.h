#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,521,1018,1266,1515,2012,4000
 * N : 25,87,149,273,521,1018,1266,1515,2012,4000
 * NB : 8,8,16,24,24,36,48,48,76,76
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 118) (nb_) = 8; \
   else if ((n_) < 211) (nb_) = 16; \
   else if ((n_) < 769) (nb_) = 24; \
   else if ((n_) < 1142) (nb_) = 36; \
   else if ((n_) < 1763) (nb_) = 48; \
   else (nb_) = 76; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
