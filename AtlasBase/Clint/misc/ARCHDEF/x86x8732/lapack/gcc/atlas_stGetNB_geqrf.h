#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,96,144,192,216,264,504,984,1944,3864
 * N : 25,72,96,144,192,216,264,504,984,1944,3864
 * NB : 2,6,30,30,30,18,42,24,48,48,48
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 2; \
   else if ((n_) < 84) (nb_) = 6; \
   else if ((n_) < 204) (nb_) = 30; \
   else if ((n_) < 240) (nb_) = 18; \
   else if ((n_) < 384) (nb_) = 42; \
   else if ((n_) < 744) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
