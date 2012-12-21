#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,288,432,648,936,1944,2952,3960,7992
 * N : 25,216,288,432,648,936,1944,2952,3960,7992
 * NB : 4,12,72,72,72,144,144,144,216,216
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 4; \
   else if ((n_) < 252) (nb_) = 12; \
   else if ((n_) < 792) (nb_) = 72; \
   else if ((n_) < 3456) (nb_) = 144; \
   else (nb_) = 216; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
