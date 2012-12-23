#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,152,228,380,456,608,836,1748,2660,3116,3572
 * N : 25,152,228,380,456,608,836,1748,2660,3116,3572
 * NB : 2,12,20,20,76,76,76,76,76,760,760
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 88) (nb_) = 2; \
   else if ((n_) < 190) (nb_) = 12; \
   else if ((n_) < 418) (nb_) = 20; \
   else if ((n_) < 2888) (nb_) = 76; \
   else (nb_) = 760; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
