#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,68,112,200,376,552,640,684,728,1432,2840
 * N : 25,68,112,200,376,552,640,684,728,1432,2840
 * NB : 6,12,12,12,12,12,12,12,24,24,24
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 46) (nb_) = 6; \
   else if ((n_) < 706) (nb_) = 12; \
   else (nb_) = 24; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
