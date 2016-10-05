#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,335,397,521,583,645,707,769,893,1018,2012,2136,2260,2509,2757,3006,4000
 * N : 25,87,149,211,273,335,397,521,583,645,707,769,893,1018,2012,2136,2260,2509,2757,3006,4000
 * NB : 8,24,40,44,44,48,48,48,48,52,52,64,64,68,84,104,104,116,116,120,140
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 8; \
   else if ((n_) < 118) (nb_) = 24; \
   else if ((n_) < 180) (nb_) = 40; \
   else if ((n_) < 304) (nb_) = 44; \
   else if ((n_) < 614) (nb_) = 48; \
   else if ((n_) < 738) (nb_) = 52; \
   else if ((n_) < 955) (nb_) = 64; \
   else if ((n_) < 1515) (nb_) = 68; \
   else if ((n_) < 2074) (nb_) = 84; \
   else if ((n_) < 2384) (nb_) = 104; \
   else if ((n_) < 2881) (nb_) = 116; \
   else if ((n_) < 3503) (nb_) = 120; \
   else (nb_) = 140; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
