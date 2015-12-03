#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,521,1018,1266,1515,2012,4000
 * N : 25,149,211,273,521,1018,1266,1515,2012,4000
 * NB : 12,12,24,28,28,32,52,52,100,100
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 242) (nb_) = 24; \
   else if ((n_) < 769) (nb_) = 28; \
   else if ((n_) < 1142) (nb_) = 32; \
   else if ((n_) < 1763) (nb_) = 52; \
   else (nb_) = 100; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
