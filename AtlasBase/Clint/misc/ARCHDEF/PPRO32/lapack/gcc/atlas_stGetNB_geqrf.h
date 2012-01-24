#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,96,120,216,312,432,840
 * N : 25,72,96,120,216,312,432,840
 * NB : 2,6,24,24,24,24,48,48
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 2; \
   else if ((n_) < 84) (nb_) = 6; \
   else if ((n_) < 372) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
