#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,240,384,528,1104,2208
 * N : 25,96,144,192,240,384,528,1104,2208
 * NB : 5,6,18,18,24,48,48,48,48
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 5; \
   else if ((n_) < 120) (nb_) = 6; \
   else if ((n_) < 216) (nb_) = 18; \
   else if ((n_) < 312) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
