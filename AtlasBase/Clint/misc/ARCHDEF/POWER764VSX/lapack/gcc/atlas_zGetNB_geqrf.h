#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,960,1920
 * N : 25,240,480,960,1920
 * NB : 12,80,80,80,160
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 12; \
   else if ((n_) < 1440) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
