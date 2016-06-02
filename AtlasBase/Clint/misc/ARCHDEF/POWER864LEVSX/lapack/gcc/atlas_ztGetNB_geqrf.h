#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,216,288,648,1368,2088,2808
 * N : 25,144,216,288,648,1368,2088,2808
 * NB : 9,12,24,36,36,36,36,48
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 9; \
   else if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 252) (nb_) = 24; \
   else if ((n_) < 2448) (nb_) = 36; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
