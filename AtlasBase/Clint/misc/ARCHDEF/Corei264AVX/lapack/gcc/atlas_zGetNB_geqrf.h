#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,240,528,1056,2160
 * N : 25,96,240,528,1056,2160
 * NB : 5,48,48,48,48,96
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 5; \
   else if ((n_) < 1608) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
