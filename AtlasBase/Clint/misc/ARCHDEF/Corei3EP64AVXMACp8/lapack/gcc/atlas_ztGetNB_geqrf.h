#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,335,397,521,583,645,769,1018,1515,2012,2136,2260,2509,2757,3006,4000
 * N : 25,87,149,211,273,335,397,521,583,645,769,1018,1515,2012,2136,2260,2509,2757,3006,4000
 * NB : 6,12,24,32,36,52,52,56,56,60,60,60,64,84,116,116,132,132,184,192
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 6; \
   else if ((n_) < 118) (nb_) = 12; \
   else if ((n_) < 180) (nb_) = 24; \
   else if ((n_) < 242) (nb_) = 32; \
   else if ((n_) < 304) (nb_) = 36; \
   else if ((n_) < 459) (nb_) = 52; \
   else if ((n_) < 614) (nb_) = 56; \
   else if ((n_) < 1266) (nb_) = 60; \
   else if ((n_) < 1763) (nb_) = 64; \
   else if ((n_) < 2074) (nb_) = 84; \
   else if ((n_) < 2384) (nb_) = 116; \
   else if ((n_) < 2881) (nb_) = 132; \
   else if ((n_) < 3503) (nb_) = 184; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
