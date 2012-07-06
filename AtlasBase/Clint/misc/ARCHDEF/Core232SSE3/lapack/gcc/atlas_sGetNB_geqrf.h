#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,360,792,1584,3168
 * N : 25,144,360,792,1584,3168
 * NB : 4,72,72,72,72,144
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 2376) (nb_) = 72; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
