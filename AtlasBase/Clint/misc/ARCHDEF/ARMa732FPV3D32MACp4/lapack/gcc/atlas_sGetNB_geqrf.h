#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,518,1012,2000
 * N : 25,518,1012,2000
 * NB : 24,24,36,132
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 765) (nb_) = 24; \
   else if ((n_) < 1506) (nb_) = 36; \
   else (nb_) = 132; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
