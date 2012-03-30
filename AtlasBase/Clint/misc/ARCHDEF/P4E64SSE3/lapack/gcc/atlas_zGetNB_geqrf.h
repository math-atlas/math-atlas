#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,136,204,272,340,680,1360
 * N : 25,136,204,272,340,680,1360
 * NB : 9,24,16,16,68,68,68
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 80) (nb_) = 9; \
   else if ((n_) < 170) (nb_) = 24; \
   else if ((n_) < 306) (nb_) = 16; \
   else (nb_) = 68; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
