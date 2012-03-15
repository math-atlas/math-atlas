#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,216,360,792,1584,3168
 * N : 25,144,216,360,792,1584,3168
 * NB : 4,12,72,72,72,72,72
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 180) (nb_) = 12; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
