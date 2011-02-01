#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,240,480,960,1920
 * N : 25,96,144,192,240,480,960,1920
 * NB : 4,12,12,16,48,48,48,96
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 216) (nb_) = 16; \
   else if ((n_) < 1440) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
