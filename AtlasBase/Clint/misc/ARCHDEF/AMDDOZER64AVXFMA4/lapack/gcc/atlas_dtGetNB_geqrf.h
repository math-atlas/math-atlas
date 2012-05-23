#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,180,240,300,600,1260,1860,2520,5040
 * N : 25,120,180,240,300,600,1260,1860,2520,5040
 * NB : 2,12,12,60,60,60,60,60,120,120
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 2; \
   else if ((n_) < 210) (nb_) = 12; \
   else if ((n_) < 2190) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
