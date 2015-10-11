#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,521,1018,2012,3006,3503,4000
 * N : 25,87,149,211,273,521,1018,2012,3006,3503,4000
 * NB : 24,24,32,44,44,48,48,60,112,116,116
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 118) (nb_) = 24; \
   else if ((n_) < 180) (nb_) = 32; \
   else if ((n_) < 397) (nb_) = 44; \
   else if ((n_) < 1515) (nb_) = 48; \
   else if ((n_) < 2509) (nb_) = 60; \
   else if ((n_) < 3254) (nb_) = 112; \
   else (nb_) = 116; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
