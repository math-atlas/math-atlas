#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,96,144,288,408,480,552,1104
 * N : 25,72,96,144,288,408,480,552,1104
 * NB : 12,12,32,32,24,24,48,48,48
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 12; \
   else if ((n_) < 216) (nb_) = 32; \
   else if ((n_) < 444) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
