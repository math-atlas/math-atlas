#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,240,480,1008,2016
 * N : 25,96,240,480,1008,2016
 * NB : 11,48,48,48,48,48
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 11; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
