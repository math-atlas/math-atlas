#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,335,397,459,521,769,893,955,1018,1515,1763,2012,3006,3503,4000
 * N : 25,149,211,273,335,397,459,521,769,893,955,1018,1515,1763,2012,3006,3503,4000
 * NB : 24,24,24,48,48,60,60,68,80,80,80,100,116,116,124,124,124,128
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 242) (nb_) = 24; \
   else if ((n_) < 366) (nb_) = 48; \
   else if ((n_) < 490) (nb_) = 60; \
   else if ((n_) < 645) (nb_) = 68; \
   else if ((n_) < 986) (nb_) = 80; \
   else if ((n_) < 1266) (nb_) = 100; \
   else if ((n_) < 1887) (nb_) = 116; \
   else if ((n_) < 3751) (nb_) = 124; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
