#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,240,336,480,960,1920
 * N : 25,96,144,192,240,336,480,960,1920
 * NB : 4,12,12,48,48,48,72,96,96
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 408) (nb_) = 48; \
   else if ((n_) < 720) (nb_) = 72; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
