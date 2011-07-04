#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,624,1296,1920,2592
 * N : 25,144,288,624,1296,1920,2592
 * NB : 4,48,48,48,48,48,96
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 2256) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
