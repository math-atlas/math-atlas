#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,192,240,336,672,1344,2736
 * N : 25,144,192,240,336,672,1344,2736
 * NB : 4,12,48,48,48,48,48,96
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 2040) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
