#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,288,360,504,1080,2232,3384,4536,9144
 * N : 25,216,288,360,504,1080,2232,3384,4536,9144
 * NB : 7,24,24,72,72,72,72,144,144,144
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 7; \
   else if ((n_) < 324) (nb_) = 24; \
   else if ((n_) < 2808) (nb_) = 72; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
