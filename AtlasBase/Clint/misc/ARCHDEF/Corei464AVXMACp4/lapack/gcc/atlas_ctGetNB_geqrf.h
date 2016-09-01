#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,335,397,459,521,1018,2012,2260,2509,3006,3503,4000
 * N : 25,87,149,211,273,335,397,459,521,1018,2012,2260,2509,3006,3503,4000
 * NB : 4,16,16,24,24,24,52,52,56,64,96,152,160,232,236,240
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 180) (nb_) = 16; \
   else if ((n_) < 366) (nb_) = 24; \
   else if ((n_) < 490) (nb_) = 52; \
   else if ((n_) < 769) (nb_) = 56; \
   else if ((n_) < 1515) (nb_) = 64; \
   else if ((n_) < 2136) (nb_) = 96; \
   else if ((n_) < 2384) (nb_) = 152; \
   else if ((n_) < 2757) (nb_) = 160; \
   else if ((n_) < 3254) (nb_) = 232; \
   else if ((n_) < 3751) (nb_) = 236; \
   else (nb_) = 240; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
