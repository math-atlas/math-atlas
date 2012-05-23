#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,432,864,1800,3672
 * N : 25,216,432,864,1800,3672
 * NB : 5,72,72,72,72,144
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 5; \
   else if ((n_) < 2736) (nb_) = 72; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
