#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,335,397,521,645,769,1018,1515,1763,1887,1949,2012,4000
 * N : 25,149,211,273,335,397,521,645,769,1018,1515,1763,1887,1949,2012,4000
 * NB : 12,12,16,48,56,56,56,56,60,68,68,68,68,68,76,76
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 242) (nb_) = 16; \
   else if ((n_) < 304) (nb_) = 48; \
   else if ((n_) < 707) (nb_) = 56; \
   else if ((n_) < 893) (nb_) = 60; \
   else if ((n_) < 1980) (nb_) = 68; \
   else (nb_) = 76; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
