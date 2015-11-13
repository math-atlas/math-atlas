#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,521,1018,2012,4000
 * N : 25,87,149,273,521,1018,2012,4000
 * NB : 4,24,24,24,48,48,60,60
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 397) (nb_) = 24; \
   else if ((n_) < 1515) (nb_) = 48; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
