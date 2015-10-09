#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,521,583,645,769,831,893,1018,2012,2260,2509,3006,4000
 * N : 25,87,149,273,521,583,645,769,831,893,1018,2012,2260,2509,3006,4000
 * NB : 10,15,15,25,25,25,28,28,40,40,40,45,45,65,65,65
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 10; \
   else if ((n_) < 211) (nb_) = 15; \
   else if ((n_) < 614) (nb_) = 25; \
   else if ((n_) < 800) (nb_) = 28; \
   else if ((n_) < 1515) (nb_) = 40; \
   else if ((n_) < 2384) (nb_) = 45; \
   else (nb_) = 65; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
