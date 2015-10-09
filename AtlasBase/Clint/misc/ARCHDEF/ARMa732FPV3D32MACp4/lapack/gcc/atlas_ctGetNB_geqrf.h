#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,273,397,521,1018,2012,3006,4000
 * N : 25,273,397,521,1018,2012,3006,4000
 * NB : 24,24,36,36,36,36,36,132
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 335) (nb_) = 24; \
   else if ((n_) < 3503) (nb_) = 36; \
   else (nb_) = 132; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
