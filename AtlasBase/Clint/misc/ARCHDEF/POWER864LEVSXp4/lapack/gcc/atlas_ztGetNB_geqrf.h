#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,521,1018,1515,2012,2509,2571,2633,2757,3006,4000
 * N : 25,87,149,211,273,521,1018,1515,2012,2509,2571,2633,2757,3006,4000
 * NB : 4,24,24,24,36,36,40,44,44,68,72,72,72,72,72
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 242) (nb_) = 24; \
   else if ((n_) < 769) (nb_) = 36; \
   else if ((n_) < 1266) (nb_) = 40; \
   else if ((n_) < 2260) (nb_) = 44; \
   else if ((n_) < 2540) (nb_) = 68; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
