#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,521,1018,2012,4000
 * N : 25,87,149,211,273,521,1018,2012,4000
 * NB : 10,15,25,25,40,40,40,40,40
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 10; \
   else if ((n_) < 118) (nb_) = 15; \
   else if ((n_) < 242) (nb_) = 25; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
