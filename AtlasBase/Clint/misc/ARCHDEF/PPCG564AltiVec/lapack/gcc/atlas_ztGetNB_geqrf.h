#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,240,480,720,816,960,1920
 * N : 25,96,144,192,240,480,720,816,960,1920
 * NB : 3,12,12,48,48,48,48,48,96,96
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 3; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 888) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
