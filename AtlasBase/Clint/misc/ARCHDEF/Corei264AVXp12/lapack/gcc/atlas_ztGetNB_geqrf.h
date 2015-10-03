#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,521,1018,1515,2012,4000
 * N : 25,149,211,273,521,1018,1515,2012,4000
 * NB : 12,12,48,48,60,60,60,68,72
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 397) (nb_) = 48; \
   else if ((n_) < 1763) (nb_) = 60; \
   else if ((n_) < 3006) (nb_) = 68; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
