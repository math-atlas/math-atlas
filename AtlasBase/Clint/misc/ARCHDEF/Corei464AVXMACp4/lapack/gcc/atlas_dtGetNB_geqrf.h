#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,397,459,521,1018,1266,1515,2012,2509,3006,3503,4000
 * N : 25,87,149,273,397,459,521,1018,1266,1515,2012,2509,3006,3503,4000
 * NB : 8,8,24,24,24,72,72,72,92,100,144,144,180,180,200
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 118) (nb_) = 8; \
   else if ((n_) < 428) (nb_) = 24; \
   else if ((n_) < 1142) (nb_) = 72; \
   else if ((n_) < 1390) (nb_) = 92; \
   else if ((n_) < 1763) (nb_) = 100; \
   else if ((n_) < 2757) (nb_) = 144; \
   else if ((n_) < 3751) (nb_) = 180; \
   else (nb_) = 200; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
