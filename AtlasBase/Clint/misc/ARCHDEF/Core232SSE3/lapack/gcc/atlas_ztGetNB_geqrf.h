#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,384,816,1632,3312
 * N : 25,96,144,192,384,816,1632,3312
 * NB : 7,12,12,48,48,96,96,96
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 7; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 600) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
