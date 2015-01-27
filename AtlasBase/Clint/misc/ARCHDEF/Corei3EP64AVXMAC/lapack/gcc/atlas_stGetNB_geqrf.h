#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,397,521,769,893,955,1018,2012,3006,4000
 * N : 25,87,149,211,273,397,521,769,893,955,1018,2012,3006,4000
 * NB : 12,12,16,24,24,24,60,60,60,60,100,100,132,148
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 118) (nb_) = 12; \
   else if ((n_) < 180) (nb_) = 16; \
   else if ((n_) < 459) (nb_) = 24; \
   else if ((n_) < 986) (nb_) = 60; \
   else if ((n_) < 2509) (nb_) = 100; \
   else if ((n_) < 3503) (nb_) = 132; \
   else (nb_) = 148; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
