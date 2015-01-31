#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,521,769,1018,1515,2012,4000
 * N : 25,149,211,273,521,769,1018,1515,2012,4000
 * NB : 24,24,24,48,48,56,80,80,92,92
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 242) (nb_) = 24; \
   else if ((n_) < 645) (nb_) = 48; \
   else if ((n_) < 893) (nb_) = 56; \
   else if ((n_) < 1763) (nb_) = 80; \
   else (nb_) = 92; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
