#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,96,120,240,264,288,336,456,888,1752
 * N : 25,72,96,120,240,264,288,336,456,888,1752
 * NB : 4,4,24,24,16,24,24,24,24,24,72
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 180) (nb_) = 24; \
   else if ((n_) < 252) (nb_) = 16; \
   else if ((n_) < 1320) (nb_) = 24; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
