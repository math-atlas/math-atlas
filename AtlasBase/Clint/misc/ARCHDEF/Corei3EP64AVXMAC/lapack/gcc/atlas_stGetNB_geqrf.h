#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,273,335,397,459,521,1018,2012,3006,3503,4000
 * N : 25,273,335,397,459,521,1018,2012,3006,3503,4000
 * NB : 24,24,24,28,28,76,76,88,132,132,140
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 366) (nb_) = 24; \
   else if ((n_) < 490) (nb_) = 28; \
   else if ((n_) < 1515) (nb_) = 76; \
   else if ((n_) < 2509) (nb_) = 88; \
   else if ((n_) < 3751) (nb_) = 132; \
   else (nb_) = 140; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
