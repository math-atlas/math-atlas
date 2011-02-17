#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,240,528,1056,2160,4368
 * N : 25,96,144,192,240,528,1056,2160,4368
 * NB : 4,8,12,48,48,48,48,96,144
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 120) (nb_) = 8; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 1608) (nb_) = 48; \
   else if ((n_) < 3264) (nb_) = 96; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
