#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,335,397,521,1018,1266,1515,1577,1639,1763,1887,2012,3006,4000
 * N : 25,87,149,273,335,397,521,1018,1266,1515,1577,1639,1763,1887,2012,3006,4000
 * NB : 12,24,52,56,60,60,60,60,72,92,92,124,124,132,132,132,148
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 12; \
   else if ((n_) < 118) (nb_) = 24; \
   else if ((n_) < 211) (nb_) = 52; \
   else if ((n_) < 304) (nb_) = 56; \
   else if ((n_) < 1142) (nb_) = 60; \
   else if ((n_) < 1390) (nb_) = 72; \
   else if ((n_) < 1608) (nb_) = 92; \
   else if ((n_) < 1825) (nb_) = 124; \
   else if ((n_) < 3503) (nb_) = 132; \
   else (nb_) = 148; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
