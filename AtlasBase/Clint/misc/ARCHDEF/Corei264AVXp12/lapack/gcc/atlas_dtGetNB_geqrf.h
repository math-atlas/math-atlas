#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,335,397,521,769,893,955,1018,1515,2012,3006,4000
 * N : 25,149,211,273,335,397,521,769,893,955,1018,1515,2012,3006,4000
 * NB : 12,12,12,56,68,68,68,68,80,84,100,100,120,120,144
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 242) (nb_) = 12; \
   else if ((n_) < 304) (nb_) = 56; \
   else if ((n_) < 831) (nb_) = 68; \
   else if ((n_) < 924) (nb_) = 80; \
   else if ((n_) < 986) (nb_) = 84; \
   else if ((n_) < 1763) (nb_) = 100; \
   else if ((n_) < 3503) (nb_) = 120; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
