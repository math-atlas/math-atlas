#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,360,792,1584,3240
 * N : 25,144,360,792,1584,3240
 * NB : 5,72,72,72,72,72
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 5; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
