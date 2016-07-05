#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,240,528,1104
 * N : 25,96,240,528,1104
 * NB : 12,48,48,48,48
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 12; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
