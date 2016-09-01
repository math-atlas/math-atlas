#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,521,583,645,769,831,893,955,1018,1515,2012,2260,2509,3006,4000
 * N : 25,87,149,273,521,583,645,769,831,893,955,1018,1515,2012,2260,2509,3006,4000
 * NB : 12,16,24,24,24,64,64,68,76,76,96,96,96,120,120,140,172,208
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 12; \
   else if ((n_) < 118) (nb_) = 16; \
   else if ((n_) < 552) (nb_) = 24; \
   else if ((n_) < 707) (nb_) = 64; \
   else if ((n_) < 800) (nb_) = 68; \
   else if ((n_) < 924) (nb_) = 76; \
   else if ((n_) < 1763) (nb_) = 96; \
   else if ((n_) < 2384) (nb_) = 120; \
   else if ((n_) < 2757) (nb_) = 140; \
   else if ((n_) < 3503) (nb_) = 172; \
   else (nb_) = 208; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
