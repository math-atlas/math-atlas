#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,624,1296,2640
 * N : 25,144,288,624,1296,2640
 * NB : 4,12,48,48,48,96
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 216) (nb_) = 12; \
   else if ((n_) < 1968) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
