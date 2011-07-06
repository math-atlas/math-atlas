#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,384,816,1632,2016,2448,3312
 * N : 25,96,144,192,384,816,1632,2016,2448,3312
 * NB : 1,24,48,48,48,48,48,48,96,96
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 1; \
   else if ((n_) < 120) (nb_) = 24; \
   else if ((n_) < 2232) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
