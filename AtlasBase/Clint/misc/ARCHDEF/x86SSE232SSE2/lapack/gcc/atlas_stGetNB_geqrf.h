#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,384,816,1680,3360
 * N : 25,96,144,192,384,816,1680,3360
 * NB : 9,12,12,48,48,48,48,96
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 9; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 2520) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
