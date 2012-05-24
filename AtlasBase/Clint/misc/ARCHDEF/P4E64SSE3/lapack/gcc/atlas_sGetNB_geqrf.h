#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,216,288,648,1368,2736
 * N : 25,144,216,288,648,1368,2736
 * NB : 4,12,72,72,72,72,144
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 2052) (nb_) = 72; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
