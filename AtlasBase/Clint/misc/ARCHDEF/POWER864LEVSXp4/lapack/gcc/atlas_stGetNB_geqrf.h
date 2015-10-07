#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,521,1018,2012,4000
 * N : 25,87,149,211,273,521,1018,2012,4000
 * NB : 4,16,24,28,44,44,44,44,44
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 118) (nb_) = 16; \
   else if ((n_) < 180) (nb_) = 24; \
   else if ((n_) < 242) (nb_) = 28; \
   else (nb_) = 44; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
