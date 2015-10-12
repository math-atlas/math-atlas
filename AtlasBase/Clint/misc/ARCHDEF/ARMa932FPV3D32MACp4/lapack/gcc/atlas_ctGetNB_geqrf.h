#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,521,769,831,893,1018,2012,4000
 * N : 25,87,149,273,521,769,831,893,1018,2012,4000
 * NB : 4,24,24,24,24,24,24,48,48,48,60
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 862) (nb_) = 24; \
   else if ((n_) < 3006) (nb_) = 48; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
